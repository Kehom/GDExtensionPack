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



#ifndef _kehgdextpack_internal_h_included
#define _kehgdextpack_internal_h_included 1

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/templates/hash_map.hpp>


// Forward declares
namespace godot
{
   class Control;
   class Font;
   class ImageTexture;
   class Node2D;
   class Script;
   class StyleBoxFlat;
   class Texture2D;
   class Window;
}


class Internals
{
public:
   // Retrieve the scene tree root
   static godot::Window* get_tree_root();

   // Enable or disable the specific flag within the incoming bitset
   static void set_flag(bool enabled, uint32_t flag, uint32_t& out_bitset) { out_bitset = enabled ? (out_bitset | flag) : (out_bitset & ~flag); }

   // Return true if the specified flag is set within the incoming bitset
   static bool is_flag_set(uint32_t flag, uint32_t bitset) { return bitset & flag; }


   // Takes base64 data which should be encoding a PNG file and converts into ImageTexture.
   // There is zero checking if the incoming data is valid or not.
   static godot::Ref<godot::ImageTexture> texture_from_base64(const char* data);


   // Connect the incoming event handler (Callable) into the specified event of the given object.
   // This function is here simply because it checks first if the event is not already connected to the
   // given Callable. This is mostly to avoid having warning messages
   static void connector(godot::Object* obj, const godot::StringName& event, const godot::Callable& handler);

   // Disconnect the incoming event handler (Callable) from the specified event of the given object.
   // This function is here simply because it checks first if the event handler is actually connected into
   // the give event
   static void disconnector(godot::Object* obj, const godot::StringName& event, const godot::Callable& handler);


   // Given a font and a "box height", calculate the Y coordinate so text can be vertically centered within that "box"
   static float get_text_vertical_center(const godot::Ref<godot::Font>& font, int32_t font_size, float box_height);


   // In the Core there are GLOBAL_DEF and GLOBAL_GET macros used to set and get values from the ProjectSettings.
   // Those don't exist in GDExtension. The next two functions are used to perform those two tasks
   static godot::Variant global_set(const godot::String& var, const godot::Variant& defval, bool restart = false, bool basic = false, bool internal = false);
   static godot::Variant global_get(const godot::String& var);


   // Create a style box following a function similar to the one found in the "scene/theme/default_theme.cpp" file that
   // is part of the core Godot
   static godot::Ref<godot::StyleBoxFlat> make_flat_stylebox(const godot::Color& color, float margin_left = 4, float margin_top = 4, float margin_right = 4, float margin_bottom = 4, float corner_radius = 3, bool draw_center = true, int border_width = 0);
   
   // Within the core source code there are two functions defined in the "core/math/math_funcs.cpp" file that are used
   // by the SpinBox class and some others. Nevertheless, there is a comment in there telling that it's only meant
   // for editor. Those help format text based on a "step", controlling amount of decimals to be shown. Add those
   // functions here. Indeed, the codes are basically copy/paste
   static int step_decimals(double step);
   static int range_step_decimals(double step);


   // Provided an array (godot::Vector), move an element from 'from' index/position into 'to' index/position
   template <class T>
   static void move_array_element(godot::Vector<T>& array, int64_t from, int64_t to)
   {
      T element = array[from];
      array.remove_at(from);

      if (to >= array.size())
      {
         array.append(element);
      }
      else
      {
         array.insert(to, element);
      }
   }


   // Provides "min()" and "max()" with an arbitrary number of arguments instead of just two.
   template <class T, class ...R>
   inline static T vmax(T a, R ... r) { return cmax(a, r...); }

   template <class T, class ...R>
   inline static T vmin(T a, R ... r) { return cmin(a, r...); }

protected:

private:
   template <class T>
   inline static T cmax(T a) { return a; }

   template <class T, class ...R>
   inline static T cmax(T a, T b, R ... r) { return cmax(a > b ? a : b, r...); }

   template <class T>
   inline static T cmin(T a) { return a; }

   template <class T, class ...R>
   inline static T cmin(T a, T b, R ... r) { return cmin(a < b ? a : b, r...); }
};


// In the Core Godot source, including "script_language.h" provides access to `ScriptServer`, which contains information regarding all "global" classes.
// Global classes in this sense are scripts that contain a "class_name". There are also several functions that make querying information very easy. Of
// particular interest is checking if a script class inherits from a another specified one.
// Since "ScriptServer" is not exposed to GDExtension, a work around is required. This can be done by first obtaining the full list of scripted classes,
// which can be done by "ProjectSettings::get_singleton()->get_global_class_list()". To help reduce the number of times in which this list must be
// iterated through, this class has been created, which duplicates some of the code in "ScriptServer". Another aspect here is that some other functions
// can be created that depend on the data
class GlobalClassCache
{
private:
   struct ScriptClass
   {
      //godot::StringName language;
      godot::String path;
      godot::StringName base;
   };

   static godot::HashMap<godot::StringName, ScriptClass> m_global_class;

public:


   static void scan_global_classes();

   static bool is_global_class(const godot::StringName& class_name);
   static godot::String get_global_class_path(const godot::StringName& class_name);
   static godot::StringName get_global_class_base(const godot::StringName& class_name);
   static godot::StringName get_global_class_native_base(const godot::StringName& class_name);

};


#endif

