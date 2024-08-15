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

#include "tabular_box_cell.h"
#include "../spin_slider/spin_slider.h"
#include "../../internal.h"
#include "../../extpackutils.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_uid.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/text_edit.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TabularBoxCell
bool TabularBoxCell::is_odd_row() const
{
   godot::Node* parent = get_parent();
   bool ret = true;
   if (parent && parent->has_meta("is_odd"))
   {
      ret = parent->get_meta("is_odd", true);
   }
   return ret;
}


void TabularBoxCell::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_DRAW:
      {
         if (!m_theme_cache) { return; }

         const godot::Rect2 rect(godot::Vector2(), get_size());
         
         godot::Ref<godot::StyleBox> style = is_odd_row() ? m_theme_cache->odd_row : m_theme_cache->even_row;
         draw_style_box(style, rect);

         if (m_selected)
         {
            draw_style_box(m_theme_cache->focus, rect);
         }
      } break;
   }
}

void TabularBoxCell::_bind_methods()
{
   using namespace godot;

   GDVIRTUAL_BIND(_check_theme);
   GDVIRTUAL_BIND(_apply_extra_settings, "extra_settings");
   GDVIRTUAL_BIND(_assign_value, "value");
   GDVIRTUAL_BIND(_commit_changes);
   GDVIRTUAL_BIND(_selected_changed, "selected");
   GDVIRTUAL_BIND(_requires_file_dialog);
   GDVIRTUAL_BIND(_setup_shared, "parent");
   GDVIRTUAL_BIND(_share_with, "other_cell");

   ClassDB::bind_method(D_METHOD("has_theme"), &TabularBoxCell::has_theme);
   ClassDB::bind_method(D_METHOD("set_min_height", "height"), &TabularBoxCell::set_min_height);
   ClassDB::bind_method(D_METHOD("get_row_index"), &TabularBoxCell::get_row_index);
   ClassDB::bind_method(D_METHOD("get_internal_margin_left"), &TabularBoxCell::get_internal_margin_left);
   ClassDB::bind_method(D_METHOD("get_internal_margin_top"), &TabularBoxCell::get_internal_margin_top);
   ClassDB::bind_method(D_METHOD("get_internal_margin_right"), &TabularBoxCell::get_internal_margin_right);
   ClassDB::bind_method(D_METHOD("get_internal_margin_bottom"), &TabularBoxCell::get_internal_margin_bottom);
   ClassDB::bind_method(D_METHOD("apply_line_edit_style", "line_edit"), &TabularBoxCell::apply_line_edit_style);
   ClassDB::bind_method(D_METHOD("apply_button_style", "into_button"), &TabularBoxCell::apply_button_style);
   ClassDB::bind_method(D_METHOD("get_button_height"), &TabularBoxCell::get_button_height);
   ClassDB::bind_method(D_METHOD("get_button_min_height", "include_font", "icon", "make_square"), &TabularBoxCell::get_button_min_height);
   ClassDB::bind_method(D_METHOD("get_style_empty"), &TabularBoxCell::get_style_empty);
   ClassDB::bind_method(D_METHOD("get_style_background"), &TabularBoxCell::get_style_background);
   ClassDB::bind_method(D_METHOD("get_style_button_normal"), &TabularBoxCell::get_style_button_normal);
   ClassDB::bind_method(D_METHOD("get_style_button_hover"), &TabularBoxCell::get_style_button_hover);
   ClassDB::bind_method(D_METHOD("get_style_button_pressed"), &TabularBoxCell::get_style_button_pressed);
   ClassDB::bind_method(D_METHOD("get_style_focus"), &TabularBoxCell::get_style_focus);
   ClassDB::bind_method(D_METHOD("get_style_font"), &TabularBoxCell::get_font);
   ClassDB::bind_method(D_METHOD("get_style_font_size"), &TabularBoxCell::get_font_size);
   ClassDB::bind_method(D_METHOD("get_style_font_color"), &TabularBoxCell::get_font_color);
   ClassDB::bind_method(D_METHOD("get_style_font_selected_color"), &TabularBoxCell::get_font_selected_color);
   ClassDB::bind_method(D_METHOD("get_style_selection_color"), &TabularBoxCell::get_selection_color);
   ClassDB::bind_method(D_METHOD("get_style_caret_color"), &TabularBoxCell::get_caret_color);
   ClassDB::bind_method(D_METHOD("get_style_icon_checked"), &TabularBoxCell::get_icon_checked);
   ClassDB::bind_method(D_METHOD("get_style_icon_unchecked"), &TabularBoxCell::get_icon_unchecked);
   ClassDB::bind_method(D_METHOD("get_style_icon_trash"), &TabularBoxCell::get_icon_trash);
   ClassDB::bind_method(D_METHOD("get_style_icon_no_texture"), &TabularBoxCell::get_icon_no_texture);
   ClassDB::bind_method(D_METHOD("get_style_separation"), &TabularBoxCell::get_theme_separation);
   ClassDB::bind_method(D_METHOD("notify_value_changed", "new_value"), &TabularBoxCell::notify_value_changed);
   ClassDB::bind_method(D_METHOD("notify_selected"), &TabularBoxCell::notify_selected);
   ClassDB::bind_method(D_METHOD("request_select_next"), &TabularBoxCell::request_select_next);
   ClassDB::bind_method(D_METHOD("request_unselect"), &TabularBoxCell::request_unselect);
   ClassDB::bind_method(D_METHOD("request_file_dialog", "title", "filters", "selected_handler"), &TabularBoxCell::request_file_dialog);

   ADD_SIGNAL(MethodInfo("value_changed", PropertyInfo(Variant::NIL, "new_value")));
   ADD_SIGNAL(MethodInfo("selected"));

   ADD_SIGNAL(MethodInfo("select_next"));
   ADD_SIGNAL(MethodInfo("unselect"));
   //ADD_SIGNAL(MethodInfo("open_file_dialog", PropertyInfo(Variant::STRING, "title"), PropertyInfo(Variant::PACKED_STRING_ARRAY, "filters")));
}


