# KRunner LLM Plugin

A KRunner plugin for KDE Plasma 6 that allows you to query Large Language Models directly from KRunner.

## Features

- 🚀 Query LLMs directly from KRunner
- 🤖 Support for multiple LLM providers:
  - OpenAI (GPT-4, GPT-3.5-turbo, etc.)
  - OpenAI-compatible proxies (custom `/v1` API base)
  - Anthropic (Claude 3.5 Sonnet, etc.)
  - OpenRouter (various models)
  - Gemini (gemini-2.5-flash, etc.)
  - Groq (allam-2-7b, etc.)
- 📋 Copy responses to clipboard with a single click
- ⚙️ Configurable
- 🔑 API key and proxy settings stored in KDE configuration
- 🌐 Custom API base URL for OpenAI-compatible proxies
- ⏱️ Configurable timeout and token limits
- 🎯 Keyword-based triggering to avoid accidental queries

## Requirements

- KDE Plasma 6
- Qt 6.6.0 or later
- KF6 (KDE Frameworks 6)
- CMake 3.28 or later
- C++23 compatible compiler (GCC 14+, Clang 17+)

## Building

### Install Dependencies

On Arch Linux:

```bash
sudo pacman -S \
    extra-cmake-modules \
    qt6-base \
    qt6-tools \
    qt6-networkauth \
    kf6-runner \
    kf6-i18n \
    kf6-config \
    kf6-kcmutils
```

On Ubuntu/Debian:

```bash
sudo apt install \
    extra-cmake-modules \
    qt6-base-dev \
    qt6-base-dev-tools \
    libkf6runner-dev \
    libkf6i18n-dev \
    libkf6config-dev \
    libkf6configwidgets-dev \
    libkf6kcmutils-dev
```

On Fedora:

```bash
sudo dnf install \
    extra-cmake-modules \
    qt6-qtbase-devel \
    kf6-kcmutils-devel \
    kf6-kconfig-devel \
    kf6-kconfigwidgets-devel \
    kf6-ki18n-devel \
    kf6-krunner-devel
```

### Build

```bash
chmod u+x ./build.sh && ./build.sh
```

### Install

```bash
chmod u+x ./install.sh && ./install.sh
```

### Uninstall

```bash
chmod u+x ./uninstall.sh && ./uninstall.sh
```

### Restart KRunner

After installation, restart KRunner to load the plugin:

```bash
kquitapp6 krunner
krunner &
```

## Configuration

1. Open KDE System Settings
2. Navigate to Search → KRunner → LLM Runner (or search for "LLM Runner Configuration")
3. Configure the following settings:
   - **Trigger Word**: The keyword to activate the plugin (default: "llm")
   - **Provider**: Choose your LLM provider. Use **OpenAI Compatible** for a self-hosted or proxied OpenAI-style API.
   - **API Key**: Your API key from the provider. This can be empty for OpenAI-compatible proxies that do not require client-side auth.
   - **API Base URL**: Base URL for OpenAI-compatible APIs. Use either the `/v1` base URL, such as `http://localhost:8000/v1`, or the full `/v1/chat/completions` endpoint.
   - **Model**: The specific model to use (e.g., gpt-4, claude-3-5-sonnet-20241022)
   - **Max Tokens**: Maximum length of the response (default: 150)
   - **Timeout**: Request timeout in seconds (default: 30)
   - **Debounce Delay**: The delay from last keystroke after which query is sent to LLM

### Getting API Keys

- **OpenAI**: [https://platform.openai.com/api-keys](https://platform.openai.com/api-keys)
- **Anthropic**: [https://console.anthropic.com/settings/keys](https://console.anthropic.com/settings/keys)
- **OpenRouter**: [https://openrouter.ai/keys](https://openrouter.ai/keys)
- **Gemini**: [https://developers.generativeai.google/products/gemini](https://developers.generativeai.google/products/gemini)
- **Groq**: [https://platform.groq.ai/account/api-keys](https://platform.groq.ai/account/api-keys)

### OpenAI-Compatible Proxies

Choose **OpenAI Compatible** when your server accepts OpenAI-style chat completion requests.

Example configuration:

```text
Provider: OpenAI Compatible
API Base URL: http://localhost:8000/v1
API Key: optional, depending on your proxy
Model: the model name exposed by your proxy
```

The plugin sends requests to `/chat/completions`. If the base URL already ends with `/chat/completions`, it is used as-is.

## Usage

1. Open KRunner
2. Type your trigger word followed by your question:
   ```
   llm what is the capital of France?
   llm explain quantum computing in simple terms
   llm write a haiku about programming
   ```
3. The plugin will query the configured provider and display the response
4. Click on the result to copy it to your clipboard

## Testing

The project includes unit tests, but they are currently disabled in the top-level CMake file. To run them, enable the test section in `CMakeLists.txt`, rebuild, then run:

```bash
cd build
ctest --output-on-failure
```

Or run specific tests:

```bash
./tests/test_llmclient
./tests/test_llmrunner
```

## Troubleshooting

### Plugin doesn't appear in KRunner

1. Verify installation:

   ```bash
   qdbus org.kde.krunner /App org.kde.krunner.App.display
   ```

2. Check if the plugin is loaded:

   ```bash
   kreadconfig6 --file krunnerrc --group Plugins --key krunner_llmEnabled
   ```

3. Enable the plugin manually:
   ```bash
   kwriteconfig6 --file krunnerrc --group Plugins --key krunner_llmEnabled true
   kquitapp6 krunner && krunner &
   ```

### API Key Issues

- Verify your API key is correct in the configuration
- For OpenAI-compatible proxies, confirm whether your proxy expects an API key. Leave the field empty only if the proxy accepts unauthenticated requests.
- Check that you have sufficient credits/quota with your provider
- Some providers require payment method setup even for API access

### Network Errors

- Check your internet connection
- For OpenAI-compatible proxies, verify the API Base URL is reachable and points to a chat-completions-compatible endpoint
- HTTP 503 usually comes from the configured provider or proxy, not KRunner itself. Check that the proxy is running, the upstream model is available, and the configured model name matches what the proxy exposes.
- Verify firewall settings allow outbound HTTPS connections
- Some corporate networks block API endpoints - try from a different network

### Build Errors

If you encounter C++23 errors:

- Ensure you're using GCC 14+ or Clang 17+
- Check that CMake 3.28+ is installed
- Try enabling verbose output: `make VERBOSE=1`

### Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

This project is licensed under the MIT License. See the LICENSE file for details.

## Privacy & Security

- API keys and custom API base URLs are stored in KDE's configuration system
- No prompts or responses are logged or stored locally by the plugin beyond normal provider/network behavior
- Hosted providers use HTTPS endpoints. For OpenAI-compatible proxies, transport security depends on the API Base URL you configure.
- The plugin only sends data when explicitly triggered by the user

## Credits

- Built for KDE Plasma 6
- Uses Qt 6 and KDE Frameworks 6
- Inspired by the need for quick AI assistance while coding

## Roadmap

- [x] Support OpenAI-compatible API proxies
- [ ] Add native local LLM provider presets (Ollama, etc.)
- [ ] Add streaming response support
- [ ] Implement conversation history
- [ ] Custom system prompts
- [ ] Response formatting options
- [ ] Multi-language support

---

Made with ❤️ for the KDE community
