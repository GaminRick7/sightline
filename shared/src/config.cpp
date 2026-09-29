#include <sightline/config.h>

namespace sightline {

double tick_duration_ms(const ServerConfig& config) {
    return 1000.0 / config.tick_rate;
}

}
