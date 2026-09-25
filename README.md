# llm-agent

[![CI](https://github.com/Mattbusel/llm-agent/actions/workflows/ci.yml/badge.svg)](https://github.com/Mattbusel/llm-agent/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Single header](https://img.shields.io/badge/single-header-green.svg)

A tool-calling agent loop in one C++ header: define tools as lambdas, and the model decides when to call them.

> Part of **[llm-cpp](https://github.com/Mattbusel/llm-cpp)**, a family of 26 single-header C++ libraries for building on LLM APIs. Each one stands alone: copy one header, include it, done.

Most agent frameworks assume Python. If your program is C++ (a game, a desktop tool, a service), llm-agent gives you the OpenAI function-calling loop with no framework and no runtime: describe your tools, hand over a prompt, and read back the answer plus a full trace of what the model did.

## Features

- Tools are plain C++ lambdas: `std::string(std::map<std::string, std::string> args)`
- Builds the OpenAI `tools` JSON schema from each tool's name, description and parameter list
- Runs the loop for you: model asks for a tool, your lambda runs, the result goes back, repeat until a final answer or `max_iterations`
- Exceptions thrown inside a tool are caught and returned to the model as `[error] ...` so it can recover
- Full trace in `AgentResult::steps`: every tool call with its arguments and result, then the final answer
- Configurable model, OpenAI-compatible endpoint URL, system prompt, timeout and verbose logging to stderr

## Quick start

Requirements: a C++17 compiler and libcurl (`apt install libcurl4-openssl-dev`, `brew install curl`, or `vcpkg install curl`).

1. Copy [`include/llm_agent.hpp`](include/llm_agent.hpp) into your project.
2. In exactly one `.cpp` file, `#define LLM_AGENT_IMPLEMENTATION` before including it. Other files just `#include "llm_agent.hpp"`.

```cpp
#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"
#include <cstdlib>
#include <iostream>

int main() {
    std::vector<llm::Tool> tools = {{
        "add", "Add two numbers", {"a", "b"}, {"First number", "Second number"},
        [](std::map<std::string, std::string> args) {
            return std::to_string(std::stod(args["a"]) + std::stod(args["b"]));
        }
    }};

    llm::AgentConfig cfg;
    cfg.api_key = std::getenv("OPENAI_API_KEY");
    cfg.model   = "gpt-4o-mini";

    llm::AgentResult r = llm::run_agent("What is 1234 + 5678?", tools, cfg);
    std::cout << r.answer << "\n";
    std::cout << "steps: " << r.steps.size() << ", iterations: " << r.iterations_used << "\n";
}
```

Build and run:

```bash
g++ -std=c++17 -I include example.cpp -o example -lcurl
export OPENAI_API_KEY=sk-...
./example
```

## API

Everything lives in namespace `llm`.

| Function / type | What it does |
|---|---|
| `run_agent(prompt, tools, config)` | Run the agent loop and return an `AgentResult` (answer, steps, iterations used, whether the cap was hit) |
| `Tool` | Name, description, parameter names, parameter descriptions, and the function to call |
| `AgentConfig` | API key, model (default `gpt-4o-mini`), endpoint URL, `max_iterations` (default 10), timeout, verbose, system prompt |

## How it works

Each iteration POSTs the conversation plus the generated `tools` array to the chat completions endpoint with `tool_choice: auto`. If the reply contains a tool call, the header parses the arguments into a string map, invokes the matching lambda, and appends both the assistant tool call and the `tool` result message to the conversation. A reply without a tool call is treated as the final answer.

## Examples

The [`examples/`](examples) folder has runnable programs:

- [`calculator.cpp`](examples/calculator.cpp)
- [`file_agent.cpp`](examples/file_agent.cpp)
- [`web_search_mock.cpp`](examples/web_search_mock.cpp)

Build the examples with CMake (needs libcurl):

```bash
cmake -B build
cmake --build build
```

Examples that call the API read `OPENAI_API_KEY` from the environment.

## Limitations

- All tool parameters are declared as strings; convert them inside your lambda (`std::stod`, etc.).
- Only the first tool call in a response is executed, so parallel tool calls are not supported.
- Speaks the OpenAI tool-calling format only. No retries on rate limits.

## License

MIT. See [LICENSE](LICENSE).
