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

#ifndef _kehui_tabular_box_header_h_included
#define _kehui_tabular_box_header_h_included 1

#ifndef TABULAR_BOX_DISABLED



#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/templates/vector.hpp>


#include "data_source.h"
#include "tbox_theme_cache.h"
#include "../../internal.h"

// Forward declares
namespace godot
{
   class LineEdit;
   class Button;
   class InputEvent;
}


/// TODO: Allow adding/removing small arrow indicating sort order
/// For the above task:
/// - Create icons (arrow up and arrow down)
/// - Add two new entries in the Theme for those two icons
/// - Create an option allowing to show/hide the "sorted column"
/// - If show sorted column is enabled, then all columns should allocate some extra espace between LineEdit and move_right button
///   This space should be icon width, remembering to add "separation" on both sides of the icon
class TabularBoxHeader : public godot::Control
{
   GDCLASS(TabularBoxHeader, godot::Control);
private:
   static const int32_t MIN_TITLE_CHARS = 2;
   static const int32_t SIZER_WIDTH = 6;
   static const int32_t HALF_SIZER_WIDTH = 3;
   /// Internal variables
   // Allow editing column title - it also shows it
   godot::LineEdit* m_txt_title;

   // Cache minimum size here
   godot::Size2i m_min_size;

   // This will be provided by the owning TabularBox
   TBoxThemeCache* m_theme_cache;

   // Buttons to move the column to the left or to the right
   godot::Button* m_bt_move_left;
   godot::Button* m_bt_move_right;

   // This will be set as top level so drawn on top of headers and cells
   // That said, it should help resize the column represented by this header
   godot::Control* m_sizer;

   // Just so the sizer can be used through the cells, an extra height is required
   // This caches said value. It's responsibility of the TabularBox to correctly
   // provide one that will not go beyond it
   int32_t m_extra_sizer_height;

   // This is the maximum height the sizer can be and will be set by the owning TabularBox
   int32_t m_max_sizer_height;

   // And holds some information related to the column associated with this header
   // This struct holds the title of the header. While it might seem like duplicated data because the title is also set
   // within the m_txt_title, it provides means to revert title change within the UI. The title within the ColumnInfo should
   // match that of the data source, while the one in the LineEdit is meant to allow editing and displaying it
   TabularDataSource::ColumnInfo m_info;


   struct
   {
      // This is the initial X coordinate of the mouse when drag started.
      float initial_x;

      // This is the initial width of the header (column).
      int initial_width;

      // And a flag telling if there is a drag attempt
      bool dragging;
   } m_drag_state;
   

   /// Exposed variables

   /// Internal functions
   void apply_styling();
   void check_layout();

   void setup_sizer();

   void check_flags();

   /// Event handlers
   void on_sizer_gui_input(const godot::Ref<godot::InputEvent>& evt);

   void on_move_left_clicked();
   void on_move_right_clicked();

   void on_title_unfocused();
   void on_title_input(const godot::Ref<godot::InputEvent>& evt);
   void on_title_entered(const godot::String& new_title);


   void on_debug_draw(godot::Control* ctrl, const godot::Color& color);

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   godot::Vector2 _get_minimum_size() const { return m_min_size; };       // override

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void set_info(const TabularDataSource::ColumnInfo& info);

   bool check_info(const TabularDataSource::ColumnInfo& info) const;

   int32_t get_half_sizer_width() const { return HALF_SIZER_WIDTH; }

   void set_allow_title_edit(bool allow);
   bool get_allow_title_edit() const;

   void set_allow_menu(bool allow);
   bool get_allow_menu() const;

   void set_allow_resize(bool allow);
   bool get_allow_resize() const;

   void set_allow_type_change(bool allow);
   bool get_allow_type_change() const;

   void set_allow_sorting(bool allow);
   bool get_allow_sorting() const;

   void set_allow_move(bool allow);
   bool get_allow_move() const;

   void set_value_change_signal(bool enable);
   bool get_value_change_signal() const;

   bool needs_move_buttons() const;


   void set_alignment(godot::HorizontalAlignment align);


   void confirm_title_change();
   void revert_title_change();
   godot::String get_title() const { return m_info.title; }

   const TabularDataSource::ColumnInfo& get_info() const { return m_info; }

   void set_width(int32_t w);


   //void set_value_type(int32_t type) { m_info.type_code = type; }
   int32_t get_value_type() const { return m_info.type_code; }
   godot::String get_cell_class() const { return m_info.cell_class; }

   void setup(TBoxThemeCache* theme_cache, const godot::Size2i& min_size);

   void set_extra_sizer_height(int32_t e);
   void set_max_sizer_height(int32_t e);

   TabularBoxHeader();
};


#endif  // TABULAR_BOX_DISABLED

#endif   //_kehui_tabular_box_header_h_included
