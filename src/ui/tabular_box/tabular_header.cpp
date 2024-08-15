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

#ifndef TABULAR_BOX_DISABLED

#include "tabular_header.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/line_edit.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


void TabularBoxHeader::apply_styling()
{
   if (!m_theme_cache) { return; }
   
   m_bt_move_left->begin_bulk_theme_override();
   m_bt_move_left->add_theme_stylebox_override("normal", m_theme_cache->button_normal);
   m_bt_move_left->add_theme_stylebox_override("pressed", m_theme_cache->button_pressed);
   m_bt_move_left->add_theme_stylebox_override("hover", m_theme_cache->button_hover);
   m_bt_move_left->add_theme_stylebox_override("focus", m_theme_cache->stl_empty);
   m_bt_move_left->end_bulk_theme_override();
   m_bt_move_left->set_button_icon(m_theme_cache->left_arrow);
   m_bt_move_left->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);

   m_txt_title->begin_bulk_theme_override();
   m_txt_title->add_theme_stylebox_override("normal", m_theme_cache->stl_empty);
   m_txt_title->add_theme_stylebox_override("focus", m_theme_cache->stl_empty);
   m_txt_title->add_theme_stylebox_override("read_only", m_theme_cache->stl_empty);
   m_txt_title->add_theme_font_override("font", m_theme_cache->header_font);
   m_txt_title->add_theme_font_size_override("font_size", m_theme_cache->header_font_size);
   m_txt_title->end_bulk_theme_override();

   m_bt_move_right->begin_bulk_theme_override();
   m_bt_move_right->add_theme_stylebox_override("normal", m_theme_cache->button_normal);
   m_bt_move_right->add_theme_stylebox_override("pressed", m_theme_cache->button_pressed);
   m_bt_move_right->add_theme_stylebox_override("hover", m_theme_cache->button_hover);
   m_bt_move_right->add_theme_stylebox_override("focus", m_theme_cache->stl_empty);
   m_bt_move_right->end_bulk_theme_override();
   m_bt_move_right->set_button_icon(m_theme_cache->right_arrow);
   m_bt_move_right->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
}


void TabularBoxHeader::check_layout()
{
   const bool has_buttons = needs_move_buttons();

   const float margin_left = m_theme_cache->header->get_margin(godot::SIDE_LEFT);
   const float margin_top = m_theme_cache->header->get_margin(godot::SIDE_TOP);
   const float margin_right = m_theme_cache->header->get_margin(godot::SIDE_RIGHT);
   const float margin_bottom = m_theme_cache->header->get_margin(godot::SIDE_BOTTOM);
   const int32_t separation = m_theme_cache->separation;

   const int btsize = m_min_size.height - (margin_top + margin_bottom);

   const float txt_left = has_buttons ? margin_left + separation + btsize : margin_left;
   const float txt_right = has_buttons ? -(margin_right + separation + btsize) : -margin_right;

   m_bt_move_left->set_custom_minimum_size(godot::Vector2(btsize, btsize));
   m_bt_move_left->set_anchor_and_offset(godot::SIDE_LEFT, 0, margin_left);
   m_bt_move_left->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top);
   m_bt_move_left->set_anchor_and_offset(godot::SIDE_RIGHT, 0, margin_left + btsize);
   m_bt_move_left->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -margin_bottom);

   m_txt_title->set_anchor_and_offset(godot::SIDE_LEFT, 0, txt_left);
   m_txt_title->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top);
   m_txt_title->set_anchor_and_offset(godot::SIDE_RIGHT, 1, txt_right);
   m_txt_title->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -margin_bottom);

   m_bt_move_right->set_custom_minimum_size(godot::Vector2(btsize, btsize));
   m_bt_move_right->set_anchor_and_offset(godot::SIDE_LEFT, 1, -(margin_right + btsize));
   m_bt_move_right->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top);
   m_bt_move_right->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -margin_right);
   m_bt_move_right->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -margin_bottom);
}


