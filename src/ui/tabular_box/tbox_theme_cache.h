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

#ifndef _kehui_tabular_box_theme_cache_h_included
#define _kehui_tabular_box_theme_cache_h_included 1

// Normally the theme cache would be an internal struct/class (of the owning Control).
// However most of the drawing of the TabularBox is done by child controls. So an instance
// of this struct will be held by the TabularBox and given to the children.
// Derived column classes will have access to the cached theme data through functions. This
// means that even scripted columns will have access to this data, although not directly

#ifndef TABULAR_BOX_DISABLED

#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/font.hpp>


struct TBoxThemeCache
{
   godot::Ref<godot::StyleBox> stl_empty;

   godot::Ref<godot::StyleBox> background;
   godot::Ref<godot::StyleBox> header;
   godot::Ref<godot::StyleBox> row_number;

   godot::Ref<godot::StyleBox> odd_row;
   godot::Ref<godot::StyleBox> even_row;
   godot::Ref<godot::StyleBox> focus;

   godot::Ref<godot::StyleBox> h_scrollbar;
   godot::Ref<godot::StyleBox> v_scrollbar;
   godot::Ref<godot::StyleBox> scrollbar_grabber;
   godot::Ref<godot::StyleBox> scrollbar_highlight;
   godot::Ref<godot::StyleBox> scrollbar_grabber_pressed;

   // Not every theme items of the button are used. Reason for this is that some of them are related to text.
   // The buttons used within the TabularBox are not using text!
   godot::Ref<godot::StyleBox> button_normal;
   godot::Ref<godot::StyleBox> button_hover;
   godot::Ref<godot::StyleBox> button_pressed;

   godot::Ref<godot::Texture2D> unchecked;
   godot::Ref<godot::Texture2D> checked;
   
   godot::Ref<godot::Texture2D> down_arrow;
   godot::Ref<godot::Texture2D> left_arrow;
   godot::Ref<godot::Texture2D> right_arrow;
   godot::Ref<godot::Texture2D> trash_bin;
   godot::Ref<godot::Texture2D> no_texture;

   godot::Ref<godot::Font> header_font;
   godot::Ref<godot::Font> cell_font;

   godot::Color header_text;
   godot::Color header_selected;
   godot::Color header_selection;
   godot::Color cell_text;
   godot::Color cell_selected;
   godot::Color cell_selection;
   godot::Color caret;

   int32_t header_font_size;
   int32_t cell_font_size;
   int32_t separation;


   TBoxThemeCache() : header_font_size(16), cell_font_size(16), separation(2) {}
};


#endif      //TABULAR_BOX_DISABLED

#endif     //_kehui_tabular_box_theme_cache_h_included
