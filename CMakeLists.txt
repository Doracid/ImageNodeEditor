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
