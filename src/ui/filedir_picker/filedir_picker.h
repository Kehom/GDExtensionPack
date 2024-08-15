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

#ifndef _kehui_filedir_picker_h_included
#define _kehui_filedir_picker_h_included 1

#ifndef FILEDIRPICKER_DISABLED

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/file_dialog.hpp>

namespace godot
{
   class Button;
   class FileDialog;
   class HBoxContainer;
   class LineEdit;
   class StyleBox;
}

class FileDirPicker : public godot::Control
{
   GDCLASS(FileDirPicker, godot::Control);
public:
   enum PickMode
   {
      PICK_MODE_FILE,
      PICK_MODE_DIRECTORY,
   };
private:
   /// Internal variables
   struct
   {
      godot::Ref<godot::StyleBox> empty;

      godot::Ref<godot::StyleBox> normal;
      godot::Ref<godot::StyleBox> focus;
   } m_theme_cache;

   godot::HBoxContainer* m_box;
   godot::LineEdit* m_value;
   godot::Button* m_btbrowse;

   godot::FileDialog* m_dialog;

   /// Exposed variables


   /// Internal functions

   /// Event handlers
   void on_browse_clicked();
   void on_path_selected(const godot::String& path);
   void on_text_changed(const godot::String& text);


protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   godot::Vector2 _get_minimum_size() const;        // override

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters
   //void set_allow_manual_edit(bool enable);
   //bool get_allow_manual_edit() const;

   void set_access(godot::FileDialog::Access access);
   godot::FileDialog::Access get_access() const;

   void set_current_dir(const godot::String& dir);
   godot::String get_current_dir() const;

   void set_current_file(const godot::String& file);
   godot::String get_current_file() const;

   void set_current_path(const godot::String& path);
   godot::String get_current_path() const;

   void set_pick_mode(PickMode mode);
   PickMode get_pick_mode() const;

   void set_filters(const godot::PackedStringArray& filter);
   godot::PackedStringArray get_filters() const;

   void set_root_subfolder(const godot::String& root);
   godot::String get_root_subfolder() const;

   void set_show_hidden_files(bool enable);
   bool is_showing_hidden_files() const;

   /// Public non exposed functions
   FileDirPicker();
};

VARIANT_ENUM_CAST(FileDirPicker::PickMode);


#endif  //FILEDIRPICKER_DISABLED
#endif   // _kehui_filedir_picker_h_included

