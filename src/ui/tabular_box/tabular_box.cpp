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


#include "tabular_box.h"
#include "tabular_box_cell.h"


#include "../custom_theme.h"
#include "../../internal.h"

#include <godot_cpp/classes/file_dialog.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_scroll_bar.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>

#include <godot_cpp/templates/hash_set.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


// The next two icons where taken from 'scene/theme/icons' directory within Godot source code, converted to PNG with a third party app
// then converted into Base64 string by Godot itself
static const char* ICON_CHECKED = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kb9LQlEUxz9qYWSRUENDg4Q1aZiB1NKg9AuqwQyyWvT5K/DH4z0jpDVoDQqiln4N9RfUGjQHQVEE0exc1FLxOk8FJfJczj2f+733HO49F6yRrJLTW3yQyxe18FTQtRRddtnL2HDgxEtbTNHVuYXJCE3t8xGLGe+9Zq3m5/41RyKpK2BpEx5XVK0oPC08u1FUTd4T7lEysYTwhbBHkwsKP5h6vMplk9NV/jZZi4RDYHUKu9INHG9gJaPlhOXluHPZdaV2H/MlHcn84oLEfvE+dMJMEcTFDBOECDDMmMwB6Y6fIVnRJN9XyZ+nILmKzColNNZIk6GIR9R1qZ6UmBI9KSNLyez/377qqRF/tXpHEFpfDeN9AOy78LNjGF8nhvFzCrYXuM7X8wvHMPoh+k5dcx9B1xZc3tS1+D5cbUPvsxrTYhXJJm5NpeDtHDqj0H0H7SvVntX2OXuCyKZ81S0cHMKgnO9a/QVPKGfbDrZnPAAAAAlwSFlzAAALEwAACxMBAJqcGAAAAS5JREFUOI2l0zFrwkAYxvH/SdASKDROmnyBDoUWXLJYcOg3EHfr7jeyQ7JkyJK5Q0H3gFDqJCFDpmDFIaG0selQQ89BL9Ab797f894Ld/DPJQDKsrSACdAHNIUpgAUwE0IkjcPmBBjUwBxqBsAjQBXQr3PdzWYjN7iXA5SdgyBo27ZtB0HQlk3jjDnC0+n0Js9zzXVdUz5TBlS4KIpGr9d7dxzn7WTAfr8XURS1TmHP8151Xf+WjSbj8Xh8HYbhled5y/V6faHCRwFpmmqr1epyu922RqPRXZZlmgofjdDpdL5831+appnvdrtmHQx/L/Gl2ojjuDUcDm+73e6HEgsxqAKe5XGSJGkahlGcw0AhhHioRljIJ5ZlfSowwByp6wwo+X2edT7THHhS1NVbP0M2kECdFuOWAAAAAElFTkSuQmCC";
static const char* ICON_UNCHECKED = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kb9LQlEUxz9qYWSRUENDg4Q1aZiB1NKg9AuqwQyyWvT5K/DH4z0jpDVoDQqiln4N9RfUGjQHQVEE0exc1FLxOk8FJfJczj2f+733HO49F6yRrJLTW3yQyxe18FTQtRRddtnL2HDgxEtbTNHVuYXJCE3t8xGLGe+9Zq3m5/41RyKpK2BpEx5XVK0oPC08u1FUTd4T7lEysYTwhbBHkwsKP5h6vMplk9NV/jZZi4RDYHUKu9INHG9gJaPlhOXluHPZdaV2H/MlHcn84oLEfvE+dMJMEcTFDBOECDDMmMwB6Y6fIVnRJN9XyZ+nILmKzColNNZIk6GIR9R1qZ6UmBI9KSNLyez/377qqRF/tXpHEFpfDeN9AOy78LNjGF8nhvFzCrYXuM7X8wvHMPoh+k5dcx9B1xZc3tS1+D5cbUPvsxrTYhXJJm5NpeDtHDqj0H0H7SvVntX2OXuCyKZ81S0cHMKgnO9a/QVPKGfbDrZnPAAAAAlwSFlzAAALEwAACxMBAJqcGAAAAIVJREFUOI3t0zEKwjAYhuFH2909N5CCgwdw8MLewIKXkOKe3V1blwQyCLVxcfDd8pHvzc8P4UtWEELY4IAtmpnOE1dcYoz3NoVH7BY8vEeL0zoF3YJypoMsmBv7HU0pqOYv+CXBWNEdS8FQIRhKQY8bpg+KU7p7Jv3GTAihNb+XMcb4yIcXHhgWr1Ext1cAAAAASUVORK5CYII=";
// These base64 strings were converted from .png files by Godot itself
static const char* ICON_DOWN_ARROW = "iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAYAAADED76LAAABgmlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kblLQ0EQhz+TaEQjEbWwsAgSrYx4QNDGIsEL1CKJ4NUkL5eQ4/FeggRbwTagINp4FfoXaCtYC4KiCGInWCvaaHjOS4SImFlm59vf7gy7s2AJpZS0bhuAdCanBSZ9roXFJZf9GRtO2hilIazo6mxwIkRN+7ijzow3HrNW7XP/WnM0pitQ1yg8pqhaTnhKeGYtp5q8LdyhJMNR4VPhPk0uKHxr6pEKv5icqPCXyVoo4AdLq7Ar8Ysjv1hJamlheTnudCqv/NzHfIkjlpkPSuwW70InwCQ+XEwzjh8vg9IXrwwPQ/TLihr5A+X8ObKSq8isUkBjlQRJcvSJmpfqMYlx0WMyUhTM/v/tqx4fHqpUd/ig/skw3nrAvgWlomF8HhpG6Qisj3CRqeZnD2DkXfRiVXPvg3MDzi6rWmQHzjeh80ENa+GyZBW3xOPwegIti9B+DU3LlZ797HN8D6F1+aor2N2DXjnvXPkGbfhn6bf3XWYAAAAJcEhZcwAACxMAAAsTAQCanBgAAABzSURBVBiVdY69DcJgFAPvkbRIr8kK2SCKKJiGDjYiGYifDTIDxUkMEJoPFAGx5MY+Sw61Bbb817MG9sC4AhxC3QAXoPsqb8AuANQOuAJRyhnoM/P+wdVBnYvP7zwWQANMZd1m5uPnkXpSjyuHQa3Vapm9ADBVMJyh9s/fAAAAAElFTkSuQmCC";
static const char* ICON_LEFT_ARROW = "iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAYAAADED76LAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz9mRsQIsZCUSVgZjVETG4uZ/CosZkYZbGbe/FLz4/XeSLJVtooSG78W/AVslbVSREp2yprYMD3nzUzNJHNu557P/d57TveeC5ZgSknrNhekMznNP+l1LIQWHXWv2OikFRfdYUVXZwMTQara1wM1ZrxzmrWqn/vXGqMxXYGaeuExRdVywlPCM2s51eRd4XYlGY4KnwsPaHJB4XtTjxT5zeREkX9M1oJ+H1hahB2JCo5UsJLU0sLycnrTqVWldB/zJfZYZj4gsUe8Cx0/k3hxMM04PjwMMSqzByduBmVFlXxXIX+OrOQqMquso7FCgiQ5BkRdleoxiXHRYzJSrJv9/9tXPT7sLla3e6H2xTA++qBuB/LbhvF9bBj5E7A+w1WmnJ89gpFP0bfLWu8hNG/CxXVZi+zB5RZ0PKlhLVyQrOKWeBzez6ApBG230LBU7Flpn9NHCG7IV93A/gH0y/nm5V+nHmgDSWyqPwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAGlJREFUGJVtj7ENg1AMBQ+EUESBfk2ZkejTpGYTFqBjAFZgIJTqBkCChh8hZFfPvucnG4JS26zLAH6BPvfVDdTACHyA93OrU1f1UOc7KwO9R3ehvtRJ/alNnhcPUwEMwJZSWsKky/h/8wRAcC2wp0yBpAAAAABJRU5ErkJggg==";
static const char* ICON_RIGHT_ARROW = "iVBORw0KGgoAAAANSUhEUgAAAAgAAAAICAYAAADED76LAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz9mRsQIsZCUSVgZjVETG4uZ/CosZkYZbGbe/FLz4/XeSLJVtooSG78W/AVslbVSREp2yprYMD3nzUzNJHNu557P/d57TveeC5ZgSknrNhekMznNP+l1LIQWHXWv2OikFRfdYUVXZwMTQara1wM1ZrxzmrWqn/vXGqMxXYGaeuExRdVywlPCM2s51eRd4XYlGY4KnwsPaHJB4XtTjxT5zeREkX9M1oJ+H1hahB2JCo5UsJLU0sLycnrTqVWldB/zJfZYZj4gsUe8Cx0/k3hxMM04PjwMMSqzByduBmVFlXxXIX+OrOQqMquso7FCgiQ5BkRdleoxiXHRYzJSrJv9/9tXPT7sLla3e6H2xTA++qBuB/LbhvF9bBj5E7A+w1WmnJ89gpFP0bfLWu8hNG/CxXVZi+zB5RZ0PKlhLVyQrOKWeBzez6ApBG230LBU7Flpn9NHCG7IV93A/gH0y/nm5V+nHmgDSWyqPwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAGVJREFUGJV1zzEKwmAQhNFRC5EU/nZCTmXjBXIisbLzIJ7IxncAIWkSCLJOs8vyzQybzMIxhbar/YKhgpaEEz64Yf8PemLEC/1vRZJ857mr3B3eeOCw3Dcr4JrknOTeWhurhPLNCZk2Nqw1s9PvAAAAAElFTkSuQmCC";
static const char* ICON_NOTE_X = "iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAABDklEQVRYR81XQRKDMAhMTn0JT+itz+/NJ+QlPbWDIzPRksDGaOJBnRF2F0iQxLBdKaWvvN/xJKLIPOvtbnIJkEXEUeQiYifgQxSW7csrpa6VeBOteM8QwiPD3glgIzbgi4X0ElHDLQroJSIn1zD/1oDlgNTFg6UuQo+jJcSLUdwFXgBNCOJb3YYIkAhBfcw+gAAitmofKNXVA+yx0fDNDHhS20q+/guQVqwRSXcToWgDgwQwyVFEnlaUHM5AqRxnuiacgVIWWqJvysDQEgxdhLWtdvk29BB4bJoaEQKM2LpacQsg6jPn7xiN4sxMMNdI1iPyYzYszLnG8uEHE2vSveL7+MOpRIVMRj0yIcfzH3ZuK/DlVXYvAAAAAElFTkSuQmCC";
static const char* ICON_TRASH_BIN = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAY0lEQVQ4T2NkoBAwwvT/////PylmMTIygvWCCZhmmCAhg5DVoxhASCO6PMhC6hiA7g10LyHz0eUwAhHkqlEDyAwDWEwQE4jICQ4eC1QzgJjUiNMFyAkKl0Ho+QXFC8TYjq4GAN6MnBHPWXK7AAAAAElFTkSuQmCC";





void TabularBox::save_data_source()
{
   if (!get_autosave_source()) { return; }

   const godot::String res_path = m_data_source->get_path();

   if (res_path.begins_with("res://") && res_path.find("::") == -1)
   {
      const godot::Error err = godot::ResourceSaver::get_singleton()->save(m_data_source, res_path);
   }
}


void TabularBox::calculate_min_sizes()
{
   godot::Ref<godot::Font> font = m_theme_cache.header_font;
   const int32_t font_height = font->get_height(m_theme_cache.header_font_size);
   const int32_t icon_height = godot::Math::max<int32_t>(m_theme_cache.left_arrow->get_height(), m_theme_cache.right_arrow->get_height());

   // Button vertical margins
   const int32_t bt_normal_vmargin = m_theme_cache.button_normal->get_margin(godot::SIDE_TOP) + m_theme_cache.button_normal->get_margin(godot::SIDE_BOTTOM);
   const int32_t bt_pressed_vmargin = m_theme_cache.button_pressed->get_margin(godot::SIDE_TOP) + m_theme_cache.button_pressed->get_margin(godot::SIDE_BOTTOM);
   const int32_t bt_hover_vmargin = m_theme_cache.button_pressed->get_margin(godot::SIDE_TOP) + m_theme_cache.button_hover->get_margin(godot::SIDE_BOTTOM);
   const int32_t bt_vmargin = Internals::vmax<int32_t>(bt_normal_vmargin, bt_pressed_vmargin, bt_hover_vmargin);

   const int32_t bt_size = godot::Math::max<int32_t>(bt_vmargin + icon_height, bt_vmargin + font_height);

   // godot::LineEdit uses the "M" character to determine its minimum width
   const float em_space = font->get_char_size('M', m_theme_cache.header_font_size).x;


   m_min_size.title_buttons = godot::Size2i(bt_size, bt_size);
   m_min_size.title_size = godot::Size2i(godot::Math::round(em_space * MIN_TITLE_CHARS), bt_size);
   m_min_size.title_size.width += m_theme_cache.header->get_margin(godot::SIDE_LEFT) + m_theme_cache.header->get_margin(godot::SIDE_RIGHT);
   m_min_size.title_size.height += m_theme_cache.header->get_margin(godot::SIDE_TOP) + m_theme_cache.header->get_margin(godot::SIDE_BOTTOM);

   m_min_size.row_num_margins.width = m_theme_cache.row_number->get_margin(godot::SIDE_LEFT) + m_theme_cache.row_number->get_margin(godot::SIDE_RIGHT);
   m_min_size.row_num_margins.height = m_theme_cache.row_number->get_margin(godot::SIDE_TOP) + m_theme_cache.row_number->get_margin(godot::SIDE_BOTTOM);

   m_min_size.row_checkbox.width = godot::Math::max<int32_t>(m_theme_cache.checked->get_width(), m_theme_cache.unchecked->get_width());
   m_min_size.row_checkbox.height = godot::Math::max<int32_t>(m_theme_cache.checked->get_height(), m_theme_cache.unchecked->get_height());

   m_min_size.row_number.width = godot::Math::round(em_space * 3);
   m_min_size.row_number.height = font_height;

   m_min_size.final_rownum.height = godot::Math::max<int32_t>(m_min_size.row_number.height, m_min_size.row_checkbox.height) + m_min_size.row_num_margins.height;
}


