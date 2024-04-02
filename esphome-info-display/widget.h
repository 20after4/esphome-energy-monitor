#include <algorithm>
#include <cstdint>
#include <map>
#include <utility>
#include <typeinfo>
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/image/image.h"
#include "esphome/core/color.h"
#include "esphome/core/component.h"
#include "esphome/components/light/esp_hsv_color.h"
#include "esphome/components/display/display.h"

namespace widget {

using namespace esphome;
using namespace esphome::display;
using namespace esphome::light;

using ::LargeFont;
using ::SmallFont;

const Color COLOR_SHADOW = Color(170, 170, 170);
const Color COLOR_HIGHLIGHT = Color(15, 15, 15);
// BaseFont *LargeFont = nullptr;
// BaseFont *SmallFont = nullptr;


void hbar(Display *it, int x, int y, int width, int height, int max, BaseFont *font, int hue, sensor::Sensor *sensor)
{
    const int offset = width / 2;
    width = width - offset;
    auto value = sensor->get_state();
    if (isnan(value)) return;
    int w = (width * value) / max;
    if (w < 0) w = 0;
    if (w > width) w = width;

    unsigned int v = static_cast<unsigned int>(value);
    it->rectangle(x+offset, y, w, height, ESPHSVColor(hue, 220, 30).to_rgb());
    it->filled_rectangle(x+offset+1, y+1, w-2, height-2, ESPHSVColor(hue, 255, 200).to_rgb());
    it->printf(x, y+1, font, COLOR_SHADOW, "%-8s %4u%s", sensor->get_name().c_str(), v, sensor->get_unit_of_measurement().c_str());
    it->printf(x-1, y, font, COLOR_HIGHLIGHT, "%-8s %4u%s", sensor->get_name().c_str(), v, sensor->get_unit_of_measurement().c_str());
    it->printf(x, y, font, COLOR_ON, "%-8s %4u%s", sensor->get_name().c_str(), v, sensor->get_unit_of_measurement().c_str());
}

void statusbar(Display *it, int y, int width, BaseFont *font, ESPTime time) {
    it->strftime(2, 0, font, TextAlign::TOP_LEFT, "%Y-%m-%d", time);
    it->strftime(width-32, 0, font, TextAlign::TOP_RIGHT, "%H:%M", time);
}

void show_sensor(Display *it, int x, int y, BaseFont *font, Color color, TextAlign align, sensor::Sensor *sensor, bool showlabel = true)
{
    if (!sensor->has_state()) {
        if (!showlabel) return;
        it->printf(x, y, font, color, align, "%-11s %s", sensor->get_name().c_str(), "n/a");
        return;
    }
    auto value = sensor->get_state();
    if (showlabel) {
        it->printf(x, y, font, color, align, "%-11s %1.*f%s", sensor->get_name().c_str(), sensor->get_accuracy_decimals(), value, sensor->get_unit_of_measurement().c_str());
    } else {
        it->printf(x, y, font, color, align, "%1.*f%s", sensor->get_accuracy_decimals(), value, sensor->get_unit_of_measurement().c_str());
    }
}

void show_sensors(Display *it, std::vector<EntityBase*> &sensors, BaseFont *font, Color color = COLOR_ON)
{
    int display_width = it->get_width();
    int display_height = it->get_height();
    int x = 0, y = 36;

    int x_start, y_start, width, height;
    auto text = "1IiGgQpPyY";
    it->get_text_bounds(x, y, text, font, TextAlign::TOP_LEFT, &x_start, &y_start, &width, &height);

    for(EntityBase *sensor : sensors) {
        it->printf(x, y, font, color, TextAlign::TOP_LEFT, "%-11s", sensor->get_name().c_str());
        it->printf(display_width, y, font, color, TextAlign::TOP_RIGHT, "%s", sensor->get_display_value().c_str());
        y += height + 1;
        if (y + height > display_height) {
            ESP_LOGE("widget", "Too many sensors for a single page. y > display_height");
            break;
        }
    }
}


class Label {
  public:
    Label(display::Display *buff, BaseFont *font, Color color, TextAlign align) {
        this->buff_ = buff;
        this->font_ = font;
        this->color_ = color;
        this->align_ = align;
    }
    void set_font(BaseFont *font) { this->font_ = font; };
    void set_color(Color color) { this->color_ = color; };
    void set_align(TextAlign align) { this->align_ = align; };
    void set_pos(int x, int y) { this->x_ = x; this->y_ = y; };
    void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; };
    void set_sensor(text_sensor::TextSensor *text_sensor) { this->text_sensor = text_sensor; };

    void draw(int x, int y, Color color) {
        sensor::Sensor *sensor = this->sensor_;
        auto buff = this->buff_;
        if (!sensor && this->text_sensor && this->text_sensor->has_state()) {
            buff->printf(this->x_, this->y_, this->font_, this->color_, this->align_,
                "%-20s %s",
                this->text_sensor->get_state().c_str(),
                "n/a"
            );
            return;
        }
        if (!sensor->has_state()) {
            buff->printf(this->x_, this->y_, this->font_, this->color_, this->align_,
                "%-11s %s",
                sensor_->get_name().c_str(),
                "n/a"
            );
            return;
        }
        buff->printf(this->x_, this->y_, this->font_, this->color_, this->align_,
            "%-11s %1.*f%s",
            sensor_->get_name().c_str(),
            sensor->get_accuracy_decimals(),
            sensor->get_state(),
            sensor->get_unit_of_measurement().c_str()
        );

    }

    void draw() {
        this->draw(this->x_, this->y_, this->color_);
    }

