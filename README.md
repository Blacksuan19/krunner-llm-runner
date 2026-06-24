# KRunner LLM Plugin

A KDE Plasma 6 KRunner plugin for sending short prompts to hosted LLM APIs or OpenAI-compatible proxies.

## Supported Providers

- OpenAI
- OpenAI-compatible APIs and proxies
- Anthropic
- OpenRouter
- Gemini
- Groq

## Requirements

- KDE Plasma 6
- Qt 6.6 or later
- KDE Frameworks 6
- CMake 3.28 or later
- C++23 compiler

## Dependencies

Arch Linux:

```bash
sudo pacman -S \
    extra-cmake-modules \
    qt6-base \
    qt6-tools \
    kf6-runner \
    kf6-i18n \
    kf6-config \
    kf6-kcmutils
```

Ubuntu/Debian:

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

Fedora:

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

## Build And Install

```bash
./build.sh
./install.sh
```

Restart KRunner after installing:

```bash
kquitapp6 krunner
krunner &
```

To uninstall:

```bash
./uninstall.sh
```

## Configuration

Open KDE System Settings, then go to Search -> KRunner -> LLM Runner.

Available settings:

- **Trigger Word**: Prefix used in KRunner, for example `llm`.
- **Provider**: Hosted provider or `OpenAI Compatible`.
- **API Key**: Provider API key. For local proxies this may be empty if the proxy does not require auth.
- **API Base URL**: Only used for OpenAI-compatible APIs.
- **Model**: Model name sent to the provider. For OpenAI, OpenRouter, and OpenAI-compatible APIs, the settings UI can fetch available model IDs from `/models`.
- **System Prompt**: Optional instructions sent with each request. Defaults to concise 2-3 sentence answers.
- **Max Tokens**: Response length limit.
- **Timeout**: Request timeout in seconds.
- **Debounce Delay**: Delay before sending the request after typing stops.
- **Status Row**: Show or hide the temporary `Querying LLM...` row while waiting for a response.

For OpenAI-compatible proxies, configure the API base as either the `/v1` base URL or the full chat completions endpoint:

```text
Provider: OpenAI Compatible
API Base URL: http://localhost:8000/v1
Model: your-proxy-model-name
```

The plugin appends `/chat/completions` unless the URL already ends with `/chat/completions`.
Model discovery uses the same base URL and queries `/models`.

## Usage

Open KRunner and type the trigger word followed by your prompt:

```text
llm what is the capital of France?
llm explain quantum computing in simple terms
```

Click the result to copy the response to the clipboard.

## Notes

- Provider errors are shown directly in KRunner when possible, including HTTP status codes and API error messages.
- API keys and proxy settings are stored in KDE configuration.
- The plugin sends prompts only when the configured trigger word is used.

## License

MIT. See [LICENSE](LICENSE).
