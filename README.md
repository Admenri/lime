<div align="center">

# RGSSX

**轻量级 RGSS 运行时** · Lightweight RGSS Runtime

使用 **C++20** 编写、以 **mruby** 为脚本引擎的 RPG Maker 游戏运行时。
兼容 **RGSS1 / RGSS2 / RGSS3**（RPG Maker XP / VX / VX Ace）脚本与数据结构，
开箱即用地运行现成的 RPG Maker 工程。

</div>

---

## ✨ 特性

- **完整 RGSS 运行时**：实现 `Graphics`、`Input`、`Audio`、`Bitmap`、`Sprite`、`Viewport`、`Plane`、`Window`、`Tilemap`、`Font`、`Table`、`Color`、`Rect`、`Tone` 等核心 API。
- **三大版本兼容**：根据脚本文件（`.rxdata` / `.rvdata` / `.rvdata2`）自动识别 RGSS1/2/3，并加载对应版本的 `RPG` 数据结构。
- **mruby 脚本引擎**：内置 Ruby 兼容的轻量脚本解释器，直接运行解密的 `Data/Scripts.rvdata*` 脚本。
- **Marshal 兼容**：支持 `load_data` / `save_data` / `_dump` / `_load`，与官方 RGSS 的 Marshal 数据格式互通。
- **动态 Unicode 字体系统**：基于 **FreeType + HarfBuzz** 的按需字形图集，支持 CJK / 阿拉伯文（RTL）/ 日文 / 韩文 / 彩色 Emoji，以及粗体、斜体、描边、阴影。
- **视口与绘制系统**：Z 排序绘制列表、多级视口、色调 / 颜色 / 透明度滤镜、闪烁与转场特效。
- **音效系统**：BGM / BGS / ME / SE 播放，支持音量、音高、淡入淡出与定位，并通过 **TinySoundFont** 原生支持 **MIDI（SoundFont）** 播放。
- **虚拟文件系统**：基于 **PhysicsFS** 的多路径挂载、读取 / 写入分离，支持 Windows **RTP**（Runtime Package）自动定位。
- **硬件加速渲染**：基于 **raylib** 的 OpenGL 渲染管线，内置精灵着色器、转场着色器与视口缓存。

## 🚀 快速开始

### 环境要求

- **CMake** ≥ 3.30
- 支持 **C++20** 的编译器（本项目主要在 MSVC / Visual Studio 下开发）
- 第三方依赖通过 **git submodule** 管理

### 克隆并构建

```bash
# 1. 克隆仓库（含全部子模块）
git clone --recursive https://github.com/Admenri/rgssx.git
cd rgssx

# 2. 配置
cmake -S . -B build

# 3. 构建
cmake --build build --config Release
```

构建完成后，在 `build/Release/`（或 `build/Debug/`）下得到 `rgssx.exe`。

### 运行游戏

将 `rgssx.exe` 放到 RPG Maker 工程目录中（与 `Game.exe` 同级），运行即可加载并执行该工程的脚本。

可通过与可执行文件同名的 `.ini` 配置文件调整运行参数：

```ini
[Game]
RGSS=3               ; RGSS 版本：1 / 2 / 3（留空或为 0 时按脚本扩展名自动识别）
Scripts=Data/Scripts.rvdata2
Title=RGSSX
RTP=
Soundfont=font.sf2    ; MIDI 播放用的 SoundFont

[Window]
Width=544
Height=416
VSync=false
Fullscreen=false
```

## 🧱 架构

