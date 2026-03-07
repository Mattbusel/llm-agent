# llm-agent

Tool-calling agent loop in C++. Define tools as lambdas, let the model call them. No LangChain. No frameworks. One header.

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![License: MIT](https://img.shields.io/badge/License-MIT-green)
![Single Header](https://img.shields.io/badge/library-single--header-orange)

---

## 30-second quickstart

```cpp
#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"
#include <cstdlib>
#include <iostream>

int main() {
    std::vector<llm::Tool> tools;
    tools.push_back({
        "add", "Add two numbers",
        {"a", "b"}, {"First number", "Second number"},
        [](std::map<std::string, std::string> args) -> std::string {
            return std::to_string(std::stod(args["a"]) + std::stod(args["b"]));
        }
    });

    llm::AgentConfig cfg;
    cfg.api_key = std::getenv("OPENAI_API_KEY");
    cfg.model   = "gpt-4o-mini";

    auto result = llm::run_agent("What is 15 + 27?", tools, cfg);
    std::cout << result.answer << "\n";
    // Output: The answer is 42.
}
```

**No LangChain. No frameworks. ~200 lines of C++.**

---

## Installation

```bash
cp include/llm_agent.hpp your-project/
```

Link with `-lcurl`.

---

## API Reference

```cpp
// Define a tool
llm::Tool t;
t.name        = "search";
t.description = "Search the web for a query";
t.param_names = {"query"};
t.param_descriptions = {"The search query string"};
t.fn = [](std::map<std::string, std::string> args) -> std::string {
    return fetch_search_results(args["query"]);
};

// Configure the agent
llm::AgentConfig cfg;
cfg.api_key        = "sk-...";
cfg.model          = "gpt-4o-mini";
cfg.max_iterations = 10;
cfg.verbose        = true;  // print each step to stderr
cfg.system_prompt  = "You are a research assistant.";

// Run
llm::AgentResult result = llm::run_agent(prompt, tools, cfg);

// Inspect trace
for (const auto& step : result.steps) {
    if (step.type == llm::AgentStep::Type::ToolCall)
        std::cout << step.tool_name << " -> " << step.tool_result << "\n";
    else
        std::cout << "Answer: " << step.content << "\n";
}
std::cout << result.answer << "\n";
```

---

## Example trace (calculator)

```
Question: What is (15 + 27) * 3 divided by 6?

=== Agent Trace ===
  TOOL: add -> 42.000000
  TOOL: multiply -> 126.000000
  TOOL: divide -> 21.000000
  ANSWER: The result is 21.

Final answer: The result is 21.
Iterations:   3
```

---

## Building

```bash
cmake -B build && cmake --build build
export OPENAI_API_KEY=sk-...
./build/calculator
./build/web_search_mock
```

---

## See Also

| Repo | What it does |
|------|-------------|
| [llm-stream](https://github.com/Mattbusel/llm-stream) | Stream OpenAI & Anthropic responses token by token |
| [llm-cache](https://github.com/Mattbusel/llm-cache) | Cache responses, skip redundant calls |
| [llm-cost](https://github.com/Mattbusel/llm-cost) | Token counting + cost estimation |
| [llm-retry](https://github.com/Mattbusel/llm-retry) | Retry with backoff + circuit breaker |
| [llm-format](https://github.com/Mattbusel/llm-format) | Structured output enforcement |
| [llm-embed](https://github.com/Mattbusel/llm-embed) | Text embeddings + nearest-neighbor search |
| [llm-pool](https://github.com/Mattbusel/llm-pool) | Concurrent request pool + rate limiting |
| [llm-log](https://github.com/Mattbusel/llm-log) | Structured JSONL logger for LLM calls |
| [llm-template](https://github.com/Mattbusel/llm-template) | Prompt templating with loops + conditionals |
| [llm-agent](https://github.com/Mattbusel/llm-agent) | Tool-calling agent loop |
| [llm-rag](https://github.com/Mattbusel/llm-rag) | Full RAG pipeline |
| [llm-eval](https://github.com/Mattbusel/llm-eval) | Consistency and quality evaluation |

---

## License

MIT — see [LICENSE](LICENSE).
