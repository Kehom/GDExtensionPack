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

/// Even though it's possible to call ClassDB::get_default_theme(), adding custom theme elements into it results
/// in a problem. If done at initialization then Godot Editor loads very slowly. If delayed to another moment then
/// it will result in a "non responsive" time of the editor. The reason is that each time a new element
/// is added into the theme the notification _THEME_CHANGED is given to all instanced Control objects. And well,
/// the editor is full of them!
/// https://github.com/godotengine/godot-cpp/issues/1332
/// To still allow custom controls to have their own theming this class has been created in order to help with this
/// task. The thing is, custom theme elements are displayed in the inspector through the _get_property_list() system.
/// This class is meant to automate at least part of this. Custom controls still need to override the functions trio,
/// but this class can be used to help fill the list of properties as well as get and set the appropriate values.
///
/// One thing that is done in the core Controls is that theme items are cached. And the ThemeDB sort of "automates" the
/// updating of the cached data.
/// The way this is done is by assigning a functor into each registered theme item property. That functor is meant to
/// update the cache. Registration is done through a macro that created a lambda and use that as the functor.
///
/// This class follows a similar approach, however there are a few differences:
/// 1) Instead of one macro dealing with all theme item types, there is one macro for each type. The reason for this is
///    the fact that in the core there is a Control::get_theme_item(), which does not exist in GDExtension
/// 2) The macros used for this class require the default theme item value to given, so upon registration the default
///    value must exist.
/// 3) The base Control class deals with the theme changed notification and automatically calls the required functors.
///    Custom controls will have to call CustomControlThemeDB::get_instance()->update_control(this); when dealing with
///    said notification. This class will then take care of calling the functors.
/// 4) The base Control class overrides the _get_property_list(), _get() and _set() functions to build a list of
///    entries within the Inspector meant to add theme overrides. Custom controls will have to call a function from
///    this class to build the list of items within the Inspector. From the _get_property_list() call
///    CustomControlThemeDB::get_instance()->fill_property_list(prop_list) providing the list argument. This class
///    will then take care of filling the entries in a way that it should not be necessary to also deal with _set()
///    _get() functions. Obviously that if the custom control does indeed add extra properties, make sure to call
///    the CustomControlThemeDB near the end of _get_property_list() so the overrides are populated at the end just
///    like core controls. Then in the _set() and _get() functions only deal with the relevant properties, ignoring
///    the custom theme entries.



#ifndef _kehui_custom_theme_h_included
#define _kehui_custom_theme_h_included 1

#ifndef ALL_UI_DISABLED

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <functional>

#include <godot_cpp/variant/utility_functions.hpp>

// Forward declares
namespace godot
{
   class Font;
   class Texture2D;
   class StyleBox;
   class Theme;
}

class CustomControlThemeDB : public godot::Object
{
   GDCLASS(CustomControlThemeDB, godot::Object);
private:
   // Whenever a custom theme item is registered an instance of this struct will be created and
   // added into the relevant map associated with the control performing the registration
   template <class T>
   struct theme_item
   {
      typedef std::function<void(godot::Control*, const T)> functor_type;

      // This functor is used to update the theme cache
      functor_type updater;
      // This is the default value of the theme item
      T def_val;
   };

   typedef theme_item<godot::Ref<godot::Texture2D>> item_icon_type;
   typedef theme_item<godot::Ref<godot::StyleBox>> item_style_type;
   typedef theme_item<godot::Ref<godot::Font>> item_font_type;
   typedef theme_item<int> item_font_size_type;
   typedef theme_item<godot::Color> item_color_type;
   typedef theme_item<int> item_constant_type;

   typedef godot::HashMap<godot::StringName, item_icon_type> icon_map_type;
   typedef godot::HashMap<godot::StringName, item_style_type> style_map_type;
   typedef godot::HashMap<godot::StringName, item_font_type> font_map_type;
   typedef godot::HashMap<godot::StringName, item_font_size_type> font_size_map_type;
   typedef godot::HashMap<godot::StringName, item_color_type> color_map_type;
   typedef godot::HashMap<godot::StringName, item_constant_type> constant_map_type;

   // Each control will have an instance of this struct
   struct custom_theme
   {
      icon_map_type icon_map;
      style_map_type style_map;
      font_map_type font_map;
      font_size_map_type font_size_map;
      color_map_type color_map;
      constant_map_type constant_map;
   };

   typedef godot::HashMap<godot::StringName, custom_theme> theme_map_type;
   

   /// Internal variables
   static CustomControlThemeDB* s_singleton;

   theme_map_type m_theme_map;

   /// Exposed variables

   /// Internal functions
   custom_theme& get_theme(const godot::StringName& ctrl_name);

   /// Event handlers

protected:
   //void _notification(int what);
   static void _bind_methods();
public:
   typedef typename item_icon_type::functor_type icon_updater_type;
   typedef typename item_style_type::functor_type style_updater_type;
   typedef typename item_font_type::functor_type font_updater_type;
   typedef typename item_font_size_type::functor_type font_size_updater_type;
   typedef typename item_color_type::functor_type color_updater_type;
   typedef typename item_constant_type::functor_type constant_updater_type;

