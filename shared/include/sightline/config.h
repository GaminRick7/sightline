#pragma once

namespace sightline {
    
// filled in server's main() from flags later
struct ServerConfig {
    int tick_rate = 128;
};

// L]length of one tick in milliseconds (7.8125 at 128 Hz).
double tick_duration_ms(const ServerConfig& config);

}
