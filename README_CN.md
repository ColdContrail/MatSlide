# MatSlide

基于物理的材质渲染与实时 BRDF 推断工具

中文 | [English](README.md)

## 项目描述

MatSlide 是一个用于实时预览神经网络输出材质的交互式工具，集成了 ONNX Runtime 推理引擎。该工具允许用户实时调整材质参数用于神经网络推理，神经网络接收参数并生成 MERL BRDF 格式的完整测量BRDF，用户可以实时查看渲染结果。

![Platform](https://img.shields.io/badge/platform-Windows%20x64-blue)
![OpenGL](https://img.shields.io/badge/OpenGL-4.4%2B-green)
![License](https://img.shields.io/badge/license-MIT-orange)

![Demo](assets/demo.png)

## 功能特性

- **环境贴图加载**：支持 EXR 格式的高动态范围环境贴图
- **MERL BRDF 渲染**：使用 MERL 双向反射分布函数数据库进行精确材质渲染
- **实时参数调整**：通过 GUI 滑块实时调整 GGX 粗糙度、漫反射和镜面反射参数
- **机器学习推断**：使用 ONNX Runtime 进行 BRDF 参数推断
- **交互式相机控制**：支持旋转和缩放以查看不同角度
- **多通道渲染**：支持环境贴图、MERL BRDF 和 IBL 渲染通道

## 依赖项

- **CMake** 3.20 或更高版本
- **OpenGL** 3.3 或更高版本
- **GLFW** 3.x
- **GLAD** (OpenGL 加载器)
- **GLM** (OpenGL 数学库)
- **ImGui** (即时模式 GUI)
- **ONNX Runtime** 1.24.4 (Windows x64)
- **TinyEXR** (EXR 图像加载)
- **BRDF-Loader**

## 构建步骤

1. **克隆仓库**
   ```bash
   git clone <repository-url>
   cd MatSlide
   ```

2. **初始化子模块**
   ```bash
   git submodule update --init --recursive
   ```

3. **下载并解压 ONNX Runtime**
   - 下载 [onnxruntime-win-x64-1.24.4.zip](https://github.com/microsoft/onnxruntime/releases/download/v1.24.4/onnxruntime-win-x64-1.24.4.zip)
   - 解压到 `dependencies/onnxruntime-win-x64-1.24.4/`

4. **配置 CMake**
   ```bash
   mkdir build
   cd build
   cmake ..
   ```

5. **编译项目**
   ```bash
   cmake --build . --config Release
   ```

6. **运行应用程序**
   ```bash
   cd ../bin
   MatSlideApp.exe
   ```

## 使用方法

1. 确保 `assets` 目录包含以下内容：
   - `assets/onnx/decoder.onnx` - ONNX 模型文件
   - `assets/env/envmap.exr` - 环境贴图文件

2. 启动应用程序后，您将看到：
   - **左侧**：材质参数调整面板
   - **右侧**：实时渲染视图

3. 调整参数：
   - 拖动 **GGX 粗糙度** 滑块控制材质粗糙度
   - 调整 **漫反射 RGB** 值控制基础颜色
   - 调整 **镜面反射 RGB** 值控制高光颜色

## 项目结构

```
MatSlide/
├── src/
│   ├── core/           # 核心应用逻辑、参数管理、EXR 加载
│   ├── rendering/      # 渲染通道：环境贴图、MERL、IBL
│   ├── inference/      # ONNX Runtime 封装和 BRDF 提供器
│   ├── shader/         # GLSL 着色器文件
│   └── utils/          # 工具程序（转换、推断）
├── dependencies/       # 第三方库
├── assets/             # 资源文件（模型、贴图）
└── build_vs/           # Visual Studio 构建目录
```

## 文件说明

- **main.cpp** - 应用程序入口点，初始化渲染通道和 BRDF 提供器
- **Application.hpp/cpp** - 主应用类，管理窗口、事件和渲染通道
- **ParameterManager.hpp/cpp** - 材质参数管理和 GUI 集成
- **BrdfProvider.hpp/cpp** - BRDF 数据提供和 ONNX 推断
- **IblMerlPass.hpp/cpp** - IBL 与 MERL BRDF 结合的渲染通道
- **EnvMapPass.hpp/cpp** - 环境贴图渲染通道
- **MerlPass.hpp/cpp** - 纯 MERL BRDF 渲染通道