void TabularBoxHeader::setup_sizer()
{
   // The 'sizer' is set as top level, meaning that global position must be used. More than that, control clipping
   // does not occur on top level widgets. As a result must manually check if the sizer will be placed within the
   // confines. That said, things become a little tricky here. The horizontal positioning is slightly less complicated.
   // Basically, this header is inserted into a HBoxContainer that moves when scrolling occurs. This box is added
   // into another Control that serves as the "clipper". That Control is the one that provides the horizontal confines
   // of the headers.
   // Ther vertical confines, well. That is way more complicated to be entirely done directly from here. For that
   // a maximum height must be provided from outside.
   const int base_x = get_parent_control()->get_parent_control()->get_global_position().x;
   const int base_width = get_parent_control()->get_parent_control()->get_size().x;
   const float base_right = base_x + base_width;

   const godot::Vector2 gpos = get_global_position();
   const godot::Vector2 size = get_size();
   const godot::Vector2 sizer_pos = godot::Vector2(gpos.x + size.x - HALF_SIZER_WIDTH, gpos.y);
   if ((sizer_pos.x - HALF_SIZER_WIDTH < base_x || sizer_pos.x + HALF_SIZER_WIDTH > base_right) && !m_drag_state.dragging)
   {
      m_sizer->set_visible(false);
   }
   else
   {
      m_sizer->set_visible(true);
      m_sizer->set_position(sizer_pos);
      const int32_t height = size.y + godot::Math::min<int32_t>(m_extra_sizer_height, m_max_sizer_height);
      m_sizer->set_size(godot::Vector2(SIZER_WIDTH, height));
   }
}


void TabularBoxHeader::check_flags()
{
   m_txt_title->set_editable(Internals::is_flag_set(TabularDataSource::AllowTitleEdit, m_info.flags));
   m_sizer->set_visible(Internals::is_flag_set(TabularDataSource::AllowResize, m_info.flags));

   const bool bt_visible = ((Internals::is_flag_set(TabularDataSource::AllowMove, m_info.flags)) && (m_info.move_bt_mode == TabularDataSource::AlwaysVisible));

   m_bt_move_left->set_visible(bt_visible);
   m_bt_move_right->set_visible(bt_visible);
}


void TabularBoxHeader::on_sizer_gui_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventMouseButton> mb = evt;
   if (mb.is_valid())
   {
      if (mb->get_button_index() == godot::MouseButton::MOUSE_BUTTON_LEFT)
      {
         if (mb->is_double_click())
         {
            int32_t width = m_theme_cache->header->get_margin(godot::SIDE_LEFT) + m_theme_cache->header->get_margin(godot::SIDE_RIGHT);
            
            const godot::HorizontalAlignment align = m_txt_title->get_horizontal_alignment();
            width += m_theme_cache->header_font->get_string_size(get_title(), align, -1, m_theme_cache->header_font_size).x;

            if (needs_move_buttons())
            {
               const int32_t separation = m_theme_cache->separation;
               // At this point the "move" button should be properly sized, so simply retrieve it.
               const int32_t btw = m_bt_move_left->get_size().x;
               width += btw + btw + separation + separation;
            }

            set_width(width);
         }
         else if (mb->is_pressed())
         {
            m_drag_state.dragging = true;
            m_drag_state.initial_width = get_size().x;
            m_drag_state.initial_x = mb->get_position().x;
         }
         else
         {
            m_drag_state.dragging = false;

            // This is to make sure its visibility is correct
            setup_sizer();
         }

         accept_event();
      }
   }

   godot::Ref<godot::InputEventMouseMotion> mm = evt;
   if (mm.is_valid() && m_drag_state.dragging)
   {
      const float delta = mm->get_position().x - m_drag_state.initial_x;
      godot::Size2i size = get_size();
      set_width(size.width + delta);
      accept_event();
   }
}


void TabularBoxHeader::on_move_left_clicked()
{
   emit_signal("move_left_clicked");
}


void TabularBoxHeader::on_move_right_clicked()
{
   emit_signal("move_right_clicked");
}


void TabularBoxHeader::on_title_unfocused()
{
   m_txt_title->select(0, 0);
}


void TabularBoxHeader::on_title_input(const godot::Ref<godot::InputEvent>& evt)
{
   // When the 'ui_cancel' action is triggered (Escape key for example) LineEdit simply release focus.
   // Unfortunately there is no way to easily identify if the focus exited event was triggered by this
   // or by clicking focusing another control or even pressing the TAB key. So handling the 'ui_cancel'
   // event here. The idea is to revert the title change when that happens
   godot::Ref<godot::InputEventKey> key = evt;
   if (key.is_valid() && key->is_pressed())
   {
      if (key->is_action("ui_cancel"))
      {
         // Trigger the 'title entered' event but providing a blank one just so the TabularBox takes care of
         // reverting the title change - there was any
         //on_title_entered("");
         m_txt_title->set_text(m_info.title);
         accept_event();
         m_txt_title->release_focus();
      }
   }
}


