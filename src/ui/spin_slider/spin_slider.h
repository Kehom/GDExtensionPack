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

#ifndef _kehui_spin_slider_h_included
#define _kehui_spin_slider_h_included 1

/// A lot of the Controls designed to deal with ranged values in the core Godot are derived
/// from a common class, Range. I did consider deriving SpinSlider from Range too, however it
/// does automatically export some variables to the Inspector in a way that somewhat clashes
/// with the desired design. Because of that this is derived directly from Control.

#ifndef SPIN_SLIDER_DISABLED

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>

// Forward declares
namespace godot
{
   class Font;
   class LineEdit;
   class StyleBox;
   class StyleBoxFlat;
   class Timer;
}

class SpinSlider : public godot::Control
{
   GDCLASS(SpinSlider, godot::Control);
private:
   struct ThemeCache
   {
      godot::Ref<godot::StyleBox> normal;
      godot::Ref<godot::StyleBox> read_only;
      godot::Ref<godot::StyleBox> focus;

      godot::Color font_color;
      godot::Color font_selected_color;
      godot::Color font_read_only_color;
      godot::Color font_outline_color;
      godot::Color caret_color;
      godot::Color selection_color;

      godot::Color slider_color;
      godot::Color slider_ratio;
      godot::Color slider_read_only_color;

      godot::Ref<godot::Font> font;
      int32_t font_size;

      godot::Ref<godot::Texture2D> grabber;
      godot::Ref<godot::Texture2D> grabber_highlight;
      godot::Ref<godot::Texture2D> up_down;

      int slider_height;
   } m_theme_cache;

   /// Internal variables
   struct DragData
   {
      double base_value;
      bool allowed;
      bool dragging;
      godot::Vector2 capture_pos;
      double diff_y;

      DragData() :
         base_value(0.0),
         allowed(false),
         dragging(false),
         diff_y(0.0)
      {}
   } m_drag;

   godot::LineEdit* m_value_input;
   godot::Control* m_slider;
   godot::Timer* m_timer;

   bool m_mouse_over;
   bool m_mouse_over_slider;
   bool m_grabbing_grabber;


   // This is used to track the last valid text entered within the m_value_input LineEdit. This is meant to disallow
   // non numeric digits from being typed in. In this regard it would have been fantastic if the LineEdit Control had
   // any way to validate keystrokes
   godot::String m_last_valid;

   /// Exposed variables
   bool m_allow_expression;         // If this is true then there will be no attempt to filter keystrokes as math expressions will be allowed
   bool m_update_on_t_change;       // If true, update internal value on each keystroke
   godot::String m_prefix;
   godot::String m_suffix;
   double m_step;
   double m_value;
   bool m_rounded_values;
   bool m_no_spin;              // no spin with slider
   bool m_minmax_on_rclick;     // If false then don't assign min/max on right click

   bool m_use_min_value;
   double m_min_value;

   bool m_use_max_value;
   double m_max_value;

   /// Internal functions
   void apply_internal_styles();

   // This is used to assign the value taking min_val, max_val, step, rounding and all of that into account
   // The idea here is that if outside code manually call "set_value()" then no signal will be emitted. However
   // if internal code changes the value then a signal must be emitted.
   void assign_value(double val, bool signal);

   void update_text();

   void check_entered_text(const godot::String& new_text);

   bool use_slider() const { return m_use_min_value && m_use_max_value; }
   bool use_spin() const { return !use_slider() || !m_no_spin; }

   void adjust_layout();

   void draw_ctrl();

   void release_mouse();

   /// Event handlers
   void on_text_changed(const godot::String& new_text);
   void on_text_entered(const godot::String& new_text);
   void on_input_focus_enter();
   void on_input_focus_exit();

   void on_draw_slider();
   void on_slider_mouse_entered();
   void on_slider_mouse_exited();
   void on_slider_gui_input(const godot::Ref<godot::InputEvent>& event);

   void on_range_click_timeout();


protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* list) const;
   void _notification(int what);
   void _validate_property(godot::PropertyInfo& property) const;
   static void _bind_methods();
public:
   /// Overrides
   godot::Vector2 _get_minimum_size() const;       // override
   virtual void _gui_input(const godot::Ref<godot::InputEvent>& event) override;

   /// Exposed functions
   void set_as_ratio(double r);
   double get_as_ratio() const;

   godot::LineEdit* get_line_edit() const;

   // When the SpinSlider receives focus, it's relayed into the internal LineEdit. This means that grab_focus() will
   // result in expected behavior. However release_focus() will not do anything because the base Control class check
   // if the instance has focus. Since that was relayed into an internal widget, the check returns false. To still
   // allow focus to be removed from the SpinSlider from external code, this function is provided
   //void unfocus();

   /// Setters/Getters
   void set_alignment(godot::HorizontalAlignment align);
   godot::HorizontalAlignment get_alignment() const;

   void set_allow_expression(bool val) { m_allow_expression = val; }
   bool get_allow_expression() const { return m_allow_expression; }

   void set_read_only(bool val);
   bool get_read_only() const;

   void set_update_on_text_changed(bool val) { m_update_on_t_change = val; }
   bool get_update_on_text_changed() const { return m_update_on_t_change; }

   void set_prefix(const godot::String& val);
   godot::String get_prefix() const { return m_prefix; }

   void set_suffix(const godot::String& val);
   godot::String get_suffix() const { return m_suffix; }

   void set_step(double val);
   double get_step() const { return m_step; }

   void set_value(double val);
   double get_value() const { return m_value; }

   void set_rounded_values(bool val) { m_rounded_values = val; }
   bool get_rounded_values() const { return m_rounded_values; }

   void set_select_all_on_focus(bool val);
   bool get_select_all_on_focus() const;

   void set_no_spin_with_slider(bool val);
   bool get_no_spin_with_slider() const { return m_no_spin; }

   void set_assign_on_right_click(bool val) { m_minmax_on_rclick = val; }
   bool get_assign_on_right_click() const  { return m_minmax_on_rclick; }

   void set_use_min_value(bool val);
   bool get_use_min_value() const { return m_use_min_value; }

   void set_min_value(double val);
   double get_min_value() const { return m_min_value; }

   void set_use_max_value(bool val);
   bool get_use_max_value() const { return m_use_max_value; }

   void set_max_value(double val);
   double get_max_value() const { return m_max_value; }

   /// Public non exposed functions
   SpinSlider();
};

#endif      // SPIN_SLIDER_DISABLED

#endif     // safe include
