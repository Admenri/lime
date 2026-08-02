# MRuby C++ Binding Generator

根据项目中的 C++ 头文件，自动在 `binding/core` 目录下生成 MRuby 的 C++ 胶水代码。

---

# 生成规则

对于每一个需要导出的类，生成两个文件：

```
binding/core/
    binding_<Class>.h
    binding_<Class>.cc
```

例如：

```
binding_bitmap.h
binding_bitmap.cc
```

---

# Header 模板

每个头文件使用以下模板：

```cpp
#include "mruby_utils.h"

namespace binding {

MRB_DATATYPE_DECLARE(ClassName);

void InitClassNameBinding(mrb_state* mrb);

} // namespace binding
```

其中 `ClassName` 替换为实际类名。

---

# Source 模板

每个 `.cc` 文件结构如下：

```cpp
#include "binding_ClassName.h"

namespace binding {

// Define mrb data type
MRB_DATATYPE_DEFINE(ClassName);

MRB_FUNC(ClassName_Method) {
  auto* self_obj = GetSelfData<ClassName>(self);

  EXC_BEGIN {
    auto result = self_obj->Method(...);
    return result;
  } EXC_END;

  return mrb_nil_value();
}

void InitClassNameBinding(mrb_state* mrb) {
  auto klass = DefineClass(mrb, "ClassName");

  mrb_define_method(
      mrb,
      klass,
      "method",
      ClassName_Method,
      MRB_ARGS_ANY());
}

} // namespace binding
```

---

# 普通成员函数

对于普通成员函数：

```cpp
ReturnType Foo(...);
```

生成：

```cpp
MRB_FUNC(Class_Foo) {
    auto* self_obj = GetSelfData<Class>(self);

    EXC_BEGIN {
        auto result = self_obj->Foo(...);
        return result;
    } EXC_END;

    return mrb_nil_value();
}
```

随后在 `InitClassBinding()` 中：

```cpp
mrb_define_method(
    mrb,
    klass,
    "foo",
    Class_Foo,
    MRB_ARGS_ANY());
```

---

# ATTR 属性

对于 `ATTR(...)` 定义的属性，不生成特殊处理，而是按普通成员函数模拟。

例如：

```cpp
ATTR(int, width)
```

视为：

```cpp
int width();
void width=(int);
```

因此需要生成两个 Ruby 方法：

```ruby
width
width=
```

即：

```cpp
Class_width
Class_width_set
```

Ruby 方法名分别为：

```
width
width=
```

---

# initialize（构造函数）

所有构造函数均需要导出为 Ruby 的 `initialize`。

模板如下：

```cpp
MRB_FUNC(Class_initialize) {
  rgssx::RefPtr<Class> obj = nullptr;

  EXC_BEGIN {
    obj = rgssx::MakeRefCounted<Class>(...);
  } EXC_END;

  SetupSelfData(
      self,
      obj.get(),
      kClassDataType);

  return self;
}
```

生成时需要根据构造函数参数选择对应的重载。

---

# 参数数量校验

当使用 `mrb_get_args(mrb, "*", &args, &argc)` 解析不定参数，并按 `argc` 选择重载时，**必须**在访问 `args[i]` 之前校验 `argc` 是否匹配某个合法的重载，否则在参数数量不合法时（例如用户没有传任何参数）访问 `args[i]` 会越界。

推荐使用 `if / else if / else` 分支进行校验，最后一个 `else` 分支抛出 `ArgumentError`：

```cpp
MRB_FUNC(Class_Method) {
  const mrb_value* args;
  mrb_int argc;
  mrb_get_args(mrb, "*", &args, &argc);

  EXC_BEGIN {
    if (argc == 5) {
      // 重载 A
      self_obj->Method(args[0], args[1], args[2], args[3], args[4]);
    } else if (argc == 2) {
      // 重载 B
      self_obj->Method(args[0], args[1]);
    } else {
      // 非法参数数量
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  } EXC_END;
  return mrb_nil_value();
}
```