void TabularBox::calculate_sizes()
{
   // Calculate (and cache) the total height of rows
   m_cell_data.total_size.height = 0;
   for (const RowData& row : m_cell_data.row)
   {
      m_cell_data.total_size.height += row.height;
   }

   // Apply minimum sizes into the headers - while also calculating total cell width
   m_cell_data.total_size.width = 0;
   for (TabularBoxHeader* header : m_header_data.array)
   {
      setup_header(header);

      m_cell_data.total_size.width += header->get_size().x;
   }
}


void TabularBox::calculate_available_height()
{
   // Then calculate the available height for the cells. As explained in the .h file, this information is required to limit the
   // heights of the column sizers.
   const float mtop = m_theme_cache.background->get_margin(godot::SIDE_TOP);
   const float mbottom = m_theme_cache.background->get_margin(godot::SIDE_BOTTOM);
   const float hbar = m_hor_bar->is_visible() ? m_hor_bar->get_combined_minimum_size().y : 0;
   const int32_t subtract = mtop + m_min_size.title_size.height + mbottom + mbottom + hbar;
   m_cell_data.available_height = get_size().y - subtract;

   for (TabularBoxHeader* header : m_header_data.array)
   {
      header->set_max_sizer_height(m_cell_data.available_height);
      header->set_extra_sizer_height(m_cell_data.total_size.height);
   }
}


void TabularBox::calculate_rownum_width()
{
   // Assume the "row number" are is not necessary
   bool has_rnum = false;

   m_min_size.final_rownum.width = 0;
   if (get_show_row_checkboxes())
   {
      m_min_size.final_rownum.width += m_min_size.row_checkbox.width;
      has_rnum = true;
   }
   if (get_show_row_numbers())
   {
      if (has_rnum)
      {
         // If here then show cehckbox is true. Add some separation between checked/unchecked icon and the number string
         m_min_size.final_rownum.width += m_theme_cache.separation;
      }

      m_min_size.final_rownum.width += m_min_size.row_number.width;

      has_rnum = true;
   }

   if (has_rnum)
   {
      m_min_size.final_rownum.width += m_theme_cache.row_number->get_margin(godot::SIDE_LEFT) + m_theme_cache.row_number->get_margin(godot::SIDE_RIGHT);
   }
}


void TabularBox::apply_styling()
{
   m_rownum_data.bt_menu->begin_bulk_theme_override();
   m_rownum_data.bt_menu->add_theme_stylebox_override("normal", m_theme_cache.button_normal);
   m_rownum_data.bt_menu->add_theme_stylebox_override("hover", m_theme_cache.button_hover);
   m_rownum_data.bt_menu->add_theme_stylebox_override("pressed", m_theme_cache.button_pressed);
   m_rownum_data.bt_menu->add_theme_stylebox_override("focus", m_theme_cache.stl_empty);
   m_rownum_data.bt_menu->end_bulk_theme_override();

   m_rownum_data.bt_menu->set_button_icon(m_theme_cache.down_arrow);

   // Ok, this is more about layout, however it doesn't change unless the style does too. So, applying this here
   m_rownum_data.bt_menu->set_anchor_and_offset(godot::SIDE_LEFT, 0, m_theme_cache.row_number->get_margin(godot::SIDE_LEFT));
   m_rownum_data.bt_menu->set_anchor_and_offset(godot::SIDE_TOP, 0, m_theme_cache.row_number->get_margin(godot::SIDE_TOP));
   m_rownum_data.bt_menu->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -m_theme_cache.row_number->get_margin(godot::SIDE_RIGHT));
   m_rownum_data.bt_menu->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -m_theme_cache.row_number->get_margin(godot::SIDE_BOTTOM));

   m_ver_bar->begin_bulk_theme_override();
   m_ver_bar->add_theme_stylebox_override("scroll", m_theme_cache.v_scrollbar);
   m_ver_bar->add_theme_stylebox_override("scroll_focus", m_theme_cache.focus);
   m_ver_bar->add_theme_stylebox_override("grabber", m_theme_cache.scrollbar_grabber);
   m_ver_bar->add_theme_stylebox_override("grabber_highlight", m_theme_cache.scrollbar_highlight);
   m_ver_bar->add_theme_stylebox_override("grabber_pressed", m_theme_cache.scrollbar_grabber_pressed);
   m_ver_bar->end_bulk_theme_override();

   m_hor_bar->begin_bulk_theme_override();
   m_hor_bar->add_theme_stylebox_override("scroll", m_theme_cache.h_scrollbar);
   m_hor_bar->add_theme_stylebox_override("scroll_focus", m_theme_cache.focus);
   m_hor_bar->add_theme_stylebox_override("grabber", m_theme_cache.scrollbar_grabber);
   m_hor_bar->add_theme_stylebox_override("grabber_highlight", m_theme_cache.scrollbar_highlight);
   m_hor_bar->add_theme_stylebox_override("grabber_pressed", m_theme_cache.scrollbar_grabber_pressed);
   m_hor_bar->end_bulk_theme_override();
}


bool TabularBox::check_scroll_bars()
{
   const godot::Vector2 hmin = m_hor_bar->get_combined_minimum_size();
   const godot::Vector2 vmin = m_ver_bar->get_combined_minimum_size();

   godot::Size2i size = get_size();

   // First calculate available size - must take margins and the "rownumbers" into account
   const float margin_left = m_theme_cache.background->get_margin(godot::SIDE_LEFT);
   const float margin_top = m_theme_cache.background->get_margin(godot::SIDE_TOP);
   const float margin_right = m_theme_cache.background->get_margin(godot::SIDE_RIGHT);
   const float margin_bottom = m_theme_cache.background->get_margin(godot::SIDE_BOTTOM);

   size.width -= (margin_left + margin_right + m_min_size.final_rownum.width);
   size.height -= (margin_top + margin_bottom + m_min_size.title_size.height);

   // Determine if there is enough size
   bool need_vscroll = m_cell_data.total_size.height > size.height;
   bool need_hscroll = m_cell_data.total_size.width > size.width;

   // However if a scroll bar is needed the available size changes. This must be taken into account
   if (need_vscroll)
   {
      size.x -= vmin.x;

      // Available width has reduced. Maybe hscroll will be needed?
      need_hscroll = m_cell_data.total_size.width > size.width;
   }
   if (need_hscroll)
   {
      size.y -= hmin.y;

      // Aavailable height has reduced. Only redo the vertical verification if it was needed before
      if (!need_vscroll)
      {
         need_vscroll = m_cell_data.total_size.height > size.height;

         if (need_vscroll)
         {
            // OK, after reducing the available height because of the horizontal scroll bar, the vertical
            // scrolling is not required. Ensure the available height is properly reduced
            size.x -= vmin.x;
         }
      }
   }

   const bool ver_was_visible = m_ver_bar->is_visible();
   const bool hor_was_visible = m_hor_bar->is_visible();

   // Change visiblity of the scroll bars accordingly
   m_ver_bar->set_visible(need_vscroll || !get_autohide_vscroll_bar());
   m_hor_bar->set_visible(need_hscroll || !get_autohide_hscroll_bar());

   if (need_vscroll)
   {
      m_ver_bar->set_max(m_cell_data.total_size.height);
      m_ver_bar->set_page(size.height);
   }
   else
   {
      m_ver_bar->set_max(0);
      m_ver_bar->set_page(0);
   }

   if (need_hscroll)
   {
      m_hor_bar->set_max(m_cell_data.total_size.width);
      m_hor_bar->set_page(size.width);
   }
   else
   {
      m_hor_bar->set_max(0);
      m_hor_bar->set_page(0);
   }

   return (m_ver_bar->is_visible() != ver_was_visible || m_hor_bar->is_visible() != hor_was_visible);
}


void TabularBox::apply_layout()
{
   const float margin_left = m_theme_cache.background->get_margin(godot::SIDE_LEFT);
   const float margin_top = m_theme_cache.background->get_margin(godot::SIDE_TOP);
   const float margin_right = m_theme_cache.background->get_margin(godot::SIDE_RIGHT);
   const float margin_bottom = m_theme_cache.background->get_margin(godot::SIDE_BOTTOM);

   const godot::Vector2 hmin = m_hor_bar->is_visible() ? m_hor_bar->get_combined_minimum_size() : godot::Vector2();
   const godot::Vector2 vmin = m_ver_bar->is_visible() ? m_ver_bar->get_combined_minimum_size() : godot::Vector2();

   const float header_height = m_min_size.title_size.height;

   const bool has_rnum = get_show_row_checkboxes() || get_show_row_numbers();

   m_rownum_data.menu_area->set_visible(has_rnum);
   m_rownum_data.area->set_visible(has_rnum);

   const int32_t rnum_width = m_min_size.final_rownum.width;

   // Vertical anchor and offsets don't change regardless of right_to_left
   m_rownum_data.menu_area->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top);
   m_rownum_data.menu_area->set_anchor_and_offset(godot::SIDE_BOTTOM, 0, margin_top + header_height + margin_bottom);
   m_rownum_data.menu_area->set_anchor_and_offset(godot::SIDE_LEFT, 0, margin_left);
   m_rownum_data.menu_area->set_anchor_and_offset(godot::SIDE_RIGHT, 0, margin_left + rnum_width);

   m_header_data.area->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top);
   m_header_data.area->set_anchor_and_offset(godot::SIDE_BOTTOM, 0, margin_top + header_height + margin_bottom);
   m_rownum_data.area->set_anchor_and_offset(godot::SIDE_LEFT, 0, margin_left);
   m_rownum_data.area->set_anchor_and_offset(godot::SIDE_RIGHT, 0, margin_left + rnum_width);

   m_rownum_data.area->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top + header_height + margin_bottom);
   m_rownum_data.area->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -(margin_bottom + hmin.y));
   m_header_data.area->set_anchor_and_offset(godot::SIDE_LEFT, 0, margin_left + rnum_width);
   m_header_data.area->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -(margin_right + vmin.x));

   m_cell_data.area->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top + header_height + margin_bottom);
   m_cell_data.area->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -(margin_bottom + hmin.y));
   m_cell_data.area->set_anchor_and_offset(godot::SIDE_LEFT, 0, margin_left + rnum_width);
   m_cell_data.area->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -(margin_right + vmin.x));

   
   m_ver_bar->set_anchor_and_offset(godot::SIDE_TOP, 0, margin_top + header_height + margin_bottom);
   m_ver_bar->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -(margin_bottom + hmin.y));
   m_ver_bar->set_anchor_and_offset(godot::SIDE_LEFT, 1, -(margin_right + vmin.x));
   m_ver_bar->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -margin_right);
   
   m_hor_bar->set_anchor_and_offset(godot::SIDE_TOP, 1, -(margin_bottom + hmin.y));
   m_hor_bar->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -margin_bottom);
   m_hor_bar->set_anchor_and_offset(godot::SIDE_LEFT, 0, margin_left + rnum_width);
   m_hor_bar->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -(margin_right + vmin.x));
}


void TabularBox::internal_request_fdialog(const godot::String& title, const godot::PackedStringArray& filters, const godot::Callable& handler, const godot::String& dir)
{
   if (!m_file_dialog.dialog)
   {
      m_file_dialog.dialog = memnew(godot::FileDialog);
      m_file_dialog.dialog->set_file_mode(godot::FileDialog::FILE_MODE_OPEN_FILE);
      m_file_dialog.dialog->set_size(godot::Vector2(640, 480));
      add_child(m_file_dialog.dialog, false, INTERNAL_MODE_BACK);

      m_file_dialog.dialog->connect("canceled", callable_mp(this, &TabularBox::on_file_dialog_closed));
   }

   m_file_dialog.dialog->set_title(title);
   m_file_dialog.dialog->set_filters(filters);

   if (!dir.is_empty())
   {
      m_file_dialog.dialog->set_current_dir(dir);
   }

   m_file_dialog.selected_handler = handler;

   m_file_dialog.dialog->connect("file_selected", m_file_dialog.selected_handler, CONNECT_ONE_SHOT);

   m_file_dialog.dialog->popup_centered();
}



