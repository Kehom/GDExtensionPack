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

#ifndef FILEDIRPICKER_DISABLED

#include "filedir_picker.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


void FileDirPicker::on_browse_clicked()
{
   m_dialog->popup_centered_clamped(godot::Vector2(1000, 680));
}


void FileDirPicker::on_path_selected(const godot::String& path)
{
   m_value->set_text(path);
   emit_signal("path_selected", path);
}


void FileDirPicker::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_value->connect("focus_entered", callable_mp((godot::CanvasItem*)this, &godot::CanvasItem::queue_redraw));
         m_value->connect("focus_exited", callable_mp((godot::CanvasItem*)this, &godot::CanvasItem::queue_redraw));

         m_btbrowse->connect("pressed", callable_mp(this, &FileDirPicker::on_browse_clicked));

         m_dialog->connect("dir_selected", callable_mp(this, &FileDirPicker::on_path_selected));
         m_dialog->connect("file_selected", callable_mp(this, &FileDirPicker::on_path_selected));
      } break;

      case NOTIFICATION_THEME_CHANGED:
      {
         m_theme_cache.normal = get_theme_stylebox("normal", "LineEdit");
         m_theme_cache.focus = get_theme_stylebox("focus", "LineEdit");

         m_box->set_anchor_and_offset(godot::SIDE_LEFT, 0, m_theme_cache.normal->get_margin(godot::SIDE_LEFT));
         m_box->set_anchor_and_offset(godot::SIDE_TOP, 0, m_theme_cache.normal->get_margin(godot::SIDE_TOP));
         m_box->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -m_theme_cache.normal->get_margin(godot::SIDE_RIGHT));
         m_box->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -m_theme_cache.normal->get_margin(godot::SIDE_BOTTOM));

         queue_redraw();
      } break;

      case NOTIFICATION_DRAW:
      {
         if (!m_theme_cache.normal.is_valid()) { return; }

         const godot::Rect2 r(godot::Vector2(), get_size());
         
         draw_style_box(m_theme_cache.normal, r);
         if (m_value->has_focus())
         {
            draw_style_box(m_theme_cache.focus, r);
         }
      } break;

      
   }
}


void FileDirPicker::_bind_methods()
{
   using namespace godot;

   //ClassDB::bind_method(D_METHOD("set_allow_manual_edit", "enable"), &FileDirPicker::set_allow_manual_edit);
   //ClassDB::bind_method(D_METHOD("get_allow_manual_edit"), &FileDirPicker::get_allow_manual_edit);
   ClassDB::bind_method(D_METHOD("set_access", "access"), &FileDirPicker::set_access);
   ClassDB::bind_method(D_METHOD("get_access"), &FileDirPicker::get_access);
   ClassDB::bind_method(D_METHOD("set_current_dir", "dir"), &FileDirPicker::set_current_dir);
   ClassDB::bind_method(D_METHOD("get_current_dir"), &FileDirPicker::get_current_dir);
   ClassDB::bind_method(D_METHOD("set_current_file", "file"), &FileDirPicker::set_current_file);
   ClassDB::bind_method(D_METHOD("get_current_file"), &FileDirPicker::get_current_file);
   ClassDB::bind_method(D_METHOD("set_current_path", "path"), &FileDirPicker::set_current_path);
   ClassDB::bind_method(D_METHOD("get_current_path"), &FileDirPicker::get_current_path);
   ClassDB::bind_method(D_METHOD("set_pick_mode", "mode"), &FileDirPicker::set_pick_mode);
   ClassDB::bind_method(D_METHOD("get_pick_mode"), &FileDirPicker::get_pick_mode);
   ClassDB::bind_method(D_METHOD("set_filters", "filters"), &FileDirPicker::set_filters);
   ClassDB::bind_method(D_METHOD("get_filters"), &FileDirPicker::get_filters);
   ClassDB::bind_method(D_METHOD("set_root_subfolder", "dir"), &FileDirPicker::set_root_subfolder);
   ClassDB::bind_method(D_METHOD("get_root_subfolder"), &FileDirPicker::get_root_subfolder);
   ClassDB::bind_method(D_METHOD("set_show_hidden_files", "show"), &FileDirPicker::set_show_hidden_files);
   ClassDB::bind_method(D_METHOD("is_showing_hidden_files"), &FileDirPicker::is_showing_hidden_files);


   //ADD_PROPERTY(PropertyInfo(Variant::BOOL, "allow_manual_edit"), "set_allow_manual_edit", "get_allow_manual_edit");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "access", PROPERTY_HINT_ENUM, "Resources,User data,File system"), "set_access", "get_access");
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_dir"), "set_current_dir", "get_current_dir");
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_file"), "set_current_file", "get_current_file");
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "current_path"), "set_current_path", "get_current_path");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "pick_mode", PROPERTY_HINT_ENUM, "File,Directory"), "set_pick_mode", "get_pick_mode");
   ADD_PROPERTY(PropertyInfo(Variant::PACKED_STRING_ARRAY, "filters"), "set_filters", "get_filters");
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "root_subfolder"), "set_root_subfolder", "get_root_subfolder");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_hidden_files"), "set_show_hidden_files", "is_showing_hidden_files");

   BIND_ENUM_CONSTANT(PICK_MODE_FILE);
   BIND_ENUM_CONSTANT(PICK_MODE_DIRECTORY);

   ADD_SIGNAL(MethodInfo("path_selected", PropertyInfo(Variant::STRING, "path")));
}


