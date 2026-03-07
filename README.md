# llm-agent

Tool-calling agent loop for C++. One header, libcurl dep.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![License MIT](https://img.shields.io/badge/license-MIT-green.svg)
![Single Header](https://img.shields.io/badge/single-header-orange.svg)
![Requires libcurl](https://img.shields.io/badge/deps-libcurl-yellow.svg)

## Quickstart

```cpp
#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"

// Define a tool
llm::Tool weather;
weather.name              = "get_weather";
weather.description       = "Get current weather for a city";
weather.param_names       = {"city"};
weather.param_descriptions = {"City name"};
weather.fn = [](std::map<std::string, std::string> args) -> std::string {
    return "Sunny, 22°C in " + args["city"];
};

llm::AgentConfig cfg;
cfg.api_key = "sk-...";
cfg.model   = "gpt-4o-mini";

auto result = llm::run_agent("What's the weather in Paris?", {weather}, cfg);
std::cout << result.answer << "\n";
```

## How It Works

1. Send prompt to OpenAI chat completions with tool definitions
2. If response contains `tool_calls`, dispatch to matching C++ lambda
3. Inject tool result back into conversation
4. Repeat until model returns a final text answer or `max_iterations` is reached

## API Reference

### Tool

```cpp
struct Tool {
    std::string name;
    std::string description;
    std::vector<std::string> param_names;
    std::vector<std::string> param_descriptions;
    std::function<std::string(std::map<std::string, std::string>)> fn;
};
```

### AgentConfig

```cpp
struct AgentConfig {
    std::string api_key;
    std::string model          = "gpt-4o-mini";
    int         max_iterations = 10;
    bool        verbose        = false;   // logs to stderr
    std::string system_prompt;
};
```

### AgentResult

```cpp
struct AgentResult {
    std::string            answer;
    std::vector<AgentStep> steps;
    int                    iterations_used;
    bool                   max_iterations_reached;
};
```

## Examples

| File | What it shows |
|------|--------------|
| [`examples/calculator.cpp`](examples/calculator.cpp) | add + sqrt tools, multi-step math |
| [`examples/web_search_mock.cpp`](examples/web_search_mock.cpp) | Mock search tool, multi-query |

## Building

```bash
cmake -B build && cmake --build build
export OPENAI_API_KEY=sk-...
./build/calculator
./build/web_search_mock
```

## Requirements

C++17. Requires libcurl.

## See Also

| Repo | What it does |
|------|-------------|
| [llm-stream](https://github.com/Mattbusel/llm-stream) | Streaming responses |
| [llm-cache](https://github.com/Mattbusel/llm-cache) | Response caching |
| [llm-cost](https://github.com/Mattbusel/llm-cost) | Token counting + cost |
| [llm-retry](https://github.com/Mattbusel/llm-retry) | Retry + circuit breaker |
| [llm-format](https://github.com/Mattbusel/llm-format) | Structured output |
| [llm-embed](https://github.com/Mattbusel/llm-embed) | Embeddings + vector search |
| [llm-pool](https://github.com/Mattbusel/llm-pool) | Concurrent request pool |
| [llm-log](https://github.com/Mattbusel/llm-log) | Structured JSONL logging |
| [llm-template](https://github.com/Mattbusel/llm-template) | Prompt templating |
| **llm-agent** *(this repo)* | Tool-calling agent loop |
| [llm-rag](https://github.com/Mattbusel/llm-rag) | RAG pipeline |
| [llm-eval](https://github.com/Mattbusel/llm-eval) | Evaluation + consistency scoring |

## License

MIT -- see [LICENSE](LICENSE).