void TabularBoxHeader::on_title_entered(const godot::String& new_title)
{
   m_txt_title->release_focus();
   emit_signal("title_entered", new_title);
}



void TabularBoxHeader::on_debug_draw(godot::Control* ctrl, const godot::Color& color)
{
   ctrl->draw_rect(godot::Rect2(godot::Vector2(), ctrl->get_size()), color);
}


void TabularBoxHeader::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_sizer->connect("gui_input", callable_mp(this, &TabularBoxHeader::on_sizer_gui_input));
         m_bt_move_left->connect("pressed", callable_mp(this, &TabularBoxHeader::on_move_left_clicked));
         m_bt_move_right->connect("pressed", callable_mp(this, &TabularBoxHeader::on_move_right_clicked));

         m_txt_title->connect("focus_exited", callable_mp(m_txt_title, &godot::LineEdit::select).bind(0, 0), CONNECT_DEFERRED);
         m_txt_title->connect("gui_input", callable_mp(this, &TabularBoxHeader::on_title_input));
         m_txt_title->connect("text_submitted", callable_mp(this, &TabularBoxHeader::on_title_entered));


         /// This is DEBUG thingy. Keeping this commented for easy debugging if necessary
         //m_sizer->connect("draw", callable_mp(this, &TabularBoxHeader::on_debug_draw).bind(m_sizer, godot::Color(0, 1, 1, 0.5)));
      } break;

      case NOTIFICATION_THEME_CHANGED:
      {
         if (m_theme_cache)
         {
            apply_styling();
            check_layout();
            queue_redraw();
         }
      } break;

      case NOTIFICATION_DRAW:
      {
         if (m_theme_cache)
         {
            draw_style_box(m_theme_cache->header, godot::Rect2(godot::Vector2(), get_size()));
         }
      } break;

      case NOTIFICATION_ENTER_TREE:
      case NOTIFICATION_TRANSFORM_CHANGED:
      case NOTIFICATION_RESIZED:
      {
         // Directly calling setup_sizer() works in editor but not when running. Deferring
         // the call works on both cases.
         callable_mp(this, &TabularBoxHeader::setup_sizer).call_deferred();
      } break;

      case NOTIFICATION_MOUSE_ENTER:
      {
         if (Internals::is_flag_set(TabularDataSource::AllowMove, m_info.flags) && m_info.move_bt_mode == TabularDataSource::ShowOnMouseOver)
         {
            m_bt_move_left->set_visible(true);
            m_bt_move_right->set_visible(true);
         }
      } break;

      case NOTIFICATION_MOUSE_EXIT:
      {
         if (Internals::is_flag_set(TabularDataSource::AllowMove, m_info.flags) && m_info.move_bt_mode == TabularDataSource::ShowOnMouseOver)
         {
            m_bt_move_left->set_visible(false);
            m_bt_move_right->set_visible(false);
         }
      } break;
   }
}


void TabularBoxHeader::_bind_methods()
{
   using namespace godot;

   ADD_SIGNAL(MethodInfo("title_entered", PropertyInfo(Variant::STRING, "new_title")));
   ADD_SIGNAL(MethodInfo("move_left_clicked"));
   ADD_SIGNAL(MethodInfo("move_right_clicked"));
}


void TabularBoxHeader::set_info(const TabularDataSource::ColumnInfo& info)
{
   m_info.title = info.title;
   m_info.type_code = info.type_code;
   m_info.flags = info.flags;
   m_info.cell_class = info.cell_class;
   m_info.move_bt_mode = info.move_bt_mode;
   m_info.extra_settings = info.extra_settings;

   m_txt_title->set_text(m_info.title);

   check_flags();
}


bool TabularBoxHeader::check_info(const TabularDataSource::ColumnInfo& info) const
{
   return m_info == info;
}


void TabularBoxHeader::set_allow_title_edit(bool allow)
{
   Internals::set_flag(allow, TabularDataSource::AllowTitleEdit, m_info.flags);
   check_flags();
}


bool TabularBoxHeader::get_allow_title_edit() const
{
   return Internals::is_flag_set(TabularDataSource::AllowTitleEdit, m_info.flags);
}


void TabularBoxHeader::set_allow_menu(bool allow)
{
   Internals::set_flag(allow, TabularDataSource::AllowMenu, m_info.flags);
}


bool TabularBoxHeader::get_allow_menu() const
{
   return Internals::is_flag_set(TabularDataSource::AllowMenu, m_info.flags);
}


