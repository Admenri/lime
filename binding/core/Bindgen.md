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

### 对象引用属性（BINDING_ATTR_OBJECT_REF）

当一个类在导出块中声明了 `MARSHAL_DUMP` / `MARSHAL_LOAD`（即可被 marshal 序列化的类，
当前为 Table、Rect、Color、Tone），那么**该类作为其它类的属性时**，必须使用
`BINDING_ATTR_OBJECT_REF`，**不能**使用普通的 `BINDING_ATTR_OBJECT`。

例如 Tone 具有 `MARSHAL_DUMP` / `MARSHAL_LOAD`，因此在 Window 中：

```cpp
BINDING_ATTR_OBJECT_REF(Window, rgssx::Window, Tone, rgssx::Tone, kToneDataType);
```

`BINDING_ATTR_OBJECT_REF` 与 `BINDING_ATTR_OBJECT` 的区别：前者会把包装后的 Ruby 对象
缓存在实例变量 `@_<attr>` 中，保证多次读取返回**同一个** Ruby 对象（对象身份一致），
这是 marshal 序列化 / 反序列化时保持对象引用关系所必需的；后者每次读取都会新建一个
包装对象。

判断依据：只要属性类型（`objty`）对应的类拥有 `MARSHAL_DUMP` / `MARSHAL_LOAD` 定义，
该处属性一律使用 `BINDING_ATTR_OBJECT_REF`。

---

# initialize（构造函数）

所有构造函数均需要导出为 Ruby 的 `initialize`。

模板如下（使用 `mrb_get_argc` 判断参数个数，按重载分支，每个分支内使用带具体类型的
`mrb_get_args` 获取参数；函数最后**直接 `return SetupSelfData(...)`**，不要再写
`SetupSelfData(...); return self;`）：

```cpp
MRB_FUNC(Class_initialize) {
  mrb_int argc = mrb_get_argc(mrb);

  rgssx::RefPtr<Class> obj = nullptr;

  EXC_BEGIN {
    if (argc == N) {
      // 重载 A
      mrb_int x, y, w, h;
      mrb_get_args(mrb, "iiii", &x, &y, &w, &h);
      obj = rgssx::MakeRefCounted<Class>(x, y, w, h);
    } else if (argc == M) {
      // 重载 B
      ...
    } else {
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  } EXC_END;

  return SetupSelfData(self, obj.get(), kClassDataType);
}
```

生成时需要根据构造函数参数选择对应的重载。`initialize_copy` 等其它以 `SetupSelfData`
结尾的函数同样统一写成 `return SetupSelfData(...);`。

---

# 参数数量校验

**不再使用 `mrb_get_args(mrb, "*", &args, &argc)` 解析不定参数。** 应先用
`mrb_get_argc(mrb)` 获取实际参数个数，再按重载分支，并在每个分支内使用**带具体类型**的
`mrb_get_args(mrb, "iii...", ...)` 获取参数，由 MRuby 负责参数的类型检查/转换，避免手写
`mrb_integer(args[i])` / `mrb_as_float(args[i])` 等手工类型提取，也避免越界访问
`args[i]`。

推荐使用 `if / else if / else` 分支进行校验，最后一个 `else` 分支抛出 `ArgumentError`：

```cpp
MRB_FUNC(Class_Method) {
  auto* self_obj = GetSelfData<Class>(self);
  mrb_int argc = mrb_get_argc(mrb);

  EXC_BEGIN {
    if (argc == 5) {
      // 重载 A
      mrb_int x, y, w, h;
      mrb_value color;
      mrb_get_args(mrb, "iiiio", &x, &y, &w, &h, &color);
      self_obj->Method(x, y, w, h,
                       GetObject<...>(mrb, color, ...));
    } else if (argc == 2) {
      // 重载 B
      ...
    } else {
      // 非法参数数量
      mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
    }
  } EXC_END;
  return mrb_nil_value();
}
```

注意：

- 每个合法的重载对应一个 `if / else if` 分支，其条件必须**精确匹配**该重载的参数数量（如 `argc == 4`）；每个分支内再调用带具体类型的 `mrb_get_args`，由 MRuby 完成参数类型校验与转换。
- 最后一个 `else` 分支必须抛出 `ArgumentError`，保证任何非法参数数量在调用 `mrb_get_args` 之前被拦截。
- 构造函数、`set` 等方法的重载选择一律遵循本规则。
- 固定参数个数的函数（无重载）不需要 `mrb_get_argc`，直接使用带具体类型的 `mrb_get_args` 即可。