void TabularBox::set_rows_odd_even(bool request_redraw)
{
   bool is_odd = true;
   for (const RowData& row : m_cell_data.row)
   {
      if (!row.box->is_visible())
      {
         // This might happen if the row is filtered out.
         continue;
      }
      
      // As explained in the declaration of this function, not relying on row index because some of them might be filtered out.
      // Using meta allows for such functionality to be added without having to create a specialized Control just for rows
      row.box->set_meta("is_odd", is_odd);

      if (request_redraw)
      {
         for (TabularBoxCell* cell : row.cell)
         {
            cell->queue_redraw();
         }
      }

      is_odd = !is_odd;
   }
}


void TabularBox::setup_header(TabularBoxHeader* header)
{
   godot::Size2i min_size = m_min_size.title_size;

   if (header->needs_move_buttons())
   {
      min_size.width += (m_min_size.title_buttons.width + m_theme_cache.separation) * 2;
   }

   header->setup(&m_theme_cache, min_size);
   header->set_max_sizer_height(m_cell_data.available_height);
   header->set_extra_sizer_height(m_cell_data.total_size.height);
}


void TabularBox::create_header(const TabularDataSource::ColumnInfo& info, int64_t index)
{
   TabularBoxHeader* header = memnew(TabularBoxHeader);
   header->set_info(info);
   header->set_alignment(m_title_halign);
   setup_header(header);

   m_cell_data.total_size.width += header->get_minimum_size().x;

   m_header_data.box->add_child(header);
   if (index < 0 || index >= m_header_data.array.size())
   {
      m_header_data.array.append(header);
   }
   else
   {
      m_header_data.array.insert(index, header);

      m_header_data.box->move_child(header, index);
   }

   m_header_data.map[info.title] = header;
   header->connect("resized", callable_mp(this, &TabularBox::on_header_resized).bind(header));
   header->connect("move_left_clicked", callable_mp(this, &TabularBox::on_header_move_left).bind(header));
   header->connect("move_right_clicked", callable_mp(this, &TabularBox::on_header_move_right).bind(header));
   header->connect("title_entered", callable_mp(this, &TabularBox::on_header_renamed).bind(header));


   set_process_internal(true);
}


void TabularBox::remove_header(int64_t index)
{
   TabularBoxHeader* header = m_header_data.array[index];
   const godot::String title = header->get_title();

   m_cell_data.total_size.width -= header->get_minimum_size().x;

   // First remove the cells corresponding to the column
   for (RowData& row : m_cell_data.row)
   {
      row.cell[index]->queue_free();
      row.cell.remove_at(index);
      row.cell_map.erase(title);
   }

   // Then remove header from the UI and internal containers
   header->queue_free();
   m_header_data.array.remove_at(index);
   m_header_data.map.erase(title);

   set_process_internal(true);
}


void TabularBox::create_cell(const TabularDataSource::ColumnInfo& info, const godot::Variant& value, RowData& row, int64_t column_index)
{
   godot::String cell_class = info.cell_class;

   TabularBoxCell* cell = nullptr;

   if (!godot::ClassDB::can_instantiate(cell_class))
   {
      WARN_PRINT(vformat("Attempting to create cell for column '%s' but specified class name (%s) for it can't be instantiated.", info.title, cell_class));
      cell_class = "TabularBoxCellString";
   }

   if (godot::ClassDB::is_parent_class(cell_class, "TabularBoxCell"))
   {
      cell = godot::Object::cast_to<TabularBoxCell>(godot::ClassDB::instantiate(cell_class));
   }
   else
   {
      const bool is_global = GlobalClassCache::is_global_class(cell_class);
      const godot::StringName base = is_global ? GlobalClassCache::get_global_class_native_base(cell_class) : "";

      if (base == godot::StringName("TabularBoxCell"))
      {
         godot::Ref<godot::Script> script = godot::ResourceLoader::get_singleton()->load(GlobalClassCache::get_global_class_path(cell_class));

         if (!script.is_valid())
         {
            WARN_PRINT(godot::vformat("Attempting to create cell of '%s' for column '%s'. However failed to load its script.", cell_class, info.title));
         }
         else if (!script->can_instantiate())
         {
            WARN_PRINT(godot::vformat("Attempting to create cell of '%s for column '%s'. However can't create instance of this script'", cell_class, info.title));
         }
         else
         {
            cell = memnew(TabularBoxCell);
            cell->set_script(script);
         }
      }
      else
      {
         WARN_PRINT(godot::vformat("Creating cell for column '%s' however specified class '%s' does not seem to inherit from 'TabularBoxCell'.", info.title, cell_class));
      }
   }

   if (!cell)
   {
      // Fall back to a String Cell. Something must be created otherwise this will completely break if attempting to select a different type
      // on the same column.
      cell = memnew(TabularBoxCellString);
   }


   cell->setup_theme(&m_theme_cache);

   row.box->add_child(cell);
   if (column_index < row.box->get_child_count() - 1 && column_index >= 0)
   {
      row.box->move_child(cell, column_index);
      row.cell.insert(column_index, cell);
   }
   else
   {
      row.cell.append(cell);
   }
   row.cell_map[info.title] = cell;

   // Use the header to determine the width of the cell
   TabularBoxHeader* header = m_header_data.map[info.title];

   const int width = godot::Math::max<int>(header->get_minimum_size().x, header->get_size().x);

   if (row.box->get_index() == 0)
   {
      // This cell is in the very first row, so perform "initial shared setup"
      cell->setup_shared(m_extra_parent);
   }
   else
   {
      m_cell_data.row[0].cell[column_index]->share_with(cell);
   }

   cell->apply_extra_settings(info.extra_settings);

   cell->assign_value(value);
   cell->set_custom_minimum_size(godot::Vector2(width, 0));

   cell->connect("value_changed", callable_mp(this, &TabularBox::on_cell_value_changed).bind(header, row.box));
   cell->connect("selected", callable_mp(this, &TabularBox::select_cell).bind(cell));
   cell->connect("select_next", callable_mp(this, &TabularBox::on_cell_select_next).bind(cell));
   cell->connect("unselect", callable_mp(this, &TabularBox::on_cell_unselect));

   if (cell->requires_file_dialog())
   {
      cell->set_dialog_requester(callable_mp(this, &TabularBox::internal_request_fdialog).bind(godot::String()));
   }
}


void TabularBox::create_row(const godot::Dictionary& values, int64_t index)
{
   // Make sure the scripted class list is up to date
   GlobalClassCache::scan_global_classes();

   int64_t use_index = index;

   {
      // First create the row entry and add into the array container.
      RowData rdata;

      if (index < 0 || index >= m_cell_data.row.size())
      {
         use_index = m_cell_data.row.size();
         m_cell_data.row.append(rdata);
      }
      else
      {
         m_cell_data.row.insert(index, rdata);
      }
   }

   // Obtain a reference to the stored row data and use it to finish creating cell instances and filling other internal data
   {
      RowData& rdata = m_cell_data.row.ptrw()[use_index];
      rdata.box = memnew(godot::HBoxContainer);
      rdata.box->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      rdata.box->add_theme_constant_override("separation", 0);
      rdata.box->connect("mouse_entered", callable_mp(this, &TabularBox::on_mouse_enter_row).bind(rdata.box));
      rdata.box->connect("mouse_exited", callable_mp(this, &TabularBox::on_mouse_leave_row).bind(rdata.box));
      rdata.box->connect("minimum_size_changed", callable_mp(this, &TabularBox::on_row_min_size_changed).bind(rdata.box));
      // Just to have a value - this will be changed later
      rdata.box->set_meta("is_odd", true);
      m_cell_data.box->add_child(rdata.box);

      const int64_t csize = m_header_data.array.size();
      for (int64_t i = 0; i < csize; i++)
      {
         TabularBoxHeader* header = m_header_data.array[i];
         const TabularDataSource::ColumnInfo& info = header->get_info();
         const godot::Variant value = values.get(info.title, nullptr);
         create_cell(info, value, rdata, i);
      }


      if (use_index != rdata.box->get_index())
      {
         m_cell_data.box->move_child(rdata.box, use_index);
      }
      
      rdata.height = rdata.box->get_combined_minimum_size().y;

      // Update the total height of rows
      m_cell_data.total_size.height += rdata.height;

      // Create the row number
      TabularBoxCellRowNumber* rownum = memnew(TabularBoxCellRowNumber(&m_theme_cache));
      rownum->set_draw_chk(get_show_row_checkboxes());
      rownum->set_draw_num(get_show_row_numbers());
      rownum->set_num_align(m_rownum_data.align);
      rownum->set_h_size_flags(SIZE_EXPAND_FILL);
      rownum->set_custom_minimum_size(godot::Vector2(0, rdata.height));
      m_rownum_data.box->add_child(rownum);
      m_rownum_data.array.append(rownum);

      rownum->connect("selected_changed", callable_mp(this, &TabularBox::on_row_selected_changed));
   }

   /// TODO: Update only the rows above "index"
   check_row_selection();
}


void TabularBox::remove_row(int64_t index)
{
   // Using this unnamed scope just to ensure the RowData reference will not be used after it's removed from the array
   {
      const RowData& rdata = m_cell_data.row[index];
      m_cell_data.total_size.height -= rdata.height;
      m_cell_data.box->remove_child(rdata.box);
      memdelete(rdata.box);
   }
   m_cell_data.row.remove_at(index);

   {
      TabularBoxCellRowNumber* rownum = m_rownum_data.array[index];
      m_rownum_data.array.remove_at(index);
      m_rownum_data.box->remove_child(rownum);
      memdelete(rownum);
   }
}


void TabularBox::select_cell(TabularBoxCell* cell)
{
   if (m_selected.cell == cell) { return; }

   if (m_selected.cell)
   {
      m_selected.cell->set_selected(false);
   }

   m_selected.cell = cell;
   if (cell)
   {
      cell->set_selected(true);
   }
}


void TabularBox::clear()
{
   for (TabularBoxHeader* header : m_header_data.array)
   {
      m_header_data.box->remove_child(header);
      memdelete(header);
   }
   m_header_data.array.clear();
   m_header_data.map.clear();

   for (RowData& row : m_cell_data.row)
   {
      m_cell_data.box->remove_child(row.box);
      memdelete(row.box);
   }
   m_cell_data.row.clear();
   
   m_cell_data.total_size.height = 0;
   m_selected.cell = nullptr;

   for(TabularBoxCellRowNumber* rnum : m_rownum_data.array)
   {
      m_rownum_data.box->remove_child(rnum);
      memdelete(rnum);
   }
   m_rownum_data.array.clear();
}



void TabularBox::check_ds_signals(bool disconnect)
{
   if (!m_data_source.is_valid())
   {
      return;
   }

   const godot::Callable on_column_inserted = callable_mp(this, &TabularBox::on_column_inserted);
   const godot::Callable on_column_removed = callable_mp(this, &TabularBox::on_column_removed);
   const godot::Callable on_column_moved = callable_mp(this, &TabularBox::on_column_moved);
   const godot::Callable on_column_renamed = callable_mp(this, &TabularBox::on_column_title_changed);
   const godot::Callable on_column_rename_rejected = callable_mp(this, &TabularBox::on_column_title_change_rejected);
   const godot::Callable on_type_changed = callable_mp(this, &TabularBox::on_type_changed);
   const godot::Callable on_row_inserted = callable_mp(this, &TabularBox::on_row_inserted);
   const godot::Callable on_row_removed = callable_mp(this, &TabularBox::on_row_removed);
   const godot::Callable on_row_moved = callable_mp(this, &TabularBox::on_row_moved);
   const godot::Callable on_value_changed = callable_mp(this, &TabularBox::on_value_changed);
   const godot::Callable on_value_change_rejected = callable_mp(this, &TabularBox::on_value_change_rejected);
   const godot::Callable on_sort_changed = callable_mp(this, &TabularBox::on_sorted);
   const godot::Callable on_filter_changed = callable_mp(this, &TabularBox::on_filter_changed);

   if (disconnect)
   {
      Internals::disconnector(*m_data_source, "column_inserted", on_column_inserted);
      Internals::disconnector(*m_data_source, "column_removed", on_column_removed);
      Internals::disconnector(*m_data_source, "column_moved", on_column_moved);
      Internals::disconnector(*m_data_source, "column_renamed", on_column_renamed);
      Internals::disconnector(*m_data_source, "column_rename_rejected", on_column_rename_rejected);
      Internals::disconnector(*m_data_source, "type_changed", on_type_changed);
      Internals::disconnector(*m_data_source, "row_inserted", on_row_inserted);
      Internals::disconnector(*m_data_source, "row_removed", on_row_removed);
      Internals::disconnector(*m_data_source, "row_moved", on_row_moved);
      Internals::disconnector(*m_data_source, "value_changed", on_value_changed);
      Internals::disconnector(*m_data_source, "value_change_rejected", on_value_change_rejected);
      Internals::disconnector(*m_data_source, "data_sorting_changed", on_sort_changed);
      Internals::disconnector(*m_data_source, "filter_changed", on_filter_changed);
   }
   else
   {
      Internals::connector(*m_data_source, "column_inserted", on_column_inserted);
      Internals::connector(*m_data_source, "column_removed", on_column_removed);
      Internals::connector(*m_data_source, "column_moved", on_column_moved);
      Internals::connector(*m_data_source, "column_renamed", on_column_renamed);
      Internals::connector(*m_data_source, "column_rename_rejected", on_column_rename_rejected);
      Internals::connector(*m_data_source, "type_changed", on_type_changed);
      Internals::connector(*m_data_source, "row_inserted", on_row_inserted);
      Internals::connector(*m_data_source, "row_removed", on_row_removed);
      Internals::connector(*m_data_source, "row_moved", on_row_moved);
      Internals::connector(*m_data_source, "value_changed", on_value_changed);
      Internals::connector(*m_data_source, "value_change_rejected", on_value_change_rejected);
      Internals::connector(*m_data_source, "data_sorting_changed", on_sort_changed);
      Internals::connector(*m_data_source, "filter_changed", on_filter_changed);
   }
}


