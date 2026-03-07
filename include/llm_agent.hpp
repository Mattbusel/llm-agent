#pragma once

// llm_agent.hpp -- Zero-dependency single-header C++ tool-calling agent loop.
// Define tools as C++ lambdas, let the model call them via OpenAI function calling.
//
// USAGE:
//   #define LLM_AGENT_IMPLEMENTATION  (in exactly one .cpp)
//   #include "llm_agent.hpp"
//
// Requires: libcurl

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace llm {

struct Tool {
    std::string name;
    std::string description;
    std::vector<std::string> param_names;
    std::vector<std::string> param_descriptions;
    std::function<std::string(std::map<std::string, std::string>)> fn;
};

struct AgentConfig {
    std::string api_key;
    std::string model          = "gpt-4o-mini";
    int         max_iterations = 10;
    bool        verbose        = false;
    std::string system_prompt;
};

struct AgentStep {
    enum class Type { ToolCall, FinalAnswer };
    Type        type = Type::FinalAnswer;
    std::string tool_name;
    std::map<std::string, std::string> tool_args;
    std::string tool_result;
    std::string content;
};

struct AgentResult {
    std::string             answer;
    std::vector<AgentStep>  steps;
    int                     iterations_used        = 0;
    bool                    max_iterations_reached = false;
};

AgentResult run_agent(
    const std::string&       prompt,
    const std::vector<Tool>& tools,
    const AgentConfig&       config
);

} // namespace llm

// ---------------------------------------------------------------------------
// Implementation
// ---------------------------------------------------------------------------

#ifdef LLM_AGENT_IMPLEMENTATION

#include <curl/curl.h>
#include <cstdio>
#include <iostream>
#include <sstream>

