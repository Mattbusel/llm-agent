# CLAUDE.md -- llm-agent

## Build
```bash
cmake -B build && cmake --build build
```

## THE ONE RULE: SINGLE HEADER
`include/llm_agent.hpp` is the entire library. Never split it.

## API Surface
```cpp
namespace llm {
    struct Tool { name, description, param_names, param_descriptions, fn };
    struct AgentConfig { api_key, model, max_iterations, verbose, system_prompt };
    struct AgentStep { type (ToolCall|FinalAnswer), tool_name, tool_args, tool_result, content };
    struct AgentResult { answer, steps, iterations_used, max_iterations_reached };
    AgentResult run_agent(prompt, tools, config);
}
```

## Notes
- Tool params are all strings; fn receives map<string,string>
- Conversation history maintained internally as role/content pairs
- Tool result injected as user message: "Tool result for X: Y. Continue."
