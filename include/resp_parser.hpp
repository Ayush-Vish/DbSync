#pragma once

#include <string_view>
#include <vector>
#include <charconv>



struct Command{
    std::string_view type; // used string  view to avoid copying strings , Ex: SET, GET
    std::vector<std::string_view> args; // Ex: SET key value -> args = [key,value]
};

struct ParseResult {
    Command cmd;
    size_t consumed;
};

class RespParser {
public:
    static ParseResult parse(std::string_view buffer) {
        if (buffer.empty() || buffer[0] != '*') return {{"UNKNOWN", {}}, 0};

        size_t pos = 0;
        auto read_line = [&]() -> std::string_view {
            size_t start = pos;
            size_t end = buffer.find("\r\n", pos);
            if (end == std::string_view::npos) return "";
            pos = end + 2;
            return buffer.substr(start, end - start);
        };

        std::string_view header = read_line();
        if (header.empty()) return {{"INCOMPLETE", {}}, 0};
        
        int num_args = 0;
        std::from_chars(header.data() + 1, header.data() + header.size(), num_args);

        Command cmd;
        for (int i = 0; i < num_args; ++i) {
            read_line(); // Skip length
            cmd.args.push_back(read_line());
        }
        
        // The first argument is the command type
        if (!cmd.args.empty()) {
            cmd.type = cmd.args[0];
            cmd.args.erase(cmd.args.begin());
        }

        return {cmd, pos}; // Return the command AND how many bytes we ate
    }
};