namespace llm {
namespace detail {

// ---------------------------------------------------------------------------
// RAII curl
// ---------------------------------------------------------------------------
struct CurlH {
    CURL* h = nullptr;
    CurlH() : h(curl_easy_init()) {}
    ~CurlH() { if (h) curl_easy_cleanup(h); }
    CurlH(const CurlH&) = delete;
    CurlH& operator=(const CurlH&) = delete;
    bool ok() const { return h != nullptr; }
};

struct CurlSl {
    curl_slist* l = nullptr;
    ~CurlSl() { if (l) curl_slist_free_all(l); }
    CurlSl(const CurlSl&) = delete;
    CurlSl& operator=(const CurlSl&) = delete;
    CurlSl() = default;
    void append(const char* s) { l = curl_slist_append(l, s); }
};

static size_t wcb(char* p, size_t s, size_t n, void* ud) {
    static_cast<std::string*>(ud)->append(p, s * n);
    return s * n;
}

static std::string post(const std::string& url, const std::string& body,
                         const std::string& key) {
    CurlH c;
    if (!c.ok()) return {};
    CurlSl h;
    h.append("Content-Type: application/json");
    h.append(("Authorization: Bearer " + key).c_str());
    std::string resp;
    curl_easy_setopt(c.h, CURLOPT_URL,            url.c_str());
    curl_easy_setopt(c.h, CURLOPT_HTTPHEADER,     h.l);
    curl_easy_setopt(c.h, CURLOPT_POSTFIELDS,     body.c_str());
    curl_easy_setopt(c.h, CURLOPT_WRITEFUNCTION,  wcb);
    curl_easy_setopt(c.h, CURLOPT_WRITEDATA,      &resp);
    curl_easy_setopt(c.h, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_perform(c.h);
    return resp;
}

// ---------------------------------------------------------------------------
// Minimal JSON helpers
// ---------------------------------------------------------------------------
static std::string jesc(const std::string& s) {
    std::string o;
    for (unsigned char c : s) {
        switch(c) { case '"': o+="\\\""; break; case '\\': o+="\\\\"; break;
                    case '\n': o+="\\n"; break; case '\r': o+="\\r"; break;
                    case '\t': o+="\\t"; break;
                    default: if(c<0x20){char b[8];snprintf(b,sizeof(b),"\\u%04x",c);o+=b;}
                             else o+=static_cast<char>(c); }
    }
    return o;
}

static std::string jstr(const std::string& j, const std::string& k) {
    std::string pat = "\"" + k + "\"";
    auto p = j.find(pat);
    if (p == std::string::npos) return {};
    p += pat.size();
    while (p < j.size() && (j[p]==' '||j[p]==':')) ++p;
    if (p >= j.size() || j[p] != '"') return {};
    ++p;
    std::string v;
    while (p < j.size() && j[p] != '"') {
        if (j[p]=='\\' && p+1<j.size()) {
            char e=j[++p];
            switch(e){case 'n':v+='\n';break;case 't':v+='\t';break;
                      case '"':v+='"';break;case '\\':v+='\\';break;default:v+=e;}
        } else v+=j[p];
        ++p;
    }
    return v;
}

// Extract finish_reason
static std::string finish_reason(const std::string& j) {
    return jstr(j, "finish_reason");
}

// Extract content of first choice message
static std::string msg_content(const std::string& j) {
    auto p = j.find("\"message\"");
    if (p == std::string::npos) return {};
    return jstr(j.substr(p), "content");
}

// Check if response contains tool_calls
static bool has_tool_calls(const std::string& j) {
    return j.find("\"tool_calls\"") != std::string::npos;
}

// Extract tool call: name and arguments string
static bool extract_tool_call(const std::string& j,
                                std::string& tool_name,
                                std::string& args_json) {
    auto tc = j.find("\"tool_calls\"");
    if (tc == std::string::npos) return false;
    auto fn = j.find("\"function\"", tc);
    if (fn == std::string::npos) return false;
    tool_name = jstr(j.substr(fn), "name");
    args_json = jstr(j.substr(fn), "arguments");
    return !tool_name.empty();
}

// Parse flat JSON object into key->value map (string values only)
static std::map<std::string, std::string> parse_args(const std::string& json) {
    std::map<std::string, std::string> out;
    size_t p = 0;
    while (p < json.size()) {
        auto ks = json.find('"', p);
        if (ks == std::string::npos) break;
        auto ke = json.find('"', ks + 1);
        if (ke == std::string::npos) break;
        std::string key = json.substr(ks + 1, ke - ks - 1);
        p = ke + 1;
        while (p < json.size() && (json[p] == ':' || json[p] == ' ')) ++p;
        std::string val;
        if (p < json.size() && json[p] == '"') {
            ++p;
            while (p < json.size() && json[p] != '"') {
                if (json[p]=='\\' && p+1<json.size()) {
                    char e=json[++p];
                    switch(e){case 'n':val+='\n';break;case '"':val+='"';break;
                              case '\\':val+='\\';break;default:val+=e;}
                } else val+=json[p];
                ++p;
            }
            ++p;
        } else {
            // number or bool
            auto ve = json.find_first_of(",}", p);
            val = (ve != std::string::npos) ? json.substr(p, ve - p) : json.substr(p);
            p = (ve != std::string::npos) ? ve : json.size();
        }
        out[key] = val;
    }
    return out;
}

// ---------------------------------------------------------------------------
// Build request JSON
// ---------------------------------------------------------------------------
static std::string build_request(
    const std::vector<std::pair<std::string,std::string>>& messages, // role, content
    const std::vector<Tool>& tools,
    const AgentConfig& cfg)
{
    std::ostringstream ss;
    ss << "{\"model\":\"" << jesc(cfg.model) << "\",\"messages\":[";
    bool first = true;
    for (const auto& [role, content] : messages) {
        if (!first) ss << ',';
        ss << "{\"role\":\"" << jesc(role) << "\",\"content\":\"" << jesc(content) << "\"}";
        first = false;
    }
    ss << "]";

    if (!tools.empty()) {
        ss << ",\"tools\":[";
        for (size_t ti = 0; ti < tools.size(); ++ti) {
            if (ti) ss << ',';
            const auto& t = tools[ti];
            ss << "{\"type\":\"function\",\"function\":{"
               << "\"name\":\"" << jesc(t.name) << "\","
               << "\"description\":\"" << jesc(t.description) << "\","
               << "\"parameters\":{\"type\":\"object\",\"properties\":{";
            for (size_t pi = 0; pi < t.param_names.size(); ++pi) {
                if (pi) ss << ',';
                std::string desc = pi < t.param_descriptions.size()
                    ? t.param_descriptions[pi] : t.param_names[pi];
                ss << "\"" << jesc(t.param_names[pi]) << "\":"
                   << "{\"type\":\"string\",\"description\":\"" << jesc(desc) << "\"}";
            }
            ss << "},\"required\":[";
            for (size_t pi = 0; pi < t.param_names.size(); ++pi) {
                if (pi) ss << ',';
                ss << '"' << jesc(t.param_names[pi]) << '"';
            }
            ss << "]}}}";
        }
        ss << "],\"tool_choice\":\"auto\"";
    }
    ss << "}";
    return ss.str();
}

} // namespace detail

// ---------------------------------------------------------------------------
// Public: run_agent
// ---------------------------------------------------------------------------

AgentResult run_agent(
    const std::string&       prompt,
    const std::vector<Tool>& tools,
    const AgentConfig&       cfg)
{
    AgentResult result;

    std::vector<std::pair<std::string,std::string>> messages;
    if (!cfg.system_prompt.empty())
        messages.push_back({"system", cfg.system_prompt});
    messages.push_back({"user", prompt});

    for (int iter = 0; iter < cfg.max_iterations; ++iter) {
        result.iterations_used = iter + 1;

        std::string body = detail::build_request(messages, tools, cfg);
        std::string resp = detail::post(
            "https://api.openai.com/v1/chat/completions", body, cfg.api_key);

        if (cfg.verbose)
            std::cerr << "[llm-agent] iter " << iter << " response: " << resp << "\n";

        if (detail::has_tool_calls(resp)) {
            std::string tool_name, args_json;
            if (!detail::extract_tool_call(resp, tool_name, args_json)) break;

            auto args = detail::parse_args(args_json);

            // Find and call the tool
            std::string tool_result = "[tool not found: " + tool_name + "]";
            for (const auto& t : tools) {
                if (t.name == tool_name) { tool_result = t.fn(args); break; }
            }

            AgentStep step;
            step.type        = AgentStep::Type::ToolCall;
            step.tool_name   = tool_name;
            step.tool_args   = args;
            step.tool_result = tool_result;
            result.steps.push_back(step);

            if (cfg.verbose)
                std::cerr << "[llm-agent] tool=" << tool_name
                          << " result=" << tool_result << "\n";

            // Add assistant message + tool result to conversation
            messages.push_back({"assistant",
                "[called " + tool_name + "(" + args_json + ") -> " + tool_result + "]"});
            messages.push_back({"user",
                "Tool result for " + tool_name + ": " + tool_result + ". Continue."});

        } else {
            // Final answer
            std::string answer = detail::msg_content(resp);
            AgentStep step;
            step.type    = AgentStep::Type::FinalAnswer;
            step.content = answer;
            result.steps.push_back(step);
            result.answer = answer;
            return result;
        }
    }

    result.max_iterations_reached = true;
    result.answer = "[max iterations reached]";
    return result;
}

} // namespace llm

#endif // LLM_AGENT_IMPLEMENTATION