void TabularBoxCell::_gui_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventMouseButton> mb = evt;
   if (mb.is_valid())
   {
      if (mb->is_pressed())
      {
         notify_selected();
      }
   }
}


void TabularBoxCell::_unhandled_input(const godot::Ref<godot::InputEvent>& evt)
{
   if (!m_selected) { return; }
   if (evt->is_action("ui_text_submit"))
   {
      if (evt->is_pressed())
      {
         if (!evt->is_echo())
         {
            request_select_next();
         }
      }
      else
      {
         m_can_select_next = true;
      }
   }
   else if (evt->is_action("ui_cancel"))
   {
      request_unselect();
   }
}


void TabularBoxCell::set_min_height(int32_t height)
{
   m_min_size.height = height;
   update_minimum_size();
}


int64_t TabularBoxCell::get_row_index() const
{
   if (!is_inside_tree()) { return -1; }
   // A cell is added into an instance of HBoxContainer in column order. This container itself is added into an instance of
   // VBoxContainer, in row order. That said, calling 'get_index()' within the parent of this cell should result in the row index
   return get_parent_control()->get_index();
}


double TabularBoxCell::get_internal_margin_left() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, 0.0, "Attempting to retrieve internal margin before theme has been correctly setup.");
   godot::Ref<godot::StyleBox> style = is_odd_row() ? m_theme_cache->odd_row : m_theme_cache->even_row;

   return style->get_margin(godot::SIDE_LEFT);
}


double TabularBoxCell::get_internal_margin_top() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, 0.0, "Attempting to retrieve internal margin before theme has been correctly setup.");
   godot::Ref<godot::StyleBox> style = is_odd_row() ? m_theme_cache->odd_row : m_theme_cache->even_row;

   return style->get_margin(godot::SIDE_TOP);
}


double TabularBoxCell::get_internal_margin_right() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, 0.0, "Attempting to retrieve internal margin before theme has been correctly setup.");
   godot::Ref<godot::StyleBox> style = is_odd_row() ? m_theme_cache->odd_row : m_theme_cache->even_row;

   return style->get_margin(godot::SIDE_RIGHT);
}


double TabularBoxCell::get_internal_margin_bottom() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, 0.0, "Attempting to retrieve internal margin before theme has been correctly setup.");
   godot::Ref<godot::StyleBox> style = is_odd_row() ? m_theme_cache->odd_row : m_theme_cache->even_row;

   return style->get_margin(godot::SIDE_BOTTOM);
}


void TabularBoxCell::apply_line_edit_style(godot::LineEdit* edit)
{
   ERR_FAIL_COND_MSG(!m_theme_cache, "Attempting to apply styling into a LineEdit before theme has been correctly setup.");
   
   edit->set_context_menu_enabled(edit);
   edit->set_mouse_filter(MOUSE_FILTER_PASS);
   edit->set_caret_blink_enabled(true);
   edit->set_clip_contents(true);
   edit->set_select_all_on_focus(true);

   edit->begin_bulk_theme_override();
   edit->add_theme_constant_override("minimum_character_width", 1);
   edit->add_theme_stylebox_override("normal", get_style_empty());
   edit->add_theme_stylebox_override("focus", get_style_empty());
   edit->add_theme_font_override("font", get_font());
   edit->add_theme_font_size_override("font_size", get_font_size());
   edit->add_theme_color_override("font_color", get_font_color());
   edit->add_theme_color_override("font_selected_color", get_font_selected_color());
   edit->add_theme_color_override("selection_color", get_selection_color());
   edit->add_theme_color_override("caret", get_caret_color());
   edit->end_bulk_theme_override();
}


void TabularBoxCell::apply_button_style(godot::Button* into)
{
   ERR_FAIL_COND_MSG(!m_theme_cache, "Attempting to apply styling into a button before theme has been correctly setup.");

   into->begin_bulk_theme_override();
   into->add_theme_stylebox_override("normal", m_theme_cache->button_normal);
   into->add_theme_stylebox_override("pressed", m_theme_cache->button_pressed);
   into->add_theme_stylebox_override("hover_pressed", m_theme_cache->button_pressed);
   into->add_theme_stylebox_override("hover", m_theme_cache->button_hover);
   into->add_theme_stylebox_override("disabled", m_theme_cache->stl_empty);
   into->add_theme_stylebox_override("focus", m_theme_cache->stl_empty);
   into->add_theme_color_override("font_color", m_theme_cache->cell_text);
   into->add_theme_color_override("font_focus_color", m_theme_cache->cell_text);
   into->add_theme_color_override("font_hover_color", m_theme_cache->cell_text);
   into->add_theme_color_override("font_hover_pressed_color", m_theme_cache->cell_text);
   into->add_theme_color_override("font_pressed_color", m_theme_cache->cell_text);
   into->add_theme_color_override("icon_normal_color", m_theme_cache->cell_text);
   into->add_theme_color_override("icon_pressed_color", m_theme_cache->cell_text);
   into->add_theme_color_override("icon_hover_color", m_theme_cache->cell_text);
   into->add_theme_color_override("icon_hover_pressed_color", m_theme_cache->cell_text);
   into->add_theme_color_override("icon_focus_color", m_theme_cache->cell_text);
   into->add_theme_font_override("font", m_theme_cache->cell_font);
   into->add_theme_font_size_override("font_size", m_theme_cache->cell_font_size);
   into->end_bulk_theme_override();

   into->set_clip_text(true);
}


float TabularBoxCell::get_button_height() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, -1, "Attempting to calculate button height before theme has been correctly setup.");

   godot::Ref<godot::StyleBox> btnormal = m_theme_cache->button_normal;
   godot::Ref<godot::StyleBox> btpressed = m_theme_cache->button_pressed;
   godot::Ref<godot::StyleBox> bthover = m_theme_cache->button_hover;
   godot::Ref<godot::Font> font = m_theme_cache->cell_font;

   const float vnormal = btnormal->get_margin(godot::SIDE_TOP) + btnormal->get_margin(godot::SIDE_BOTTOM);
   const float vpressed = btpressed->get_margin(godot::SIDE_TOP) + btpressed->get_margin(godot::SIDE_BOTTOM);
   const float vhover = bthover->get_margin(godot::SIDE_TOP) + bthover->get_margin(godot::SIDE_BOTTOM);
   const float vmargin = Internals::vmax<float>(vnormal, vpressed, vhover);

   return font->get_height(m_theme_cache->cell_font_size) + vmargin;
}


