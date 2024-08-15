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

#include "custom_theme.h"

#ifndef ALL_UI_DISABLED

#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/theme.hpp>


CustomControlThemeDB* CustomControlThemeDB::s_singleton = nullptr;


CustomControlThemeDB::custom_theme& CustomControlThemeDB::get_theme(const godot::StringName& ctrl_name)
{
   if (!m_theme_map.has(ctrl_name))
   {
      m_theme_map[ctrl_name] = custom_theme();
   }

   return m_theme_map[ctrl_name];
}


void CustomControlThemeDB::_bind_methods()
{

}


void CustomControlThemeDB::create_icon(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Ref<godot::Texture2D>& def, const icon_updater_type& updater)
{
   item_icon_type item;
   item.updater = updater;
   item.def_val = def;
   get_theme(ctrl_name).icon_map[item_name] = item;
}

void CustomControlThemeDB::create_style(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Ref<godot::StyleBox>& def, const style_updater_type& updater)
{
   item_style_type item;
   item.updater = updater;
   item.def_val = def;
   get_theme(ctrl_name).style_map[item_name] = item;
}

void CustomControlThemeDB::create_font(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Ref<godot::Font>& def, const font_updater_type& updater)
{
   item_font_type item;
   item.updater = updater;
   item.def_val = def;
   get_theme(ctrl_name).font_map[item_name] = item;
}

void CustomControlThemeDB::create_font_size(const godot::StringName& ctrl_name, const godot::StringName& item_name, int def, const font_size_updater_type& updater)
{
   item_font_size_type item;
   item.updater = updater;
   item.def_val = def;
   get_theme(ctrl_name).font_size_map[item_name] = item;
}

void CustomControlThemeDB::create_color(const godot::StringName& ctrl_name, const godot::StringName& item_name, const godot::Color& def, const color_updater_type& updater)
{
   item_color_type item;
   item.updater = updater;
   item.def_val = def;
   get_theme(ctrl_name).color_map[item_name] = item;
}

void CustomControlThemeDB::create_constant(const godot::StringName& ctrl_name, const godot::StringName& item_name, int def, const constant_updater_type& updater)
{
   item_constant_type item;
   item.updater = updater;
   item.def_val = def;
   get_theme(ctrl_name).constant_map[item_name] = item;
}


void CustomControlThemeDB::update_control(godot::Control* ctrl)
{
   theme_map_type::Iterator iter = m_theme_map.find(ctrl->get_class());
   if (iter == m_theme_map.end()) { return; }

   custom_theme& theme = iter->value;

   // Update the colors
   for (color_map_type::Iterator iter = theme.color_map.begin(); iter != theme.color_map.end(); ++iter)
   {
      iter->value.updater(ctrl, iter->value.def_val);
   }

   // Update constants
   for (constant_map_type::Iterator iter = theme.constant_map.begin(); iter != theme.constant_map.end(); ++iter)
   {
      iter->value.updater(ctrl, iter->value.def_val);
   }

   // Update fonts
   for (font_map_type::Iterator iter = theme.font_map.begin(); iter != theme.font_map.end(); ++iter)
   {
      iter->value.updater(ctrl, iter->value.def_val);
   }

   // Update font sizes
   for (font_size_map_type::Iterator iter = theme.font_size_map.begin(); iter != theme.font_size_map.end(); ++iter)
   {
      iter->value.updater(ctrl, iter->value.def_val);
   }

   // Update icons
   for (icon_map_type::Iterator iter = theme.icon_map.begin(); iter != theme.icon_map.end(); ++iter)
   {
      iter->value.updater(ctrl, iter->value.def_val);
   }

   // Update style boxes
   for (style_map_type::Iterator iter = theme.style_map.begin(); iter != theme.style_map.end(); ++iter)
   {
      iter->value.updater(ctrl, iter->value.def_val);
   }

}


