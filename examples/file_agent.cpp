#define LLM_AGENT_IMPLEMENTATION
#include "llm_agent.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem>

int main() {
    const char* key = std::getenv("OPENAI_API_KEY");
    if (!key || !*key) { std::cerr << "Set OPENAI_API_KEY\n"; return 1; }

    std::vector<llm::Tool> tools = {
        {
            "read_file",
            "Read the contents of a file",
            {"path"},
            {"File path to read"},
            [](std::map<std::string,std::string> args) -> std::string {
                std::ifstream f(args.at("path"));
                if (!f) return "Error: cannot open " + args.at("path");
                std::ostringstream ss; ss << f.rdbuf();
                return ss.str();
            }
        },
        {
            "write_file",
            "Write content to a file",
            {"path", "content"},
            {"File path to write", "Content to write"},
            [](std::map<std::string,std::string> args) -> std::string {
                std::ofstream f(args.at("path"));
                if (!f) return "Error: cannot write " + args.at("path");
                f << args.at("content");
                return "Written " + std::to_string(args.at("content").size()) + " bytes to " + args.at("path");
            }
        },
        {
            "list_dir",
            "List files in a directory",
            {"path"},
            {"Directory path"},
            [](std::map<std::string,std::string> args) -> std::string {
                std::string result;
                try {
                    for (const auto& e : std::filesystem::directory_iterator(args.at("path")))
                        result += e.path().filename().string() + "\n";
                } catch (...) { return "Error listing directory"; }
                return result.empty() ? "(empty)" : result;
            }
        },
    };

    // Seed a test file
    { std::ofstream f("agent_test.txt"); f << "Hello from the file agent test!\nLine 2.\n"; }

    llm::AgentConfig cfg;
    cfg.api_key     = key;
    cfg.model       = "gpt-4o-mini";
    cfg.verbose     = false;
    cfg.max_iterations = 10;

    auto result = llm::run_agent(
        "Read agent_test.txt, then write its contents in reverse line order to agent_output.txt. "
        "Confirm what you wrote.",
        tools, cfg);

    std::cout << "Answer: " << result.answer << "\n";
    std::cout << "Steps taken: " << result.steps.size() << "\n";

    // Show what was written
    std::ifstream out("agent_output.txt");
    if (out) { std::cout << "\nagent_output.txt:\n"; std::cout << out.rdbuf(); }
    return 0;
}