float TabularBoxCell::get_button_min_height(bool include_font, const godot::Ref<godot::Texture2D>& icon, bool make_square) const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, -1, "Attempting to calculate button height before theme has been correctly setup.");

   godot::Ref<godot::StyleBox> btnormal = m_theme_cache->button_normal;
   godot::Ref<godot::StyleBox> btpressed = m_theme_cache->button_pressed;
   godot::Ref<godot::StyleBox> bthover = m_theme_cache->button_hover;
   godot::Ref<godot::Font> font = m_theme_cache->cell_font;

   const float vnormal = btnormal->get_margin(godot::SIDE_TOP) + btnormal->get_margin(godot::SIDE_BOTTOM);
   const float vpressed = btpressed->get_margin(godot::SIDE_TOP) + btpressed->get_margin(godot::SIDE_BOTTOM);
   const float vhover = bthover->get_margin(godot::SIDE_TOP) + bthover->get_margin(godot::SIDE_BOTTOM);
   
   const float fheight = include_font ? font->get_height(m_theme_cache->cell_font_size) : 0.0f;
   const float iheight = icon.is_valid() ? icon->get_height() : 0.0f;

   const float vertical = Internals::vmax<float>(vnormal, vpressed, vhover) + Internals::vmax<float>(fheight, iheight);

   if (!make_square)
   {
      return vertical;
   }

   const float hnormal = btnormal->get_margin(godot::SIDE_LEFT) + btnormal->get_margin(godot::SIDE_RIGHT);
   const float hpressed = btpressed->get_margin(godot::SIDE_LEFT) + btpressed->get_margin(godot::SIDE_RIGHT);
   const float hhover = bthover->get_margin(godot::SIDE_LEFT) + bthover->get_margin(godot::SIDE_RIGHT);

   const float iwidth = icon.is_valid() ? icon->get_width() : 0.0f;

   const float horizontal = Internals::vmax<float>(hnormal, hpressed, hhover) + iwidth;
   
   return Internals::vmax<float>(vertical, horizontal);
}


godot::Ref<godot::StyleBox> TabularBoxCell::get_style_empty() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve empty style before theme has been correctly setup.");
   return m_theme_cache->stl_empty;
}


godot::Ref<godot::StyleBox> TabularBoxCell::get_style_background() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve background style before theme has been correctly setup.");
   return m_theme_cache->background;
}


godot::Ref<godot::StyleBox> TabularBoxCell::get_style_button_normal() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve normal button style before theme has been correctly setup.");
   return m_theme_cache->button_normal;
}


godot::Ref<godot::StyleBox> TabularBoxCell::get_style_button_hover() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve hover button style before theme has been correctly setup.");
   return m_theme_cache->button_hover;
}


godot::Ref<godot::StyleBox> TabularBoxCell::get_style_button_pressed() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve pressed button style before theme has been correctly setup.");
   return m_theme_cache->button_pressed;
}


godot::Ref<godot::StyleBox> TabularBoxCell::get_style_focus() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve focus button style before theme has been correctly setup.");
   return m_theme_cache->focus;
}


godot::Ref<godot::Font> TabularBoxCell::get_font() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve font before theme has been correctly setup.");
   return m_theme_cache->cell_font;
}


int32_t TabularBoxCell::get_font_size() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, 12, "Attempting to retrieve font size before theme has been correctly setup.");
   return m_theme_cache->cell_font_size;
}


godot::Color TabularBoxCell::get_font_color() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, godot::Color(0, 0, 0, 1), "Attempting to retrieve color before theme has been correctly setup.");
   return m_theme_cache->cell_text;
}


godot::Color TabularBoxCell::get_font_selected_color() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, godot::Color(0, 0, 0, 1), "Attempting to retrieve color before theme has been correctly setup.");
   return m_theme_cache->cell_selected;
}


godot::Color TabularBoxCell::get_selection_color() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, godot::Color(0, 0, 0, 1), "Attempting to retrieve color before theme has been correctly setup.");
   return m_theme_cache->cell_selection;
}


godot::Color TabularBoxCell::get_caret_color() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, godot::Color(0, 0, 0, 1), "Attempting to retrieve color before theme has been correctly setup.");
   return m_theme_cache->caret;
}


godot::Ref<godot::Texture2D> TabularBoxCell::get_icon_checked() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve icon checked before theme has been correctly setup.");
   return m_theme_cache->checked;
}


godot::Ref<godot::Texture2D> TabularBoxCell::get_icon_unchecked() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve icon unchecked before theme has been correctly setup.");
   return m_theme_cache->unchecked;
}


godot::Ref<godot::Texture2D> TabularBoxCell::get_icon_trash() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve icon trash bin before theme has been correctly setup.");
   return m_theme_cache->trash_bin;
}


godot::Ref<godot::Texture2D> TabularBoxCell::get_icon_no_texture() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, nullptr, "Attempting to retrieve icon no texture before theme has been correctly setup.");
   return m_theme_cache->no_texture;
}


int32_t TabularBoxCell::get_theme_separation() const
{
   ERR_FAIL_COND_V_MSG(!m_theme_cache, 0, "Attempting to retrieve separation before theme has been correctly setup.");
   return m_theme_cache->separation;
}



void TabularBoxCell::notify_value_changed(const godot::Variant& new_value)
{
   emit_signal("value_changed", new_value);
}


void TabularBoxCell::notify_selected()
{
   emit_signal("selected");
}