    void draw_shadow() {
        this->draw(this->x_ + 1, this->y_ + 1, COLOR_SHADOW);
        this->draw(this->x_, this->y_, this->color_);
    }

  protected:
    display::Display *buff_{nullptr};
    BaseFont *font_{nullptr};
    Color color_{COLOR_ON};
    TextAlign align_{ TextAlign::TOP_LEFT };
    int x_{0};
    int y_{0};
    sensor::Sensor *sensor_{nullptr};
    text_sensor::TextSensor *text_sensor{nullptr};
};



void TimeWidget(display::Display *buff)
{
  auto time = id(sntp_time).now();

  if (time.is_valid()) {
    buff->strftime(0, 0, LargeFont, TextAlign::TOP_LEFT, "%Y-%m-%d", time);
    buff->strftime(312, 0, LargeFont, TextAlign::TOP_RIGHT, "%H:%M", time);
    auto lineColor = Color(250, 250, 0);
    buff->horizontal_line(0, 31, 320, lineColor.fade_to_black(200));
    buff->horizontal_line(0, 32, 318, lineColor.fade_to_black(150));
    buff->horizontal_line(0, 33, 316, lineColor.fade_to_black(100));
    buff->horizontal_line(0, 34, 314, lineColor);
  }

}

class WeatherWidget {
public:
  WeatherWidget(display::Display *buff): buff_(buff) {};

  void draw(text_sensor::TextSensor *weather_condition, sensor::Sensor *nws_temp, sensor::Sensor *outside_temp, sensor::Sensor *humidity, sensor::Sensor *temperature, sensor::Sensor *temperature2, sensor::Sensor *pressure) {
      static std::map<std::string, BaseImage*> weather_state {
          { "sunny", ::sunny },
          { "clear-night", ::clearnight },
          { "cloudy", ::cloudy },
          { "partlycloudy", ::partlycloudy },
          { "partly-cloudy-night", ::partlycloudynight },
          { "rainy", ::rain },
          { "pouring", ::rain },
          { "hail", ::hail },
          { "snowy", ::snow },
          { "windy", ::wind },
          { "fog", ::fog },
          { "lightning", ::lightning }
      };
      auto time = id(sntp_time).now();
      auto temp_label = "Outside";
      auto outside_t = id(outside_temp);
      Display *buff = this->buff_;

      if (isnan(outside_temp->get_state())) {
        outside_t = id(nws_temp);
        temp_label = "Forecast";

      }
      if (!outside_t.has_state()) {
        return;
      }

      float min_temp = 30; // Blue color
      float max_temp = 90.0; // Red color
      float current_temp = id(outside_t).get_state();
      if (current_temp < min_temp) current_temp = min_temp;
      if (current_temp > max_temp) current_temp = max_temp;

      float ratio = (current_temp - min_temp) / (max_temp - min_temp);
      float hue = 220 - (ratio*140);
      auto shadow_color = ESPHSVColor(hue, 150, 50).to_rgb();
      auto highlight_color = ESPHSVColor(hue, 200, 250).to_rgb();
      auto textcolor = ESPHSVColor(hue, 250, 100).to_rgb();
      buff->printf(10, 66, SmallFont, Color(200, 200, 200), TextAlign::TOP_LEFT, temp_label);
      buff->printf(200, 65, LargeFont, highlight_color, TextAlign::TOP_RIGHT, "%4.0f°F", outside_t.state);
      buff->printf(202, 67, LargeFont, shadow_color, TextAlign::TOP_RIGHT, "%4.0f°F", outside_t.state);
      buff->printf(201, 66, LargeFont, textcolor, TextAlign::TOP_RIGHT, "%4.0f°F", outside_t.state);

      buff->line(10, 105, 310, 105, Color(200, 200, 200));
      buff->printf(10, 110, SmallFont, Color(200, 200, 200), TextAlign::TOP_LEFT, "Inside");
      buff->printf(10, 150, SmallFont, Color(200, 200, 200), TextAlign::TOP_LEFT, "...");

      widget::show_sensor(buff, 210, 110, LargeFont, Color(200, 200, 200), TextAlign::TOP_RIGHT, temperature, false);
      widget::show_sensor(buff, 312, 110, SmallFont, Color(200, 200, 200), TextAlign::TOP_RIGHT, humidity, false);
      widget::show_sensor(buff, 210, 150, LargeFont, Color(200, 200, 200), TextAlign::TOP_RIGHT, temperature2, false);
      widget::show_sensor(buff, 312, 150, SmallFont, Color(200, 200, 200), TextAlign::TOP_RIGHT, pressure, false);

      if (id(weather_condition).has_state()) {
        auto condition_state = weather_condition->state;
        if  (time.hour > 18 || time.hour < 6) {
          if (condition_state == "partlycloudy") {
              condition_state = "partly-cloudy-night";
          }
        }
        auto weather_icon = weather_state.find(condition_state);
        if (weather_icon != weather_state.end()) {
            buff->image(310, 40, weather_icon->second,  ImageAlign::TOP_RIGHT);
        } else {
            auto condition_widget = new Label(buff, LargeFont, textcolor, TextAlign::TOP_RIGHT);
            condition_widget->set_sensor(weather_condition);
            condition_widget->set_pos(310, 40);
            condition_widget->draw_shadow();
            // buff->printf(310, 45, id(LargeFont), TextAlign::TOP_RIGHT, id(weather_condition).state.c_str());
        }
      }
    }

protected:
  display::Display *buff_{nullptr};
};

} // namespace widget