void TabularBox::check_row_selection()
{
   const int64_t rcount = m_rownum_data.array.size();
   for (int64_t i = 0; i < rcount; i++)
   {
      m_rownum_data.array[i]->set_selected(m_data_source->is_row_selected(i));
   }
}


int64_t TabularBox::get_column_under_mouse(const godot::Vector2& mouse_pos) const
{
   // First make mouse position local to the header area - this is assuming incoming mouse position is local to
   // this TabularBox
   const godot::Vector2 harea_mouse_pos = mouse_pos - m_header_data.area->get_position();

   //for (TabularBoxHeader* header : m_header_data.array)
   const int64_t csize = m_header_data.array.size();
   for (int64_t i = 0; i < csize; i++)
   {
      TabularBoxHeader* header = m_header_data.array[i];

      // Header is a child of a HBoxContainer. When scrolling, this entire box moves sideways and things are clipped
      // by `m_header_data.area`. Convert header's position into "m_header_data.area space"
      const godot::Vector2 hpos = header->get_position() - m_header_data.box->get_position();
      const float header_width = header->get_size().x;

      const int right_pos = hpos.x + header_width;

      if (harea_mouse_pos.x > hpos.x && harea_mouse_pos.x < right_pos)
      {
         return i;
      }
   }

   return -1;
}


void TabularBox::handle_right_click(int64_t column_index, int64_t row_index)
{
   // If there is a selected cell, unselect it. If the implementation is correct any pending value change
   // should be comitted.
   select_cell(nullptr);

   /// FIXME: Use localization system

   m_context_menu->clear();

   // At least back in Godot 3 the PopupMenu grows when items are added but never shrinks back when those are removed
   // So reset to 0 to have the most compact size
   m_context_menu->set_size(godot::Vector2());

   m_context_menu->set_meta("column", column_index);
   m_context_menu->set_meta("row", row_index);

   m_context_menu->add_item("Append column", ContextMenuEntry::AppendColumn);

   if (column_index >= 0)
   {
      TabularBoxHeader* header = m_header_data.array[column_index];

      m_context_menu->add_submenu_item("Sort", "sub_sortrows");
      m_context_menu->set_item_disabled(m_context_menu->get_item_count() - 1, !header->get_allow_sorting());

      if (header->get_allow_menu())
      {
         m_context_menu->add_submenu_item("Move column", "sub_movecol");

         const bool is_locked = !header->get_allow_move();
         m_context_menu->set_item_disabled(m_context_menu->get_item_count() - 1, is_locked);

         m_context_menu->add_submenu_item("Insert column", "sub_insertcol");
      }

      m_context_menu->add_submenu_item("Value type", "sub_valuetype");
      m_context_menu->set_item_disabled(m_context_menu->get_item_count() - 1, !header->get_allow_type_change());

      if (header->get_allow_type_change())
      {
         const int64_t icount = m_value_type_menu->get_item_count();
         for (int64_t i = 0; i < icount; i++)
         {
            const int id = m_value_type_menu->get_item_id(i);
            m_value_type_menu->set_item_checked(i, id == header->get_value_type());
         }
      }

      if (header->get_allow_menu())
      {
         m_context_menu->add_item("Remove column", ContextMenuEntry::RemoveColumn);
      }
   }

   m_context_menu->add_separator();
   m_context_menu->add_item("Append row", ContextMenuEntry::AppendRow);

   if (row_index >= 0)
   {
      m_context_menu->add_item("Toggle selected", ContextMenuEntry::ToggleSelected);

      m_context_menu->add_submenu_item("Move row", "sub_moverow");
      m_context_menu->add_submenu_item("Insert row", "sub_insertrow");

      m_context_menu->add_item("Remove row", ContextMenuEntry::RemoveRow);
   }

   m_context_menu->set_position(get_global_mouse_position());
   m_context_menu->popup();
}


void TabularBox::handle_mouse_button(const godot::Ref<godot::InputEventMouseButton>& evt)
{
   switch (evt->get_button_index())
   {
      case godot::MouseButton::MOUSE_BUTTON_RIGHT:
      {
         if (evt->is_pressed() && m_data_source.is_valid() && get_allow_context_menu())
         {
            const int64_t column_index = get_column_under_mouse(evt->get_position());
            const int64_t row_index = m_cell_data.row_under_mouse ? m_cell_data.row_under_mouse->get_index() : -1;
            handle_right_click(column_index, row_index);
         }
      } break;

      case godot::MouseButton::MOUSE_BUTTON_WHEEL_UP:
      case godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN:
      {
         if (evt->is_pressed())
         {
            godot::ScrollBar* bar = (m_hor_bar->is_visible() && !m_ver_bar->is_visible()) ? (godot::ScrollBar*)m_hor_bar : (godot::ScrollBar*)m_ver_bar;
            float delta = bar->get_page() / 8.0 * evt->get_factor();

            if (evt->get_button_index() == godot::MouseButton::MOUSE_BUTTON_WHEEL_UP) { delta = -delta; }

            bar->set_value(bar->get_value() + delta);
         }
      } break;

      case godot::MouseButton::MOUSE_BUTTON_WHEEL_LEFT:
      case godot::MouseButton::MOUSE_BUTTON_WHEEL_RIGHT:
      {
         if (evt->is_pressed() && m_hor_bar->is_visible())
         {
            float delta = m_hor_bar->get_page() / 8.0 * evt->get_factor();

            if (evt->get_button_index() == godot::MouseButton::MOUSE_BUTTON_WHEEL_LEFT) { delta = -delta; }

            m_hor_bar->set_value(m_hor_bar->get_value() + delta);
         }
      } break;
   }
}


void TabularBox::on_file_dialog_closed()
{
   Internals::disconnector((godot::Object*)m_file_dialog.dialog, "file_selected", m_file_dialog.selected_handler);
   m_file_dialog.selected_handler = godot::Callable();
}


void TabularBox::on_scroll_value_changed(float value)
{
   const int32_t x = -m_hor_bar->get_value();
   const int32_t y = -m_ver_bar->get_value();

   // Deal with the "header". It only moves horizontally
   {
      godot::Vector2 pos = m_header_data.box->get_position();
      pos.x = x;
      m_header_data.box->set_position(pos);
   }

   // Deal with "rownumbers". It only moves vertically
   {
      godot::Vector2 pos = m_rownum_data.box->get_position();
      pos.y = y;
      m_rownum_data.box->set_position(pos);
   }

   // Deal with the "cells". It moves on both directions
   {
      const godot::Vector2 pos(x, y);
      m_cell_data.box->set_position(pos);
   }
   
}


void TabularBox::on_context_menu_id_selected(int32_t id)
{
   // Something to remember here. When the popup is setup to be shown, two meta data entries are created:
   // - "column": an integer with the index of the column under the mouse cursor. If -1 then there was no column
   //   under it when the right click occurred.
   // - "row": an integer with the row index of the row uner the mouse cursor. If -1 then there was no row
   //   under it when the right click occurred.

   switch (id)
   {
      case ContextMenuEntry::AppendColumn:
      {
         if (get_auto_handle_col_insertion())
         {
            m_data_source->insert_column("", -1, -1);
         }
         else
         {
            emit_signal("insert_column_requested", m_data_source->get_column_count());
         }
      } break;

      case ContextMenuEntry::RemoveColumn:
      {
         const int64_t column = m_context_menu->get_meta("column");
         if (column == -1) { return; }

         if (get_auto_handle_remove_column())
         {
            // The data source shuold emit a signal indicating if the column was removed or not. In that case the "on_column_removed"
            // event handler will deal with updating the UI
            m_data_source->remove_column(column);
         }
         else
         {
            // Automatic column removal is disabled. Emit a signal so outside code can deal with this request
            emit_signal("column_remove_requested", column);
         }
      } break;

      case ContextMenuEntry::InsertColumnBefore:
      {
         const int64_t column = m_context_menu->get_meta("column");
         if (column == -1) { return; }
         if (get_auto_handle_col_insertion())
         {
            m_data_source->insert_column("", -1, column);
         }
         else
         {
            emit_signal("insert_column_requested", column);
         }
      } break;

      case ContextMenuEntry::InsertColumnAfter:
      {
         const int64_t column = m_context_menu->get_meta("column");
         if (column == -1) { return; }
         if (get_auto_handle_col_insertion())
         {
            m_data_source->insert_column("", -1, column + 1);
         }
         else
         {
            emit_signal("insert_column_requested", column + 1);
         }
      } break;

      case ContextMenuEntry::MoveColumnLeft:
      {
         const int64_t column = m_context_menu->get_meta("column");
         if (column == -1) { return; }
         on_header_move_left(m_header_data.array[column]);
      } break;

      case ContextMenuEntry::MoveColumnRight:
      {
         const int64_t column = m_context_menu->get_meta("column");
         if (column == -1) { return; }
         on_header_move_right(m_header_data.array[column]);
      } break;

      case ContextMenuEntry::SortAscending:
      {
         const int64_t column = m_context_menu->get_meta("column");
         if (column == -1) { return; }

         if (get_auto_handle_row_sort())
         {
            m_data_source->sort_by_column(column, true);
         }
         else
         {
            emit_signal("row_sort_requested", column, true);
         }
      } break;

      case ContextMenuEntry::SortDescending:
      {
         const int64_t column = m_context_menu->get_meta("column");
         if (column == -1) { return; }

         if (get_auto_handle_row_sort())
         {
            m_data_source->sort_by_column(column, false);
         }
         else
         {
            emit_signal("row_sort_requested", column, false);
         }
      } break;

      case ContextMenuEntry::AppendRow:
      {
         if (get_auto_handle_row_insertion())
         {
            m_data_source->insert_row(godot::Dictionary(), -1);
         }
         else
         {
            emit_signal("insert_row_requested", m_data_source->get_row_count());
         }
      } break;

      case ContextMenuEntry::ToggleSelected:
      {
         //const int64_t row = m_context_menu->get_meta("row");
         //m_left_panel->toggle_selected(row, false);
      } break;

      case ContextMenuEntry::RemoveRow:
      {
         const int64_t row = m_context_menu->get_meta("row");
         if (get_auto_handle_remove_row())
         {
            m_data_source->remove_row(row);
         }
         else
         {
            emit_signal("row_remove_requested", godot::Array::make(row));
         }
      } break;

      case ContextMenuEntry::InsertRowAbove:
      {
         const int64_t row = m_context_menu->get_meta("row");
         if (get_auto_handle_row_insertion())
         {
            m_data_source->insert_row(godot::Dictionary(), row);
         }
         else
         {
            emit_signal("insert_row_requested", row);
         }
      } break;

      case ContextMenuEntry::InsertRowBellow:
      {
         const int64_t row = m_context_menu->get_meta("row");
         if (get_auto_handle_row_insertion())
         {
            m_data_source->insert_row(godot::Dictionary(), row + 1);
         }
         else
         {
            emit_signal("insert_row_requested", row + 1);
         }
      } break;

      case ContextMenuEntry::MoveRowUp:
      {
         const int64_t row = m_context_menu->get_meta("row");
         on_move_up(row);
      } break;

      case ContextMenuEntry::MoveRowDown:
      {
         const int64_t row = m_context_menu->get_meta("row");
         on_move_down(row);
      }
   }

   // A menu option has been used. Clear the meta data to avoid any possible problem that may be caused by
   // incorrect setup when the menu is used again
   m_context_menu->remove_meta("column");
   m_context_menu->remove_meta("row");
}


