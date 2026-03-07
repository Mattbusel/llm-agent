#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>

// Mock "web search" tool — returns canned results
static std::string mock_search(const std::string& query) {
    static const std::map<std::string, std::string> db = {
        {"tokio", "Tokio is an async runtime for Rust. Current version: 1.40."},
        {"openai", "OpenAI was founded in 2015. GPT-4o is their latest model."},
        {"rust",  "Rust is a systems programming language. Version 1.82 (2024)."},
    };
    std::string lq = query;
    for (char& c : lq) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    for (const auto& [k, v] : db)
        if (lq.find(k) != std::string::npos) return v;
    return "No results found for: " + query;
}

int main() {
    const char* key = std::getenv("OPENAI_API_KEY");
    if (!key || !key[0]) { std::cerr << "OPENAI_API_KEY not set\n"; return 1; }

    llm::Tool search;
    search.name              = "web_search";
    search.description       = "Search the web for information about a topic";
    search.param_names       = {"query"};
    search.param_descriptions = {"The search query"};
    search.fn = [](std::map<std::string, std::string> args) -> std::string {
        return mock_search(args["query"]);
    };

    llm::AgentConfig cfg;
    cfg.api_key      = key;
    cfg.model        = "gpt-4o-mini";
    cfg.system_prompt = "You are a helpful assistant. Use the web_search tool to look up facts.";

    auto result = llm::run_agent(
        "What is Tokio and when was OpenAI founded?",
        {search},
        cfg
    );

    std::cout << "Answer:\n" << result.answer << "\n";
    std::cout << "\nTool calls made: " << result.steps.size() - 1 << "\n";
    return 0;
}