注意：

- 每个合法的重载对应一个 `if / else if` 分支，其条件必须**精确匹配**该重载的参数数量（如 `argc == 4`），不能出现访问 `args[i]` 而 `i >= argc` 的分支。
- 最后一个 `else` 分支必须抛出 `ArgumentError`，保证任何非法参数数量在访问 `args[]` 之前被拦截。
- 构造函数、`set` 等方法的重载选择一律遵循本规则。

### 带默认参数的函数

如果函数带有默认参数，例如：

```cpp
Color(float red, float green, float blue, float alpha = 255)
```

则每个可能的参数个数都必须单独生成一个分支，分支内只传递实际传入的参数，默认值由 C++ 侧负责：

```cpp
if (argc == 3) {
  // Color.new(red, green, blue)      // alpha 使用默认值 255
  obj = rgssx::MakeRefCounted<Color>(r, g, b);
} else if (argc == 4) {
  // Color.new(red, green, blue, alpha)
  obj = rgssx::MakeRefCounted<Color>(r, g, b, a);
} else {
  mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
}
```

**禁止**把多个参数个数合并到一个分支，再在分支内部用 `argc == N ? args[N] : 默认值` 的方式补默认值；也禁止用 `argc >= N` 忽略多余参数。构造函数、`set` 等方法的重载选择一律遵循本规则。

---

# 导出哪些函数

仅扫描项目目录：

```
src/
```

中的头文件。

对于每个类：

1. 导出 **所有构造函数**
2. 导出位于

```cpp
/*-export.begin-*/

/*-export.end-*/
```

之间声明的所有成员函数。

其它函数全部忽略。

---

# 继承处理

如果一个类存在继承关系：

```cpp
class Sprite : public Drawable
```

且父类 **不是** `RefCounted`，则：

- 需要继续检查父类；
- 将父类中 `/*-export.begin-*/` 与 `/*-export.end-*/` 之间的导出内容，也视为当前类需要生成的绑定；
- 如果父类继续继承其它非 `RefCounted` 类，则递归处理。

即导出内容需要包含：

- 当前类导出函数
- 所有非 `RefCounted` 父类导出函数

---

# 特殊规则

以下类：

- Rect
- Color
- Tone

虽然源码中没有使用 `ATTR`，但为了性能采用普通成员变量实现。

生成绑定时，需要将其成员变量**视为 ATTR 属性**处理，即自动生成：

```
x
x=

y
y=

...
```

等对应 getter / setter。

---

# binding_init

所有类生成完成之后，再额外生成：

```
binding_init.h
binding_init.cc
```

用于统一初始化所有 Binding。

例如：

```cpp
void InitBinding(mrb_state* mrb);
```

实现：

```cpp
void InitBinding(mrb_state* mrb) {
    InitBitmapBinding(mrb);
    InitSpriteBinding(mrb);
    InitWindowBinding(mrb);
    ...
}
```

---

# MRuby C API

涉及 MRuby 的 C API 时，请参考：

```
3rdparty/mruby-cmake/mruby/doc/guides/capi.md
```

按照其中推荐的接口与用法生成代码。

---

# 输出要求

对于每个导出类，应生成：

```
binding_<Class>.h
binding_<Class>.cc
```

以及最终生成：

```
binding_init.h
binding_init.cc
```

所有代码风格应保持一致：

- 使用 `namespace binding`
- 使用 `MRB_FUNC`
- 使用 `MRB_DATATYPE_DECLARE`
- 使用 `MRB_DATATYPE_DEFINE`
- 使用 `EXC_BEGIN / EXC_END`
- 使用 `SetupSelfData`
- 使用 `GetSelfData`
- 使用 `DefineClass`
- 使用 `mrb_define_method`
- 保持与现有工程代码风格一致
