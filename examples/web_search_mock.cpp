#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"
#include <cstdlib>
#include <iostream>
#include <map>
int main() {
    const char* key = std::getenv("OPENAI_API_KEY");
    if (!key || !key[0]) { std::cerr << "OPENAI_API_KEY not set\n"; return 1; }
    // Mock search database
    std::map<std::string, std::string> search_db = {
        {"climate change", "Global temperatures have risen 1.1C since pre-industrial times."},
        {"renewable energy", "Solar and wind now account for 12% of global electricity."},
        {"carbon capture", "Direct air capture removes CO2 at ~$400 per tonne currently."},
    };
    std::vector<llm::Tool> tools;
    tools.push_back({
        "search", "Search for information on a topic",
        {"query"}, {"The search query"},
        [&search_db](std::map<std::string, std::string> args) -> std::string {
            std::string q = args["query"];
            for (const auto& [k, v] : search_db) {
                if (q.find(k) != std::string::npos || k.find(q) != std::string::npos)
                    return v;
            }
            return "No results found for: " + q;
        }
    });
    llm::AgentConfig cfg;
    cfg.api_key = key;
    cfg.model   = "gpt-4o-mini";
    cfg.max_iterations = 5;
    cfg.system_prompt = "You are a research assistant. Use the search tool to find relevant information before answering.";
    std::string prompt = "What is the current state of climate change and what solutions exist?";
    std::cout << "Question: " << prompt << "\n\n=== Agent Trace ===\n";
    auto result = llm::run_agent(prompt, tools, cfg);
    for (const auto& step : result.steps) {
        if (step.type == llm::AgentStep::Type::ToolCall)
            std::cout << "  SEARCH: \"" << step.tool_args.at("query")
                      << "\" -> " << step.tool_result << "\n";
        else
            std::cout << "\nAnswer: " << step.content << "\n";
    }
    return 0;
}
