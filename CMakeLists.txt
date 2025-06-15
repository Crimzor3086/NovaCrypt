cmake_minimum_required(VERSION 3.10)
project(NovaCrypt LANGUAGES CXX)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Add ImGui
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
add_subdirectory(${IMGUI_DIR} imgui)

# Add ImPlot
set(IMPLOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/implot)
add_subdirectory(${IMPLOT_DIR} implot)

set(SRC_FILES
    src/NovaCryptBot.cpp
    src/MarketData.cpp
    src/AIEngine.cpp
    src/Strategy.cpp
    src/TradeExecutor.cpp
    src/RiskManager.cpp
    src/ai/EnsembleModel.cpp
    src/backtesting/Backtester.cpp
    src/indicators/MarketData.cpp
    src/indicators/IndicatorManager.cpp
    src/sentiment/SentimentAnalyzer.cpp
    src/data/MarketDataPipeline.cpp
    src/data/DataQualityMetrics.cpp
    src/ui/Dashboard.cpp
)

add_executable(NovaCrypt main.cpp ${SRC_FILES})

# Test files
set(TEST_FILES
    src/tests/DataQualityTest.cpp
)

# Test executable
add_executable(DataQualityTest ${TEST_FILES})
target_link_libraries(DataQualityTest pthread)

# Include directories
target_include_directories(NovaCrypt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${IMGUI_DIR}
    ${IMPLOT_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIR}
    ${CURL_INCLUDE_DIRS}
)
target_include_directories(DataQualityTest PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Link libraries
target_link_libraries(NovaCrypt PRIVATE
    imgui
    implot
    ${OPENGL_LIBRARIES}
    glfw
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
)

# Set C++ standard
set_target_properties(NovaCrypt PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Copy shaders and resources
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}) 