void TabularBoxCell::request_select_next()
{
   if (!m_can_select_next) { return; }

   emit_signal("select_next");
}


void TabularBoxCell::request_unselect()
{
   emit_signal("unselect");
}


void TabularBoxCell::request_file_dialog(const godot::String& title, const godot::PackedStringArray& filters, const godot::Callable& selected_handler)
{
   if (m_request_file_dialog.is_valid())
   {
      m_request_file_dialog.call(title, filters, selected_handler);
   }
}


void TabularBoxCell::check_theme()
{
   GDVIRTUAL_REQUIRED_CALL(_check_theme);
}


void TabularBoxCell::apply_extra_settings(const godot::Dictionary& extra_settings)
{
   GDVIRTUAL_CALL(_apply_extra_settings, extra_settings);
}


void TabularBoxCell::assign_value(const godot::Variant& value)
{
   GDVIRTUAL_REQUIRED_CALL(_assign_value, value);
}


void TabularBoxCell::commit_changes()
{
   GDVIRTUAL_CALL(_commit_changes);
}


void TabularBoxCell::selected_changed(bool selected)
{
   GDVIRTUAL_CALL(_selected_changed, selected);
}


bool TabularBoxCell::requires_file_dialog() const
{
   bool ret = false;
   if (GDVIRTUAL_CALL(_requires_file_dialog, ret))
   {
      return ret;
   }

   return false;
}


void TabularBoxCell::setup_shared(godot::Control* parent)
{
   GDVIRTUAL_CALL(_setup_shared, parent);
}


void TabularBoxCell::share_with(TabularBoxCell* other_cell)
{
   GDVIRTUAL_CALL(_share_with, other_cell);
}


void TabularBoxCell::setup_theme(TBoxThemeCache* theme_cache)
{
   m_theme_cache = theme_cache;
   check_theme();
}


void TabularBoxCell::set_dialog_requester(const godot::Callable& func)
{
   m_request_file_dialog = func;
}


void TabularBoxCell::set_selected(bool selected)
{
   m_selected = selected;
   if (is_inside_tree())
   {
      queue_redraw();
   }

   //if (m_selected) { grab_focus(); }

   m_can_select_next = !(m_selected && godot::Input::get_singleton()->is_action_pressed("ui_text_submit"));

   selected_changed(m_selected);
}


TabularBoxCell::TabularBoxCell()
{
   set_focus_mode(FOCUS_ALL);
   set_clip_contents(true);

   m_theme_cache = nullptr;
   m_selected = false;
   set_mouse_filter(MOUSE_FILTER_PASS);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TabularBoxCellString
void TabularBoxCellString::on_txt_entered(const godot::String& new_value)
{
   if (new_value != m_assigned)
   {
      notify_value_changed(new_value);
   }

   request_select_next();
}


void TabularBoxCellString::on_txt_unfocus()
{
   if (m_assigned != m_txt_cell->get_text())
   {
      notify_value_changed(m_txt_cell->get_text());
   }
   m_txt_cell->select(0, 0);
}


void TabularBoxCellString::on_txt_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventKey> key = evt;
   if (key.is_valid() && key->is_pressed() && key->is_action("ui_cancel"))
   {
      m_txt_cell->set_text(m_assigned);
      m_txt_cell->release_focus();
   }
}


void TabularBoxCellString::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_txt_cell->connect("text_submitted", callable_mp(this, &TabularBoxCellString::on_txt_entered));
         m_txt_cell->connect("focus_entered", godot::create_custom_callable_function_pointer<TabularBoxCell>(this, &TabularBoxCell::notify_selected));
         m_txt_cell->connect("focus_exited", callable_mp(this, &TabularBoxCellString::on_txt_unfocus));
         m_txt_cell->connect("gui_input", callable_mp(this, &TabularBoxCellString::on_txt_input));
      } break;
   }
}


void TabularBoxCellString::check_theme()
{
   godot::Ref<godot::Font> font = get_font();
   const int32_t font_size = get_font_size();
   const double mtop = get_internal_margin_top();
   const double mbottom = get_internal_margin_bottom();

   m_txt_cell->begin_bulk_theme_override();
   m_txt_cell->add_theme_stylebox_override("normal", get_style_empty());
   m_txt_cell->add_theme_stylebox_override("focus", get_style_empty());
   m_txt_cell->add_theme_font_override("font", font);
   m_txt_cell->add_theme_font_size_override("font_size", font_size);
   m_txt_cell->add_theme_color_override("font_color", get_font_color());
   m_txt_cell->add_theme_color_override("font_selected_color", get_font_selected_color());
   m_txt_cell->add_theme_color_override("selection_color", get_selection_color());
   m_txt_cell->add_theme_color_override("caret", get_caret_color());
   m_txt_cell->end_bulk_theme_override();

   m_txt_cell->set_anchor_and_offset(godot::SIDE_LEFT, 0, get_internal_margin_left());
   m_txt_cell->set_anchor_and_offset(godot::SIDE_TOP, 0, mtop);
   m_txt_cell->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -get_internal_margin_right());
   m_txt_cell->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -mbottom);

   set_min_height(mtop + m_txt_cell->get_combined_minimum_size().y + mbottom);
}


void TabularBoxCellString::assign_value(const godot::Variant& value)
{
   if (value.get_type() != godot::Variant::NIL)
   {
      const godot::String strval = value;
      m_txt_cell->set_text(strval);
   }
   else
   {
      m_txt_cell->set_text("");
   }
   m_assigned = m_txt_cell->get_text();
}


void TabularBoxCellString::selected_changed(bool selected)
{
   if (selected && !m_txt_cell->has_focus())
   {
      m_txt_cell->grab_focus();
   }
   else if (!selected && m_txt_cell->has_focus())
   {
      m_txt_cell->release_focus();
   }
}


