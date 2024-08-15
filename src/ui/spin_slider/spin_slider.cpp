/***
 * Copyright (c) 2024 Yuri Sarudiansky
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef SPIN_SLIDER_DISABLED

#include "spin_slider.h"
#include "../custom_theme.h"
#include "../../internal.h"

#include <godot_cpp/classes/expression.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/text_server_manager.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


// Icons converted taken from Godot source (scene/theme/icons), converted to PNG then into Base64. This last step was
// done by Godot itself.
const static char* ICON_GRABBER_SLIDER = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgGlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kb9LQlEUxz9qYZShUENDg4Q1WZSB1NJglAXVoAZZLfr8Ffjj8Z4R0hq0CgVRS7+G+gtqDZqDoCiCaG4uaql4naeCEnku557P/d57DveeC9ZIVsnpLcOQyxe1UDDgXoouu+2vWHHhpIP2mKKr8+HpCE3t8wGLGe8GzVrNz/1rHYmkroClTXhCUbWi8Izw3EZRNXlXuFvJxBLC58JeTS4ofG/q8Sq/mpyu8rfJWiQ0CVaXsDvdwPEGVjJaTlhejieXXVdq9zFf4kjmF8MS+8R70QkRJICbWaaYxM8I4zL7GcTHkKxokj9cyV+gILmKzColNNZIk6GIV9R1qZ6UmBI9KSNLyez/377qqVFftbojAK0vhvHeD/Yd+CkbxtexYfycgO0ZrvL1/MIRjH2IXq5rnkNwbsHFdV2L78HlNvQ8qTEtVpFs4tZUCt7OoDMKXbfQvlLtWW2f00eIbMpX3cD+AQzIeefqLw9pZ765GX60AAAACXBIWXMAAAsTAAALEwEAmpwYAAAAtElEQVQ4jaWTsQ2DMBBFX67MAMkIliizCRnFDSuEIqvAKOkseQQyQEorBWdkAUYYfnnyf3dn/YOTuswLIYQr8AQqwGjZAw7oROSXBYQQHkAD3DINB6AVkc8CoOb3zslthEgydrPTDNCoZwQw7pwbe013oE4BVYE5qkoBZuNhTiYFHFYE+ANenwLcAYBLAR3wLTAPQD8BNJ6vAkAbIz19oibLKn2rs12NcpQmrGb9mPr5MZ3WH3xbL3iEmdQFAAAAAElFTkSuQmCC";
const static char* ICON_GRABBER_SL_HL = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgGlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kb9LQlEUxz9qYZShUENDg4Q1WZSB1NJglAXVoAZZLfr8Ffjj8Z4R0hq0CgVRS7+G+gtqDZqDoCiCaG4uaql4naeCEnku557P/d57DveeC9ZIVsnpLcOQyxe1UDDgXoouu+2vWHHhpIP2mKKr8+HpCE3t8wGLGe8GzVrNz/1rHYmkroClTXhCUbWi8Izw3EZRNXlXuFvJxBLC58JeTS4ofG/q8Sq/mpyu8rfJWiQ0CVaXsDvdwPEGVjJaTlhejieXXVdq9zFf4kjmF8MS+8R70QkRJICbWaaYxM8I4zL7GcTHkKxokj9cyV+gILmKzColNNZIk6GIV9R1qZ6UmBI9KSNLyez/377qqVFftbojAK0vhvHeD/Yd+CkbxtexYfycgO0ZrvL1/MIRjH2IXq5rnkNwbsHFdV2L78HlNvQ8qTEtVpFs4tZUCt7OoDMKXbfQvlLtWW2f00eIbMpX3cD+AQzIeefqLw9pZ765GX60AAAACXBIWXMAAAsTAAALEwEAmpwYAAAAmklEQVQ4jaWTsQ3CMBBFH+7xCLCEC9dmnWQZWCqFhwgz0H9oEmGs2FGOL11z8n8+2ffhT53qhiQPjEAEwtLOwATcnXOvJk3STdJT0rtRs6TUM7eMdaXa7Hdu3prEA7iFMQKXA293BYYSEA+YV8USEDoHWwolwKwVkA3eXAImA+DrMX7j+QcnKZkXqYLMR1a5FaaB7TA9umGy6APc/yacq1rJiAAAAABJRU5ErkJggg==";
const static char* ICON_UPDOWN = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GEaPxKykWk4bVjBg1sbGYiaGwGKMMNjPP/FDz4/XeTJKtsp2ixMavBX8BW2WtFJGS9ayJDdNz3owayZzbuedzv/ee073ngiWYVFJ63RCk0lkt4PfaF0NL9oYCFtpop4uesKKrs/OTQaraxwM1ZrxzmbWqn/vXmlejugI1jcLjiqplhaeEZ9azqsm7wp1KIrwqfC7s1OSCwvemHilzweR4mb9M1oIBH1hahe3xXxz5xUpCSwnLy3Gkkjnl5z7mS6zR9MK8xD7xXnQC+PFiZ5oJfHgYZkxmDy7cDMqKKvlDpfw5MpKryKyygcYacRJkcYqak+pRiTHRozKSbJj9/9tXPTbiLle3eqH+xTDe+qFhB4p5w/g8NoziCdQ+w1W6kp85gtF30fMVzXEIti24uK5okT243IbuJzWshUtSrbglFoPXM2gJQcctNC2Xe/azz+kjBDflq25g/wAG5Lxt5RtVsWfeW5+c5AAAAAlwSFlzAAALEwAACxMBAJqcGAAAANdJREFUOI21kzFqAlEQhr8nCDZpsyfYOh4gOUDAA9grbGcOEG3MHbzOXiGdd7CwCAHtvhQZZfN0cVfIDwOPmfl/+GfmwX9CnaiTe4hJnavbiLmaupKH6lKt1X1Erb6rw7w/ZeQH4AN4Ag5AEaUdMAI+gVVK6fvEGWSCz0HeA2/AV8QicmPg5ZaFqVrEu1breBfqtMMU/oidBdqQW+iNC4GmhSx/1cIga3oFKmCjlo18CWyAKnrOuLbGNb/TPgKPUWpd4wX6HlKbSFJnjVOedT7lTOi+z9QHP7ctu1Ji9OZvAAAAAElFTkSuQmCC";


void SpinSlider::apply_internal_styles()
{
   m_value_input->begin_bulk_theme_override();
   m_value_input->add_theme_color_override("font_color", m_theme_cache.font_color);
   m_value_input->add_theme_color_override("font_selected_color", m_theme_cache.font_selected_color);
   m_value_input->add_theme_color_override("font_uneditable_color", m_theme_cache.font_read_only_color);
   m_value_input->add_theme_color_override("font_outline_color", m_theme_cache.font_outline_color);
   m_value_input->add_theme_color_override("caret_color", m_theme_cache.caret_color);
   m_value_input->add_theme_color_override("selection_color", m_theme_cache.selection_color);
   m_value_input->add_theme_font_override("font", m_theme_cache.font);
   m_value_input->add_theme_font_size_override("font_size", m_theme_cache.font_size);
   m_value_input->end_bulk_theme_override();
}


void SpinSlider::assign_value(double val, bool signal)
{
   if (m_step > 0.0)
   {
      val = godot::Math::round(val / m_step) * m_step;
   }

   if (m_rounded_values)
   {
      val = godot::Math::round(val);
   }

   if (m_use_max_value && val > m_max_value)
   {
      val = m_max_value;
   }

   if (m_use_min_value && val < m_min_value)
   {
      val = m_min_value;
   }

   if (m_value == val) { return; }

   m_value = val;
   
   if (signal)
   {
      emit_signal("value_changed", m_value);
   }

   queue_redraw();
   m_slider->queue_redraw();
}


void SpinSlider::update_text()
{
   
   godot::String strval = godot::String::num(m_value, Internals::range_step_decimals(m_step));
   if (is_localizing_numeral_system())
   {
      // In the Core there is a Define named TS that expands to TextServerManager::get_singleton()->get_primary_interface()
      strval = godot::TextServerManager::get_singleton()->get_primary_interface()->format_number(strval);
   }

   m_last_valid = strval;

   if (!m_value_input->has_focus())
   {
      if (!m_prefix.is_empty())
      {
         strval = m_prefix + " " + strval;
      }
      if (!m_suffix.is_empty())
      {
         strval += " " + m_suffix;
      }
   }

   m_value_input->set_text(strval);
}


void SpinSlider::check_entered_text(const godot::String& new_text)
{
   if (m_allow_expression)
   {
      // The text is potentially a math expression
      godot::Ref<godot::Expression> expr;
      expr.instantiate();

      // Some languages use "," as decimal separator. Convert that to "." to perform internal tasks
      godot::String txt = new_text.replace(",", ".");
      txt = txt.replace(";", ",");
      txt = godot::TextServerManager::get_singleton()->get_primary_interface()->format_number(txt);

      // This is most likely not necessary (when focus enters that happens anyway), however entered text is somewhat
      // unpredictable, so strip away prefix and suffix
      txt = txt.trim_prefix(m_prefix + " ").trim_suffix(" " + m_suffix);

      if (expr->parse(txt) != godot::Error::OK)
      {
         // Attempt again without converting "," into "."
         txt = godot::TextServerManager::get_singleton()->get_primary_interface()->format_number(new_text);
         txt = txt.trim_prefix(m_prefix + " ").trim_suffix(" " + m_suffix);

         if (expr->parse(txt) != godot::Error::OK)
         {
            return;
         }
      }

      godot::Variant val = expr->execute(godot::Array(), nullptr, false, true);
      if (val.get_type() != godot::Variant::NIL)
      {
         assign_value(val, true);
      }

      update_text();
   }
   else
   {
      // When "allow expression" is false the text should already be valid so there is no need to "update" it.
      // Nevertheless, the prefix/suffix (which is built through update_text()) will be added once focus is removed
      // from the internal LineEdit
      assign_value(new_text.to_float(), true);
   }
}


void SpinSlider::adjust_layout()
{
   const int margin_left = m_theme_cache.normal->get_margin(godot::Side::SIDE_LEFT);
   const int margin_top = m_theme_cache.normal->get_margin(godot::Side::SIDE_TOP);
   const int margin_right = m_theme_cache.normal->get_margin(godot::Side::SIDE_RIGHT);

   const int upd_width = m_theme_cache.up_down->get_width();

   m_value_input->set_offset(godot::Side::SIDE_LEFT, margin_left);
   m_value_input->set_offset(godot::Side::SIDE_TOP, margin_top);
   m_value_input->set_offset(godot::Side::SIDE_RIGHT, use_spin() ? -(upd_width + margin_right) : -margin_right);

   if (use_slider())
   {
      int rm = margin_right;
      if (use_spin())
      {
         rm += upd_width;
      }

      m_slider->set_visible(true);
      m_slider->set_offset(godot::Side::SIDE_LEFT, margin_left);
      m_slider->set_offset(godot::Side::SIDE_TOP, -m_theme_cache.slider_height);
      m_slider->set_offset(godot::Side::SIDE_RIGHT, -rm);

      m_value_input->set_offset(godot::Side::SIDE_BOTTOM, -m_theme_cache.slider_height);
   }
   else
   {
      m_slider->set_visible(false);
      m_value_input->set_offset(godot::Side::SIDE_BOTTOM, -m_theme_cache.normal->get_margin(godot::Side::SIDE_BOTTOM));
   }
}


void SpinSlider::draw_ctrl()
{
   const int width = use_spin() ? get_size().x - m_theme_cache.up_down->get_width() : get_size().x;
   const int height = get_size().y;
   const godot::Rect2 rec(godot::Vector2(), godot::Vector2(width, height));

   draw_style_box(m_value_input->is_editable() ? m_theme_cache.normal : m_theme_cache.read_only, rec);

   if (m_value_input->has_focus())
   {
      draw_style_box( m_theme_cache.focus, rec);
   }

   if (use_spin())
   {
      const int x = get_size().x - m_theme_cache.up_down->get_width();
      const int y = (height - m_theme_cache.up_down->get_height()) * 0.5;

      draw_texture(m_theme_cache.up_down, godot::Vector2(x, y));
   }

   // This is debug code - so yes, creating the stylebox every time it's needed.
   //godot::Ref<godot::StyleBoxFlat> tmp_style = Internals::make_flat_stylebox(godot::Color(), 4, 4, 4, 4, 3, false, 1);
   //tmp_style->set_border_color(godot::Color(0.1, 0.1, 0.9, 1.0));
   //draw_style_box(tmp_style, godot::Rect2(m_value_input->get_position(), m_value_input->get_size()));
}


void SpinSlider::release_mouse()
{
   if (m_drag.dragging)
   {
      m_drag.dragging = false;
      godot::Input::get_singleton()->set_mouse_mode(godot::Input::MOUSE_MODE_HIDDEN);
      warp_mouse(m_drag.capture_pos);
      godot::Input::get_singleton()->set_mouse_mode(godot::Input::MOUSE_MODE_VISIBLE);
   }
}



void SpinSlider::on_text_changed(const godot::String& new_text)
{
   if (m_allow_expression)
   {
      if (m_update_on_t_change)
      {
         const int cpos = m_value_input->get_caret_column();
         check_entered_text(new_text);
         m_value_input->set_caret_column(cpos);
      }
   }
   else
   {
      // Attempt to filter out character that would make the input invalid since math expressions are not allowed
      // NOTE: Should "," be allowed? There are some languages that use "," as decimal separator instead of "."
      if (new_text.is_empty() || new_text.is_valid_float() || new_text.is_valid_int() || new_text == "." || new_text == "-")
      {
         m_last_valid = new_text;

         if (m_update_on_t_change)
         {
            const int cpos = m_value_input->get_caret_column();
            check_entered_text(m_last_valid);
            m_value_input->set_caret_column(cpos);
         }
      }
      else
      {
         m_value_input->set_text(m_last_valid);
         m_value_input->set_caret_column(m_last_valid.length());
      }
   }
}

void SpinSlider::on_text_entered(const godot::String& new_text)
{
   check_entered_text(new_text);
   emit_signal("value_entered", m_value);
}

void SpinSlider::on_input_focus_enter()
{
   // 
   if (!m_value_input->is_editable())
   {
      // Currently in read only mode. Just bail
      return;
   }

   queue_redraw();
   const int pos = m_value_input->get_caret_column();
   update_text();
   m_value_input->set_caret_column(pos);

   if (m_value_input->is_select_all_on_focus() && !godot::Input::get_singleton()->is_mouse_button_pressed(godot::MouseButton::MOUSE_BUTTON_LEFT))
   {
      m_value_input->select_all();
   }
}

void SpinSlider::on_input_focus_exit()
{
   queue_redraw();
   const godot::Viewport* vp = get_viewport();
   if (!vp || vp->gui_get_focus_owner() == m_value_input)
   {
      // Focus exit was caused by clicking the arrows
      return;
   }

   if (m_value_input->is_menu_visible())
   {
      // Focus was removed because of context menu so bail
      return;
   }

   if (godot::Input::get_singleton()->is_action_pressed("ui_cancel"))
   {
      // The signal has happened because of canceling
      update_text();
      return;
   }

   check_entered_text(m_value_input->get_text());
   update_text();
}


void SpinSlider::on_draw_slider()
{
   const bool ronly = get_read_only();

   godot::Color fc = m_theme_cache.font_color;
   
   m_slider->draw_rect(godot::Rect2(godot::Vector2(), m_slider->get_size()), ronly ? m_theme_cache.slider_read_only_color : m_theme_cache.slider_color);

   const int ix = (get_as_ratio() * m_slider->get_size().x) - 2;

   if (!ronly && (m_mouse_over || m_mouse_over_slider))
   {
      // If here the grabber should be shown
      godot::Ref<godot::Texture2D> grabber = m_mouse_over_slider ? m_theme_cache.grabber_highlight : m_theme_cache.grabber;

      const int gx = (ix + 2) - (grabber->get_width() * 0.5);
      const int gy = (m_slider->get_size().y - grabber->get_height()) * 0.5;

      m_slider->draw_texture(grabber, godot::Vector2(gx, gy), godot::Color(1.0, 1.0, 1.0, 1.0));
   }
   else
   {
      // Just show a small square to depict the current value's ratio.
      m_slider->draw_rect(godot::Rect2(godot::Vector2(ix, 0), godot::Vector2(4, m_slider->get_size().y)), m_theme_cache.slider_ratio);
   }
}

void SpinSlider::on_slider_mouse_entered()
{
   m_mouse_over_slider = true;
   m_slider->queue_redraw();
}

void SpinSlider::on_slider_mouse_exited()
{
   m_mouse_over_slider = false;
   m_slider->queue_redraw();
}

void SpinSlider::on_slider_gui_input(const godot::Ref<godot::InputEvent>& event)
{
   bool calc_pos = false;
   godot::Vector2 mpos;

   godot::Ref<godot::InputEventMouseButton> mb = event;
   if (mb.is_valid())
   {
      if (mb->get_button_index() == godot::MouseButton::MOUSE_BUTTON_LEFT)
      {
         if (mb->is_pressed())
         {
            m_value_input->grab_focus();
            m_grabbing_grabber = true;
            calc_pos = true;
            mpos = mb->get_position();
         }
         else
         {
            m_grabbing_grabber = false;
         }
      }
   }

   godot::Ref<godot::InputEventMouseMotion> mm = event;
   if (mm.is_valid())
   {
      if (m_grabbing_grabber)
      {
         calc_pos = true;
         mpos = mm->get_position();
      }
   }

   if (calc_pos)
   {
      const double r = godot::Math::clamp<double>(mpos.x / m_slider->get_size().x, 0.0, 1.0);
      set_as_ratio(r);
      // Set set as ratio does not emit a signal, but one is required.
      emit_signal("value_changed", m_value);
   }
}


void SpinSlider::on_range_click_timeout()
{
   if (!m_drag.dragging && godot::Input::get_singleton()->is_mouse_button_pressed(godot::MouseButton::MOUSE_BUTTON_LEFT))
   {
      const bool is_up = get_local_mouse_position().y < (get_size().y * 0.5);
      
      assign_value(m_value + (is_up ? m_step : -m_step), true);
      update_text();

      if (m_timer->is_one_shot())
      {
         m_timer->set_wait_time(0.075);
         m_timer->set_one_shot(false);
         m_timer->start();
      }
   }
   else
   {
      m_timer->stop();
   }
}


void SpinSlider::_get_property_list(godot::List<godot::PropertyInfo>* list) const
{
   // Append the custom theme entries
   CustomControlThemeDB::get_singleton()->fill_property_list("ExpandablePanel", list, this);
}


void SpinSlider::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         // As per recommendation for GDExtensions, connecting event handlers within this notification rather than constructor
         m_value_input->connect("text_changed", callable_mp(this, &SpinSlider::on_text_changed), CONNECT_DEFERRED);
         m_value_input->connect("text_submitted", callable_mp(this, &SpinSlider::on_text_entered), CONNECT_DEFERRED);
         m_value_input->connect("focus_entered", callable_mp(this, &SpinSlider::on_input_focus_enter), CONNECT_DEFERRED);
         m_value_input->connect("focus_exited", callable_mp(this, &SpinSlider::on_input_focus_exit), CONNECT_DEFERRED);

         m_slider->connect("draw", callable_mp(this, &SpinSlider::on_draw_slider));
         m_slider->connect("mouse_entered", callable_mp(this, &SpinSlider::on_slider_mouse_entered));
         m_slider->connect("mouse_exited", callable_mp(this, &SpinSlider::on_slider_mouse_exited));
         m_slider->connect("gui_input", callable_mp(this, &SpinSlider::on_slider_gui_input));

         m_timer->connect("timeout", callable_mp(this, &SpinSlider::on_range_click_timeout));

         CustomControlThemeDB::get_singleton()->update_control(this);
         apply_internal_styles();
      } break;

      case NOTIFICATION_THEME_CHANGED:
      {
         CustomControlThemeDB::get_singleton()->update_control(this);
         apply_internal_styles();
         adjust_layout();
         queue_redraw();
         if (m_slider->is_visible())
         {
            m_slider->queue_redraw();
         }
         update_minimum_size();
      } break;

      case NOTIFICATION_DRAW:
      {
         //update_text();
         draw_ctrl();
      } break;

      case NOTIFICATION_ENTER_TREE:
      {
         adjust_layout();
      } break;

      case NOTIFICATION_MOUSE_ENTER:
      {
         m_mouse_over = true;
         m_slider->queue_redraw();
      } break;

      case NOTIFICATION_MOUSE_EXIT:
      {
         m_mouse_over = false;
         m_slider->queue_redraw();
      } break;

      /*case NOTIFICATION_FOCUS_ENTER:
      {
         m_value_input->call_deferred("grab_focus");
      } break;*/
   }
}


