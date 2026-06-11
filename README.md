# ImageNodeEditor — 基于节点的图像处理工作流工具

## 环境要求

| 依赖 | 版本要求 |
|------|----------|
| Visual Studio | 2022 (v143 平台工具集) |
| Qt | 5.15+（需安装 **Qt5 Core、Gui、Widgets** 模块） |
| Qt VS Tools | Visual Studio 扩展 |

## 构建步骤

1. 安装 [Qt 5.15.2](https://www.qt.io/download-open-source)（选择 MSVC 2019/2022 版本）
2. 在 Visual Studio 中安装 **Qt VS Tools** 扩展（扩展 → 管理扩展 → 搜索 "Qt Visual Studio Tools"）
3. 打开 Qt VS Tools → Qt Options，添加 Qt5 路径（指向 Qt 安装目录，例如 `C:\Qt\5.15.2\msvc2019_64`）
4. 打开 `main.sln`
5. 在解决方案资源管理器中右键 `main` 项目 → **重新扫描 Qt 模块**
6. 选择 **Release | x64** 配置 → 生成解决方案

> 如果遇到 Qt 模块识别失败，检查 Qt VS Tools 中是否正确配置了 Qt5 路径，并在项目属性 → Qt Settings → Qt Modules 中确认包含 `core;gui;widgets`。

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
- **色调曲线编辑器**：交互式曲线编辑（点击添加、拖拽调整节点），支持白/R/G/B 四通道独立调节，Catmull-Rom 平滑曲线插值，一键恢复初始
- **图像加载诊断**：加载失败时显示详细的格式支持和错误信息

## 分类结构

| 分类 | 包含的节点 |
|------|-----------|
| 输入 | 图像输入 |
| 输出 | 图像输出、图像查看器 |
| 滤波 | 高斯模糊、锐化、边缘检测 |
| 风格化 | 怀旧、像素化、暗角、铅笔画、卡通风格 |
| 色彩调整 | 亮度/对比度、饱和度、色相偏移、伽马校正、反色、曝光度、自然饱和度、白平衡、阴影/高光、黑/白场、清晰度、色调曲线 |
| 几何变换 | 裁剪、缩放、旋转、水印 |
| 转换 | 灰度化、阈值、冷暖色调、褪色 |
| 多端口 | 通道分离、通道合并 |