void TabularBox::on_value_type_id_selected(int32_t id)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to set column value type but data source is invalid");

   const int64_t column_index = m_context_menu->get_meta("column");
   TabularBoxHeader* header = m_header_data.array[column_index];

   m_context_menu->remove_meta("column");
   m_context_menu->remove_meta("row");

   if (header->get_value_type() == id)
   {
      return;
   }


   if (get_auto_handle_col_type_change())
   {
      m_data_source->change_column_value_type(column_index, id);
   }
   else
   {
      emit_signal("column_type_change_requested", column_index, id);
   }
}

/// TODO: Attempt to provide a "delta width" as event argument. Or alternatively store size and calculate delta within the handler
void TabularBox::on_header_resized(TabularBoxHeader* header)
{
   // Since headers are added into the HBoxContainer in column order, retrieving the Node index should give the correct
   // column index
   const int64_t cindex = header->get_index();

   const int32_t new_width = header->get_size().x;

   // Apply the width to all cells of this column
   for (RowData& row : m_cell_data.row)
   {
      row.cell[cindex]->set_custom_minimum_size(godot::Vector2(new_width, 0));
   }

   // Recalculate total width. Way easier than attempting to update based on old value
   int32_t width_to_resized = 0;
   m_cell_data.total_size.width = 0;
   for (TabularBoxHeader* h : m_header_data.array)
   {
      m_cell_data.total_size.width += h->get_size().x;
      if (header == h)
      {
         width_to_resized = m_cell_data.total_size.width;
      }
   }

   set_process_internal(true);

   if (godot::Input::get_singleton()->is_mouse_button_pressed(godot::MOUSE_BUTTON_LEFT))
   {
      // If here then most likely user is resizing the header. Try to make sure the right side of that header is visible
      int32_t x = -m_hor_bar->get_value();
      const int32_t right_side = x + width_to_resized;

      const int32_t rlimit = m_header_data.area->get_size().x - header->get_half_sizer_width();
      int delta = 0;

      if (right_side > rlimit)
      {
         delta = right_side - rlimit;
      }
      else if (right_side < header->get_half_sizer_width())
      {
         delta = right_side + header->get_half_sizer_width();
      }

      if (delta != 0)
      {
         m_hor_bar->set_value(m_hor_bar->get_value() + delta);
      }
   }

   if (!m_header_data.manually_resizing)
   {
      emit_signal("column_resized", header->get_title(), new_width);
   }
}


void TabularBox::on_header_move_left(TabularBoxHeader* header)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to move column to the left, but data source is not valid!");

   // The header is added into the HBoxContainer in column order, so retrieving Node index should provide the correct value
   const int64_t column_index = header->get_index();
   if (column_index == 0)
   {
      // The column is already the left-most one. Bail.
      return;
   }

   if (get_auto_handle_col_move())
   {
      // The data source should emit a signal if the column is properly moved. In that case the on_column_moved() event handler
      // should take care of updating the UI
      m_data_source->move_column(column_index, column_index - 1);
   }
   else
   {
      emit_signal("column_move_requested", column_index, column_index - 1);
   }
}


void TabularBox::on_header_move_right(TabularBoxHeader* header)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to move column to the right, but data source is not valid!");

   const int64_t column_index = header->get_index();
   if (column_index + 1 == m_header_data.array.size())
   {
      // This is already the right-most one. Bail.
      return;
   }

   if (get_auto_handle_col_move())
   {
      m_data_source->move_column(column_index, column_index + 1);
   }
   else
   {
      emit_signal("column_move_requested", column_index, column_index + 1);
   }
}


void TabularBox::on_mouse_enter_row(godot::HBoxContainer* row)
{
   m_cell_data.row_under_mouse = row;
}


void TabularBox::on_mouse_leave_row(godot::HBoxContainer* row)
{
   if (m_cell_data.row_under_mouse == row)
   {
      m_cell_data.row_under_mouse = nullptr;
   }
}


void TabularBox::on_move_up(int64_t which_row)
{
   if (!m_data_source.is_valid()) { return; }

   if (which_row == 0)
   {
      // Attempting to move up the first row. Nothing to do here, just bail
      return;
   }

   if (get_auto_handle_row_move())
   {
      m_data_source->move_row(which_row, which_row - 1);
   }
   else
   {
      emit_signal("row_move_requested", which_row, which_row - 1);
   }
}


void TabularBox::on_move_down(int64_t which_row)
{
   if (!m_data_source.is_valid()) { return; }

   if (which_row == m_data_source->get_row_count() - 1)
   {
      // Attempting to move down the last row. Nothing to d ohere, just bail
      return;
   }

   if (get_auto_handle_row_move())
   {
      m_data_source->move_row(which_row, which_row + 1);
   }
   else
   {
      emit_signal("row_move_requested", which_row, which_row + 1);
   }
}


void TabularBox::on_row_min_size_changed(godot::HBoxContainer* row)
{
   // Rows are added into a VBoxContainer in "row order". This means that row->get_index() should match the row index
   // within the data source.
   const int64_t row_index = row->get_index();
   const int32_t new_height = row->get_combined_minimum_size().y;
   const int32_t delta = new_height - m_cell_data.row[row_index].height;

   if (delta == 0) { return; }

   m_cell_data.total_size.height += delta;
   m_cell_data.row.ptrw()[row_index].height = new_height;

   m_rownum_data.array[row_index]->set_custom_minimum_size(godot::Vector2(0, new_height));

   set_process_internal(true);
}


void TabularBox::on_row_selected_changed(int64_t row_index, bool selected)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to change row selection state, but data source is not valid!");

   m_data_source->set_row_selected(row_index, selected);
}


void TabularBox::on_header_renamed(const godot::String& new_title, TabularBoxHeader* header)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to rename a column title, but data source is not valid!");

   // This function handles the TabularBoxHeader's event that is triggered when the title "editor" is committed.
   // Yet, the new title might not be allowed. Perhaps because it's already used or maybe it's a reserved title.
   // So, if auto handle title edit is enable, directly request it from the data source, otherwise request it from
   // outside code by emitting a signal.
   // In either case, the data source should emit a signal telling if the title change occurred or was rejected.
   // Those will be handled by on_column_title_changed() or on_column_title_change_rejected(), respectively.
   if (new_title.is_empty())
   {
      on_column_title_change_rejected(header->get_index());
      return;
   }

   if (get_auto_handle_col_rename())
   {
      m_data_source->rename_column(header->get_index(), new_title);
   }
   else
   {
      emit_signal("column_rename_requested", header->get_index(), new_title);
   }
}


void TabularBox::on_cell_value_changed(const godot::Variant& new_value, TabularBoxHeader* header, godot::HBoxContainer* row)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to change cell value, but data source is not valid!");

   if (header->get_value_change_signal())
   {
      emit_signal("value_change_requested", header->get_index(), row->get_index(), new_value);
   }
   else
   {
      // The data source should trigger a "value_changed" or "value_change_rejected" event
      m_data_source->set_value(header->get_index(), row->get_index(), new_value);
   }
}


void TabularBox::on_cell_select_next(TabularBoxCell* cell)
{
   if (m_selected.cell != cell)
   {
      return;
   }

   // Get the column and row of currently selected cell. In this casel 'cell->get_index()' should give column index
   // as it's added into an instance of HBoxContainer in column order.
   // That HBoxContainer instance is the "row", so calling 'get_index()' from it should result in the row index
   int64_t column_index = cell->get_index();
   int64_t row_index = cell->get_parent()->get_index();


   TabularBoxCell* next = nullptr;
   switch (m_next_cell_mode)
   {
      case NCM_Row:
      {
         row_index++;
         if (row_index >= m_cell_data.row.size())
         {
            row_index = 0;
            column_index++;
         }

         if (column_index < m_header_data.array.size())
         {
            next = m_cell_data.row[row_index].cell[column_index];
         }
      } break;

      case NCM_Column:
      {
         column_index++;
         if (column_index >= m_header_data.array.size())
         {
            column_index = 0;
            row_index++;
         }

         if (row_index < m_cell_data.row.size())
         {
            next = m_cell_data.row[row_index].cell[column_index];
         }
      } break;
   }

   select_cell(next);
}


void TabularBox::on_cell_unselect()
{
   select_cell(nullptr);
}


void TabularBox::on_column_inserted(int64_t index)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Got the column inserted signal, but data source is not valid!");

   TabularDataSource::ColumnInfo cinfo;
   m_data_source->fill_column_info(index, cinfo);

   create_header(cinfo, index);

   // Make sure the scripted class list is up to date
   GlobalClassCache::scan_global_classes();

   const int64_t rcount = m_data_source->get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      const godot::Variant value = m_data_source->get_value(index, i);

      create_cell(cinfo, value, m_cell_data.row.ptrw()[i], index);
   }

   save_data_source();
   set_process_internal(true);
}


void TabularBox::on_column_removed(int64_t index)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Got the column removed signal, but data source is not valid!");

   remove_header(index);
   save_data_source();
   set_process_internal(true);
}


void TabularBox::on_column_moved(int64_t from, int64_t to)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Got the column moved signal, but data source is not valid!");

   // First move all cells
   for (RowData& row : m_cell_data.row)
   {
      TabularBoxCell* cell = row.cell[from];
      row.box->move_child(cell, to);
      Internals::move_array_element(row.cell, from, to);
   }

   // Then move the header
   TabularBoxHeader* header = m_header_data.array[from];
   m_header_data.box->move_child(header, to);
   Internals::move_array_element(m_header_data.array, from, to);

   calculate_sizes();
}


void TabularBox::on_column_title_changed(int64_t index)
{
   // This error should not occur! This function handles an event that comes directly from the data source! Yet, error checking
   // is never enoguh!
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to change column title but data source is invalid!");
   ERR_FAIL_INDEX_MSG(index, m_header_data.array.size(), "Attempting to change column title but incoming column index is out of bounds!");

   TabularBoxHeader* header = m_header_data.array[index];

   // At this point 'header->get_title()' will return the old title. The thing is, it returns the value stored within the
   // ColumnInfo struct, which hasn't been updated yet.
   const godot::String old_title = header->get_title();

   // The data source contains the "most correct" title. Even though header might be correct, the "title changed" might have
   // been triggered by another cause other than editing it within the UI. This means that a verification here is needed
   // To begin with, retrieve the "most correct" title, that is, from the data source
   TabularDataSource::ColumnInfo info;
   m_data_source->fill_column_info(index, info);

   // Request the header to confirm its title data. After that get_title() should be updated *if* the change has been triggered
   // by editing it through the corresponding LineEdit
   header->confirm_title_change();
   godot::String new_title = header->get_title();

   if (new_title != info.title)
   {
      // If here then the title change has been triggered by a different thing, most likely by calling a function within the
      // data source through code. So, force the update
      header->set_info(info);
      new_title = info.title;
   }

   m_header_data.map.erase(old_title);
   m_header_data.map[new_title] = header;

   save_data_source();
}


void TabularBox::on_column_title_change_rejected(int64_t index)
{
   // This error should not occur! This function handles an event that comes directly from the data source! Yet, error checking
   // is never enough!
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to reject column title change, but data source is not valid!");
   ERR_FAIL_INDEX_MSG(index, m_header_data.array.size(), "Attempting to revert column title change, but incoming column index is out of bounds!");

   TabularBoxHeader* header = m_header_data.array[index];
   header->revert_title_change();
}


void TabularBox::on_type_changed(int64_t column)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Received signal that a column has changed its value type but data source is invalid");

   TabularBoxHeader* header = m_header_data.array[column];
   TabularDataSource::ColumnInfo info;
   m_data_source->fill_column_info(column, info);

   header->set_info(info);

   // Make sure the scripted class list is up to date
   GlobalClassCache::scan_global_classes();

   for (RowData& row : m_cell_data.row)
   {
      TabularBoxCell* cell = row.cell[column];
      row.box->remove_child(cell);
      memdelete(cell);
      
      row.cell.remove_at(column);
      row.cell_map.erase(info.title);

      const godot::Variant value = m_data_source->get_value(column, row.box->get_index());
      create_cell(info, value, row, column);
   }
}


void TabularBox::on_row_inserted(int64_t index)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Got the row inserted signal, but data source is not valid!");

   const godot::Dictionary row = m_data_source->get_row(index);
   create_row(row, index);
   save_data_source();
   set_process_internal(true);
}


void TabularBox::on_row_removed(int64_t index)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Got the row inserted signal, but data source is not valid!");

   remove_row(index);
   save_data_source();
   set_process_internal(true);
}