godot::Vector2 FileDirPicker::_get_minimum_size() const
{
   godot::Vector2 ret = m_box->get_combined_minimum_size();
   if (m_theme_cache.normal.is_valid())
   {
      ret += m_theme_cache.normal->get_minimum_size();
   }
   
   return ret;
}


//void FileDirPicker::set_allow_manual_edit(bool enable)
//{
//   m_value->set_editable(enable);
//}


//bool FileDirPicker::get_allow_manual_edit() const
//{
//   return m_value->is_editable();
//}


void FileDirPicker::set_access(godot::FileDialog::Access access)
{
   m_dialog->set_access(access);
   m_value->set_text(m_dialog->get_current_path());
}


godot::FileDialog::Access FileDirPicker::get_access() const
{
   return m_dialog->get_access();
}


void FileDirPicker::set_current_dir(const godot::String& dir)
{
   m_dialog->set_current_dir(dir);
   m_value->set_text(m_dialog->get_current_path());
}


godot::String FileDirPicker::get_current_dir() const
{
   return m_dialog->get_current_dir();
}


void FileDirPicker::set_current_file(const godot::String& file)
{
   m_dialog->set_current_file(file);
   m_value->set_text(m_dialog->get_current_path());
}


godot::String FileDirPicker::get_current_file() const
{
   return m_dialog->get_current_file();
}


void FileDirPicker::set_current_path(const godot::String& path)
{
   m_dialog->set_current_path(path);
   m_value->set_text(m_dialog->get_current_path());
}


godot::String FileDirPicker::get_current_path() const
{
   return m_dialog->get_current_path();
}


void FileDirPicker::set_pick_mode(PickMode mode)
{
   if (mode == PICK_MODE_FILE)
   {
      m_dialog->set_file_mode(godot::FileDialog::FILE_MODE_OPEN_FILE);
   }
   else if (mode == PICK_MODE_DIRECTORY)
   {
      m_dialog->set_file_mode(godot::FileDialog::FILE_MODE_OPEN_DIR);
   }

   m_value->set_text(m_dialog->get_current_path());
}


FileDirPicker::PickMode FileDirPicker::get_pick_mode() const
{
   const godot::FileDialog::FileMode fmode = m_dialog->get_file_mode();
   if (fmode == godot::FileDialog::FILE_MODE_OPEN_FILE)
   {
      return PICK_MODE_FILE;
   }
   return PICK_MODE_DIRECTORY;
}


void FileDirPicker::set_filters(const godot::PackedStringArray& filter)
{
   m_dialog->set_filters(filter);
}


godot::PackedStringArray FileDirPicker::get_filters() const
{
   return m_dialog->get_filters();
}


void FileDirPicker::set_root_subfolder(const godot::String& root)
{
   m_dialog->set_root_subfolder(root);
   m_value->set_text(m_dialog->get_current_path());
}


godot::String FileDirPicker::get_root_subfolder() const
{
   return m_dialog->get_root_subfolder();
}


void FileDirPicker::set_show_hidden_files(bool enable)
{
   m_dialog->set_show_hidden_files(enable);
}


bool FileDirPicker::is_showing_hidden_files() const
{
   return m_dialog->is_showing_hidden_files();
}


FileDirPicker::FileDirPicker()
{
   godot::Ref<godot::StyleBoxEmpty> empty;
   empty.instantiate();
   m_theme_cache.empty = empty;

   m_box = memnew(godot::HBoxContainer);
   add_child(m_box, false, INTERNAL_MODE_BACK);


   m_value = memnew(godot::LineEdit);
   m_value->set_h_size_flags(SIZE_EXPAND_FILL);
   m_value->set_v_size_flags(SIZE_EXPAND_FILL);
   m_value->add_theme_stylebox_override("normal", empty);
   m_value->add_theme_stylebox_override("focus", empty);
   m_value->add_theme_stylebox_override("read_only", empty);
   m_value->set_editable(false);
   m_value->set_clear_button_enabled(true);
   m_value->set_clip_contents(true);
   m_box->add_child(m_value);

   m_btbrowse = memnew(godot::Button);
   m_btbrowse->set_text("...");
   m_box->add_child(m_btbrowse);

   m_dialog = memnew(godot::FileDialog);
   m_dialog->set_file_mode(godot::FileDialog::FILE_MODE_OPEN_FILE);
   add_child(m_dialog, false, INTERNAL_MODE_BACK);
}


#endif  //FILEDIRPICKER_DISABLED