TabularBoxCellString::TabularBoxCellString()
{
   m_txt_cell = memnew(godot::LineEdit);
   m_txt_cell->set_context_menu_enabled(false);
   m_txt_cell->set_mouse_filter(MOUSE_FILTER_PASS);
   m_txt_cell->add_theme_constant_override("minimum_character_width", 1);
   m_txt_cell->set_caret_blink_enabled(true);
   m_txt_cell->set_clip_contents(true);
   m_txt_cell->set_select_all_on_focus(true);
   add_child(m_txt_cell);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TabularBoxCellBool
void TabularBoxCellBool::toggle_value()
{
   m_value = !m_value;
   queue_redraw();
   notify_value_changed(m_value);
}


void TabularBoxCellBool::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_DRAW:
      {
         if (!has_theme()) { return; }

         godot::Ref<godot::Texture2D> tex = m_value ? get_icon_checked() : get_icon_unchecked();
         const int ypos = (get_size().y - tex->get_height()) * 0.5;
         const int xpos = (get_size().x - tex->get_width()) * 0.5;

         draw_texture_rect(tex, godot::Rect2(godot::Vector2(xpos, ypos), tex->get_size()), false);
      } break;
   }
}


void TabularBoxCellBool::check_theme()
{
   godot::Ref<godot::Texture2D> checked = get_icon_checked();
   godot::Ref<godot::Texture2D> unchecked = get_icon_unchecked();

   const int min_icon_height = godot::Math::max<int>(checked->get_height(), unchecked->get_height());

//   m_min_size.height = get_internal_margin_top() + get_internal_margin_bottom() + min_icon_height;
   set_min_height(get_internal_margin_top() + min_icon_height + get_internal_margin_bottom());

   update_minimum_size();
}


void TabularBoxCellBool::assign_value(const godot::Variant& value)
{
   const bool bval = value;
   m_value = value;
   queue_redraw();
}


void TabularBoxCellBool::_gui_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventMouseButton> mb = evt;
   if (mb.is_valid() && mb->is_pressed())
   {
      /// TODO: Add an option that allows directly changing value without having to select the cell first
      if (mb->get_button_index() == godot::MOUSE_BUTTON_LEFT && get_selected())
      {
         toggle_value();
      }

      // Regardless of button, request selection
      notify_selected();
   }

   godot::Ref<godot::InputEventKey> kb = evt;
   if (kb.is_valid() && kb->is_pressed())
   {
      if (kb->is_action("ui_select"))
      {
         toggle_value();
      }
   }
}


TabularBoxCellBool::TabularBoxCellBool()
{
   m_value = false;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TabularBoxCellNumeric
#if defined(SPIN_SLIDER_DISABLED) || defined(FORCE_SPIN_BOX)
void TabularBoxCellNumeric::on_spin_le_txt_submitted(const godot::String& ntext)
{
   if (ntext.is_valid_float())
   {
      on_value_entered(ntext.to_float());
   }
}
#endif

void TabularBoxCellNumeric::on_value_entered(double val)
{
   if (val != m_assigned)
   {
      notify_value_changed(val);
   }

   request_select_next();
}


void TabularBoxCellNumeric::on_value_changed(double val)
{
   if (m_spin->get_line_edit()->has_focus()) { return; }
   if (val == m_assigned) { return; }

   notify_value_changed(val);
}


void TabularBoxCellNumeric::on_le_gui_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventKey> key = evt;
   if (key.is_valid() && key->is_pressed() && key->is_action("ui_cancel"))
   {
      m_spin->set_value(m_assigned);
      m_spin->get_line_edit()->release_focus();
   }
}


void TabularBoxCellNumeric::on_spin_min_size_changed()
{
   const float mtop = get_internal_margin_top();
   const float mbottom = get_internal_margin_bottom();
   set_min_height(mtop + m_spin->get_minimum_size().y + mbottom);
}


void TabularBoxCellNumeric::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
      #if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
         m_spin->connect("value_entered", callable_mp(this, &TabularBoxCellNumeric::on_value_entered));
      #else
         m_spin->get_line_edit()->connect("text_submitted", callable_mp(this, &TabularBoxCellNumeric::on_spin_le_txt_submitted));
      #endif

         m_spin->get_line_edit()->connect("focus_entered", godot::create_custom_callable_function_pointer<TabularBoxCell>(this, &TabularBoxCell::notify_selected));
         m_spin->get_line_edit()->connect("gui_input", callable_mp(this, &TabularBoxCellNumeric::on_le_gui_input));
         m_spin->connect("value_changed", callable_mp(this, &TabularBoxCellNumeric::on_value_changed));
         m_spin->connect("minimum_size_changed", callable_mp(this, &TabularBoxCellNumeric::on_spin_min_size_changed));
      } break;
   }
}


void TabularBoxCellNumeric::_validate_property(godot::PropertyInfo& property) const
{
   const godot::StringName pname("floating_point");
   if (m_is_fixed && property.name == pname)
   {
      property.usage = godot::PROPERTY_USAGE_NONE;
   }
}


void TabularBoxCellNumeric::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("set_floating_point", "enable"), &TabularBoxCellNumeric::set_floating_point);
   ClassDB::bind_method(D_METHOD("is_floating_point"), &TabularBoxCellNumeric::is_floating_point);

   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "floating_point"), "set_floating_point", "is_floating_point");
}


