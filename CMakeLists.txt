cmake_minimum_required(VERSION 3.16)
project(ImageNodeEditor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

set(SOURCES
    main/main.cpp
    main/core/Node.cpp
    main/core/NodeRegistry.cpp
    main/engine/DataPacket.cpp
    main/engine/WorkflowEngine.cpp
    main/io/WorkflowSerializer.cpp
    main/algorithms/ImageAlgorithm.cpp
    main/nodes/ImageInputNode.cpp
    main/nodes/ImageOutputNode.cpp
    main/nodes/ImageViewerNode.cpp
    main/nodes/CropNode.cpp
    main/nodes/ResizeNode.cpp
    main/nodes/BrightnessContrastNode.cpp
    main/nodes/GaussianBlurNode.cpp
    main/nodes/GrayscaleNode.cpp
    main/nodes/EdgeDetectionNode.cpp
    main/nodes/ChannelSplitNode.cpp
    main/nodes/ChannelMergeNode.cpp
    main/nodes/WatermarkNode.cpp
    main/nodes/RotateNode.cpp
    main/nodes/InvertNode.cpp
    main/nodes/SaturationNode.cpp
    main/nodes/HueShiftNode.cpp
    main/nodes/GammaNode.cpp
    main/nodes/ThresholdNode.cpp
    main/nodes/SepiaNode.cpp
    main/nodes/SharpenNode.cpp
    main/nodes/ColorTemperatureNode.cpp
    main/nodes/FadeNode.cpp
    main/nodes/PixelateNode.cpp
    main/nodes/VignetteNode.cpp
    main/nodes/PencilSketchNode.cpp
    main/nodes/CartoonNode.cpp
    main/nodes/ExposureNode.cpp
    main/nodes/VibranceNode.cpp
    main/nodes/WhiteBalanceNode.cpp
    main/nodes/HighlightsShadowsNode.cpp
    main/nodes/WhitesBlacksNode.cpp
    main/nodes/ClarityNode.cpp
    main/nodes/ToneCurveNode.cpp
    main/nodes/AutoEnhanceNode.cpp
    main/gui/MainWindow.cpp
    main/gui/NodeScene.cpp
    main/gui/NodeView.cpp
    main/gui/NodeGraphicsItem.cpp
    main/gui/PortGraphicsItem.cpp
    main/gui/ConnectionGraphicsItem.cpp
    main/gui/NodeListPanel.cpp
    main/gui/LogPanel.cpp
    main/gui/PropertyPanel.cpp
    main/gui/PreviewDialog.cpp
    main/cli/CommandLineRunner.cpp
)

set(HEADERS
    main/core/Node.h
    main/core/Port.h
    main/core/DataType.h
    main/core/NodeRegistry.h
    main/engine/DataPacket.h
    main/engine/WorkflowEngine.h
    main/io/WorkflowSerializer.h
    main/algorithms/ImageAlgorithm.h
    main/nodes/ImageInputNode.h
    main/nodes/ImageOutputNode.h
    main/nodes/ImageViewerNode.h
    main/nodes/CropNode.h
    main/nodes/ResizeNode.h
    main/nodes/BrightnessContrastNode.h
    main/nodes/GaussianBlurNode.h
    main/nodes/GrayscaleNode.h
    main/nodes/EdgeDetectionNode.h
    main/nodes/ChannelSplitNode.h
    main/nodes/ChannelMergeNode.h
    main/nodes/WatermarkNode.h
    main/nodes/RotateNode.h
    main/nodes/InvertNode.h
    main/nodes/SaturationNode.h
    main/nodes/HueShiftNode.h
    main/nodes/GammaNode.h
    main/nodes/ThresholdNode.h
    main/nodes/SepiaNode.h
    main/nodes/SharpenNode.h
    main/nodes/ColorTemperatureNode.h
    main/nodes/FadeNode.h
    main/nodes/PixelateNode.h
    main/nodes/VignetteNode.h
    main/nodes/PencilSketchNode.h
    main/nodes/CartoonNode.h
    main/nodes/ExposureNode.h
    main/nodes/VibranceNode.h
    main/nodes/WhiteBalanceNode.h
    main/nodes/HighlightsShadowsNode.h
    main/nodes/WhitesBlacksNode.h
    main/nodes/ClarityNode.h
    main/nodes/ToneCurveNode.h
    main/nodes/AutoEnhanceNode.h
    main/gui/MainWindow.h
    main/gui/NodeScene.h
    main/gui/NodeView.h
    main/gui/NodeGraphicsItem.h
    main/gui/PortGraphicsItem.h
    main/gui/ConnectionGraphicsItem.h
    main/gui/NodeListPanel.h
    main/gui/LogPanel.h
    main/gui/PropertyPanel.h
    main/gui/PreviewDialog.h
    main/cli/CommandLineRunner.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main)

# 构建后自动复制 Qt DLL 及插件到输出目录（conda Qt5 部署）
set(QT_DLL_DIR "$ENV{CONDA_PREFIX}/Library/bin")
if(NOT QT_DLL_DIR OR NOT EXISTS "${QT_DLL_DIR}")
    set(QT_DLL_DIR "C:/Users/sunli/anaconda3/Library/bin")
endif()
get_filename_component(QT_LIB_DIR "${QT_DLL_DIR}" DIRECTORY)  # Library/
set(QT_PLUGIN_DIR "${QT_LIB_DIR}/plugins")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_DLL_DIR}/Qt5Core_conda.dll"
        "${QT_DLL_DIR}/Qt5Gui_conda.dll"
        "${QT_DLL_DIR}/Qt5Widgets_conda.dll"
        "${QT_DLL_DIR}/libpng16.dll"
        "${QT_DLL_DIR}/zlib.dll"
        "${QT_DLL_DIR}/libjpeg.dll"
        "${QT_DLL_DIR}/zstd.dll"
        "${QT_DLL_DIR}/icuin73.dll"
        "${QT_DLL_DIR}/icuuc73.dll"
        "${QT_DLL_DIR}/icudt73.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${QT_PLUGIN_DIR}/platforms/qwindows.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/platforms/
    COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${QT_PLUGIN_DIR}/imageformats"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/imageformats
    COMMENT "复制 Qt DLL 及依赖到输出目录"
)
