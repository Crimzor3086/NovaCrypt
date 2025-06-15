#include "Dashboard.h"
#include <imgui_internal.h>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace novacrypt {

Dashboard::Dashboard()
    : window_(nullptr),
      running_(false),
      showSettings_(false),
      showTradeLog_(true),
      showPerformance_(true),
      liveTrading_(false)
{
    // Initialize metrics
    currentMetrics_ = {0.0, 0.0, 0.0, 0.0, 0};
}

Dashboard::~Dashboard() {
    shutdown();
}

bool Dashboard::initialize() {
    if (!glfwInit()) {
        return false;
    }

    // Create window with OpenGL context
    window_ = glfwCreateWindow(1280, 720, "NovaCrypt Dashboard", nullptr, nullptr);
    if (!window_) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    setupTheme();
    setupFonts();

    running_ = true;
    return true;
}

void Dashboard::run() {
    while (!glfwWindowShouldClose(window_) && running_) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        renderMainWindow();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window_, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window_);
    }
}

void Dashboard::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    glfwTerminate();
}

void Dashboard::renderMainWindow() {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("NovaCrypt Dashboard", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Top toolbar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Settings", nullptr, &showSettings_)) {}
            if (ImGui::MenuItem("Exit", "Alt+F4")) { running_ = false; }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Trade Log", nullptr, &showTradeLog_);
            ImGui::MenuItem("Performance", nullptr, &showPerformance_);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // Main content area
    ImGui::Columns(2, "MainColumns", true);
    
    // Left column: Chart
    ImGui::BeginChild("ChartArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    renderChart();
    ImGui::EndChild();

    // Right column: Trade signals and metrics
    ImGui::NextColumn();
    ImGui::BeginChild("SignalsArea", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()));
    renderTradeSignals();
    if (showPerformance_) {
        renderPerformanceMetrics();
    }
    ImGui::EndChild();

    // Bottom area: Trade log
    if (showTradeLog_) {
        ImGui::Columns(1);
        ImGui::BeginChild("TradeLog", ImVec2(0, 200), true);
        renderTradeLog();
        ImGui::EndChild();
    }

    ImGui::End();
}

void Dashboard::renderChart() {
    ImGui::Text("Market Chart");
    ImGui::Separator();

    // Chart area
    ImVec2 chartSize = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("Chart", chartSize, true);
    
    drawCandlestickChart();
    drawIndicators();
    drawTradeSignals();
    
    ImGui::EndChild();
}

void Dashboard::renderTradeSignals() {
    ImGui::Text("Trade Signals");
    ImGui::Separator();

    for (const auto& signal : tradeSignals_) {
        ImGui::PushID(&signal);
        
        // Signal type indicator
        ImVec4 color;
        switch (signal.type) {
            case TradeSignal::Type::BUY:
                color = ImVec4(0.0f, 0.8f, 0.0f, 1.0f);
                ImGui::TextColored(color, "BUY");
                break;
            case TradeSignal::Type::SELL:
                color = ImVec4(0.8f, 0.0f, 0.0f, 1.0f);
                ImGui::TextColored(color, "SELL");
                break;
            case TradeSignal::Type::HOLD:
                color = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);
                ImGui::TextColored(color, "HOLD");
                break;
        }

        // Confidence bar
        ImGui::ProgressBar(signal.confidence, ImVec2(-1, 0), 
            std::to_string(static_cast<int>(signal.confidence * 100)).append("%").c_str());

        // Reason and timestamp
        ImGui::TextWrapped("%s", signal.reason.c_str());
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&std::chrono::system_clock::to_time_t(signal.timestamp)),
                          "%Y-%m-%d %H:%M:%S");
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", ss.str().c_str());
        
        ImGui::Separator();
        ImGui::PopID();
    }
}

