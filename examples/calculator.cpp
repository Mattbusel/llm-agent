#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
    const char* key = std::getenv("OPENAI_API_KEY");
    if (!key || !key[0]) { std::cerr << "OPENAI_API_KEY not set\n"; return 1; }

    llm::Tool add_tool;
    add_tool.name              = "add";
    add_tool.description       = "Add two numbers together";
    add_tool.param_names       = {"a", "b"};
    add_tool.param_descriptions = {"First number", "Second number"};
    add_tool.fn = [](std::map<std::string, std::string> args) -> std::string {
        try {
            double a = std::stod(args["a"]), b = std::stod(args["b"]);
            return std::to_string(a + b);
        } catch (...) { return "error: invalid numbers"; }
    };

    llm::Tool sqrt_tool;
    sqrt_tool.name              = "sqrt";
    sqrt_tool.description       = "Compute the square root of a number";
    sqrt_tool.param_names       = {"x"};
    sqrt_tool.param_descriptions = {"Number to take the square root of"};
    sqrt_tool.fn = [](std::map<std::string, std::string> args) -> std::string {
        try {
            double x = std::stod(args["x"]);
            if (x < 0) return "error: negative number";
            return std::to_string(std::sqrt(x));
        } catch (...) { return "error: invalid number"; }
    };

    llm::AgentConfig cfg;
    cfg.api_key = key;
    cfg.model   = "gpt-4o-mini";
    cfg.verbose = false;

    auto result = llm::run_agent(
        "What is the square root of (3 + 13)?",
        {add_tool, sqrt_tool},
        cfg
    );

    std::cout << "Answer: " << result.answer << "\n";
    std::cout << "Steps:  " << result.iterations_used << "\n";
    for (const auto& step : result.steps) {
        if (step.type == llm::AgentStep::Type::ToolCall)
            std::cout << "  tool=" << step.tool_name
                      << " result=" << step.tool_result << "\n";
    }
    return 0;
}
