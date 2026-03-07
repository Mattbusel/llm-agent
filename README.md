# llm-agent

Tool-calling agent loop in C++. Define tools as lambdas, let the model call them. Single header.

No LangChain. No frameworks. ~250 lines of C++.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![License MIT](https://img.shields.io/badge/license-MIT-green.svg)
![Single Header](https://img.shields.io/badge/single-header-orange.svg)
![libcurl](https://img.shields.io/badge/dep-libcurl-lightgrey.svg)

## Quickstart

```cpp
#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"
#include <iostream>

int main() {
    llm::Tool add_tool{
        "add", "Add two numbers", {"a","b"}, {"First","Second"},
        [](std::map<std::string,std::string> args) -> std::string {
            return std::to_string(std::stod(args["a"]) + std::stod(args["b"]));
        }
    };

    llm::AgentConfig cfg;
    cfg.api_key = std::getenv("OPENAI_API_KEY");

    auto result = llm::run_agent("What is 42 + 58?", {add_tool}, cfg);
    std::cout << result.answer << "\n";  // "The sum of 42 and 58 is 100."
}
```

## Trace output from calculator example

```
[agent] Iteration 1 request...
[agent] Tool multiply(a=42, b=3) -> 126.000000
[agent] Iteration 2 request...
[agent] Tool divide(a=126, b=6) -> 21.000000
[agent] Iteration 3 request...

Answer: The result is 21.
Iterations: 3
```

## Installation

Copy `include/llm_agent.hpp`. Link `-lcurl`.

## API

```cpp
// Define tools
llm::Tool my_tool{
    "tool_name",
    "What this tool does (shown to the model)",
    {"param1", "param2"},              // parameter names
    {"Description of param1", "..."},  // parameter descriptions
    [](std::map<std::string,std::string> args) -> std::string {
        // your implementation
        return result_string;
    }
};

// Configure
llm::AgentConfig cfg;
cfg.api_key       = "sk-...";
cfg.model         = "gpt-4o-mini";
cfg.max_iterations = 10;
cfg.verbose       = true;  // print each step to stderr
cfg.system_prompt = "You are a helpful assistant.";

// Run
llm::AgentResult result = llm::run_agent(prompt, {tool1, tool2}, cfg);

result.answer;                  // final answer text
result.steps;                   // full trace
result.iterations_used;
result.max_iterations_reached;  // true if hit limit without finishing

// Inspect steps
for (const auto& step : result.steps) {
    if (step.type == llm::AgentStep::Type::ToolCall) {
        step.tool_name;   // which tool was called
        step.tool_args;   // map of param -> value
        step.tool_result; // what the tool returned
    } else {
        step.content;     // final answer text
    }
}
```

## Examples

- [`examples/calculator.cpp`](examples/calculator.cpp) — add/multiply/divide tools, multi-step math
- [`examples/web_search_mock.cpp`](examples/web_search_mock.cpp) — mock search + summarize, multi-step reasoning
- [`examples/file_agent.cpp`](examples/file_agent.cpp) — read_file/write_file/list_dir tools

## Building

```bash
cmake -B build && cmake --build build
export OPENAI_API_KEY=sk-...
./build/calculator
```

## Requirements

C++17. libcurl.

## See Also

| Repo | What it does |
|------|-------------|
| [llm-stream](https://github.com/Mattbusel/llm-stream) | Stream OpenAI and Anthropic responses via SSE |
| [llm-cache](https://github.com/Mattbusel/llm-cache) | LRU response cache |
| [llm-cost](https://github.com/Mattbusel/llm-cost) | Token counting and cost estimation |
| [llm-retry](https://github.com/Mattbusel/llm-retry) | Retry and circuit breaker |
| [llm-format](https://github.com/Mattbusel/llm-format) | Structured output / JSON schema |
| [llm-embed](https://github.com/Mattbusel/llm-embed) | Embeddings and vector search |
| [llm-pool](https://github.com/Mattbusel/llm-pool) | Concurrent request pool |
| [llm-log](https://github.com/Mattbusel/llm-log) | Structured JSONL logging |
| [llm-template](https://github.com/Mattbusel/llm-template) | Prompt templating |
| [llm-agent](https://github.com/Mattbusel/llm-agent) | Tool-calling agent loop |
| [llm-rag](https://github.com/Mattbusel/llm-rag) | RAG pipeline |
| [llm-eval](https://github.com/Mattbusel/llm-eval) | Evaluation and consistency scoring |
| [llm-chat](https://github.com/Mattbusel/llm-chat) | Conversation memory manager |
| [llm-vision](https://github.com/Mattbusel/llm-vision) | Multimodal image+text |
| [llm-mock](https://github.com/Mattbusel/llm-mock) | Mock LLM for testing |
| [llm-router](https://github.com/Mattbusel/llm-router) | Model routing by complexity |
| [llm-guard](https://github.com/Mattbusel/llm-guard) | PII detection and injection guard |
| [llm-compress](https://github.com/Mattbusel/llm-compress) | Context compression |
| [llm-batch](https://github.com/Mattbusel/llm-batch) | Batch processing and checkpointing |

## License

MIT — see [LICENSE](LICENSE).