   /// Overrides

   /// Exposed functions
   // Used to register a new theme item for the given custom control name. To make things easier use one of the macros defined
   // bellow this class as it takes care of defining the functor
   void create_icon(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Ref<godot::Texture2D>& def, const icon_updater_type& updater);
   void create_style(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Ref<godot::StyleBox>& def, const style_updater_type& updater);
   void create_font(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Ref<godot::Font>& def, const font_updater_type& updater);
   void create_font_size(const godot::StringName& ctrl_name, const godot::StringName& item_name, int def, const font_size_updater_type& updater);
   void create_color(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Color& def, const color_updater_type& updater);
   void create_constant(const godot::StringName& ctrl_name, const godot::StringName& item_name, int def, const constant_updater_type& updater);

   // This must be called when the Control receives the theme changed notification. It's also a good idea to call this
   // in the post initialize notification.
   void update_control(godot::Control* ctrl);

   // Returns true if there is a theme for the given Control. This can be useful to perform the registration of theme
   // items only once.
   bool has_theme(const godot::StringName& ctrl_name) const { return m_theme_map.has(ctrl_name); }


   // Because of how the core godot::Control works, there is no need to override _set() and _get() specifically for the custom theme
   // entries!
   void fill_property_list(const godot::StringName& ctrl_name, godot::List<godot::PropertyInfo>* list, const godot::Control* ctrl) const;


   // Fill the incoming theme resource with all registered theme entries
   void fill_theme_resource(godot::Ref<godot::Theme>& theme) const;


   /// Setters/Getters

   /// Public non exposed functions
   static CustomControlThemeDB* get_singleton() { return s_singleton; }

   CustomControlThemeDB();
   ~CustomControlThemeDB();
};


// Register a theme icon within the custom control
#define REGISTER_CUSTOM_ICON(iname, cname, default_val, cache) \
   CustomControlThemeDB::get_singleton()->create_icon(#cname, #iname, default_val, [](godot::Control* ctrl, const godot::Ref<godot::Texture2D>& val) { \
      cname* ccast = Object::cast_to<cname>(ctrl); \
      ccast->cache.iname = ccast->has_theme_icon(#iname, #cname) || ccast->has_theme_icon_override(#iname) ? ccast->get_theme_icon(#iname, #cname) : val; \
   })

// Register a theme style box within the custom control
#define REGISTER_CUSTOM_STYLE(iname, cname, default_val, cache) \
   CustomControlThemeDB::get_singleton()->create_style(#cname, #iname, default_val, [](godot::Control* ctrl, const godot::Ref<godot::StyleBox>& val) { \
      cname* ccast = Object::cast_to<cname>(ctrl);    \
      ccast->cache.iname = ccast->has_theme_stylebox(#iname, #cname) || ccast->has_theme_stylebox_override(#iname) ? ccast->get_theme_stylebox(#iname, #cname) : val;   \
   })

// Register a theme font within the custom control
#define REGISTER_CUSTOM_FONT(iname, cname, default_val, cache) \
   CustomControlThemeDB::get_singleton()->create_font(#cname, #iname, default_val, [](godot::Control* ctrl, const godot::Ref<godot::Font>& val) { \
      cname* ccast = Object::cast_to<cname>(ctrl);   \
      ccast->cache.iname = ccast->has_theme_font(#iname, #cname) || ccast->has_theme_font_override(#iname) ? ccast->get_theme_font(#iname, #cname) : val;   \
   })

// Register a theme font size within the custom control
#define REGISTER_CUSTOM_FONT_SIZE(iname, cname, default_val, cache) \
   CustomControlThemeDB::get_singleton()->create_font_size(#cname, #iname, default_val, [](godot::Control* ctrl, int val) { \
      cname* ccast = Object::cast_to<cname>(ctrl);   \
      ccast->cache.iname = ccast->has_theme_font_size(#iname, #cname) || ccast->has_theme_font_size_override(#iname) ? ccast->get_theme_font_size(#iname, #cname) : val; \
   })

// Register a theme color within the custom control
#define REGISTER_CUSTOM_COLOR(iname, cname, default_val, cache) \
   CustomControlThemeDB::get_singleton()->create_color(#cname, #iname, default_val, [](godot::Control* ctrl, const godot::Color& val) { \
      cname* ccast = Object::cast_to<cname>(ctrl);   \
      ccast->cache.iname = ccast->has_theme_color(#iname, #cname) || ccast->has_theme_color_override(#iname) ? ccast->get_theme_color(#iname, #cname) : val; \
   })

// Register a theme constant within the custom control
#define REGISTER_CUSTOM_CONSTANT(iname, cname, default_val, cache) \
   CustomControlThemeDB::get_singleton()->create_constant(#cname, #iname, default_val, [](godot::Control* ctrl, int val) { \
      cname* ccast = Object::cast_to<cname>(ctrl);   \
      ccast->cache.iname = ccast->has_theme_constant(#iname, #cname) || ccast->has_theme_constant_override(#iname) ? ccast->get_theme_constant(#iname, #cname) : val; \
   })



#endif     // ALL_UI_DISABLED

#endif      // safe include
