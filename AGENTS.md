# AGENTS.md -- llm-agent

## Purpose
Single-header C++ tool-calling agent loop using OpenAI function calling API.
Runs a ReAct-style loop: call model → if tool_calls, dispatch to C++ lambda →
append result → repeat until final answer or max_iterations.

## Architecture
```
llm-agent/
  include/llm_agent.hpp   <- THE ENTIRE LIBRARY. Do not split.
  examples/
    calculator.cpp
    web_search_mock.cpp
  CMakeLists.txt
```

## Build
```bash
cmake -B build && cmake --build build
```

## Rules
- Single header only.
- Only libcurl as external dep.
- All public API in namespace llm.
- Implementation inside #ifdef LLM_AGENT_IMPLEMENTATION guard.

## API Surface
- Tool struct: name, description, param_names, param_descriptions, fn
- AgentConfig: api_key, model, max_iterations, verbose, system_prompt
- AgentResult: answer, steps, iterations_used, max_iterations_reached
- run_agent(prompt, tools, config) -> AgentResult
