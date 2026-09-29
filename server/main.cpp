#include <iostream>
#include <sightline/config.h>

int main()
{
    sightline::ServerConfig config;
    std::cout << "tick_rate: " << config.tick_rate << std::endl;
    std::cout << "tick_duration_ms: " << sightline::tick_duration_ms(config) << std::endl;
}