# NovaCrypt: AI-Powered Cryptocurrency Trading Bot

NovaCrypt is a sophisticated cryptocurrency trading bot that leverages artificial intelligence and real-time market analysis to make informed trading decisions. The bot combines technical indicators, sentiment analysis, and machine learning to optimize trading strategies.

## Features

### Core Components
- **Market Data Processing**: Real-time processing of OHLCV and order book data
- **AI Engine**: Machine learning models for price prediction and strategy optimization
- **Strategy System**: Modular trading strategies with customizable parameters
- **Risk Management**: Dynamic position sizing and risk control
- **Trade Execution**: Efficient order execution with slippage control

### Advanced Features
- **Ensemble Models**: Multiple AI models working together for improved accuracy
- **Sentiment Analysis**: Multi-source sentiment tracking (Twitter, Reddit, News)
- **Technical Indicators**: Comprehensive set of market indicators
- **Backtesting**: Historical performance analysis and strategy optimization
- **Real-time Data Pipeline**: Efficient processing of market data and sentiment

### User Interface
- **Modern Dashboard**: Sleek, dark-themed interface built with Dear ImGui
- **Real-time Charts**: Interactive candlestick charts with technical indicators
- **Trade Signals**: Color-coded buy/sell signals with confidence scores
- **Performance Metrics**: Live tracking of P&L, win rate, and drawdown
- **Trade Log**: Detailed history of executed trades
- **Strategy Controls**: Intuitive parameter adjustment and strategy selection

## Requirements

### System Requirements
- C++17 or later
- CMake 3.10 or later
- OpenGL 3.3 or later
- GLFW 3.3 or later

### Required Libraries
- **Graphics & UI**:
  - Dear ImGui
  - ImPlot
  - OpenGL
  - GLFW
- **Networking & Security**:
  - OpenSSL
  - cURL
- **Data Processing**:
  - Boost (system, filesystem)
  - JSON for Modern C++

## Building

1. Clone the repository:
```bash
git clone https://github.com/yourusername/NovaCrypt.git
cd NovaCrypt
```

2. Install dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev
sudo apt-get install libboost-all-dev libssl-dev libcurl4-openssl-dev

# macOS
brew install glfw boost openssl curl

# Windows
# Use vcpkg or download pre-built binaries
```

3. Create a build directory:
```bash
mkdir build && cd build
```

4. Configure and build:
```bash
cmake ..
make
```

## Configuration

The bot can be configured through a configuration file. Key settings include:
- API credentials
- Trading parameters
- Risk management rules
- Strategy settings
- Data sources
- UI preferences

## Usage

1. Configure your API credentials and trading parameters
2. Start the bot:
```bash
./NovaCrypt --config config.json
```

3. Use the dashboard to:
   - Monitor market data and indicators
   - View trade signals and performance
   - Adjust strategy parameters
   - Track executed trades
   - Toggle live/simulation mode

## Project Structure

```
NovaCrypt/
├── src/
│   ├── ai/              # AI and machine learning components
│   ├── backtesting/     # Backtesting framework
│   ├── data/           # Data processing and pipeline
│   ├── indicators/     # Technical indicators
│   ├── sentiment/      # Sentiment analysis
│   ├── ui/            # User interface components
│   └── main components # Core trading components
├── include/            # Header files
├── external/          # Third-party dependencies
│   ├── imgui/        # Dear ImGui
│   └── implot/       # ImPlot
├── tests/             # Unit tests
├── docs/              # Documentation
└── config/            # Configuration files
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Disclaimer

This software is for educational purposes only. Use at your own risk. The authors are not responsible for any financial losses incurred through the use of this software.

## Acknowledgments

- Thanks to all contributors
- Inspired by various open-source trading projects
- Built with modern C++ best practices
- UI powered by Dear ImGui and ImPlot 