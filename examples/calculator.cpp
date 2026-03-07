#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>
int main() {
    const char* key = std::getenv("OPENAI_API_KEY");
    if (!key || !key[0]) { std::cerr << "OPENAI_API_KEY not set\n"; return 1; }
    std::vector<llm::Tool> tools;
    tools.push_back({
        "add", "Add two numbers",
        {"a", "b"}, {"First number", "Second number"},
        [](std::map<std::string, std::string> args) -> std::string {
            double a = std::stod(args["a"]), b = std::stod(args["b"]);
            return std::to_string(a + b);
        }
    });
    tools.push_back({
        "multiply", "Multiply two numbers",
        {"a", "b"}, {"First number", "Second number"},
        [](std::map<std::string, std::string> args) -> std::string {
            double a = std::stod(args["a"]), b = std::stod(args["b"]);
            return std::to_string(a * b);
        }
    });
    tools.push_back({
        "divide", "Divide two numbers",
        {"a", "b"}, {"Numerator", "Denominator"},
        [](std::map<std::string, std::string> args) -> std::string {
            double b = std::stod(args["b"]);
            if (b == 0) return "error: division by zero";
            return std::to_string(std::stod(args["a"]) / b);
        }
    });
    llm::AgentConfig cfg;
    cfg.api_key = key;
    cfg.model   = "gpt-4o-mini";
    cfg.verbose = false;
    cfg.system_prompt = "You are a calculator assistant. Use tools to compute exact answers.";
    std::string prompt = "What is (15 + 27) * 3 divided by 6?";
    std::cout << "Question: " << prompt << "\n\n";
    auto result = llm::run_agent(prompt, tools, cfg);
    std::cout << "=== Agent Trace ===\n";
    for (const auto& step : result.steps) {
        if (step.type == llm::AgentStep::Type::ToolCall)
            std::cout << "  TOOL: " << step.tool_name << " -> " << step.tool_result << "\n";
        else
            std::cout << "  ANSWER: " << step.content << "\n";
    }
    std::cout << "\nFinal answer: " << result.answer << "\n"
              << "Iterations:   " << result.iterations_used << "\n";
    return 0;
}