void TabularBox::on_row_moved(int64_t from, int64_t to)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Got row moved signal, but data source is not valid");
   
   if (from < 0 || from >= m_data_source->get_row_count()) { return; }
   if (to < 0 || to >= m_data_source->get_row_count()) { return; }

   godot::HBoxContainer* row = m_cell_data.row[from].box;
   m_cell_data.box->move_child(row, to);
   Internals::move_array_element(m_cell_data.row, from, to);

   TabularBoxCellRowNumber* rownum = m_rownum_data.array[from];
   m_rownum_data.box->move_child(rownum, to);
   Internals::move_array_element(m_rownum_data.array, from, to);

   set_rows_odd_even(true);
   
   save_data_source();
}


void TabularBox::on_value_changed(int64_t column, int64_t row, const godot::Variant& value)
{
   // The data source should trigger an event (signal) that is handled by this function. And if here either the column
   // does not require external dealings with the new value or it was already confirmed
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Received signal that data value has changed, but data source is invalid");

   TabularBoxCell* cell = m_cell_data.row[row].cell[column];

   cell->assign_value(value);
   save_data_source();
}


void TabularBox::on_value_change_rejected(int64_t column, int64_t row)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Attempting to reject value change but data source is invalid");
   ERR_FAIL_INDEX_MSG(column, m_header_data.array.size(), "Attempting to reject value change but incoming column index is out of bounds");
   ERR_FAIL_INDEX_MSG(row, m_data_source->get_row_count(), "Attempting to reject value change but incoming row index is out of bounds");

   const godot::Variant value = m_data_source->get_value(column, row);
   m_cell_data.row[row].cell[column]->assign_value(value);
}



void TabularBox::on_sorted(int64_t column)
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Received sorted signal but data source is invalid");

   const int64_t rcount = m_data_source->get_row_count();
   const int64_t ccount = m_data_source->get_column_count();

   for (int64_t r = 0; r < rcount; r++)
   {
      const RowData& rdata = m_cell_data.row[r];

      for (int64_t c = 0; c < ccount; c++)
      {
         rdata.cell[c]->assign_value(m_data_source->get_value(c, r));
      }

      const bool selected = m_data_source->is_row_selected(r);
      m_rownum_data.array[r]->set_selected(selected);
   }

   /// TODO: Use the column information to add/assign the sort order icon within the header
}


void TabularBox::on_draw_header(godot::Control* ctrl)
{
   ctrl->draw_style_box(m_theme_cache.header, godot::Rect2(godot::Vector2(), ctrl->get_size()));
}


void TabularBox::on_row_menu_popup()
{
   select_cell(nullptr);

   // Disable the 'delete selected' option from the menu if there are no selected rows
   const int32_t index = m_rownum_data.bt_menu->get_popup()->get_item_index(DeleteSelected);
   m_rownum_data.bt_menu->get_popup()->set_item_disabled(index, !m_data_source->has_selected_row());
}


void TabularBox::on_rownum_id_selected(int32_t id)
{
   switch (id)
   {
      case SelectAll:
      {
         m_data_source->select_all_rows();
         for (TabularBoxCellRowNumber* rnum : m_rownum_data.array)
         {
            rnum->set_selected(true);
         }
      } break;

      case DeselectAll:
      {
         m_data_source->deselect_all_rows();
         for (TabularBoxCellRowNumber* rnum : m_rownum_data.array)
         {
            rnum->set_selected(false);
         }
      } break;

      case InvertSelection:
      {
         m_data_source->invert_row_selection();
         check_row_selection();
      } break;

      case DeleteSelected:
      {
         // This option is disabled if there are no selected items. So, no need to check if row selection is empty here
         if (get_auto_handle_remove_row())
         {
            m_data_source->delete_selected_rows();
            check_row_selection();
         }
         else
         {
            emit_signal("row_remove_requested", m_data_source->get_selected_list());
         }
      } break;
   }
}


void TabularBox::on_filter_changed()
{
   ERR_FAIL_COND_MSG(!m_data_source.is_valid(), "Received filter changed signal but data source is invalid");

   // Assume nothing changes
   bool changed = false;

   const int64_t rcount = m_cell_data.row.size();
   for (int64_t r = 0; r < rcount; r++)
   {
      const bool filtered = m_data_source->is_filtered(r);

      if (filtered && m_cell_data.row[r].box->is_visible())
      {
         changed = true;
         m_cell_data.row[r].box->hide();
         m_cell_data.total_size.height -= m_cell_data.row[r].height;

         m_rownum_data.array[r]->hide();
      }
      else if (!filtered && !m_cell_data.row[r].box->is_visible())
      {
         changed = true;
         m_cell_data.row[r].box->show();
         m_cell_data.total_size.height += m_cell_data.row[r].height;

         m_rownum_data.array[r]->show();
      }
   }

   if (changed)
   {
      set_rows_odd_even(false);
      set_process_internal(true);
   }
}


void TabularBox::on_debug_draw(godot::Control* ctrl, const godot::Color& color)
{
   ctrl->draw_rect(godot::Rect2(godot::Vector2(), ctrl->get_size()), color);
}


void TabularBox::_get_property_list(godot::List<godot::PropertyInfo>* out_list) const
{
   /// Append Custom control theme entries
   CustomControlThemeDB::get_singleton()->fill_property_list("TabularBox", out_list, this);
}


void TabularBox::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         CustomControlThemeDB::get_singleton()->update_control(this);
         calculate_min_sizes();

         const godot::Callable on_ctx_menu = callable_mp(this, &TabularBox::on_context_menu_id_selected);
         m_context_menu->connect("id_pressed", on_ctx_menu);
         m_context_menu->get_node<godot::PopupMenu>("sub_movecol")->connect("id_pressed", on_ctx_menu);
         m_context_menu->get_node<godot::PopupMenu>("sub_insertcol")->connect("id_pressed", on_ctx_menu);
         m_context_menu->get_node<godot::PopupMenu>("sub_sortrows")->connect("id_pressed", on_ctx_menu);
         m_context_menu->get_node<godot::PopupMenu>("sub_moverow")->connect("id_pressed", on_ctx_menu);
         m_context_menu->get_node<godot::PopupMenu>("sub_insertrow")->connect("id_pressed", on_ctx_menu);

         m_value_type_menu->connect("id_pressed", callable_mp(this, &TabularBox::on_value_type_id_selected));

         m_rownum_data.menu_area->connect("draw", callable_mp(this, &TabularBox::on_draw_header).bind(m_rownum_data.menu_area));

         const godot::Callable on_scroll_val_changed = callable_mp(this, &TabularBox::on_scroll_value_changed);
         m_hor_bar->connect("value_changed", on_scroll_val_changed);
         m_ver_bar->connect("value_changed", on_scroll_val_changed);

         m_rownum_data.bt_menu->connect("about_to_popup", callable_mp(this, &TabularBox::on_row_menu_popup));
         m_rownum_data.bt_menu->get_popup()->connect("id_pressed", callable_mp(this, &TabularBox::on_rownum_id_selected));


         /// This is DEBUG thingy. Keeping commented instead of removing for easier debugging when/if necessary
         //m_header_data.area->connect("draw", callable_mp(this, &TabularBox::on_debug_draw).bind(m_header_data.area, godot::Color(1, 0, 0, 1)));
         //m_header_data.box->connect("draw", callable_mp(this, &TabularBox::on_debug_draw).bind(m_header_data.box, godot::Color(0, 0.8, 0, 0.5)));
         //m_cell_data.area->connect("draw", callable_mp(this, &TabularBox::on_debug_draw).bind(m_cell_data.area, godot::Color(0, 0, 0.8, 0.5)));
         //m_cell_data.box->connect("draw", callable_mp(this, &TabularBox::on_debug_draw).bind(m_cell_data.box, godot::Color(0, 0.5, 0, 0.5)));
      } break;

      
      case NOTIFICATION_THEME_CHANGED:
      {
         // The theme cache has to be updated. Again, easier to simply do it instead of attempting to check if there
         // was any change at al within the relevant data.
         CustomControlThemeDB::get_singleton()->update_control(this);

         apply_styling();
         calculate_min_sizes();
         calculate_rownum_width();
         calculate_sizes();
      } // Fallthrough on purpose

      case NOTIFICATION_RESIZED:
      {
         check_scroll_bars();
         calculate_available_height();

         for (RowData& rdata : m_cell_data.row)
         {
            for (TabularBoxCell* cell : rdata.cell)
            {
               cell->check_theme();
            }
         }
         
         apply_layout();

         for (TabularBoxHeader* header : m_header_data.array)
         {
            header->queue_redraw();
         }
      } break;

      case NOTIFICATION_DRAW:
      {
         draw_style_box(m_theme_cache.background, godot::Rect2(godot::Vector2(), get_size()));
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         // Several operations might be triggered multiple times in a very small amount of time. Instead of
         // recalculating scroll bar visibility/position, available height and layout every time the events
         // are triggered, those should enable internal processing. Indeed this will delay the entire calculations
         // by one frame, however this means that the required tasks will be done only once after all the
         // events have been handled within that frame.
         // This also eliminates the need to have a "freeze/thaw" system within the data source!
         set_process_internal(false);

         set_rows_odd_even(true);

         const bool changed = check_scroll_bars();
         calculate_available_height();
         if (changed)
         {
            apply_layout();
         }
      } break;
   }
}


void TabularBox::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("get_data_source"), &TabularBox::get_data_source);
   ClassDB::bind_method(D_METHOD("set_data_source", "data_source"), &TabularBox::set_data_source);
   ClassDB::bind_method(D_METHOD("get_title_horizontal_alignment"), &TabularBox::get_title_horizontal_alignment);
   ClassDB::bind_method(D_METHOD("set_title_horizontal_alignment", "alignment"), &TabularBox::set_title_horizontal_alignment);
//   ClassDB::bind_method(D_METHOD("get_move_column_buttons_mode"), &TabularBox::get_column_buttons_mode);
//   ClassDB::bind_method(D_METHOD("set_move_column_buttons_mode", "mode"), &TabularBox::set_column_buttons_mode);
   ClassDB::bind_method(D_METHOD("get_allow_context_menu"), &TabularBox::get_allow_context_menu);
   ClassDB::bind_method(D_METHOD("set_allow_context_menu", "enabled"), &TabularBox::set_allow_context_menu);
   ClassDB::bind_method(D_METHOD("get_edit_next_cell_mode"), &TabularBox::get_next_cell_mode);
   ClassDB::bind_method(D_METHOD("set_edit_next_cell_mode", "mode"), &TabularBox::set_next_cell_mode);
   ClassDB::bind_method(D_METHOD("get_auto_handle_remove_row"), &TabularBox::get_auto_handle_remove_row);
   ClassDB::bind_method(D_METHOD("set_auto_handle_remove_row", "enabled"), &TabularBox::set_auto_handle_remove_row);
   ClassDB::bind_method(D_METHOD("get_auto_handle_remove_column"), &TabularBox::get_auto_handle_remove_column);
   ClassDB::bind_method(D_METHOD("set_auto_handle_remove_column", "enabled"), &TabularBox::set_auto_handle_remove_column);
   ClassDB::bind_method(D_METHOD("get_show_row_numbers"), &TabularBox::get_show_row_numbers);
   ClassDB::bind_method(D_METHOD("set_show_row_numbers", "enabled"), &TabularBox::set_show_row_numbers);
   ClassDB::bind_method(D_METHOD("get_row_numbers_alignment"), &TabularBox::get_row_numbers_alignment);
   ClassDB::bind_method(D_METHOD("set_row_numbers_alignment", "align"), &TabularBox::set_row_numbers_alignment);
   ClassDB::bind_method(D_METHOD("get_show_row_checkboxes"), &TabularBox::get_show_row_checkboxes);
   ClassDB::bind_method(D_METHOD("set_show_row_checkboxes", "enabled"), &TabularBox::set_show_row_checkboxes);
   ClassDB::bind_method(D_METHOD("get_autosave_source"), &TabularBox::get_autosave_source);
   ClassDB::bind_method(D_METHOD("set_autosave_source", "enabled"), &TabularBox::set_autosave_source);
   ClassDB::bind_method(D_METHOD("get_auto_handle_column_insertion"), &TabularBox::get_auto_handle_col_insertion);
   ClassDB::bind_method(D_METHOD("set_auto_handle_column_insertion", "enabled"), &TabularBox::set_auto_handle_col_insertion);
   ClassDB::bind_method(D_METHOD("get_auto_handle_row_insertion"), &TabularBox::get_auto_handle_row_insertion);
   ClassDB::bind_method(D_METHOD("set_auto_handle_row_insertion", "enabled"), &TabularBox::set_auto_handle_row_insertion);
   ClassDB::bind_method(D_METHOD("get_auto_handle_column_rename"), &TabularBox::get_auto_handle_col_rename);
   ClassDB::bind_method(D_METHOD("set_auto_handle_column_rename", "enabled"), &TabularBox::set_auto_handle_col_rename);
   ClassDB::bind_method(D_METHOD("get_auto_handle_column_move"), &TabularBox::get_auto_handle_col_move);
   ClassDB::bind_method(D_METHOD("set_auto_handle_column_move", "enabled"), &TabularBox::set_auto_handle_col_move);
   ClassDB::bind_method(D_METHOD("get_auto_handle_column_type_change"), &TabularBox::get_auto_handle_col_type_change);
   ClassDB::bind_method(D_METHOD("set_auto_handle_column_type_change", "enabled"), &TabularBox::set_auto_handle_col_type_change);
   ClassDB::bind_method(D_METHOD("get_auto_handle_row_move"), &TabularBox::get_auto_handle_row_move);
   ClassDB::bind_method(D_METHOD("set_auto_handle_row_move", "enabled"), &TabularBox::set_auto_handle_row_move);
   ClassDB::bind_method(D_METHOD("get_auto_handle_row_sort"), &TabularBox::get_auto_handle_row_sort);
   ClassDB::bind_method(D_METHOD("set_auto_handle_row_sort", "enabled"), &TabularBox::set_auto_handle_row_sort);
   ClassDB::bind_method(D_METHOD("get_autohide_h_scrollbar"), &TabularBox::get_autohide_hscroll_bar);
   ClassDB::bind_method(D_METHOD("set_autohide_h_scrollbar", "enable"), &TabularBox::set_autohide_hscroll_bar);
   ClassDB::bind_method(D_METHOD("get_autohide_v_scrollbar"), &TabularBox::get_autohide_vscroll_bar);
   ClassDB::bind_method(D_METHOD("set_autohide_v_scrollbar", "enable"), &TabularBox::set_autohide_vscroll_bar);

   ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "data_source", PROPERTY_HINT_RESOURCE_TYPE, "TabularDataSource"), "set_data_source", "get_data_source");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "title_horizontal_alignment", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill"), "set_title_horizontal_alignment", "get_title_horizontal_alignment");