void Dashboard::renderPerformanceMetrics() {
    ImGui::Text("Performance Metrics");
    ImGui::Separator();

    ImGui::Columns(2, "MetricsColumns");
    
    // Left column
    ImGui::Text("Total P&L");
    ImGui::Text("Win Rate");
    ImGui::Text("Avg Trade");
    ImGui::Text("Max Drawdown");
    ImGui::Text("Total Trades");
    
    ImGui::NextColumn();
    
    // Right column with values
    ImGui::TextColored(currentMetrics_.totalPnL >= 0 ? 
        ImVec4(0.0f, 0.8f, 0.0f, 1.0f) : ImVec4(0.8f, 0.0f, 0.0f, 1.0f),
        "%.2f%%", currentMetrics_.totalPnL);
    ImGui::Text("%.1f%%", currentMetrics_.winRate);
    ImGui::Text("%.2f%%", currentMetrics_.averageTrade);
    ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "%.2f%%", currentMetrics_.maxDrawdown);
    ImGui::Text("%d", currentMetrics_.totalTrades);
    
    ImGui::Columns(1);
}

void Dashboard::renderSettings() {
    if (!showSettings_) return;

    ImGui::Begin("Settings", &showSettings_);
    
    // Trading mode
    if (ImGui::Checkbox("Live Trading", &liveTrading_)) {
        if (onLiveTradingToggle_) onLiveTradingToggle_(liveTrading_);
    }
    
    // Strategy selection
    const char* strategies[] = { "Momentum", "Mean Reversion", "Trend Following" };
    static int currentStrategy = 0;
    if (ImGui::Combo("Strategy", &currentStrategy, strategies, IM_ARRAYSIZE(strategies))) {
        if (onStrategyChange_) onStrategyChange_(strategies[currentStrategy]);
    }
    
    // Parameters
    ImGui::Text("Strategy Parameters");
    ImGui::Separator();
    
    static float riskLevel = 0.5f;
    if (ImGui::SliderFloat("Risk Level", &riskLevel, 0.0f, 1.0f)) {
        std::map<std::string, double> params;
        params["risk_level"] = riskLevel;
        if (onParameterUpdate_) onParameterUpdate_(params);
    }
    
    ImGui::End();
}

void Dashboard::renderTradeLog() {
    ImGui::Text("Trade Log");
    ImGui::Separator();

    // Example trade log entries
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "2024-03-20 14:30:15 - BUY  BTC/USD  @ 65,432.10");
    ImGui::TextColored(ImVec4(0.8f, 0.0f, 0.0f, 1.0f), "2024-03-20 14:35:22 - SELL BTC/USD  @ 65,789.50");
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.0f, 1.0f), "2024-03-20 14:40:05 - HOLD ETH/USD  @ 3,456.78");
}

void Dashboard::setupTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.26f, 0.26f, 0.40f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.67f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    
    // Style
    style.WindowPadding = ImVec2(8, 8);
    style.FramePadding = ImVec2(4, 3);
    style.ItemSpacing = ImVec2(8, 4);
    style.ItemInnerSpacing = ImVec2(4, 4);
    style.IndentSpacing = 21.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 8.0f;
    
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;
}

void Dashboard::setupFonts() {
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
}

void Dashboard::onMarketDataUpdate(const OHLCV& data) {
    marketData_.push_back(data);
    // Keep only last 1000 data points
    if (marketData_.size() > 1000) {
        marketData_.erase(marketData_.begin());
    }
}

void Dashboard::onTradeSignal(const TradeSignal& signal) {
    tradeSignals_.push_back(signal);
    // Keep only last 50 signals
    if (tradeSignals_.size() > 50) {
        tradeSignals_.erase(tradeSignals_.begin());
    }
}

void Dashboard::onPerformanceUpdate(const PerformanceMetrics& metrics) {
    currentMetrics_ = metrics;
}

void Dashboard::drawCandlestickChart() {
    // TODO: Implement candlestick chart drawing
    // This would use ImPlot or custom OpenGL rendering
}

void Dashboard::drawIndicators() {
    // TODO: Implement indicator overlays
    // This would draw RSI, MACD, Moving Averages, etc.
}

void Dashboard::drawTradeSignals() {
    // TODO: Implement trade signal markers on chart
    // This would show buy/sell points on the chart
}

} // namespace novacrypt 