# AGENTS.md — llm-agent
## Purpose
Single-header C++ tool-calling agent loop. Define tools as C++ lambdas, let the model call them.
## Architecture
Everything in `include/llm_agent.hpp`. Guard: `#ifdef LLM_AGENT_IMPLEMENTATION`. Requires libcurl.
## Build
cmake -B build && cmake --build build
## Constraints
Single header, libcurl only, C++17, namespace llm. OpenAI function calling API. Hand-rolled JSON.
