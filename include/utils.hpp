#pragma once
#include <iostream>

inline void print_cli() {
    std::cout << "\033[1;36m";   // cyan bold

    std::cout << R"(

██████╗ ██████╗ ███████╗██╗   ██╗███╗   ██╗ ██████╗
██╔══██╗██╔══██╗██╔════╝╚██╗ ██╔╝████╗  ██║██╔════╝
██║  ██║██████╔╝███████╗ ╚████╔╝ ██╔██╗ ██║██║     
██║  ██║██╔══██╗╚════██║  ╚██╔╝  ██║╚██╗██║██║     
██████╔╝██████╔╝███████║   ██║   ██║ ╚████║╚██████╗
╚═════╝ ╚═════╝ ╚══════╝   ╚═╝   ╚═╝  ╚═══╝ ╚═════╝

)" << "\033[0m";

    std::cout << "\033[1mDbSync Key-Value Engine\033[0m\n";
    std::cout << "────────────────────────────────────────────\n";

    std::cout << "🚀 Architecture   : Shared-Nothing Reactor\n";
    std::cout << "⚡ IO Engine      : io_uring (SQPOLL-ready)\n";
    std::cout << "🧠 Sharding       : Consistent Hash Partitioning\n";
    std::cout << "📡 ITC Transport  : Lock-Free Concurrent Queues\n";
    std::cout << "💾 Persistence    : Append-Only File (AOF)\n";
    std::cout << "🧹 Janitor        : Active Expiry Sampling\n";

    std::cout << "────────────────────────────────────────────\n";
    std::cout << "Status: \033[1;32mREADY\033[0m  |  Listening on port "
              << PORT << "\n\n";
}