void TabularBoxCellNumeric::check_theme()
{
#if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
   m_spin->begin_bulk_theme_override();
   m_spin->add_theme_stylebox_override("normal", get_style_empty());
   m_spin->add_theme_stylebox_override("focus", get_style_empty());
   /*
   REGISTER_CUSTOM_ICON(grabber, SpinSlider, gslider, m_theme_cache);
   REGISTER_CUSTOM_ICON(grabber_highlight, SpinSlider, gslider_hl, m_theme_cache);
   REGISTER_CUSTOM_ICON(up_down, SpinSlider, ico_updown, m_theme_cache);
   
   REGISTER_CUSTOM_COLOR(slider_color, SpinSlider, sl_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(slider_ratio, SpinSlider, sl_ratio_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(slider_read_only_color, SpinSlider, sl_ro_color, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(slider_height, SpinSlider, s_height, m_theme_cache);
   */
   m_spin->add_theme_color_override("font_color", get_font_color());
   m_spin->add_theme_color_override("font_selected_color", get_font_selected_color());
   m_spin->add_theme_color_override("selection_color", get_selection_color());
   m_spin->add_theme_color_override("caret_color", get_caret_color());
   m_spin->add_theme_font_override("font", get_font());
   m_spin->add_theme_font_size_override("font_size", get_font_size());
   m_spin->end_bulk_theme_override();
#else
   godot::LineEdit* le = m_spin->get_line_edit();
   le->begin_bulk_theme_override();
   le->add_theme_stylebox_override("normal", get_style_empty());
   le->add_theme_stylebox_override("focus", get_style_empty());
   le->add_theme_font_override("font", get_font());
   le->add_theme_font_size_override("font_size", get_font_size());
   le->add_theme_color_override("font_color", get_font_color());
   le->add_theme_color_override("font_selected_color", get_font_selected_color());
   le->add_theme_color_override("selection_color", get_selection_color());
   le->add_theme_color_override("caret", get_caret_color());
   le->end_bulk_theme_override();
#endif

   const float mtop = get_internal_margin_top();
   const float mbottom = get_internal_margin_bottom();

   m_spin->set_anchor_and_offset(godot::SIDE_LEFT, 0, get_internal_margin_left());
   m_spin->set_anchor_and_offset(godot::SIDE_TOP, 0, mtop);
   m_spin->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -get_internal_margin_right());
   m_spin->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -mbottom);

   const godot::Size2i spin_size = m_spin->_get_minimum_size();
   const int min_height = mtop + spin_size.height + mbottom;

   set_min_height(min_height);
}


void TabularBoxCellNumeric::apply_extra_settings(const godot::Dictionary& extra_settings)
{
   m_spin->set_step(extra_settings.get("step", m_floating_point ? 0.01f : 1.0f));
   const bool has_min_val = extra_settings.has("min_value");
   const bool has_max_val = extra_settings.has("max_value");
   const bool rounded = extra_settings.get("rounded", false);
#if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
   m_spin->set_use_min_value(has_min_val);
   m_spin->set_use_max_value(has_max_val);
   if (has_min_val)
   {
      m_spin->set_min_value(extra_settings["min_value"]);
   }
   if (has_max_val)
   {
      m_spin->set_max_value(extra_settings["max_value"]);
   }

   m_spin->set_rounded_values(rounded);
#else
   m_spin->set_allow_lesser(!has_min_val);
   m_spin->set_allow_greater(!has_max_val);
   if (has_min_val)
   {
      m_spin->set_min(extra_settings["min_value"]);
   }
   if (has_max_val)
   {
      m_spin->set_max(extra_settings["max_value"]);
   }

   m_spin->set_use_rounded_values(rounded);
#endif
}


void TabularBoxCellNumeric::assign_value(const godot::Variant& value)
{
   if (m_floating_point)
   {
      const double val = value;
      m_spin->set_value(val);
      m_assigned = val;
   }
   else
   {
      const int64_t val = value;
      m_spin->set_value(val);
      m_assigned = val;
   }
}


void TabularBoxCellNumeric::selected_changed(bool selected)
{
   godot::LineEdit* le = m_spin->get_line_edit();

   if (selected && !le->has_focus())
   {
      le->grab_focus();
   }
   else if (!selected && le->has_focus())
   {
      le->release_focus();
   }
}


void TabularBoxCellNumeric::set_floating_point(bool enable)
{
   /// NOTE: Not changing anything here within the m_spin Control. Reason is that this is relying on
   /// a subsequent call to "apply_extra_settings", which will take care of changing internal stuff
   m_floating_point = enable;
}


TabularBoxCellNumeric::TabularBoxCellNumeric(bool is_float)
{
   m_assigned = 0.0f;
   m_floating_point = is_float;

#if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
   m_spin = memnew(SpinSlider);
   m_spin->set_assign_on_right_click(false);
#else
   m_spin = memnew(godot::SpinBox);
   m_spin->get_line_edit()->set_mouse_filter(MOUSE_FILTER_PASS);
#endif

   m_spin->set_mouse_filter(MOUSE_FILTER_PASS);
   m_spin->get_line_edit()->set_context_menu_enabled(false);
   add_child(m_spin, false, INTERNAL_MODE_FRONT);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TabularBoxCellTexture
void TabularBoxCellTexture::on_btload_clicked()
{
   godot::PackedStringArray filters;
   filters.append("*.atlastex; ATLASTEX");
   filters.append("*.bmp; BMP");
   filters.append("*.curvetex; CURVETEX");
   filters.append("*.dds; DDS");   
   filters.append("*.exr; EXR");
   filters.append("*.hdr; HDR");
   filters.append("*.jpeg; JPEG");
   filters.append("*.jpg; JPG");
   filters.append("*.largetex; LARGETEX");
   filters.append("*.meshtex; MESHTEX");
   filters.append("*.pkm; PKM");
   filters.append("*.png; PNG");
   filters.append("*.pvr; PVR");
   filters.append("*.res; RES");
   filters.append("*.svg; SVG");
   filters.append("*.svgz; SVGZ");
   filters.append("*.tex; TEX");
   filters.append("*.tga; TGA");
   filters.append("*.tres; TRES");
   filters.append("*.webp; WEBP");

   request_file_dialog("Open texture", filters, callable_mp(this, &TabularBoxCellTexture::file_selected));
}


void TabularBoxCellTexture::on_btclear_clicked()
{
   assign_value("");
   notify_value_changed("");
}


void TabularBoxCellTexture::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_btload->connect("pressed", callable_mp(this, &TabularBoxCellTexture::on_btload_clicked));
         m_btclear->connect("pressed", callable_mp(this, &TabularBoxCellTexture::on_btclear_clicked));
      } break;

      case NOTIFICATION_DRAW:
      {
         const int32_t x = godot::Math::round((get_size().x - TEX_DIM) * 0.5);
         const int32_t y = get_internal_margin_top();
         const godot::Size2i constraints(TEX_DIM, TEX_DIM);

         godot::Ref<godot::Texture2D> tex = m_texture.is_valid() ? m_texture : get_icon_no_texture();

         draw_texture_rect(tex, godot::Rect2(godot::Vector2(x, y), ExtPackUtils::get_draw_rect_size(tex, constraints)), false);
         
      } break;
   }
}


