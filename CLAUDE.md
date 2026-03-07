# CLAUDE.md — llm-agent
## Build & Run
cmake -B build && cmake --build build
export OPENAI_API_KEY=sk-...
./build/calculator
## THE RULE: Single Header
include/llm_agent.hpp is the entire library.
## API
- Tool: name, description, param_names, param_descriptions, fn(map) -> string
- AgentConfig: api_key, model, max_iterations, verbose, system_prompt
- run_agent(prompt, tools, config) -> AgentResult
- AgentResult: answer, steps, iterations_used, max_iterations_reached
- AgentStep: type (ToolCall|FinalAnswer), tool_name, tool_args, tool_result, content
## Common mistakes
- Forgetting #define LLM_AGENT_IMPLEMENTATION in one TU
- Tool fn throwing uncaught exceptions (catch and return error string)
- max_iterations too low for complex multi-step tasks