//   ADD_PROPERTY(PropertyInfo(Variant::INT, "move_column_buttons_mode", PROPERTY_HINT_ENUM, "AlwaysVisible,ShowOnMouseOver,AlwaysHidden"), "set_move_column_buttons_mode", "get_move_column_buttons_mode");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_context_menu"), "set_allow_context_menu", "get_allow_context_menu");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "auto_edit_next_row_mode", PROPERTY_HINT_ENUM, "Disabled,Row,Column"), "set_edit_next_cell_mode", "get_edit_next_cell_mode");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_remove_row"), "set_auto_handle_remove_row", "get_auto_handle_remove_row");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_remove_column"), "set_auto_handle_remove_column", "get_auto_handle_remove_column");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_row_numbers"), "set_show_row_numbers", "get_show_row_numbers");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "row_numbers_alignment", PROPERTY_HINT_ENUM, "Left,Center,Right,Fill"), "set_row_numbers_alignment", "get_row_numbers_alignment");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_row_checkboxes"), "set_show_row_checkboxes", "get_show_row_checkboxes");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autosave_source"), "set_autosave_source", "get_autosave_source");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_column_insertion"), "set_auto_handle_column_insertion", "get_auto_handle_column_insertion");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_row_insertion"), "set_auto_handle_row_insertion", "get_auto_handle_row_insertion");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_column_rename"), "set_auto_handle_column_rename", "get_auto_handle_column_rename");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_column_move"), "set_auto_handle_column_move", "get_auto_handle_column_move");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_column_type_change"), "set_auto_handle_column_type_change", "get_auto_handle_column_type_change");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_row_move"), "set_auto_handle_row_move", "get_auto_handle_row_move");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "auto_handle_row_sort"), "set_auto_handle_row_sort", "get_auto_handle_row_sort");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autohide_h_scrollbar"), "set_autohide_h_scrollbar", "get_autohide_h_scrollbar");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "autohide_v_scrollbar"), "set_autohide_v_scrollbar", "get_autohide_v_scrollbar");

   BIND_ENUM_CONSTANT(NCM_Disabled);
   BIND_ENUM_CONSTANT(NCM_Row);
   BIND_ENUM_CONSTANT(NCM_Column);

   ADD_SIGNAL(MethodInfo("insert_column_requested", PropertyInfo(Variant::INT, "at_index")));
   ADD_SIGNAL(MethodInfo("insert_row_requested", PropertyInfo(Variant::INT, "at_index")));
   ADD_SIGNAL(MethodInfo("column_remove_requested", PropertyInfo(Variant::INT, "column_index")));
   ADD_SIGNAL(MethodInfo("column_rename_requested", PropertyInfo(Variant::INT, "column_index"), PropertyInfo(Variant::STRING, "new_title")));
   ADD_SIGNAL(MethodInfo("column_move_requested", PropertyInfo(Variant::INT, "from_index"), PropertyInfo(Variant::INT, "to_index")));
   ADD_SIGNAL(MethodInfo("column_type_change_requested", PropertyInfo(Variant::INT, "column_index"), PropertyInfo(Variant::INT, "to_type")));
   ADD_SIGNAL(MethodInfo("column_resized", PropertyInfo(Variant::STRING, "column_title"), PropertyInfo(Variant::INT, "new_width")));
   ADD_SIGNAL(MethodInfo("value_change_requested", PropertyInfo(Variant::INT, "column_index"), PropertyInfo(Variant::INT, "row_index"), PropertyInfo(Variant::NIL, "value")));
   ADD_SIGNAL(MethodInfo("row_remove_requested", PropertyInfo(Variant::ARRAY, "index_list")));
   ADD_SIGNAL(MethodInfo("row_move_requested", PropertyInfo(Variant::INT, "from_index"), PropertyInfo(Variant::INT, "to_index")));
   ADD_SIGNAL(MethodInfo("row_sort_requested", PropertyInfo(Variant::INT, "column_index"), PropertyInfo(Variant::BOOL, "ascending")));
   //ADD_SIGNAL(MethodInfo("file_selected", PropertyInfo(Variant::STRING, "path")));

   const float cell_internal_margin = 3.0;

   Ref<StyleBoxFlat> background = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), 1, 1, 1, 1);
   Ref<StyleBoxFlat> header = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), cell_internal_margin, cell_internal_margin, cell_internal_margin, cell_internal_margin, 0, true, 1);
   header->set_corner_radius(godot::CORNER_TOP_LEFT, 3);
   header->set_corner_radius(godot::CORNER_TOP_RIGHT, 3);
   header->set_border_color(godot::Color(0.2, 0.2, 0.2, 0.3));
   Ref<StyleBoxFlat> row_number = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), 2, 2, 2, 2, 0, true, 1);
   row_number->set_border_color(godot::Color(0.5, 0.5, 0.5, 0.5));
   Ref<StyleBoxFlat> odd_row = Internals::make_flat_stylebox(Color(0.5, 0.5, 0.5, 0.3), cell_internal_margin, cell_internal_margin, cell_internal_margin, cell_internal_margin, 0, true, 1);
   odd_row->set_border_color(godot::Color(0.5, 0.5, 0.5, 0.5));
   Ref<StyleBoxFlat> even_row = Internals::make_flat_stylebox(Color(0.5, 0.5, 0.5, 0.1), cell_internal_margin, cell_internal_margin, cell_internal_margin, cell_internal_margin, 0, true, 1);
   even_row->set_border_color(godot::Color(0.5, 0.5, 0.5, 0.5));
   Ref<StyleBoxFlat> focus = Internals::make_flat_stylebox(Color(1.0, 1.0, 1.0, 0.75), 2, 2, 2, 2, 3, false, 2);
   focus->set_border_color(Color(1.0, 1.0, 1.0, 0.75));
   Ref<StyleBoxFlat> h_scrollbar = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), 0, 4, 0, 4, 10);
   Ref<StyleBoxFlat> v_scrollbar = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), 4, 0, 4, 0, 10);
   Ref<StyleBoxFlat> scrollbar_grabber = Internals::make_flat_stylebox(Color(1.0, 1.0, 1.0, 0.4), 4, 4, 4, 4, 10);
   Ref<StyleBoxFlat> scrollbar_highlight = Internals::make_flat_stylebox(Color(1.0, 1.0, 1.0, 0.75), 4, 4, 4, 4, 10);
   Ref<StyleBoxFlat> scrollbar_grabber_pressed = Internals::make_flat_stylebox(Color(0.75, 0.75, 0.75, 0.56), 4, 4, 4, 4, 10);
   Ref<StyleBoxFlat> button_normal = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), 1, 1, 1, 1);
   Ref<StyleBoxFlat> button_pressed = Internals::make_flat_stylebox(Color(0.0, 0.0, 0.0, 0.6), 1, 1, 1, 1);
   Ref<StyleBoxFlat> button_hover = Internals::make_flat_stylebox(Color(0.225, 0.225, 0.225, 0.6), 1, 1, 1, 1);


   header->set_border_color(Color(0.1, 0.1, 0.1, 0.2));

   Ref<Texture2D> checked = Internals::texture_from_base64(ICON_CHECKED);
   Ref<Texture2D> unchecked = Internals::texture_from_base64(ICON_UNCHECKED);
   Ref<Texture2D> left_arrow = Internals::texture_from_base64(ICON_LEFT_ARROW);
   Ref<Texture2D> right_arrow = Internals::texture_from_base64(ICON_RIGHT_ARROW);
   Ref<Texture2D> down_arrow = Internals::texture_from_base64(ICON_DOWN_ARROW);
   Ref<Texture2D> trash_bin = Internals::texture_from_base64(ICON_TRASH_BIN);
   Ref<Texture2D> no_texture = Internals::texture_from_base64(ICON_NOTE_X);

   Ref<Font> font;

   const Color font_color(1.0, 1.0, 1.0, 1.0);
   const Color selected(1.0, 1.0, 1.0, 1.0);
   const Color selection(0.5, 0.5, 0.5, 1.0);
   const Color caret(0.95, 0.95, 0.95, 1.0);

   const int header_font_size = 16;
   const int cell_font_size = 16;
   const int separation = 2;


   REGISTER_CUSTOM_STYLE(background, TabularBox, background, m_theme_cache);
   REGISTER_CUSTOM_STYLE(header, TabularBox, header, m_theme_cache);
   REGISTER_CUSTOM_STYLE(row_number, TabularBox, row_number, m_theme_cache);
   REGISTER_CUSTOM_STYLE(odd_row, TabularBox, odd_row, m_theme_cache);
   REGISTER_CUSTOM_STYLE(even_row, TabularBox, even_row, m_theme_cache);
   REGISTER_CUSTOM_STYLE(focus, TabularBox, focus, m_theme_cache);
   REGISTER_CUSTOM_STYLE(h_scrollbar, TabularBox, h_scrollbar, m_theme_cache);
   REGISTER_CUSTOM_STYLE(v_scrollbar, TabularBox, v_scrollbar, m_theme_cache);
   REGISTER_CUSTOM_STYLE(scrollbar_grabber, TabularBox, scrollbar_grabber, m_theme_cache);
   REGISTER_CUSTOM_STYLE(scrollbar_highlight, TabularBox, scrollbar_highlight, m_theme_cache);
   REGISTER_CUSTOM_STYLE(scrollbar_grabber_pressed, TabularBox, scrollbar_grabber_pressed, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_normal, TabularBox, button_normal, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_pressed, TabularBox, button_pressed, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_hover, TabularBox, button_hover, m_theme_cache);

   REGISTER_CUSTOM_ICON(checked, TabularBox, checked, m_theme_cache);
   REGISTER_CUSTOM_ICON(unchecked, TabularBox, unchecked, m_theme_cache);
   REGISTER_CUSTOM_ICON(left_arrow, TabularBox, left_arrow, m_theme_cache);
   REGISTER_CUSTOM_ICON(right_arrow, TabularBox, right_arrow, m_theme_cache);
   REGISTER_CUSTOM_ICON(down_arrow, TabularBox, down_arrow, m_theme_cache);
   REGISTER_CUSTOM_ICON(trash_bin, TabularBox, trash_bin, m_theme_cache);
   REGISTER_CUSTOM_ICON(no_texture, TabularBox, no_texture, m_theme_cache);

   REGISTER_CUSTOM_FONT(header_font, TabularBox, font, m_theme_cache);
   REGISTER_CUSTOM_FONT(cell_font, TabularBox, font, m_theme_cache);

   REGISTER_CUSTOM_COLOR(header_text, TabularBox, font_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(header_selected, TabularBox, selected, m_theme_cache);
   REGISTER_CUSTOM_COLOR(header_selection, TabularBox, selection, m_theme_cache);
   REGISTER_CUSTOM_COLOR(cell_text, TabularBox, font_color, m_theme_cache);
   REGISTER_CUSTOM_COLOR(cell_selected, TabularBox, selected, m_theme_cache);
   REGISTER_CUSTOM_COLOR(cell_selection, TabularBox, selection, m_theme_cache);
   REGISTER_CUSTOM_COLOR(caret, TabularBox, caret, m_theme_cache);

   REGISTER_CUSTOM_FONT_SIZE(header_font_size, TabularBox, header_font_size, m_theme_cache);
   REGISTER_CUSTOM_FONT_SIZE(cell_font_size, TabularBox, cell_font_size, m_theme_cache);

   REGISTER_CUSTOM_CONSTANT(separation, TabularBox, separation, m_theme_cache);
}