void TabularBoxCellTexture::check_theme()
{
   const float mleft = get_internal_margin_left();
   const float mtop = get_internal_margin_top();
   const float mright = get_internal_margin_right();
   const float mbottom = get_internal_margin_bottom();
   const int32_t separation = get_theme_separation();

   apply_button_style(m_btload);

   m_btclear->begin_bulk_theme_override();
   m_btclear->add_theme_stylebox_override("normal", get_style_empty());
   m_btclear->add_theme_stylebox_override("hover", get_style_empty());
   m_btclear->add_theme_stylebox_override("pressed", get_style_empty());
   m_btclear->add_theme_stylebox_override("focus", get_style_empty());
   m_btclear->end_bulk_theme_override();
   m_btclear->set_button_icon(get_icon_trash());

   const float bth = get_button_height();
   const float toffset = bth + mbottom;
   
   ExtPackUtils::set_control_offsets(m_btload, mleft, -toffset, -(mright + separation + bth), -mbottom);
   ExtPackUtils::set_control_offsets(m_btclear, -(bth + mright), -toffset, -mright, -mbottom);

   set_min_height(mtop + separation + TEX_DIM + separation + bth + mbottom);
}


void TabularBoxCellTexture::assign_value(const godot::Variant& value)
{
   const godot::String path = value;

   godot::ResourceLoader* loader = godot::ResourceLoader::get_singleton();

   if (path.is_empty())
   {
      m_btload->set_text("...");
      /// FIXME: Use localization system here
      m_btload->set_tooltip_text("Load texture resource.");
      m_btclear->hide();
      m_texture.unref();
   }
   else if (!loader->exists(path))
   {
      m_btload->set_text(godot::vformat("!%s", path.get_file()));
      /// FIXME: Use localization system here
      m_btload->set_tooltip_text(godot::vformat("'%s' is not a Texture.\nClick to load a texture resource.", path));

      // Should this actually be hidden? The thing is, even though there is no valid texture with the given path,
      // it's still a value that is potentially stored within the data source
      m_btclear->hide();
      m_texture.unref();
   }
   else
   {
      m_btload->set_text(path.get_file());
      m_btload->set_tooltip_text(path);
      m_btclear->show();

      m_texture = loader->load(path);
   }

   queue_redraw();
}


void TabularBoxCellTexture::selected_changed(bool selected)
{
   if (selected)
   {
      m_btload->grab_focus();
   }
   else
   {
      m_btload->release_focus();
   }
}


bool TabularBoxCellTexture::_can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const
{
   if (data.get_type() != godot::Variant::DICTIONARY) { return false; }

   const godot::Dictionary ddata = data;

   const godot::String tp = ddata.get("type", "");
   if (tp != "files") { return false; }

   const godot::Array files = ddata.get("files", godot::Array());
   if (files.size() != 1)
   {
      // This is a single texture cell. Don't allow dragging multiple files
      return false;
   }

   const godot::String path = files[0];

   /// TODO: Check if there is a better way to verify if the dragged file is of a proper resource type that don't
   /// rely on loading it first
   godot::Ref<godot::Texture2D> tex = godot::ResourceLoader::get_singleton()->load(path);

   return tex.is_valid();
}


void TabularBoxCellTexture::_drop_data(const godot::Vector2& pos, const godot::Variant& data)
{
   const godot::Dictionary ddata = data;
   const godot::Array files = ddata.get("files", godot::Array());
   const godot::String path = files[0];

   notify_value_changed(path);
}


void TabularBoxCellTexture::file_selected(const godot::String& path)
{
   assign_value(path);
   notify_value_changed(path);
}


TabularBoxCellTexture::TabularBoxCellTexture()
{
   m_btload = memnew(godot::Button);
   m_btload->set_text_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
   m_btload->set_text("...");
   m_btload->set_mouse_filter(MOUSE_FILTER_PASS);
   add_child(m_btload, false, INTERNAL_MODE_BACK);

   ExtPackUtils::set_control_anchors(m_btload, 0, 1, 1, 1);

   m_btclear = memnew(godot::Button);
   /// FIXME: Use localization system for this text
   m_btclear->set_tooltip_text("Clear texture");
   m_btclear->set_mouse_filter(MOUSE_FILTER_PASS);
   m_btclear->hide();
   m_btclear->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
   add_child(m_btclear, false, INTERNAL_MODE_BACK);

   ExtPackUtils::set_control_anchors(m_btclear, 1, 1, 1, 1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TabularBoxCellMultilineString
void TabularBoxCellMultilineString::on_text_changed()
{
   const int32_t lc = m_txt_edit->get_line_count();
   if (lc != m_line_count)
   {
      const int32_t lh = m_txt_edit->get_line_height();
      m_line_count = lc;
      const int32_t mh = godot::Math::max<int>(lh, lh * m_line_count);
      set_min_height(mh + get_internal_margin_top() + get_internal_margin_bottom());
   }
}


void TabularBoxCellMultilineString::on_txt_unfocus()
{
   const godot::String txt = m_txt_edit->get_text();
   if (m_assigned != txt)
   {
      notify_value_changed(txt);
   }
}


void TabularBoxCellMultilineString::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_txt_edit->connect("text_changed", callable_mp(this, &TabularBoxCellMultilineString::on_text_changed));
         m_txt_edit->connect("focus_exited", callable_mp(this, &TabularBoxCellMultilineString::on_txt_unfocus));
      } break;
   }
}