void SpinSlider::_validate_property(godot::PropertyInfo& property) const
{
   if (property.name == godot::StringName("range_min_value") && !m_use_min_value)
   {
      property.usage |= godot::PROPERTY_USAGE_READ_ONLY;
   }
   else if (property.name == godot::StringName("range_max_value") && !m_use_max_value)
   {
      property.usage |= godot::PROPERTY_USAGE_READ_ONLY;
   }
}


void SpinSlider::_bind_methods()
{
   using namespace godot;

   /// Register functions
   ClassDB::bind_method(D_METHOD("set_as_ratio", "ratio"), &SpinSlider::set_as_ratio);
   ClassDB::bind_method(D_METHOD("get_as_ratio"), &SpinSlider::get_as_ratio);
   ClassDB::bind_method(D_METHOD("get_line_edit"), &SpinSlider::get_line_edit);
   //ClassDB::bind_method(D_METHOD("unfocus"), &SpinSlider::unfocus);
   ClassDB::bind_method(D_METHOD("set_alignment", "align"), &SpinSlider::set_alignment);
   ClassDB::bind_method(D_METHOD("get_alignment"), &SpinSlider::get_alignment);
   ClassDB::bind_method(D_METHOD("set_allow_math_expression", "value"), &SpinSlider::set_allow_expression);
   ClassDB::bind_method(D_METHOD("get_allow_math_expression"), &SpinSlider::get_allow_expression);
   ClassDB::bind_method(D_METHOD("set_read_only", "value"), &SpinSlider::set_read_only);
   ClassDB::bind_method(D_METHOD("get_read_only"), &SpinSlider::get_read_only);
   ClassDB::bind_method(D_METHOD("set_update_on_text_changed", "value"), &SpinSlider::set_update_on_text_changed);
   ClassDB::bind_method(D_METHOD("get_update_on_text_changed"), &SpinSlider::get_update_on_text_changed);
   ClassDB::bind_method(D_METHOD("set_prefix", "value"), &SpinSlider::set_prefix);
   ClassDB::bind_method(D_METHOD("get_prefix"), &SpinSlider::get_prefix);
   ClassDB::bind_method(D_METHOD("set_suffix", "value"), &SpinSlider::set_suffix);
   ClassDB::bind_method(D_METHOD("get_suffix"), &SpinSlider::get_suffix);
   ClassDB::bind_method(D_METHOD("set_step", "value"), &SpinSlider::set_step);
   ClassDB::bind_method(D_METHOD("get_step"), &SpinSlider::get_step);
   ClassDB::bind_method(D_METHOD("set_value", "value"), &SpinSlider::set_value);
   ClassDB::bind_method(D_METHOD("get_value"), &SpinSlider::get_value);
   ClassDB::bind_method(D_METHOD("set_rounded_values", "value"), &SpinSlider::set_rounded_values);
   ClassDB::bind_method(D_METHOD("get_rounded_values"), &SpinSlider::get_rounded_values);
   ClassDB::bind_method(D_METHOD("set_select_all_on_focus", "value"), &SpinSlider::set_select_all_on_focus);
   ClassDB::bind_method(D_METHOD("get_select_all_on_focus"), &SpinSlider::get_select_all_on_focus);
   ClassDB::bind_method(D_METHOD("set_no_spin_with_slider", "value"), &SpinSlider::set_no_spin_with_slider);
   ClassDB::bind_method(D_METHOD("get_no_spin_with_slider"), &SpinSlider::get_no_spin_with_slider);
   ClassDB::bind_method(D_METHOD("set_assign_minmax_on_right_click", "value"), &SpinSlider::set_assign_on_right_click);
   ClassDB::bind_method(D_METHOD("get_assign_minmax_on_right_click"), &SpinSlider::get_assign_on_right_click);
   ClassDB::bind_method(D_METHOD("set_use_min_value", "value"), &SpinSlider::set_use_min_value);
   ClassDB::bind_method(D_METHOD("get_use_min_value"), &SpinSlider::get_use_min_value);
   ClassDB::bind_method(D_METHOD("set_min_value", "value"), &SpinSlider::set_min_value);
   ClassDB::bind_method(D_METHOD("get_min_value"), &SpinSlider::get_min_value);
   ClassDB::bind_method(D_METHOD("set_use_max_value", "value"), &SpinSlider::set_use_max_value);
   ClassDB::bind_method(D_METHOD("get_use_max_value"), &SpinSlider::get_use_max_value);
   ClassDB::bind_method(D_METHOD("set_max_value", "value"), &SpinSlider::set_max_value);
   ClassDB::bind_method(D_METHOD("get_max_value"), &SpinSlider::get_max_value);

   /// Register variables
   ADD_PROPERTY(PropertyInfo(Variant::INT, "alignment", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill"), "set_alignment", "get_alignment");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_math_expression"), "set_allow_math_expression", "get_allow_math_expression");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "read_only"), "set_read_only", "get_read_only");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "update_on_text_changed"), "set_update_on_text_changed", "get_update_on_text_changed");
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "prefix"), "set_prefix", "get_prefix");
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "suffix"), "set_suffix", "get_suffix");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "step"), "set_step", "get_step");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "rounded_values"), "set_rounded_values", "get_rounded_values");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "select_all_on_focus"), "set_select_all_on_focus", "get_select_all_on_focus");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "no_spin_with_slider"), "set_no_spin_with_slider", "get_no_spin_with_slider");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "assign_minmax_on_right_click"), "set_assign_minmax_on_right_click", "get_assign_minmax_on_right_click");

   ADD_GROUP("Range", "range_");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "range_use_min_value"), "set_use_min_value", "get_use_min_value");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range_min_value"), "set_min_value", "get_min_value");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "range_use_max_value"), "set_use_max_value", "get_use_max_value");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range_max_value"), "set_max_value", "get_max_value");

   ADD_GROUP("", "");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "value"), "set_value", "get_value");

   /// Register signals
   ADD_SIGNAL(MethodInfo("value_changed", PropertyInfo(Variant::FLOAT, "value")));
   ADD_SIGNAL(MethodInfo("value_entered", PropertyInfo(Variant::FLOAT, "value")));


   /// Register custom theme items
   Ref<StyleBoxFlat> le_normal = Internals::make_flat_stylebox(godot::Color(0.1, 0.1, 0.1, 0.6));
   le_normal->set_border_width(godot::Side::SIDE_BOTTOM, 2);
   le_normal->set_border_color(godot::Color(0.0, 0.0, 0.0, 0.6));

   Ref<StyleBoxFlat> le_ronly = Internals::make_flat_stylebox(godot::Color(0.1, 0.1, 0.1, 0.3));
   le_ronly->set_border_width(godot::Side::SIDE_BOTTOM, 2);
   le_ronly->set_border_color(godot::Color(0.0, 0.0, 0.0, 0.3));

   Ref<StyleBoxFlat> le_focus = Internals::make_flat_stylebox(Color(1.0, 1.0, 1.0, 0.75), 4, 4, 4, 4, 3, false, 2);
   le_focus->set_border_color(Color(1.0, 1.0, 1.0, 0.75));
   le_focus->set_expand_margin_all(2.0);

   godot::Ref<godot::Texture2D> gslider = Internals::texture_from_base64(ICON_GRABBER_SLIDER);
   godot::Ref<godot::Texture2D> gslider_hl = Internals::texture_from_base64(ICON_GRABBER_SL_HL);
   godot::Ref<godot::Texture2D> ico_updown = Internals::texture_from_base64(ICON_UPDOWN);

   godot::Color fcolor(0.875, 0.875, 0.875, 1.0);
   godot::Color fselected_color(1.0, 1.0, 1.0, 1.0);
   godot::Color fread_only_color(0.875, 0.875, 0.875, 0.5);
   godot::Color foutline_color(1.0, 1.0, 1.0, 1.0);
   godot::Color cursor_color(0.95, 0.95, 0.95, 1.0);
   godot::Color selected_color(0.5, 0.5, 0.5, 1.0);
   godot::Color sl_color(0.5, 0.5, 0.5, 0.6);
   godot::Color sl_ratio_color(0.875, 0.875, 0.875, 0.9);
   godot::Color sl_ro_color(0.5, 0.5, 0.5, 0.3);

   // From what I have seen in the Core source code, creating a Font ref results in the default font being used
   godot::Ref<godot::Font> fnt;

   const int s_height = 4;
   const int32_t font_size = -1;


   REGISTER_CUSTOM_STYLE(normal, SpinSlider, le_normal, m_theme_cache);
   REGISTER_CUSTOM_STYLE(read_only, SpinSlider, le_ronly, m_theme_cache);
   REGISTER_CUSTOM_STYLE(focus, SpinSlider, le_focus, m_theme_cache);
   REGISTER_CUSTOM_ICON(grabber, SpinSlider, gslider, m_theme_cache);
   REGISTER_CUSTOM_ICON(grabber_highlight, SpinSlider, gslider_hl, m_theme_cache);
   REGISTER_CUSTOM_ICON(up_down, SpinSlider, ico_updown, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_color, SpinSlider, fcolor, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_selected_color, SpinSlider, fselected_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_read_only_color, SpinSlider, fread_only_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_outline_color, SpinSlider, fselected_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(caret_color, SpinSlider, cursor_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(selection_color, SpinSlider, selected_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(slider_color, SpinSlider, sl_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(slider_ratio, SpinSlider, sl_ratio_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(slider_read_only_color, SpinSlider, sl_ro_color, m_theme_cache);
   REGISTER_CUSTOM_FONT(font, SpinSlider, fnt, m_theme_cache);
   REGISTER_CUSTOM_FONT_SIZE(font_size, SpinSlider, font_size, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(slider_height, SpinSlider, s_height, m_theme_cache);
}


godot::Vector2 SpinSlider::_get_minimum_size() const
{
   // The internal LineEdit is set to use StyleBoxEmpty, so its minimum size does not take margin into account
   // Take the overal style box to provide the extra margin (if any) into the minimum size
   //godot::Vector2 ret = m_value_input->get_combined_minimum_size() + m_theme_cache.normal->get_minimum_size();
   godot::Vector2 ret = m_value_input->get_minimum_size() + m_theme_cache.normal->get_minimum_size();


   if (use_slider())
   {
      ret.y += m_theme_cache.slider_height;
   }
   if (use_spin())
   {
      ret.x += m_theme_cache.up_down->get_width();

      ret.y = godot::Math::max<int>(ret.y, m_theme_cache.up_down->get_height());
   }


   return ret;
}


void SpinSlider::_gui_input(const godot::Ref<godot::InputEvent>& event)
{
   godot::Ref<godot::InputEventMouseButton> mb = event;
   if (mb.is_valid())
   {
      if (mb->is_pressed())
      {
         const bool is_up = mb->get_position().y < (get_size().y * 0.5);

         switch (mb->get_button_index())
         {
            case godot::MouseButton::MOUSE_BUTTON_LEFT:
            {
               m_value_input->grab_focus();

               assign_value(m_value + (is_up ? m_step : -m_step), true);
               update_text();

               m_timer->set_wait_time(0.6);
               m_timer->set_one_shot(true);
               m_timer->start();

               m_drag.allowed = true;
               m_drag.capture_pos = mb->get_position();
            } break;

            case godot::MouseButton::MOUSE_BUTTON_RIGHT:
            {
               m_value_input->grab_focus();

               if (m_minmax_on_rclick)
               {
                  assign_value(is_up ? get_max_value() : get_min_value(), true);
                  update_text();
               }
            } break;

            case godot::MouseButton::MOUSE_BUTTON_WHEEL_UP:
            {
               if (m_value_input->has_focus())
               {
                  assign_value(m_value + m_step * mb->get_factor(), true);
                  update_text();
                  accept_event();
               }
            } break;

            case godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN:
            {
               if (m_value_input->has_focus())
               {
                  assign_value(m_value - m_step * mb->get_factor(), true);
                  update_text();
                  accept_event();
               }
               
            } break;
         }
      }
      else
      {
         if (mb->get_button_index() == godot::MouseButton::MOUSE_BUTTON_LEFT)
         {
            m_timer->stop();
            release_mouse();
            m_drag.allowed = false;
         }
      }
   }

   godot::Ref<godot::InputEventMouseMotion> mm = event;
   if (mm.is_valid() && mm->get_button_mask().has_flag(godot::MouseButtonMask::MOUSE_BUTTON_MASK_LEFT))
   {
      if (m_drag.dragging)
      {
         m_drag.diff_y += mm->get_relative().y;
         const double diff_y = -0.01 * godot::Math::pow((double)godot::ABS(m_drag.diff_y), 1.8) * godot::SIGN(m_drag.diff_y);
         // assign_value() clamps the value if required
         assign_value(m_drag.base_value + m_step * diff_y, true);
         update_text();
      }
      else if (m_drag.allowed && m_drag.capture_pos.distance_to(mm->get_position()) > 2)
      {
         godot::Input::get_singleton()->set_mouse_mode(godot::Input::MouseMode::MOUSE_MODE_CAPTURED);
         m_drag.dragging = true;
         m_drag.base_value = m_value;
         m_drag.diff_y = 0.0;
      }
   }
}


void SpinSlider::set_as_ratio(double r)
{
   set_value(godot::Math::lerp(m_min_value, m_max_value, r));
}


double SpinSlider::get_as_ratio() const
{
   if (godot::Math::is_equal_approx(m_min_value, m_max_value))
   {
      // This is to prevent division by 0
      return 1.0;
   }

   return (m_value - m_min_value) / (m_max_value - m_min_value);
}


godot::LineEdit* SpinSlider::get_line_edit() const
{
   return m_value_input;
}


/*void SpinSlider::unfocus()
{
   if (m_value_input->has_focus())
   {
      m_value_input->release_focus();
   }
}*/


void SpinSlider::set_alignment(godot::HorizontalAlignment align)
{
   m_value_input->set_horizontal_alignment(align);
}

godot::HorizontalAlignment SpinSlider::get_alignment() const
{
   return m_value_input->get_horizontal_alignment();
}


void SpinSlider::set_read_only(bool val)
{
   m_value_input->set_editable(!val);
   queue_redraw();
   if (use_slider())
   {
      m_slider->queue_redraw();
   }
}

bool SpinSlider::get_read_only() const
{
   return !m_value_input->is_editable();
}


void SpinSlider::set_prefix(const godot::String& val)
{
   if (m_prefix == val) { return; }

   m_prefix = val;
   update_text();
}

void SpinSlider::set_suffix(const godot::String& val)
{
   if (m_suffix == val) { return; }

   m_suffix = val;
   update_text();
}


void SpinSlider::set_step(double val)
{
   m_step = val;
   set_value(m_value);
}

void SpinSlider::set_value(double val)
{
   assign_value(val, false);
   update_text();
}


void SpinSlider::set_select_all_on_focus(bool val)
{
   m_value_input->set_select_all_on_focus(val);
}

bool SpinSlider::get_select_all_on_focus() const
{
   return m_value_input->is_select_all_on_focus();
}


void SpinSlider::set_no_spin_with_slider(bool val)
{
   m_no_spin = val;
   update_minimum_size();
   adjust_layout();
   queue_redraw();
}


void SpinSlider::set_use_min_value(bool val)
{
   m_use_min_value = val;
   notify_property_list_changed();
   update_minimum_size();
   adjust_layout();
   set_value(m_value);
}

void SpinSlider::set_min_value(double val)
{
   m_min_value = val;
   set_value(m_value);

   if (use_slider())
   {
      m_slider->queue_redraw();
   }
}

void SpinSlider::set_use_max_value(bool val)
{
   m_use_max_value = val;
   notify_property_list_changed();
   update_minimum_size();
   adjust_layout();
   set_value(m_value);
}

void SpinSlider::set_max_value(double val)
{
   m_max_value = val;
   set_value(m_value);

   if (use_slider())
   {
      m_slider->queue_redraw();
   }
}


SpinSlider::SpinSlider()
{
   m_allow_expression = false;
   m_update_on_t_change = false;
   m_step = 1.0;
   m_value = 0.0;
   m_rounded_values = false;
   m_no_spin = false;
   m_use_min_value = false;
   m_min_value = 0.0;
   m_use_max_value = false;
   m_max_value = 100.0;

   m_mouse_over = false;
   m_mouse_over_slider = false;
   m_grabbing_grabber = false;

   m_value_input = memnew(godot::LineEdit);
   add_child(m_value_input, false, INTERNAL_MODE_FRONT);
   m_value_input->set_select_all_on_focus(true);
   m_value_input->set_visible(true);
   m_value_input->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
   m_value_input->set_mouse_filter(MOUSE_FILTER_PASS);
   m_value_input->set_text("0");
   m_value_input->set_horizontal_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_LEFT);
   m_value_input->set_clip_contents(true);

   // Remove rendering of any style from the LineEdit. Drawing will be done within SpinSlider::draw_ctrl() instead
   godot::Ref<godot::StyleBoxEmpty> style_empty = godot::Ref<godot::StyleBoxEmpty>(memnew(godot::StyleBoxEmpty));
   m_value_input->begin_bulk_theme_override();
   m_value_input->add_theme_stylebox_override("normal", style_empty);
   m_value_input->add_theme_stylebox_override("read_only", style_empty);
   m_value_input->add_theme_stylebox_override("focus", style_empty);
   m_value_input->end_bulk_theme_override();

   m_slider = memnew(godot::Control);
   add_child(m_slider, false, INTERNAL_MODE_FRONT);
   m_slider->set_anchors_and_offsets_preset(PRESET_BOTTOM_WIDE);
   m_slider->set_mouse_filter(MOUSE_FILTER_STOP);
   m_slider->set_visible(false);

   m_timer = memnew(godot::Timer);
   add_child(m_timer, false, INTERNAL_MODE_FRONT);

//   set_focus_mode(godot::Control::FOCUS_ALL);
}


#endif