void TabularBox::_gui_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventMouseButton> mb = evt;
   if (mb.is_valid())
   {
      handle_mouse_button(mb);
   }
}


void TabularBox::_unhandled_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventKey> kb = evt;
   if (kb.is_valid() && kb->is_pressed())
   {
      if (kb->get_keycode() == godot::KEY_F1)
      {
         int32_t height = 0;
         for (const RowData& row : m_cell_data.row)
         {
            height += row.height;
         }

         const int32_t stored = m_cell_data.total_size.height;
         const int32_t from_box = m_cell_data.box->get_minimum_size().y;

         //godot::UtilityFunctions::print(godot::vformat("Calculated total height: %s | Stored total height: %s | From box: %s", height, stored, from_box));
      }

   }
}


void TabularBox::set_column_width(const godot::String& title, int32_t width)
{
   m_header_data.manually_resizing = true;

   godot::HashMap<godot::String, TabularBoxHeader*>::Iterator iter = m_header_data.map.find(title);
   if (iter != m_header_data.map.end())
   {
      iter->value->set_width(width);
   }

   m_header_data.manually_resizing = false;
}


int32_t TabularBox::get_column_width(const godot::String& title) const
{
   godot::HashMap<godot::String, TabularBoxHeader*>::ConstIterator iter = m_header_data.map.find(title);
   return iter != m_header_data.map.end() ? iter->value->get_size().x : 0;
}


void TabularBox::unselect_all_cells()
{
   select_cell(nullptr);
}


void TabularBox::set_data_source(const godot::Ref<TabularDataSource>& data_source)
{
   if (m_data_source == data_source) { return; }

   // Remove children. Way easier than attempting to keep relevant ones
   clear();

   // Disconnect from signals within current data source (if any)
   check_ds_signals(true);

   // Clear the value type list from the popup menu - also set its size to 0
   m_value_type_menu->clear();
   m_value_type_menu->set_size(godot::Vector2());

   // Assign the data source
   m_data_source = data_source;

   // Connect event handlers
   check_ds_signals(false);

   if (m_data_source.is_valid())
   {
      // Check if this data source is scripted. And if so, verify if it's a tool one. If it's not a tool script, then
      // warn the user
      godot::Ref<godot::Script> script = m_data_source->get_script();
      if (script.is_valid() && !script->is_tool())
      {
         WARN_PRINT("The assigned scripted data source is not a @tool one. It will work when running the game/app but not in editor. Moreover, upon loading the project error messages are likely to be shown about functions not being implemented.");
      }


      const godot::Dictionary type_list = m_data_source->get_type_list();
      const godot::Array keys = type_list.keys();
      
      const int64_t kcount = keys.size();
      for (int64_t i = 0; i < kcount; i++)
      {
         const int32_t code = keys[i];
         const godot::String tp = type_list[code];

         /// FIXME: Properly use the translation system so localization is possible
         m_value_type_menu->add_radio_check_item(tp, code);
      }

      const int64_t ccount = m_data_source->get_column_count();
      for (int64_t i = 0; i < ccount; i++)
      {
         TabularDataSource::ColumnInfo cinfo;
         m_data_source->fill_column_info(i, cinfo);

         create_header(cinfo, i);
      }

      
      bool is_odd = true;
      const int64_t rcount = m_data_source->get_row_count();
      for (int64_t i = 0; i < rcount; i++)
      {
         const godot::Dictionary row = m_data_source->get_row(i);

         create_row(row, i);

         m_cell_data.row[i].box->set_meta("is_odd", is_odd);
         is_odd = !is_odd;
      }
   }
   
   m_rownum_data.bt_menu->set_disabled(!m_data_source.is_valid());

   apply_layout();
}


void TabularBox::set_title_horizontal_alignment(godot::HorizontalAlignment align)
{
   m_title_halign = align;

   for (TabularBoxHeader* header : m_header_data.array)
   {
      header->set_alignment(align);
   }
}


void TabularBox::set_show_row_numbers(bool enabled)
{
   Internals::set_flag(enabled, ShowRowNumbers, m_setting_flags);

   const int64_t rcount = m_cell_data.row.size();
   for (int64_t i = 0; i < rcount; i++)
   {
      TabularBoxCellRowNumber* rnum = godot::Object::cast_to<TabularBoxCellRowNumber>(m_rownum_data.box->get_child(i));
      rnum->set_draw_num(enabled);
   }
   
   calculate_rownum_width();
   apply_layout();
}


godot::HorizontalAlignment TabularBox::get_row_numbers_alignment() const
{
   return m_rownum_data.align;
}


void TabularBox::set_row_numbers_alignment(godot::HorizontalAlignment align)
{
   m_rownum_data.align = align;

   for (TabularBoxCellRowNumber* rnum : m_rownum_data.array)
   {
      rnum->set_num_align(align);
   }
}


void TabularBox::set_show_row_checkboxes(bool enabled)
{
   Internals::set_flag(enabled, ShowCheckboxes, m_setting_flags);
   m_rownum_data.bt_menu->set_disabled(!enabled);

   for (TabularBoxCellRowNumber* rnum : m_rownum_data.array)
   {
      rnum->set_draw_chk(enabled);
   }

   calculate_rownum_width();
   apply_layout();
}


void TabularBox::set_autohide_vscroll_bar(bool enable)
{
   Internals::set_flag(enable, AutoHideVScrollBar, m_setting_flags);

   check_scroll_bars();
   calculate_available_height();
   apply_layout();
}


void TabularBox::set_autohide_hscroll_bar(bool enable)
{
   Internals::set_flag(enable, AutoHideHScrollBar, m_setting_flags);

   check_scroll_bars();
   calculate_available_height();
   apply_layout();
}


TabularBox::TabularBox()
{
   m_theme_cache.stl_empty = godot::Ref<godot::StyleBox>(memnew(godot::StyleBoxEmpty));
   m_setting_flags = FSDefault;
   m_title_halign = godot::HORIZONTAL_ALIGNMENT_LEFT;
   m_cell_data.row_under_mouse = nullptr;
   m_cell_data.available_height = 0;
   m_file_dialog.dialog = nullptr;
   m_selected.cell = nullptr;
   m_next_cell_mode = NCM_Row;
   m_header_data.manually_resizing = false;

   m_rownum_data.align = godot::HORIZONTAL_ALIGNMENT_RIGHT;


   // Create the header area
   {
      m_header_data.area = memnew(godot::Control);
      m_header_data.area->set_clip_contents(true);
      m_header_data.area->set_mouse_filter(MOUSE_FILTER_PASS);
      add_child(m_header_data.area, false, INTERNAL_MODE_BACK);

      m_header_data.box = memnew(godot::HBoxContainer);
      m_header_data.box->set_anchor_and_offset(godot::SIDE_LEFT, 0, 0);
      m_header_data.box->set_anchor_and_offset(godot::SIDE_TOP, 0, 0);
      m_header_data.box->set_anchor_and_offset(godot::SIDE_RIGHT, 1, 0);
      m_header_data.box->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, 0);
      m_header_data.box->add_theme_constant_override("separation", 0);
      m_header_data.box->set_mouse_filter(MOUSE_FILTER_IGNORE);
      m_header_data.area->add_child(m_header_data.box);
   }

   // Create the cell area
   {
      m_cell_data.area = memnew(godot::Control);
      m_cell_data.area->set_clip_contents(true);
      m_cell_data.area->set_mouse_filter(MOUSE_FILTER_PASS);
      add_child(m_cell_data.area, false, INTERNAL_MODE_BACK);

      m_cell_data.box = memnew(godot::VBoxContainer);
      m_cell_data.box->set_anchor_and_offset(godot::SIDE_LEFT, 0, 0);
      m_cell_data.box->set_anchor_and_offset(godot::SIDE_TOP, 0, 0);
      m_cell_data.box->set_anchor_and_offset(godot::SIDE_RIGHT, 1, 0);
      m_cell_data.box->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, 0);
      m_cell_data.box->add_theme_constant_override("separation", 0);
      m_cell_data.box->set_mouse_filter(MOUSE_FILTER_IGNORE);
      m_cell_data.area->add_child(m_cell_data.box);
   }

   // The "row number" area
   {
      m_rownum_data.area = memnew(godot::Control);
      m_rownum_data.area->set_clip_contents(true);
      m_rownum_data.area->set_mouse_filter(MOUSE_FILTER_PASS);
      add_child(m_rownum_data.area, false, INTERNAL_MODE_BACK);

      m_rownum_data.box = memnew(godot::VBoxContainer);
      m_rownum_data.box->set_anchor_and_offset(godot::SIDE_LEFT, 0, 0);
      m_rownum_data.box->set_anchor_and_offset(godot::SIDE_TOP, 0, 0);
      m_rownum_data.box->set_anchor_and_offset(godot::SIDE_RIGHT, 1, 0);
      m_rownum_data.box->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, 0);
      m_rownum_data.box->add_theme_constant_override("separation", 0);
      m_rownum_data.box->set_mouse_filter(MOUSE_FILTER_IGNORE);
      m_rownum_data.area->add_child(m_rownum_data.box);

      m_rownum_data.menu_area = memnew(godot::Control);
      add_child(m_rownum_data.menu_area, false, INTERNAL_MODE_BACK);

      m_rownum_data.bt_menu = memnew(godot::MenuButton);
      m_rownum_data.bt_menu->set_flat(false);
      m_rownum_data.bt_menu->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
      m_rownum_data.bt_menu->set_disabled(true);
      m_rownum_data.menu_area->add_child(m_rownum_data.bt_menu);

      godot::PopupMenu* pop = m_rownum_data.bt_menu->get_popup();
      pop->add_item("Select all", SelectAll);
      pop->add_item("Deselect all", DeselectAll);
      pop->add_item("Invert selection", InvertSelection);
      pop->add_separator();
      pop->add_item("Delete selected", DeleteSelected);
   }


   m_hor_bar = memnew(godot::HScrollBar);
   m_hor_bar->set_name("_horizontal_scroll_");
   m_hor_bar->set_visible(false);           // This will be handled when verifying the layout
   add_child(m_hor_bar, false, INTERNAL_MODE_BACK);

   m_ver_bar = memnew(godot::VScrollBar);
   m_ver_bar->set_name("_vertical_scroll_");
   m_ver_bar->set_visible(false);
   add_child(m_ver_bar, false, INTERNAL_MODE_BACK);


   
   m_context_menu = memnew(godot::PopupMenu);
   m_context_menu->set_name("_context_menu_");
   add_child(m_context_menu, false, INTERNAL_MODE_BACK);


   /// Setup the sub-menus
   /// FIXME: Use translation system to properly deal with localization
   godot::PopupMenu* sub_movecol = memnew(godot::PopupMenu);
   sub_movecol->set_name("sub_movecol");
   sub_movecol->add_item("Left", MoveColumnLeft);
   sub_movecol->add_item("Right", MoveColumnRight);
   m_context_menu->add_child(sub_movecol);

   godot::PopupMenu* sub_insertcol = memnew(godot::PopupMenu);
   sub_insertcol->set_name("sub_insertcol");
   sub_insertcol->add_item("Before", InsertColumnBefore);
   sub_insertcol->add_item("After", InsertColumnAfter);
   m_context_menu->add_child(sub_insertcol);

   godot::PopupMenu* sub_sortrows = memnew(godot::PopupMenu);
   sub_sortrows->set_name("sub_sortrows");
   sub_sortrows->add_item("Ascending", SortAscending);
   sub_sortrows->add_item("Descending", SortDescending);
   m_context_menu->add_child(sub_sortrows);

   godot::PopupMenu* sub_moverow = memnew(godot::PopupMenu);
   sub_moverow->set_name("sub_moverow");
   sub_moverow->add_item("Up", MoveRowUp);
   sub_moverow->add_item("Down", MoveRowDown);
   m_context_menu->add_child(sub_moverow);

   godot::PopupMenu* sub_insertrow = memnew(godot::PopupMenu);
   sub_insertrow->set_name("sub_insertrow");
   sub_insertrow->add_item("Above", InsertRowAbove);
   sub_insertrow->add_item("Bellow", InsertRowBellow);
   m_context_menu->add_child(sub_insertrow);

   // Value type menu - this will be filled dynamically, based on the assigned data source
   m_value_type_menu = memnew(godot::PopupMenu);
   m_value_type_menu->set_name("sub_valuetype");
   m_context_menu->add_child(m_value_type_menu);


   m_extra_parent = memnew(godot::Control);
   add_child(m_extra_parent, false, INTERNAL_MODE_BACK);
}



#endif
