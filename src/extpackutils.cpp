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

#include "extpackutils.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/texture2d.hpp>


void ExtPackUtils::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_quantization"), &ExtPackUtils::has_quantization);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_audio_master"), &ExtPackUtils::has_audio_master);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_auto_interpolate"), &ExtPackUtils::has_auto_interpolate);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_smooth2d"), &ExtPackUtils::has_smooth2d);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_smooth3d"), &ExtPackUtils::has_smooth3d);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_radial_impulse2d"), &ExtPackUtils::has_radial_impulse2d);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_radial_impulse3d"), &ExtPackUtils::has_radial_impulse3d);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_accordion"), &ExtPackUtils::has_accordion);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_expandable_panel"), &ExtPackUtils::has_expandable_panel);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_filedir_picker"), &ExtPackUtils::has_filedir_picker);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_spin_slider"), &ExtPackUtils::has_spin_slider);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("has_tabular_box"), &ExtPackUtils::has_tabular_box);

   //void ExtPackUtils::rename_dictionary_entry(godot::Dictionary& dict, const godot::Variant& key_from, const godot::Variant& key_to)
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("get_draw_rect_size", "texture", "constraint"), &ExtPackUtils::get_draw_rect_size);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("set_control_anchors", "control", "left", "top", "right", "bottom"), &ExtPackUtils::set_control_anchors);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("set_control_offsets", "control", "left", "top", "right", "bottom"), &ExtPackUtils::set_control_offsets);
   ClassDB::bind_static_method("ExtPackUtils", D_METHOD("change_dictionary_key", "dict", "key_from", "key_to"), &ExtPackUtils::change_dictionary_key);
}



bool ExtPackUtils::has_quantization()
{
#ifndef QUANTIZE_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_audio_master()
{
#ifndef AUDIO_MASTER_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_auto_interpolate()
{
#ifndef AUTO_INTERPOLATE_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_smooth2d()
{
#ifndef SMOOTH2D_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_smooth3d()
{
#ifndef SMOOTH3D_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_radial_impulse2d()
{
#ifndef RADIAL_IMPULSE2D_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_radial_impulse3d()
{
#ifndef RADIAL_IMPULSE3D_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_accordion()
{
#ifndef ACCORDION_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_expandable_panel()
{
#ifndef EXPANDABLE_PANEL_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_filedir_picker()
{
#ifndef FILEDIRPICKER_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_spin_slider()
{
#ifndef SPIN_SLIDER_DISABLED
   return true;
#else
   return false;
#endif
}


bool ExtPackUtils::has_tabular_box()
{
#ifndef TABULAR_BOX_DISABLED
   return true;
#else
   return false;
#endif
}



godot::Size2i ExtPackUtils::get_draw_rect_size(const godot::Ref<godot::Texture2D>& texture, const godot::Size2i& constraint)
{
   const float w = texture->get_width();
   const float h = texture->get_height();

   if (w == h)
   {
      return constraint;
   }

   const float s1 = constraint.width / w;
   const float s2 = constraint.height / h;

   const float s = s1 > s2 ? s2 : s1;

   return godot::Size2i(texture->get_width() * s, texture->get_height() * s);
}


void ExtPackUtils::set_control_anchors(godot::Control* control, float left, float top, float right, float bottom)
{
   control->set_anchor(godot::SIDE_LEFT, left);
   control->set_anchor(godot::SIDE_TOP, top);
   control->set_anchor(godot::SIDE_RIGHT, right);
   control->set_anchor(godot::SIDE_BOTTOM, bottom);
}


void ExtPackUtils::set_control_offsets(godot::Control* control, float left, float top, float right, float bottom)
{
   control->set_offset(godot::SIDE_LEFT, left);
   control->set_offset(godot::SIDE_TOP, top);
   control->set_offset(godot::SIDE_RIGHT, right);
   control->set_offset(godot::SIDE_BOTTOM, bottom);
}


void ExtPackUtils::change_dictionary_key(godot::Dictionary dict, const godot::Variant& key_from, const godot::Variant& key_to)
{
   //ERR_FAIL_COND_MSG(!dict.has(key_from), godot::vformat("Attempting to change dictionary's '%s' key, but it does not exist.", key_from));
   if (!dict.has(key_from)) { return; }

   const godot::Variant& value = dict[key_from];
   dict[key_to] = value;
   dict.erase(key_from);
}