void CustomControlThemeDB::fill_property_list(const godot::StringName& ctrl_name, godot::List<godot::PropertyInfo>* list, const godot::Control* ctrl) const
{
   using namespace godot;

   theme_map_type::ConstIterator iter = m_theme_map.find(ctrl_name);
   if (iter == m_theme_map.end()) { return; }

   const custom_theme& theme = iter->value;
   const uint32_t default_usage = PROPERTY_USAGE_EDITOR | PROPERTY_USAGE_CHECKABLE | PROPERTY_USAGE_STORAGE;
   const uint32_t overridden_usage = default_usage | PROPERTY_USAGE_CHECKED;

   list->push_back(PropertyInfo(Variant::NIL, "Theme Overrides", PROPERTY_HINT_NONE, "theme_override_", PROPERTY_USAGE_GROUP));

   // godot::Color overrides
   {
      for (color_map_type::ConstIterator iter = theme.color_map.begin(); iter != theme.color_map.end(); ++iter)
      {
         const uint32_t usage = ctrl->has_theme_color_override(iter->key) ? default_usage : overridden_usage;

         list->push_back(PropertyInfo(Variant::COLOR, String("theme_override_colors/") + iter->key, PROPERTY_HINT_NONE, "", usage));
      }
   }

   // "Constant" overrides
   {
      for (constant_map_type::ConstIterator iter = theme.constant_map.begin(); iter != theme.constant_map.end(); ++iter)
      {
         const uint32_t usage = ctrl->has_theme_constant_override(iter->key) ? default_usage : overridden_usage;

         list->push_back(PropertyInfo(Variant::INT, String("theme_override_constants/") + iter->key, PROPERTY_HINT_RANGE, "-16384,16384", usage));
      }
   }

   // godot::Font overrides
   {
      for (font_map_type::ConstIterator iter = theme.font_map.begin(); iter != theme.font_map.end(); ++iter)
      {
         const uint32_t usage = ctrl->has_theme_font_override(iter->key) ? default_usage : overridden_usage;

         list->push_back(PropertyInfo(Variant::OBJECT, String("theme_override_fonts/") + iter->key, PROPERTY_HINT_RESOURCE_TYPE, "Font", usage));
      }
   }

   // Font size overrides
   {
      for (font_size_map_type::ConstIterator iter = theme.font_size_map.begin(); iter != theme.font_size_map.end(); ++iter)
      {
         const uint32_t usage = ctrl->has_theme_font_size_override(iter->key) ? default_usage : overridden_usage;

         list->push_back(PropertyInfo(Variant::INT, String("theme_override_font_sizes/") + iter->key, PROPERTY_HINT_RANGE, "1,256,1,or_greater,suffix:px", usage));
      }
   }

   // Icon (godot::Texture2D) overrides
   {
      for (icon_map_type::ConstIterator iter = theme.icon_map.begin(); iter != theme.icon_map.end(); ++iter)
      {
         const uint32_t usage = ctrl->has_theme_icon_override(iter->key) ? default_usage : overridden_usage;

         list->push_back(PropertyInfo(Variant::OBJECT, String("theme_override_icons/") + iter->key, PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", usage));
      }
   }

   // godot::StyleBox overrides
   {
      for (style_map_type::ConstIterator iter = theme.style_map.begin(); iter != theme.style_map.end(); ++iter)
      {
         const uint32_t usage = ctrl->has_theme_stylebox_override(iter->key) ? default_usage : overridden_usage;

         list->push_back(PropertyInfo(Variant::OBJECT, String("theme_override_styles/") + iter->key, PROPERTY_HINT_RESOURCE_TYPE, "StyleBox", usage));
      }
   }
}


void CustomControlThemeDB::fill_theme_resource(godot::Ref<godot::Theme>& theme) const
{
   for (const godot::KeyValue<godot::StringName, custom_theme>& pair : m_theme_map)
   {
      theme->add_type(pair.key);

      // Add registered icons of current custom control into the theme
      for (const godot::KeyValue<godot::StringName, item_icon_type>& icon_pair : pair.value.icon_map)
      {
         theme->set_icon(icon_pair.key, pair.key, icon_pair.value.def_val->duplicate());
      }

      // Add registered style boxes of current custom control into the theme
      for (const godot::KeyValue<godot::StringName, item_style_type>& style_pair : pair.value.style_map)
      {
         theme->set_stylebox(style_pair.key, pair.key, style_pair.value.def_val->duplicate());
      }

      // Add registered fonts of current custom control into the theme
      for (const godot::KeyValue<godot::StringName, item_font_type>& font_pair : pair.value.font_map)
      {
         godot::Ref<godot::Font> fnt = font_pair.value.def_val;
         if (fnt.is_valid())
         {
            fnt = fnt->duplicate();
         }
         theme->set_font(font_pair.key, pair.key, fnt);
      }

      // Add registered font sizes of current custom control into the theme
      for (const godot::KeyValue<godot::StringName, item_font_size_type>& fsize_pair : pair.value.font_size_map)
      {
         theme->set_font_size(fsize_pair.key, pair.key, fsize_pair.value.def_val);
      }

      // Add registered colors of current custom control into the theme
      for (const godot::KeyValue<godot::StringName, item_color_type>& color_pair : pair.value.color_map)
      {
         theme->set_color(color_pair.key, pair.key, color_pair.value.def_val);
      }

      // Add registered constants of current custom control into the theme
      for (const godot::KeyValue<godot::StringName, item_constant_type>& const_pair : pair.value.constant_map)
      {
         theme->set_constant(const_pair.key, pair.key, const_pair.value.def_val);
      }
   }
}


CustomControlThemeDB::CustomControlThemeDB()
{
   ERR_FAIL_COND(s_singleton != nullptr);
   s_singleton = this;
}

CustomControlThemeDB::~CustomControlThemeDB()
{
   m_theme_map.clear();
   s_singleton = nullptr;
}



#endif
