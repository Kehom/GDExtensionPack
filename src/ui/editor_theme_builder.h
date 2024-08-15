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

#ifndef _kehui_theme_builder_h_included
#define _kehui_theme_builder_h_included 1

#ifndef ALL_UI_DISABLED
#ifndef EDITOR_DISABLED

/// The custom control theming system uses a work around (more about it in the custom_theme.h). The result of
/// this workaround is that the theme editor does not know anything about the custom control theme entries. This
/// simple editor tool is meant to generate a theme resource that contains all the default values of the registered
/// theme entries of the custom controls.
/// The idea here is to provide an new menu option that, when clicked, should display a file dialog. Once a file name
/// is selected, then the internal algorithm should create and fill a theme resource with the relevant data.

#include <godot_cpp/classes/editor_plugin.hpp>

namespace godot
{
   class FileDialog;
}

class UIThemeBuilder : public godot::EditorPlugin
{
   GDCLASS(UIThemeBuilder, godot::EditorPlugin);
private:
   /// Internal variables
   godot::FileDialog* m_file_dlg;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_generator_selected();
   void on_file_selected(const godot::String& path);

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   UIThemeBuilder();
};


#endif   // EDITOR_DISABLED
#endif   //ALL_UI_DISABLED
#endif   // _kehui_theme_builder_h_included

