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

#ifndef ALL_UI_DISABLED
#ifndef EDITOR_DISABLED

#include "editor_theme_builder.h"
#include "custom_theme.h"

#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/file_dialog.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/theme.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


void UIThemeBuilder::on_generator_selected()
{
   if (!m_file_dlg)
   {
      godot::PackedStringArray filters;
      filters.append("*.theme ; Theme Resources");
      filters.append("*.tres ; Text Resource");
      filters.append("*.res ; Binary Resource");

      m_file_dlg = memnew(godot::FileDialog);
      m_file_dlg->set_filters(filters);
      m_file_dlg->set_file_mode(godot::FileDialog::FILE_MODE_SAVE_FILE);
      m_file_dlg->set_access(godot::FileDialog::ACCESS_RESOURCES);

      get_editor_interface()->get_base_control()->add_child(m_file_dlg);

      m_file_dlg->connect("file_selected", callable_mp(this, &UIThemeBuilder::on_file_selected));
   }

   m_file_dlg->popup_centered(godot::Size2i(700, 500));
}


void UIThemeBuilder::on_file_selected(const godot::String& path)
{
   godot::Ref<godot::Theme> theme;
   theme.instantiate();

   CustomControlThemeDB::get_singleton()->fill_theme_resource(theme);

   if (godot::ResourceSaver::get_singleton()->save(theme, path) == godot::OK)
   {
      godot::UtilityFunctions::print(godot::vformat("Created theme filled with custom entries at '%s'", path));
   }
   else
   {
      WARN_PRINT(godot::vformat("Failed to create Theme resource at '%s'", path));
   }
}


void UIThemeBuilder::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_ENTER_TREE:
      {
         add_tool_menu_item("Custom Control Theme Generator...", callable_mp(this, &UIThemeBuilder::on_generator_selected));
      } break;

      case NOTIFICATION_EXIT_TREE:
      {
         if (m_file_dlg)
         {
            //memdelete(m_file_dlg);
            m_file_dlg->queue_free();
         }
         m_file_dlg = nullptr;

         remove_tool_menu_item("Custom Control Theme Generator...");
      } break;


   }
}


UIThemeBuilder::UIThemeBuilder()
{
   m_file_dlg = nullptr;
}


#endif   // EDITOR_DISABLED
#endif   //ALL_UI_DISABLED

