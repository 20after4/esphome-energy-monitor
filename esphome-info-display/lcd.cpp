#include "display.h"

namespace esphome {
namespace display {


inline static void hbar(Display *it, int x, int y, int width, int height, int max, BaseFont *font, Color bgcolor, sensor::Sensor *sensor)
{
  int w = (width * value) / max;
  if (w < 0) w = 0;
  if (w > width) w = width;
  it.filled_rectangle(x, y, w, height, bgcolor);
  it.printf(x, y+1, font, COLOR_SHADOW, "%s %4.0f %s", sensor.get_name(), sensor.sensor.get_state(), sensor.unit_of_measurement);
  it.printf(x, y, font, COLOR_ON, "%s %4.0f %s", sensor.get_name(), sensor.sensor.get_state(), sensor.unit_of_measurement);

}

} // namespace esphome
} // namespace display