```
┌─────────────────────────────────────────────────────┐
│                    app / main.cc                     │  进程入口：加载配置、初始化各子系统
├─────────────────────────────────────────────────────┤
│              binding/  (mruby 绑定层)                 │  RGSS API 绑定 + RPG 数据结构
│  core/    -> Graphics/Input/Audio/Sprite/...         │  mruby 脚本解释器
│  rpg/     -> RPG::* 数据结构 (RGSS1/2/3)             │
├─────────────────────────────────────────────────────┤
│                 src/  (rgssx_core)                   │  核心引擎（C++20）
│  graphics  drawable  viewport  sprite  window        │  渲染与绘制系统
│  bitmap    font/     tilemap  plane   shader         │  字体系统 / 着色器
│  audio     input     filesystem         profile      │  音频 / 输入 / 虚拟文件系统 / 配置
├─────────────────────────────────────────────────────┤
│                     3rdparty/                        │  第三方依赖（submodule）
│  raylib  freetype  harfbuzz  physfs  zlib           │
│  mruby-cmake  tinysf  raygui                        │
└─────────────────────────────────────────────────────┘
```

| 模块 | 职责 |
| --- | --- |
| `app/` | 可执行入口，解析 ini 配置并初始化各全局单例 |
| `binding/` | 将 C++ 核心以 Ruby 形式暴露给 mruby，并内嵌 `RPG::*` 数据结构 |
| `src/` | 核心引擎 `rgssx_core`：渲染、绘制、字体、音频、输入、文件系统 |
| `3rdparty/` | 第三方依赖（git submodule） |

## 🛠 技术栈

| 组件 | 用途 |
| --- | --- |
| [raylib](https://www.raylib.com/) | OpenGL 硬件加速渲染、窗口与输入 |
| [mruby](https://mruby.org/) | 内嵌 Ruby 兼容脚本引擎 |
| [FreeType](https://freetype.org/) + [HarfBuzz](https://harfbuzz.github.io/) | 动态字形渲染与文字整形 |
| [PhysicsFS](https://icculus.org/physfs/) | 跨平台虚拟文件系统 |
| [zlib](https://zlib.net/) | 脚本解压与数据压缩 |
| [TinySoundFont](https://github.com/schellingb/TinySoundFont) | MIDI SoundFont 合成播放 |
| raygui | 内置调试 UI（开发用） |

## 📄 项目结构

```
rgssx/
├── CMakeLists.txt
├── app/                  # 可执行入口
│   └── main.cc
├── binding/              # mruby 绑定层
│   ├── core/             #   RGSS API 绑定
│   ├── rpg/              #   RPG 数据结构（RGSS1/2/3）
│   ├── stdlib/           #   标准库扩展（Dir 等）
│   └── mruby_main.cc     #   脚本加载与执行
├── src/                  # 核心引擎（rgssx_core）
│   ├── graphics.cc       #   渲染主循环、转场、截屏
│   ├── drawable.cc       #   Z 排序绘制列表
│   ├── viewport.cc       #   视口与缓存
│   ├── sprite.cc         #   精灵
│   ├── window.cc         #   窗口（九宫格皮肤）
│   ├── tilemap.cc        #   地图瓦片
│   ├── bitmap.cc         #   位图与绘制
│   ├── font/             #   动态字体系统
│   ├── audio.cc          #   音频（含 MIDI）
│   ├── input.cc          #   输入
│   ├── filesystem.cc     #   虚拟文件系统
│   └── profile.cc        #   配置
└── 3rdparty/             # 第三方依赖（submodule）
```

## 🎯 兼容范围

- **RGSS1**（RPG Maker XP）— `.rxdata`
- **RGSS2**（RPG Maker VX）— `.rvdata`
- **RGSS3**（RPG Maker VX Ace）— `.rvdata2`

> 说明：本项目为学习与兼容性目的开发，用于运行个人拥有的游戏工程。
> RPG Maker 及 RGSS 均为 © Enterbrain / KADOKAWA 的商标，其版权归原作者所有。

## 📜 许可

本项目基于 **MIT License** 开源，详见 [LICENCE](./LICENCE)。

## 🐋 鸣谢

以上内容均为 Deepseek V4 生成，如有雷同纯属巧合。  

**Copyright 2018-2026 (C) Admenri Adev**