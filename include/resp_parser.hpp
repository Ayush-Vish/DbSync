#pragma once

#include <string_view>
#include <vector>
#include <charconv>



struct Command{
    std::string_view type; // used string  view to avoid copying strings , Ex: SET, GET
    std::vector<std::string_view> args; // Ex: SET key value -> args = [key,value]
};



class RespParser {
public:
    static Command parse(std::string_view buffer) {
        // Redis commands start with '*' if they are an array (which most are)
        if (buffer.empty() || buffer[0] != '*') return {"UNKNOWN", {}};

        size_t pos = 0;
        // This helper lambda finds the next line ending (\r\n)
        auto read_line = [&]() -> std::string_view {
            size_t start = pos;
            size_t end = buffer.find("\r\n", pos);
            if (end == std::string_view::npos) return "";
            pos = end + 2;
            return buffer.substr(start, end - start);
        };

        std::string_view header = read_line(); // This is the "*3" line
        int num_args = 0;
        // Extracting the number after the '*'
        std::from_chars(header.data() + 1, header.data() + header.size(), num_args);

        Command cmd;
        for (int i = 0; i < num_args; ++i) {
            read_line(); // Skip the "$3" (length) lines, we don't strictly need them for this simple version
            std::string_view arg = read_line(); // This is the actual word (SET, key, or value)
            if (i == 0) cmd.type = arg;
            else cmd.args.push_back(arg);
        }
        return cmd;
    }
};