### 带默认参数的函数

如果函数带有默认参数，例如：

```cpp
Color(float red, float green, float blue, float alpha = 255)
```

则每个可能的参数个数都必须单独生成一个分支，分支内使用带具体类型的 `mrb_get_args`
只获取实际传入的参数，默认值由 C++ 侧负责：

```cpp
if (argc == 3) {
  // Color.new(red, green, blue)      // alpha 使用默认值 255
  mrb_float r, g, b;
  mrb_get_args(mrb, "fff", &r, &g, &b);
  obj = rgssx::MakeRefCounted<Color>(r, g, b);
} else if (argc == 4) {
  // Color.new(red, green, blue, alpha)
  mrb_float r, g, b, a;
  mrb_get_args(mrb, "ffff", &r, &g, &b, &a);
  obj = rgssx::MakeRefCounted<Color>(r, g, b, a);
} else {
  mrb_raise(mrb, E_ARGUMENT_ERROR, "wrong number of arguments");
}
```

**禁止**把多个参数个数合并到一个分支，再在分支内部用 `argc == N ? ... : 默认值` 的方式补默认值；也禁止用 `argc >= N` 忽略多余参数。每个参数个数单独生成一个分支，分支内只获取实际传入的参数，默认值由 C++ 侧负责。构造函数、`set` 等方法的重载选择一律遵循本规则。

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

# 序列化（Marshal）

当检测到一个类的导出块（`/*-export.begin-*/` ... `/*-export.end-*/`）中声明了
`MARSHAL_DUMP` / `MARSHAL_LOAD` 这一对序列化函数（即“P D”：dump / load 成对出现）时，
需要为该类额外生成两个用于 marshal 序列化与反序列化的函数：

- `_dump`：**实例方法（method）**。mruby-marshal-c 在 `Marshal.dump` 时会以
  `obj._dump(limit)` 调用它，返回值必须是 String，随后写入 `TYPE_USERDEF`。
- `_load`：**类方法（class method）**。mruby-marshal-c 在 `Marshal.load` 读到
  `TYPE_USERDEF` 时会以 `Klass._load(data)` 调用它，传入 `_dump` 产生的 String，
  返回反序列化后的新对象。

模板如下（`Class` 替换为实际类名，`kClassDataType` 替换为对应数据类型常量）：

```cpp
MRB_FUNC(Class__dump) {
  auto* self_obj = GetSelfData<Class>(self);
  mrb_int limit;
  mrb_get_args(mrb, "i", &limit);

  EXC_BEGIN {
    auto result = Class::MarshalDump(RefPtr<Class>(self_obj));
    return mrb_str_new(mrb, result.data(), static_cast<mrb_int>(result.size()));
  } EXC_END(mrb);
  return mrb_nil_value();
}

MRB_FUNC(Class__load) {
  mrb_value data;
  mrb_get_args(mrb, "o", &data);

  rgssx::RefPtr<Class> obj = nullptr;
  EXC_BEGIN {
    obj = Class::MarshalLoad(MRBStringValue(data));
  } EXC_END(mrb);

  return WrapObject(mrb, obj.get(), kClassDataType);
}
```

注册方式：

```cpp
mrb_define_method(mrb, klass, "_dump", Class__dump, MRB_ARGS_REQ(1));
mrb_define_class_method(mrb, klass, "_load", Class__load, MRB_ARGS_REQ(1));
```

注意：

- `_dump` 的参数 `limit` 是 mruby-marshal-c 协议要求的深度参数，本项目中不使用，直接忽略即可。
- `_load` 必须注册为**类方法**（`mrb_define_class_method`），因为它没有 `self` 实例。
- 调用的是类中 `MARSHAL_DUMP` / `MARSHAL_LOAD` 展开出的静态函数 `MarshalDump` / `MarshalLoad`。

当前声明了 `MARSHAL_DUMP` / `MARSHAL_LOAD` 的类：**Table、Rect、Color、Tone**。

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