void TabularBoxHeader::set_allow_resize(bool allow)
{
   Internals::set_flag(allow, TabularDataSource::AllowResize, m_info.flags);
   check_flags();
}


bool TabularBoxHeader::get_allow_resize() const
{
   return Internals::is_flag_set(TabularDataSource::AllowResize, m_info.flags);
}


void TabularBoxHeader::set_allow_type_change(bool allow)
{
   Internals::set_flag(allow, TabularDataSource::AllowTypeChange, m_info.flags);
}


bool TabularBoxHeader::get_allow_type_change() const
{
   return Internals::is_flag_set(TabularDataSource::AllowTypeChange, m_info.flags);
}


void TabularBoxHeader::set_allow_sorting(bool allow)
{
   Internals::set_flag(allow, TabularDataSource::AllowSorting, m_info.flags);
}


bool TabularBoxHeader::get_allow_sorting() const
{
   return Internals::is_flag_set(TabularDataSource::AllowSorting, m_info.flags);
}


void TabularBoxHeader::set_allow_move(bool allow)
{
   Internals::set_flag(allow, TabularDataSource::AllowMove, m_info.flags);
   check_flags();
}


bool TabularBoxHeader::get_allow_move() const
{
   return Internals::is_flag_set(TabularDataSource::AllowMove, m_info.flags);
}


void TabularBoxHeader::set_value_change_signal(bool enable)
{
   Internals::set_flag(enable, TabularDataSource::ValueChangeSignal, m_info.flags);
}


bool TabularBoxHeader::get_value_change_signal() const
{
   return Internals::is_flag_set(TabularDataSource::ValueChangeSignal, m_info.flags);
}


bool TabularBoxHeader::needs_move_buttons() const
{
   return ((m_info.move_bt_mode != TabularDataSource::AlwaysHidden) && get_allow_move());
}


void TabularBoxHeader::set_alignment(godot::HorizontalAlignment align)
{
   m_txt_title->set_horizontal_alignment(align);
}


void TabularBoxHeader::confirm_title_change()
{
   m_info.title = m_txt_title->get_text();
}


void TabularBoxHeader::revert_title_change()
{
   m_txt_title->set_text(m_info.title);
}


void TabularBoxHeader::set_width(int32_t w)
{
   if (w < 0) { return; }

   set_custom_minimum_size(godot::Vector2(w, 0));
}


void TabularBoxHeader::setup(TBoxThemeCache* theme_cache, const godot::Size2i& min_size)
{
   m_theme_cache = theme_cache;
   m_min_size = min_size;
   apply_styling();
   check_layout();
   update_minimum_size();
}


void TabularBoxHeader::set_extra_sizer_height(int32_t e)
{
   m_extra_sizer_height = e;

   if (is_inside_tree())
   {
      setup_sizer();
   }
}


void TabularBoxHeader::set_max_sizer_height(int32_t e)
{
   m_max_sizer_height = e;

   if (is_inside_tree())
   {
      setup_sizer();
   }
}


TabularBoxHeader::TabularBoxHeader()
{
   m_theme_cache = nullptr;
   m_extra_sizer_height = 0;
   m_max_sizer_height = 0;
   m_drag_state.dragging = false;

   set_notify_transform(true);

   m_bt_move_left = memnew(godot::Button);
   m_bt_move_left->set_name("_move_left_");
   m_bt_move_left->set_mouse_filter(MOUSE_FILTER_PASS);
   add_child(m_bt_move_left);

   m_txt_title = memnew(godot::LineEdit);
   m_txt_title->set_name("_title_");
   m_txt_title->set_context_menu_enabled(false);
   m_txt_title->add_theme_constant_override("minimum_character_width", MIN_TITLE_CHARS);
   m_txt_title->set_mouse_filter(MOUSE_FILTER_PASS);
   m_txt_title->set_caret_blink_enabled(true);
   m_txt_title->set_select_all_on_focus(true);
   add_child(m_txt_title);

   m_bt_move_right = memnew(godot::Button);
   m_bt_move_right->set_name("_move_right_");
   m_bt_move_right->set_mouse_filter(MOUSE_FILTER_PASS);
   add_child(m_bt_move_right);

   m_sizer = memnew(godot::Control);
   m_sizer->set_as_top_level(true);
   m_sizer->set_mouse_filter(MOUSE_FILTER_PASS);
   m_sizer->set_default_cursor_shape(CURSOR_HSIZE);
   add_child(m_sizer);

   set_mouse_filter(MOUSE_FILTER_PASS);
}


#endif