void TabularBoxCellMultilineString::check_theme()
{
   const float mleft = get_internal_margin_left();
   const float mtop = get_internal_margin_top();
   const float mright = get_internal_margin_right();
   const float mbottom = get_internal_margin_bottom();

   m_txt_edit->begin_bulk_theme_override();
   m_txt_edit->add_theme_stylebox_override("normal", get_style_empty());
   m_txt_edit->add_theme_stylebox_override("focus", get_style_empty());
   m_txt_edit->add_theme_color_override("caret_color", get_caret_color());
   m_txt_edit->add_theme_color_override("font_color", get_font_color());
   m_txt_edit->add_theme_color_override("font_selected_color", get_font_selected_color());
   m_txt_edit->add_theme_color_override("selection_color", get_selection_color());
   m_txt_edit->add_theme_font_override("font", get_font());
   m_txt_edit->add_theme_font_size_override("font_size", get_font_size());
   m_txt_edit->end_bulk_theme_override();

   m_txt_edit->set_anchor_and_offset(godot::SIDE_LEFT, 0, mleft);
   m_txt_edit->set_anchor_and_offset(godot::SIDE_TOP, 0, mtop);
   m_txt_edit->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -mright);
   m_txt_edit->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -mbottom);
}


void TabularBoxCellMultilineString::assign_value(const godot::Variant& value)
{
   if (value.get_type() != godot::Variant::NIL)
   {
      const godot::String strval = value;
      m_txt_edit->set_text(strval);
   }
   else
   {
      m_txt_edit->set_text("");
   }
   m_assigned = m_txt_edit->get_text();
   m_line_count = m_txt_edit->get_line_count();

   const int32_t lh = m_txt_edit->get_line_height();
   const int32_t mh = godot::Math::max<int>(lh, lh * m_line_count);
   set_min_height(mh + get_internal_margin_top() + get_internal_margin_bottom());
}


void TabularBoxCellMultilineString::selected_changed(bool selected)
{
   if (selected && !m_txt_edit->has_focus())
   {
      m_txt_edit->grab_focus();
   }
   else if (!selected && m_txt_edit->has_focus())
   {
      m_txt_edit->release_focus();
   }
}


TabularBoxCellMultilineString::TabularBoxCellMultilineString()
{
   m_line_count = 0;

   m_txt_edit = memnew(godot::TextEdit);
   m_txt_edit->set_context_menu_enabled(false);
   m_txt_edit->set_mouse_filter(MOUSE_FILTER_PASS);
   add_child(m_txt_edit, false, INTERNAL_MODE_BACK);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// TabularBoxCellRowNumber
void TabularBoxCellRowNumber::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_DRAW:
      {
         if (!m_theme_cache) { return; }

         const godot::Size2i size = get_size();

         draw_style_box(m_theme_cache->row_number, godot::Rect2(godot::Vector2(), size));

         int32_t x = m_theme_cache->header->get_margin(godot::SIDE_LEFT);

         if (m_draw_chk)
         {
            godot::Ref<godot::Texture2D> tex = m_selected ? m_theme_cache->checked : m_theme_cache->unchecked;

            const int32_t y = (size.height - tex->get_height()) * 0.5;

            draw_texture_rect(tex, godot::Rect2(godot::Vector2(x, y), tex->get_size()), false);

            x += tex->get_width() + m_theme_cache->separation;
         }

         if (m_draw_num)
         {
            godot::Ref<godot::Font> font = m_theme_cache->header_font;
            const int32_t font_size = m_theme_cache->header_font_size;

            // This "cell" is added into a VBoxContainer in row order. So using get_index() should match its row index.
            // The idea here is to display the row "number", which is '1' based instead of '0' based like index.
            const int64_t num = get_index() + 1;
            const godot::String dstr = godot::vformat("%s", num);

            // Remaining width for the string
            const int32_t rem_width = size.width - x - m_theme_cache->header->get_margin(godot::SIDE_RIGHT);
            const int32_t stry = Internals::get_text_vertical_center(font, font_size, size.height);

            draw_string(font, godot::Vector2(x, stry), dstr, m_num_align, rem_width, font_size, m_theme_cache->header_text);
         }
      } break;

      case NOTIFICATION_LOCAL_TRANSFORM_CHANGED:
      {
         queue_redraw();
      } break;

   }
}


void TabularBoxCellRowNumber::_bind_methods()
{
   using namespace godot;

   ADD_SIGNAL(MethodInfo("selected_changed", PropertyInfo(Variant::INT, "row_index"), PropertyInfo(Variant::BOOL, "selected")));
}


void TabularBoxCellRowNumber::_gui_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventMouseButton> mb = evt;
   if (mb.is_valid() && mb->is_pressed())
   {
      if (mb->get_button_index() == godot::MOUSE_BUTTON_LEFT)
      {
         toggle_selected();
         emit_signal("selected_changed", get_index(), m_selected);
      }
   }
}


void TabularBoxCellRowNumber::set_draw_chk(bool enable)
{
   m_draw_chk = enable;
   if (is_inside_tree())
   {
      queue_redraw();
   }
}


void TabularBoxCellRowNumber::set_draw_num(bool enable)
{
   m_draw_num = enable;
   if (is_inside_tree())
   {
      queue_redraw();
   }
}


void TabularBoxCellRowNumber::set_num_align(godot::HorizontalAlignment align)
{
   m_num_align = align;
   if (is_inside_tree())
   {
      queue_redraw();
   }
}


void TabularBoxCellRowNumber::set_selected(bool on)
{
   m_selected = on;
   if (is_inside_tree())
   {
      queue_redraw();
   }
}


void TabularBoxCellRowNumber::toggle_selected()
{
   m_selected = !m_selected;

   /// NOTE: Not testing if inside the tree because this function is meant to be called when this cell is already drawn on screen
   queue_redraw();
}


TabularBoxCellRowNumber::TabularBoxCellRowNumber(TBoxThemeCache* theme)
{
   set_notify_local_transform(true);

   m_theme_cache = theme;
   m_draw_chk = true;
   m_draw_num = true;
   m_num_align = godot::HORIZONTAL_ALIGNMENT_LEFT;
   m_selected = false;
}


#endif  //TABULAR_BOX_DISABLED
