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

#ifndef _kehextension_pack_utilities_h_included
#define _kehextension_pack_utilities_h_included 1

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot
{
   class Control;
   class Texture2D;
}

class ExtPackUtils : public godot::RefCounted
{
   GDCLASS(ExtPackUtils, godot::RefCounted);
private:
   /// Internal variables

   /// Exposed variables

   /// Internal functions

   /// Event handlers

protected:
   static void _bind_methods();
public:
   /// Overrides

   /// Exposed virtual functions

   /// Exposed functions
   // Query if specific General plugins are compiled
   static bool has_quantization();
   
   // Query if specific Node plugins are compiled
   static bool has_audio_master();
   static bool has_auto_interpolate();
   static bool has_smooth2d();
   static bool has_smooth3d();
   static bool has_radial_impulse2d();
   static bool has_radial_impulse3d();

   // Query if specific UI plugins are compiled
   static bool has_accordion();
   static bool has_expandable_panel();
   static bool has_filedir_picker();
   static bool has_spin_slider();
   static bool has_tabular_box();


   // The actual "utilities"

   // Given a texture and a size constraint, returns a size that can be used with draw_texture_rect() in which the texture
   // will be drawn with correct aspect ratio without exceeding the constraints
   static godot::Size2i get_draw_rect_size(const godot::Ref<godot::Texture2D>& texture, const godot::Size2i& constraint);


   // Allow setup of the four anchors of the incoming Control with a single call
   static void set_control_anchors(godot::Control* control, float left, float top, float right, float bottom);

   // Allow setup of the four offsets of the incoming Control with a single call
   static void set_control_offsets(godot::Control* control, float left, float top, float right, float bottom);

   // Given a Dictionary, rename a key, "preserving" its value. The thing is, there is no out of the box function to do that
   // This simply creates an entry for the new name copying the value of the old entry, then delete the old one.
   // This function does not check previous existence of "key_to". And if "key_from" does not exist nothing will happen
   static void change_dictionary_key(godot::Dictionary dict, const godot::Variant& key_from, const godot::Variant& key_to);


   /// Setters/Getters

   /// Public non exposed functions
   ExtPackUtils() {}
};


#endif   // _kehextension_pack_utilities_h_included

