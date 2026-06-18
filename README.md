# ImageNodeEditor — 基于节点的图像处理工作流工具

## 环境要求

| 依赖 | 版本要求 |
|------|----------|
| Visual Studio | 2022 (v143 平台工具集) |
| Qt | 5.15+（需安装 **Qt5 Core、Gui、Widgets** 模块） |
| Qt VS Tools | Visual Studio 扩展 |

## 构建步骤

### 方式一：CMake（推荐）

1. 确保已安装 [Qt 5.15.2](https://www.qt.io/download-open-source) 或通过 conda 安装：`conda install qt-main`
2. 在项目根目录执行：

```bash
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt5
cmake --build . --config Release
```

3. 构建完成后，CMake 会自动复制 Qt 运行时 DLL 及依赖（`libpng16.dll`、`libjpeg.dll`、ICU 等）到输出目录
4. 直接运行 `build/Release/ImageNodeEditor.exe`

### 方式二：Visual Studio + CMake

1. 安装 [Qt 5.15.2](https://www.qt.io/download-open-source)（选择 MSVC 2019/2022 版本）
2. 在 Visual Studio 中打开 `main/CMakeLists.txt` 作为 CMake 项目
3. 选择 **Release | x64** 配置 → 生成
4. 构建后会自动部署所有运行时依赖，运行 `build/Release/ImageNodeEditor.exe`

> **注意**：使用 conda 安装的 Qt5 时，DLL 带有 `_conda` 后缀。构建时会自动将其复制到输出目录。`imageformats/` 图片格式插件也会一同部署，确保 PNG、JPEG、TIFF、WebP 等格式正常加载。

## 项目结构

```
main/
├── main.sln                  # Visual Studio 解决方案文件
├── main/                     # 项目目录
│   ├── main.vcxproj          # 项目文件
│   ├── main.vcxproj.filters  # 项目筛选器
│   ├── main.vcxproj.user     # 用户配置
│   ├── main.cpp              # 主入口
│   ├── core/                 # 核心框架
│   ├── nodes/                # 图像处理节点
│   ├── algorithms/           # 图像算法实现
│   ├── engine/               # 工作流引擎
│   ├── gui/                  # 图形界面
│   ├── cli/                  # 命令行接口
│   └── io/                   # 序列化/输入输出
└── CMakeLists.txt            # CMake 构建（备选）
```

## 功能特点

- **节点式工作流**：通过拖拽连线构建图像处理管线，支持实时预览
- **替换节点**：右键替换节点，自动保留已有连线；端口不兼容时自动回滚
- **参数边界约束**：所有数值参数限制在有效范围内，防止越界
- **撤销 (Undo)**：Ctrl+Z 撤销最多 50 步操作（添加/删除节点、连线等）
- **自动连线**：F6 或工具栏按钮，将所有节点按从左到右顺序自动连接
- **自动美化**：一键增强图像——自动白平衡 + 色阶拉伸 + S 曲线对比度 + 清晰度 + 饱和度微调
- **色调曲线编辑器**：交互式曲线编辑（点击添加、拖拽调整节点），支持白/R/G/B 四通道独立调节，Catmull-Rom 平滑曲线插值，一键恢复初始
- **图像加载诊断**：加载失败时显示详细的格式支持和错误信息

## 分类结构

| 分类 | 包含的节点 |
|------|-----------|
| 输入 | 图像输入 |
| 输出 | 图像输出、图像查看器 |
| 滤波 | 高斯模糊、锐化、边缘检测 |
| 风格化 | 怀旧、像素化、暗角、铅笔画、卡通风格 |
| 色彩调整 | 亮度/对比度、饱和度、色相偏移、伽马校正、反色、曝光度、自然饱和度、白平衡、阴影/高光、黑/白场、清晰度、色调曲线、自动美化 |
| 几何变换 | 裁剪、缩放、旋转、水印 |
| 转换 | 灰度化、阈值、冷暖色调、褪色 |
| 多端口 | 通道分离、通道合并 |
