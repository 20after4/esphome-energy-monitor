#include <cstdint>
#include <utility>
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/color.h"
#include "esphome/core/component.h"


namespace esphome {
namespace display {

const Color COLOR_SHADOW(10, 10, 10, 10);

inline static void hbar(Display *it, int x, int y, int width, int height, int max, BaseFont *font, int hue, sensor::Sensor *sensor)
{
    auto value = sensor->get_state();
    int w = (width * value) / max;
    if (w < 0) w = 0;
    if (w > width) w = width;

    unsigned int v = static_cast<unsigned int>(value);
    it->rectangle(x, y, w, height, ESPHSVColor(hue, 200, 50).to_rgb());
    it->filled_rectangle(x+1, y+1, w-2, height-2, ESPHSVColor(hue, 240, 210).to_rgb());
    it->printf(x+2, y+4, font, COLOR_SHADOW, "%-11s %4u%s", sensor->get_name().c_str(), v, sensor->get_unit_of_measurement().c_str());
    it->printf(x+1, y+3, font, COLOR_ON, "%-11s %4u%s", sensor->get_name().c_str(), v, sensor->get_unit_of_measurement().c_str());
}


}
}
