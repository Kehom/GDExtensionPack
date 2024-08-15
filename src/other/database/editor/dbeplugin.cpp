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

#ifndef DATABASE_DISABLED
#ifndef EDITOR_DISABLED 

#include "dbeplugin.h"

#include "../database.h"
#include "../../../internal.h"
#include "../../../extpackutils.h"
#include "../../../ui/filedir_picker/filedir_picker.h"
#include "../../../ui/tabular_box/tabular_box.h"

#include <godot_cpp/classes/accept_dialog.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/button_group.hpp>
#include <godot_cpp/classes/check_box.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/file_dialog.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_flow_container.hpp>
#include <godot_cpp/classes/h_split_container.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/menu_button.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/popup_menu.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/resource_uid.hpp>
#include <godot_cpp/classes/rich_text_label.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/sub_viewport.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


// Converted from PNG data by Godot itself
//static const char* ICON_DATABASE = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAFOmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6ZXhpZj0iaHR0cDovL25zLmFkb2JlLmNvbS9leGlmLzEuMC8iCiAgICB4bWxuczpwaG90b3Nob3A9Imh0dHA6Ly9ucy5hZG9iZS5jb20vcGhvdG9zaG9wLzEuMC8iCiAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgZXhpZjpDb2xvclNwYWNlPSIxIgogICBleGlmOlBpeGVsWERpbWVuc2lvbj0iMTYiCiAgIGV4aWY6UGl4ZWxZRGltZW5zaW9uPSIxNiIKICAgcGhvdG9zaG9wOkNvbG9yTW9kZT0iMyIKICAgcGhvdG9zaG9wOklDQ1Byb2ZpbGU9InNSR0IgSUVDNjE5NjYtMi4xIgogICB0aWZmOkltYWdlTGVuZ3RoPSIxNiIKICAgdGlmZjpJbWFnZVdpZHRoPSIxNiIKICAgdGlmZjpSZXNvbHV0aW9uVW5pdD0iMiIKICAgdGlmZjpYUmVzb2x1dGlvbj0iNzIuMCIKICAgdGlmZjpZUmVzb2x1dGlvbj0iNzIuMCIKICAgeG1wOk1ldGFkYXRhRGF0ZT0iMjAyMS0wNy0xM1QxNTowMjoyNC0wMzowMCIKICAgeG1wOk1vZGlmeURhdGU9IjIwMjEtMDctMTNUMTU6MDI6MjQtMDM6MDAiPgogICA8eG1wTU06SGlzdG9yeT4KICAgIDxyZGY6U2VxPgogICAgIDxyZGY6bGkKICAgICAgeG1wTU06YWN0aW9uPSJwcm9kdWNlZCIKICAgICAgeG1wTU06c29mdHdhcmVBZ2VudD0iQWZmaW5pdHkgUGhvdG8gMS45LjIiCiAgICAgIHhtcE1NOndoZW49IjIwMjEtMDctMTNUMTQ6NTI6MTMtMDM6MDAiLz4KICAgICA8cmRmOmxpCiAgICAgIHN0RXZ0OmFjdGlvbj0icHJvZHVjZWQiCiAgICAgIHN0RXZ0OnNvZnR3YXJlQWdlbnQ9IkFmZmluaXR5IFBob3RvIDEuOS4yIgogICAgICBzdEV2dDp3aGVuPSIyMDIxLTA3LTEzVDE1OjAyOjI0LTAzOjAwIi8+CiAgICA8L3JkZjpTZXE+CiAgIDwveG1wTU06SGlzdG9yeT4KICA8L3JkZjpEZXNjcmlwdGlvbj4KIDwvcmRmOlJERj4KPC94OnhtcG1ldGE+Cjw/eHBhY2tldCBlbmQ9InIiPz4h9WMrAAABgmlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8zQyNGQxQLi0nIAjFqYmMxk1+Fxcwog83Mm19qfrzem0mTrbKdosTGrwV/AVtlrRSRkp2yJjboOc9MjWTO7dzzud97z+nec8EaTClpvWYQ0pmc5p/0uhZCiy77E3YctNFMb1jR1dnARJCq9n6LxYzX/Wat6uf+tYZoTFfAUic8pqhaTnhKeGY1p5q8JdyqJMNR4RPhPk0uKHxj6pESP5ucKPGnyVrQ7wNrk7Ar8Ysjv1hJamlheTld6VReKd/HfIkjlpkPSOwU70DHzyReXEwzjg8PQ4zK7KEfNwOyokr+4E/+HFnJVWRWKaCxQoIkOfpEzUv1mMS46DEZKQpm///2VY8Pu0vVHV6ofTSM126wb8JX0TA+Dgzj6xBsD3CeqeRn92HkTfRiRevaA+c6nF5UtMg2nG1A+70a1sI/kk3cGo/DyzE0hqDlCuqXSj0r73N0B8E1+apL2NmFHjnvXP4Ga5Fn6EyCS/MAAAAJcEhZcwAACxMAAAsTAQCanBgAAABuSURBVDiN7ZNBEoAgDAOzjP9/MvFCnYrK1JMXc2shsM0AmmTbcy8L4FQPE5L6ynijBriNos8nFyi6JG3RtO3qIXnM7WmhqpaQJMkMrQCyJ0I0QJUg9gJ8n8FxY5iDovqg/gzSZxpjvckA4DXxRTvlu2UeBVM4lwAAAABJRU5ErkJggg==";
static const char* ICON_DATABASE = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kbtLA0EQh79Eg0EjUbSwUAgSrRKJD4I2FhFfoBZJBKM2yZmHkMdxlyDBVrAVFEQbX4X+BdoK1oKgKIJYp1a0UTnnEiEiZpbZ+fa3O8PuLFjDaSWj1/sgk81rwcmAayGy6GooYaOLVuwMRRVdnQ1NhKlp7/dYzHjrNWvVPvevNa3EdQUsduFRRdXywlPCM2t51eQd4XYlFV0RPhP2aHJB4TtTj1W4ZHKywp8ma+HgGFhbhF3JXxz7xUpKywjLy3Fn0gXl5z7mSxzx7HxIYrd4JzpBJgngYppxxvDTz4jMfrwM0CcrauT7yvlz5CRXkVmliMYqSVLk8YhakOpxiQnR4zLSFM3+/+2rnhgcqFR3BMD2bBivPdCwDV9bhvFxZBhfx1D3BJfZan7uEIbfRN+qau4DcG7A+VVVi+3CxSZ0PKpRLVqW6sStiQS8nEJzBNpuoHGp0rOffU4eILwuX3UNe/vQK+edy9+Mumf3Cl5yQgAAAAlwSFlzAAALEwAACxMBAJqcGAAAAItJREFUOI3tkzEOgmAMhb8mrkTuocQRJ66hR+QUzjCD98AD+D+XYkjDALIZXvKWtnl9fUlhIwxAUgHcgStwDAR4BTZAbWbdwQeS8x2YvD9XT1MHHVC7ctzGjKsSuJnZBRcY8ZA0aCG+QUxqlaRsjcB4wslDLIPV3HcM4azWQ3zuGfxVBmd+e6aerfgAUKSboGMDDD8AAAAASUVORK5CYII=";
//static const char* ICON_DATABASE = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kbtLA0EQh79Eg0EjUbSwUAgSrRKJD4I2FhFfoBZJBKM2yZmHkMdxlyDBVrAVFEQbX4X+BdoK1oKgKIJYp1a0UTnnEiEiZpbZ+fa3O8PuLFjDaSWj1/sgk81rwcmAayGy6GooYaOLVuwMRRVdnQ1NhKlp7/dYzHjrNWvVPvevNa3EdQUsduFRRdXywlPCM2t51eQd4XYlFV0RPhP2aHJB4TtTj1W4ZHKywp8ma+HgGFhbhF3JXxz7xUpKywjLy3Fn0gXl5z7mSxzx7HxIYrd4JzpBJgngYppxxvDTz4jMfrwM0CcrauT7yvlz5CRXkVmliMYqSVLk8YhakOpxiQnR4zLSFM3+/+2rnhgcqFR3BMD2bBivPdCwDV9bhvFxZBhfx1D3BJfZan7uEIbfRN+qau4DcG7A+VVVi+3CxSZ0PKpRLVqW6sStiQS8nEJzBNpuoHGp0rOffU4eILwuX3UNe/vQK+edy9+Mumf3Cl5yQgAAAAlwSFlzAAALEwAACxMBAJqcGAAAARhJREFUOI2l08EqxVEQBvDfSGFjc0V5CrKglJ1SFve+ASsray9iZ+cN2HgAKaxkY8Pq7oiFUlhobM5xjxt1ZWr61znz/+b7vpnDPyMgMzvoYQVzmG2+8ID7Ji9xFBFPCsBF/j3OWwYLWMRy6dyyMNS9MriKiOsK8IjjcjFcrAGtuYxuRMxUCTW2M3N/VA0wVjrsoY913I1gfr/8Y7wczGAVH4Xixg8etJMYx25rYiJxi5eh4taDCjqPiIioAK+YRKcU3Iwg4y0ipqqETWzhrDDIyq6JxLPBGA9bCac4MBjj+y8eTBiMcSci1loP4ARLmG58qB7MYqqlVC2Qmf0y2l5mzo+4Bv1WQgdd31e5ZpUwvMrHX4/pP/EJtRb+hisOPxYAAAAASUVORK5CYII=";
static const char* ICON_MAGNIFY = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgGlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz/GyO8ICwuLlwYbxKiJjcXIr8JijDLYzDzvzaj58XpvJk22ylZRYuPXgr+ArbJWikjJ2prYoOc8b2okc2733M/93nNO954LnnBSTVnePkils2ZoPKjMRxaUyme8VNNEGV1R1TKmZ8fClLT3O4kTu+lxapWO+9dqlzVLhbIq4WHVMLPCE8JTq1nD4W3hFjURXRY+Fe425YLCt44ec/nZ4bjLnw6b4dAIeBqFlfgvjv1iNWGmhOXl+FLJnFq4j/OSOi09Nytru8w2LEKME0RhklFGCNDPkPgAPfjplR0l8vt+8mfISK4q3iCPyQpxEmTpFjUn1TVZddE1GUnyTv//9tXSB/xu9bogVDzZ9msHVG7B16Ztfxza9tcRlD/CRbqYnzmAwTfRN4uabx8a1uHssqjFduB8A1ofjKgZ/ZHKZXp0HV5OoD4CzddQs+j2rHDO8T2E1+SrrmB3DzolvmHpGzSIZ8/sESNpAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAzUlEQVQ4ja3RvU0DQRCG4VlHbsBHADIhiWvANnSBLNwIclNGchNAARYhf7qQBh6SITnOy4G8ya70zftqZjbimAeX2OIdH7jHYih8hzescY4pbvGKzW/wPOGTnmyCFyxrgi3WlXyF3aGw5MzTiuAMLUo3G+X9I+g6DgWjUoqIeIiIeUUwj4jHrO1tcZGLmvRkTf7EVb1HNilZ5cynuEm4xawqSMkSuwTafF9jhk88oRkiKt2NJwzPgyQ90ibho0n2GP9Xsk/JxZ8FKRl/w1+O8BCnMiHLKwAAAABJRU5ErkJggg==";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GDDatabaseEditor
void GDDatabaseEditor::check_style()
{
   m_entry_style_normal = get_theme_stylebox("tab_selected", "TabContainer")->duplicate();
   m_entry_style_normal->set_content_margin_all(6);

   m_entry_style_selected = Internals::make_flat_stylebox(godot::Color(0.5, 0.5, 0.5, 0.3), 2, 2, 2, 2, 3, false, 2);

   godot::Ref<godot::StyleBoxFlat> flat_style = m_entry_style_normal;
   if (flat_style.is_valid())
   {
      flat_style->set_border_width_all(0);
   }

   if (m_db.is_valid())
   {
      for (godot::HashMap<godot::StringName, godot::Ref<DBETableEntry>>::Iterator iter = m_table_entry.begin(); iter != m_table_entry.end(); ++iter)
      {
         iter->value->ui_table->add_theme_stylebox_override("panel", m_entry_style_normal);
      }
   }

   godot::Ref<godot::StyleBox> background = get_theme_stylebox("panel", "Panel")->duplicate();
   background->set_content_margin_all(1.0);
   m_tbox_theme->set_stylebox("background", "TabularBox", background);

   godot::Ref<godot::StyleBox> header = get_theme_stylebox("normal", "LineEdit")->duplicate();
   godot::Ref<godot::StyleBoxFlat> header_flat = header;
   if (header_flat.is_valid())
   {
      header_flat->set_content_margin_all(5);
      header_flat->set_border_width_all(1);
      header_flat->set_corner_radius_all(0);
   }
   m_tbox_theme->set_stylebox("header", "TabularBox", header);

   godot::Ref<godot::StyleBox> button_normal = get_theme_stylebox("normal", "Button")->duplicate();
   godot::Ref<godot::StyleBox> button_hover = get_theme_stylebox("hover", "Button")->duplicate();
   godot::Ref<godot::StyleBox> button_pressed = get_theme_stylebox("pressed", "Button")->duplicate();

   button_normal->set_content_margin_all(2);
   button_hover->set_content_margin_all(2);
   button_pressed->set_content_margin_all(2);

   m_tbox_theme->set_stylebox("button_normal", "TabularBox", button_normal);
   m_tbox_theme->set_stylebox("button_hover", "TabularBox", button_hover);
   m_tbox_theme->set_stylebox("button_pressed", "TabularBox", button_pressed);

   // No need to duplicate the icons, right?
   godot::Ref<godot::Texture2D> checked = get_theme_icon("checked", "CheckBox");
   godot::Ref<godot::Texture2D> unchecked = get_theme_icon("unchecked", "CheckBox");
   m_tbox_theme->set_icon("checked", "TabularBox", checked);
   m_tbox_theme->set_icon("unchecked", "TabularBox", unchecked);

   // No need to duplicate font, right?
   godot::Ref<godot::Font> font = get_theme_font("font", "Label");
   m_tbox_theme->set_font("header_font", "TabularBox", font);
   m_tbox_theme->set_font("cell_font", "TabularBox", font);

   const godot::Color font_color = get_theme_color("font_color", "Label");
   m_tbox_theme->set_color("header_text", "TabularBox", font_color);
   m_tbox_theme->set_color("cell_text", "TabularBox", font_color);
}


void GDDatabaseEditor::build_msg_dialog()
{
   m_dialog_message.vbox = memnew(godot::VBoxContainer);
   m_dialog_message.vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_message.vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);

   m_dialog_message.label = memnew(godot::Label);
   m_dialog_message.label->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_message.label->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_message.vbox->add_child(m_dialog_message.label);
}


void GDDatabaseEditor::build_dbinfo_dialog()
{
   m_dialog_dbinfo.vbox = memnew(godot::VBoxContainer);
   m_dialog_dbinfo.vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_dbinfo.vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);

   m_dialog_dbinfo.info = memnew(godot::RichTextLabel);
   m_dialog_dbinfo.info->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_dbinfo.info->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_dbinfo.info->set_use_bbcode(true);
   m_dialog_dbinfo.vbox->add_child(m_dialog_dbinfo.info);
}


void GDDatabaseEditor::build_opencreate_dialog()
{
   /// NOTE: Yes, this is empty, for now. I have plans to have a custom FileDialog that will properly filter files
   /// based on their types looking beyond the extensions.
}


void GDDatabaseEditor::build_newtable_dialog()
{
   m_dialog_newtable.vbox = memnew(godot::VBoxContainer);
   m_dialog_newtable.vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_newtable.vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);

   // Line: Table name
   {
      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_newtable.vbox->add_child(line);

      godot::Label* lbl_tablename = memnew(godot::Label);
      lbl_tablename->set_custom_minimum_size(godot::Vector2(100, 0));
      lbl_tablename->set_text("Table name:");
      line->add_child(lbl_tablename);

      m_dialog_newtable.txt_tablename = memnew(godot::LineEdit);
      m_dialog_newtable.txt_tablename->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_newtable.txt_tablename->set_select_all_on_focus(true);
      m_dialog_newtable.txt_tablename->connect("text_changed", callable_mp(this, &GDDatabaseEditor::on_newtable_name_changed));
      line->add_child(m_dialog_newtable.txt_tablename);
   }

   // Line: Filename
   {
      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_newtable.vbox->add_child(line);

      godot::Label* lbl_filename = memnew(godot::Label);
      lbl_filename->set_text("File name:");
      lbl_filename->set_custom_minimum_size(godot::Vector2(100, 0));
      line->add_child(lbl_filename);

      m_dialog_newtable.txt_filename = memnew(godot::LineEdit);
      m_dialog_newtable.txt_filename->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_newtable.txt_filename->set_select_all_on_focus(true);
      line->add_child(m_dialog_newtable.txt_filename);

      godot::Label* lbl_extension = memnew(godot::Label);
      lbl_extension->set_text(".tres");
      line->add_child(lbl_extension);

      m_dialog_newtable.chk_embed = memnew(godot::CheckBox);
      m_dialog_newtable.chk_embed->set_text("Embed");
      m_dialog_newtable.chk_embed->connect("toggled", callable_mp(this, &GDDatabaseEditor::on_newtable_embed_toggled));
      line->add_child(m_dialog_newtable.chk_embed);
   }

   // Line: ID Type
   {
      godot::Ref<godot::ButtonGroup> btgroup;
      btgroup.instantiate();

      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_newtable.vbox->add_child(line);

      godot::Label* lbl_idtype = memnew(godot::Label);
      lbl_idtype->set_text("ID Type:");
      lbl_idtype->set_custom_minimum_size(godot::Vector2(100, 0));
      line->add_child(lbl_idtype);

      m_dialog_newtable.chk_integer = memnew(godot::CheckBox);
      m_dialog_newtable.chk_integer->set_text("Integer");
      m_dialog_newtable.chk_integer->set_button_group(btgroup);
      m_dialog_newtable.chk_integer->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      //m_dialog_newtable.chk_integer->set_pressed(true);
      line->add_child(m_dialog_newtable.chk_integer);

      m_dialog_newtable.chk_string = memnew(godot::CheckBox);
      m_dialog_newtable.chk_string->set_text("String");
      m_dialog_newtable.chk_string->set_button_group(btgroup);
      m_dialog_newtable.chk_string->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_newtable.chk_string->set_pressed(true);
      line->add_child(m_dialog_newtable.chk_string);

      m_dialog_newtable.chk_editable = memnew(godot::CheckBox);
      m_dialog_newtable.chk_editable->set_text("Editable");
      m_dialog_newtable.chk_editable->set_h_size_flags(godot::Control::SIZE_SHRINK_END);
      m_dialog_newtable.chk_editable->set_pressed(true);
      line->add_child(m_dialog_newtable.chk_editable);
   }
}


void GDDatabaseEditor::build_tablerename_dialog()
{
   m_dialog_tablerename.vbox = memnew(godot::VBoxContainer);
   m_dialog_tablerename.vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_tablerename.vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
   
   m_dialog_tablerename.lbl_info = memnew(godot::Label);
   m_dialog_tablerename.lbl_info->set_text("-");
   m_dialog_tablerename.vbox->add_child(m_dialog_tablerename.lbl_info);

   m_dialog_tablerename.txt_newname = memnew(godot::LineEdit);
   m_dialog_tablerename.txt_newname->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_tablerename.txt_newname->connect("text_changed", callable_mp(this, &GDDatabaseEditor::on_table_rename_input));
   m_dialog_tablerename.vbox->add_child(m_dialog_tablerename.txt_newname);
}


void GDDatabaseEditor::build_addcol_dialog()
{
   /// TODO: Improve the layout of this dialog, specifically related to the labels meant to display information regarding error and warning
   /// FIXME: Deal with localization
   m_dialog_addcol.vbox = memnew(godot::VBoxContainer);
   m_dialog_addcol.vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_addcol.vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);

   // Line: column name
   {
      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_addcol.vbox->add_child(line);

      godot::Label* lbl_colname = memnew(godot::Label);
      lbl_colname->set_custom_minimum_size(godot::Vector2(130, 0));
      lbl_colname->set_text("Column name:");
      line->add_child(lbl_colname);

      m_dialog_addcol.txt_colname = memnew(godot::LineEdit);
      m_dialog_addcol.txt_colname->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_addcol.txt_colname->set_select_all_on_focus(true);
      m_dialog_addcol.txt_colname->connect("text_changed", callable_mp(this, &GDDatabaseEditor::on_newcolumn_name_changed));
      line->add_child(m_dialog_addcol.txt_colname);

      m_dialog_addcol.lbl_colname_err = memnew(godot::Label);
      m_dialog_addcol.lbl_colname_err->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_addcol.lbl_colname_err->add_theme_font_size_override("font_size", 10);
      m_dialog_addcol.vbox->add_child(m_dialog_addcol.lbl_colname_err);
   }

   // Line: Value type
   {
      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_addcol.vbox->add_child(line);

      godot::Label* lbl_vtype = memnew(godot::Label);
      lbl_vtype->set_custom_minimum_size(godot::Vector2(130, 0));
      lbl_vtype->set_text("Type");
      line->add_child(lbl_vtype);

      m_dialog_addcol.opt_type = memnew(godot::OptionButton);
      m_dialog_addcol.opt_type->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_addcol.opt_type->connect("item_selected", callable_mp(this, &GDDatabaseEditor::on_column_value_type_selected));
      line->add_child(m_dialog_addcol.opt_type);

      m_dialog_addcol.lbl_vtype_warn = memnew(godot::Label);
      m_dialog_addcol.lbl_vtype_warn->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_addcol.lbl_vtype_warn->add_theme_font_size_override("font_size", 10);
      m_dialog_addcol.vbox->add_child(m_dialog_addcol.lbl_vtype_warn);

      // Build the type list
      godot::PopupMenu* pop = m_dialog_addcol.opt_type->get_popup();
      godot::Vector<godot::Dictionary> tplist;
      DBTable::build_type_list(tplist);

      for (godot::Dictionary tinfo : tplist)
      {
         pop->add_item(tinfo["ui"], tinfo["code"]);
      }
   }

   // Line: Another table ID
   {
      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_dialog_addcol.vbox->add_child(line);

      godot::Label* lbl_othername = memnew(godot::Label);
      lbl_othername->set_custom_minimum_size(godot::Vector2(130, 0));
      lbl_othername->set_text("External table:");
      line->add_child(lbl_othername);

      // The contents of the popup menu of this button will be filled when displaying the popup
      m_dialog_addcol.opt_other_table = memnew(godot::OptionButton);
      m_dialog_addcol.opt_other_table->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      line->add_child(m_dialog_addcol.opt_other_table);
   }
}


void GDDatabaseEditor::build_export_dialog()
{
   m_dialog_export.vbox = memnew(godot::VBoxContainer);
   m_dialog_export.vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_dialog_export.vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);

   // Line: Monolithic
   {
      m_dialog_export.chk_monolithic = memnew(godot::CheckBox);
      m_dialog_export.chk_monolithic->set_text("Monolithic");
      m_dialog_export.chk_monolithic->set_pressed(true);
      m_dialog_export.vbox->add_child(m_dialog_export.chk_monolithic);
   }

   // Line: Output directory
   {
      godot::Label* lbl_output = memnew(godot::Label);
      lbl_output->set_text("Output directory:");
      m_dialog_export.vbox->add_child(lbl_output);

      m_dialog_export.fd_picker = memnew(FileDirPicker);
      m_dialog_export.fd_picker->set_h_size_flags(SIZE_EXPAND_FILL);
      m_dialog_export.fd_picker->set_v_size_flags(SIZE_EXPAND_FILL);
      m_dialog_export.fd_picker->set_pick_mode(FileDirPicker::PICK_MODE_DIRECTORY);
      m_dialog_export.fd_picker->set_filters(godot::PackedStringArray());
      m_dialog_export.fd_picker->set_access(godot::FileDialog::ACCESS_FILESYSTEM);
      m_dialog_export.fd_picker->connect("path_selected", callable_mp(this, &GDDatabaseEditor::on_export_path_selected));
      m_dialog_export.vbox->add_child(m_dialog_export.fd_picker);
   }

   // Line: Indent type
   {
      godot::Label* lbl_indent = memnew(godot::Label);
      lbl_indent->set_text("Indent");
      m_dialog_export.vbox->add_child(lbl_indent);

      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(SIZE_EXPAND_FILL);
      m_dialog_export.vbox->add_child(line);

      m_dialog_export.mnu_indent_type = memnew(godot::OptionButton);
      m_dialog_export.mnu_indent_type->set_h_size_flags(SIZE_EXPAND_FILL);
      m_dialog_export.mnu_indent_type->add_item("Tabs");
      m_dialog_export.mnu_indent_type->add_item("Spaces");
      m_dialog_export.mnu_indent_type->select(1);
      m_dialog_export.mnu_indent_type->connect("item_selected", callable_mp(this, &GDDatabaseEditor::on_export_indent_id_selected));
      line->add_child(m_dialog_export.mnu_indent_type);

      m_dialog_export.spin_amount = memnew(godot::SpinBox);
      m_dialog_export.spin_amount->set_step(1);
      m_dialog_export.spin_amount->set_value(3);
      line->add_child(m_dialog_export.spin_amount);
   }
}


void GDDatabaseEditor::build_main_dialog()
{
   /// NOTE: At a later moment attemp to find a work around the fact that there are buttons at the title bar of the dialog
   /// Those buttons are not desired! Yet the title is a good thing to appear
   if (m_dialog_main.dialog) { return; }

   m_dialog_main.dialog = memnew(godot::Window);
   m_dialog_main.dialog->set_unparent_when_invisible(false);
   m_dialog_main.dialog->set_visible(false);
   m_dialog_main.dialog->set_title("-");
   m_dialog_main.dialog->set_keep_title_visible(true);
   m_dialog_main.dialog->set_transient(true);
   m_dialog_main.dialog->set_exclusive(true);
   //m_dialog_main.dialog->set_embedding_subwindows(true);
   add_child(m_dialog_main.dialog);
   m_dialog_main.dialog->connect("window_input", callable_mp(this, &GDDatabaseEditor::on_dialog_input));

   godot::VBoxContainer* vbox = memnew(godot::VBoxContainer);
   vbox->set_anchors_preset(godot::Control::PRESET_FULL_RECT);
   ExtPackUtils::set_control_offsets(vbox, 6, 6, -6, -6);
   m_dialog_main.dialog->add_child(vbox);

   vbox->add_child(m_dialog_message.vbox);
   vbox->add_child(m_dialog_dbinfo.vbox);
   vbox->add_child(m_dialog_newtable.vbox);
   vbox->add_child(m_dialog_tablerename.vbox);
   vbox->add_child(m_dialog_addcol.vbox);
   vbox->add_child(m_dialog_export.vbox);

   m_dialog_message.vbox->set_visible(false);
   m_dialog_dbinfo.vbox->set_visible(false);
   m_dialog_newtable.vbox->set_visible(false);
   m_dialog_tablerename.vbox->set_visible(false);
   m_dialog_addcol.vbox->set_visible(false);
   m_dialog_export.vbox->set_visible(false);

   // Line: Buttons
   {
      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      line->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      line->set_alignment(godot::BoxContainer::ALIGNMENT_CENTER);
      vbox->add_child(line);

      m_dialog_main.btok = memnew(godot::Button);
      m_dialog_main.btok->set_custom_minimum_size(godot::Vector2(70, 0));
      m_dialog_main.btok->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_dialog_ok_clicked));
      line->add_child(m_dialog_main.btok);

      m_dialog_main.btcancel = memnew(godot::Button);
      m_dialog_main.btcancel->set_custom_minimum_size(godot::Vector2(70, 0));
      m_dialog_main.btcancel->connect("pressed", callable_mp(this, &GDDatabaseEditor::hide_dialog));
      line->add_child(m_dialog_main.btcancel);
   }

   m_dialog_main.current_visible = nullptr;
}


void GDDatabaseEditor::create_openclose_dialog()
{
   /// NOTE: Once the custom file dialog is created this function should be removed
   if (m_dlg_openclose) { return; }

   godot::PackedStringArray filters;
   filters.append("*.tres; TRES");
   filters.append("*.res; RES");

   m_dlg_openclose = memnew(godot::FileDialog);
   m_dlg_openclose->set_filters(filters);
   m_dlg_openclose->set_size(godot::Vector2(640, 480));
   m_dlg_openclose->set_transient(true);
   m_dlg_openclose->set_exclusive(true);
   m_dlg_openclose->set_unparent_when_invisible(false);
   add_child(m_dlg_openclose);

   m_dlg_openclose->connect("file_selected", callable_mp(this, &GDDatabaseEditor::on_file_selected));
}


void GDDatabaseEditor::update_ui()
{
   const bool is_db_valid = m_db.is_valid();
   const bool is_selected_valid = m_selected_entry.is_valid();
   const int64_t sel_col_count = is_selected_valid ? m_selected_entry->db_table->get_column_count() : 0;


   // The button meant to display Database information should be disabled it no database is available
   m_top_bar.bt_dbinfo->set_disabled(!is_db_valid);

   // Display the Database file path + name
   const godot::String path = is_db_valid ? m_db->get_path() : "";
   m_top_bar.lbl_dbfile->set_text(path);

   // The button meant to add tables must be disabled if there is no valid database
   m_left.bt_add->set_disabled(!is_db_valid);

   // The table list should ignore events if there is no Database. This is the easieast way to prevent dragging
   // tables into the list when there is no valid database opened.
   const godot::Control::MouseFilter mf = is_db_valid ? MOUSE_FILTER_PASS : MOUSE_FILTER_IGNORE;
   m_left.vbox->set_mouse_filter(mf);

   // Of course, the add column and add row buttons must be disabled if there is no selected table
   m_right.bt_addcol->set_disabled(!is_selected_valid);
   
   // Additionaly, if the the selected table (if any) doesn't have any column (other than ID), then the "add row" should be disabled.
   m_right.bt_addrow->set_disabled(!is_selected_valid || sel_col_count < 1);


   // Finally the name of the table being edited - if any
   //const godot::String tblname = m_selected_entry.is_valid() ? m_selected_entry->db_table->get_table_name() : "";
   //m_right.lbl_tablename->set_text(tblname);
   godot::String tblname = "";
   if (m_selected_entry.is_valid())
   {
      //if (m_selected_entry->data_source->has_patch())
      //{
      //   tblname = godot::vformat("Patching '%s' through '%s'", m_selected_entry->db_table->get_table_name(), m_selected_entry->data_source->get_patch_filename());
      //}
      //else
      {
         tblname = m_selected_entry->db_table->get_table_name();
      }
   }

   m_right.lbl_tablename->set_text(tblname);
}


void GDDatabaseEditor::add_table(const godot::Ref<DBTable>& table, bool select)
{
   godot::Ref<DBETableEntry> entry = memnew(DBETableEntry);

   m_table_entry[table->get_table_name()] = entry;

   entry->data_source.instantiate();
   entry->data_source->setup(m_db, table);

   entry->ui_table = memnew(godot::PanelContainer);
   m_left.vbox->add_child(entry->ui_table);
   entry->ui_table->set_meta("selected", select);
   entry->ui_table->set_mouse_filter(MOUSE_FILTER_PASS);

   entry->ui_table->connect("draw", callable_mp(this, &GDDatabaseEditor::draw_table_entry).bind(entry));
   entry->ui_table->connect("mouse_entered", callable_mp(this, &GDDatabaseEditor::on_mouse_enter_entry).bind(entry));
   entry->ui_table->connect("mouse_exited", callable_mp(this, &GDDatabaseEditor::on_mouse_exit_entry).bind(entry));
   
   entry->db_table = table;

   // Check if layout data exists, creating it if necessary
   if (!m_layout.db_layout.has("table"))
   {
      m_layout.db_layout["table"] = godot::Dictionary();
   }
   godot::Dictionary table_layout = m_layout.db_layout["table"];
   if (!table_layout.has(table->get_table_name()))
   {
      table_layout[table->get_table_name()] = godot::Dictionary();
   }
   entry->layout = table_layout[table->get_table_name()];


   if (m_entry_style_normal.is_valid())
   {
      entry->ui_table->add_theme_stylebox_override("panel", m_entry_style_normal);
   }

   godot::VBoxContainer* vbox = memnew(godot::VBoxContainer);
   vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
   entry->ui_table->add_child(vbox);

   // "Line 1". Button holding table name and another button meant to remove the table from the Database
   {
      godot::HBoxContainer* line = memnew(godot::HBoxContainer);
      vbox->add_child(line);
      line->add_theme_constant_override("separation", 15);

      godot::Button* btname = memnew(godot::Button);
      line->add_child(btname);
      btname->set_text_alignment(godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_LEFT);
      btname->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      btname->set_text(table->get_table_name());
      btname->set_flat(false);
      entry->bt_table_name = btname;

      btname->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_entry_rename_clicked).bind(entry));
      
      godot::Button* btremove = memnew(godot::Button);
      line->add_child(btremove);
      btremove->set_text("-");
      btremove->set_flat(false);
      // Attempt to make the "remove button" square
      const godot::Vector2 bt_minsz = btremove->get_combined_minimum_size();
      const int minsz = godot::Math::max<int>(bt_minsz.x, bt_minsz.y);
      btremove->set_custom_minimum_size(godot::Vector2(minsz, minsz));

      btremove->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_entry_remove_clicked).bind(entry));
   }

   // "Line 2". This is just a label holding the name of the resource file of the table. If the table
   // is "embedded" within the owning database then <Embedded> should be shown.
   {
      godot::Label* lbl_fname = memnew(godot::Label);
      vbox->add_child(lbl_fname);
      lbl_fname->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      lbl_fname->set_mouse_filter(MOUSE_FILTER_PASS);

      const godot::String rpath = table->get_path();
      if (rpath.begins_with("res://") && rpath.find("::") == -1)
      {
         lbl_fname->set_text(rpath.get_file());
         lbl_fname->set_tooltip_text(rpath);
      }
      else
      {
         lbl_fname->set_text("<Embedded>");
         lbl_fname->set_tooltip_text("");
      }
   }

   if (select)
   {
      on_table_selection(entry);
   }
}


void GDDatabaseEditor::sort_table_list()
{
   godot::PackedStringArray tbl_list = m_db->get_table_list();
   tbl_list.sort();

   const int64_t tcount = tbl_list.size();
   for (int64_t i = 0; i < tcount; i++)
   {
      m_left.vbox->move_child(m_table_entry[tbl_list[i]]->ui_table, i);
   }
}


void GDDatabaseEditor::add_new_table()
{
   /// NOTE: The "new table" button is disabled when there is no Database resource assigned (edited). Should it be checked just
   /// to make sure?
   /// NOTE: The "OK" button should be disabled if the table already exists within the database. Should it be checked again?
   const godot::String tbl_name = m_dialog_newtable.txt_tablename->get_text();
   godot::String fname = m_dialog_newtable.txt_filename->get_text();
   if (fname.is_empty())
   {
      fname = tbl_name;
   }

   if (fname.get_extension() != "tres")
   {
      fname += ".tres";
   }

   const int itype = m_dialog_newtable.chk_integer->is_pressed() ? godot::Variant::INT : godot::Variant::STRING;
   const bool locked_id = !m_dialog_newtable.chk_editable->is_pressed();

   const godot::String base_dir = m_db->get_path().get_base_dir();

   godot::Ref<DBTable> ntable = godot::Ref<DBTable>(memnew(DBTable(itype, tbl_name, locked_id)));
   if (!m_dialog_newtable.chk_embed->is_pressed())
   {
      // This table must be created on a separate file - place it alongside the owning database resource file
      const godot::String path = godot::vformat("%s/%s", base_dir, fname);

      if (godot::ResourceSaver::get_singleton()->save(ntable, path) != godot::OK)
      {
         show_message(godot::vformat("Unable to create new table (%s) at\n%s", tbl_name, path));
         return;
      }

      // Load the file into the resource that will be added into the database, otherwise the data will not be saved
      ntable = godot::ResourceLoader::get_singleton()->load(path);
   }

   if (!m_db->add_table_res(ntable))
   {
      // It should not come here. The only failure condition is that a table with the same name already exists! Yet there are several checks
      // preventing an attempt at doing that. So... nevertheless, message error to help with possible bugs
      show_message(godot::vformat("Failed to add new table (%s). This is a bug!", tbl_name));
      return;
   }

   // Update the UI - that is, add the table entry while also selecting it. Layout data for this specific table should be handled
   // by the "add_table()" function.
   add_table(ntable, true);

   // Then save the database
   save_db();

   sort_table_list();
}


void GDDatabaseEditor::build_column_checkboxes()
{
   // This assumes the internal containers are already empty
   if (!m_selected_entry.is_valid()) { return; }

   {
      godot::CheckBox* chk = memnew(godot::CheckBox);
      chk->set_text("ID");
      m_right.hf_column->add_child(chk);

      m_right.column_checkbox["ID"] = chk;

      m_right.filter_exclude.append("ID");

      chk->connect("toggled", callable_mp(this, &GDDatabaseEditor::on_column_checkbox_toggled));
   }

   const int64_t ccount = m_selected_entry->db_table->get_column_count();
   for (int64_t i = 0; i < ccount; i++)
   {
      const godot::String title = m_selected_entry->db_table->get_column_title(i);

      godot::CheckBox* chk = memnew(godot::CheckBox);
      chk->set_text(title);
      chk->set_pressed(true);
      m_right.hf_column->add_child(chk);

      m_right.column_checkbox[title] = chk;

      chk->connect("toggled", callable_mp(this, &GDDatabaseEditor::on_column_checkbox_toggled));
   }
}


void GDDatabaseEditor::clear_column_checkboxes()
{
   m_right.filter_exclude.clear();

   for (godot::KeyValue<godot::String, godot::CheckBox*>& pair : m_right.column_checkbox)
   {
      pair.value->queue_free();
   }

   m_right.column_checkbox.clear();
}


void GDDatabaseEditor::rename_selected_table(const godot::String& into)
{
   const godot::StringName old_name = m_selected_entry->db_table->get_table_name();

   if (m_db->rename_table(old_name, into))
   {
      m_selected_entry->bt_table_name->set_text(into);
      m_table_entry.erase(old_name);
      m_table_entry[into] = m_selected_entry;
      save_db();

      m_layout.db_layout["last_table"] = into;
      godot::Dictionary table = m_layout.db_layout.get("table", godot::Dictionary());
      if (table.has(old_name))
      {
         ExtPackUtils::change_dictionary_key(table, old_name, into);
      }

      m_layout.db_layout["table"] = table;
      
      m_layout.saver.call();

      m_right.lbl_tablename->set_text(into);
   }
}


void GDDatabaseEditor::add_column()
{
   const godot::StringName tbl_name = m_selected_entry->db_table->get_table_name();
   const godot::String col_title = m_dialog_addcol.txt_colname->get_text();
   const int type = m_dialog_addcol.opt_type->get_selected_id();

   // Assume the new column is not pointing to another table
   godot::String ext = "";
   if (type == DBTable::VT_ExternalInteger || type == DBTable::VT_ExternalString)
   {
      const int selected = m_dialog_addcol.opt_other_table->get_selected();
      ext = m_dialog_addcol.opt_other_table->get_item_text(selected);
   }

   const int64_t added = m_db->table_insert_column(tbl_name, col_title, (DBTable::ValueType)type, m_dialog_addcol.at_index, ext);
   if (added >= 0)
   {
      m_selected_entry->data_source->column_added(added);
      save_db();
      // In here it's sure that a table is selected. Since a column has been added, make sure the "add row" button is enabled
      m_right.bt_addrow->set_disabled(false);
   }
   else
   {
      WARN_PRINT("Failed to create new column.");
   }
}



void GDDatabaseEditor::confirm_action()
{
   if (!m_selected_entry.is_valid())
   {
      // Should not be here but...
      return;
   }

   switch (m_confirm_code)
   {
      case CC_RemoveTable:
      {
         if (!m_db->remove_table(m_selected_entry->db_table->get_table_name()))
         {
            /// NOTE: Display an error message?
            return;
         }

         save_db();

         // Remove the table entry from the UI. Forcefully removing the entry like this so it's immediately removed from the tree
         m_left.vbox->remove_child(m_selected_entry->ui_table);

         // And free the Node
         m_selected_entry->ui_table->queue_free();

         // Make sure the Entry map is with the correct list
         m_table_entry.erase(m_selected_entry->db_table->get_table_name());

         // Select the first table entry that is available - if any
         if (m_table_entry.size() > 0)
         {
            on_table_selection(m_table_entry.begin()->value);
         }
         else
         {
            on_table_selection(nullptr);
         }
      } break;

      case CC_RemoveColumn:
      {
         const int64_t index = m_dialog_message.data;
         m_dialog_message.data = nullptr;

         if (m_db->table_remove_column(m_selected_entry->db_table->get_table_name(), index))
         {
            save_db();

            // Correcting the index to take the "fake ID column" into account
            m_selected_entry->data_source->column_removed(index + 1);

            m_right.bt_addrow->set_disabled(m_selected_entry->db_table->get_column_count() < 1);
         }
      } break;

      case CC_RemoveRow:
      {
         godot::Array index_list = m_dialog_message.data;
         m_dialog_message.data = nullptr;

         if (m_db->table_remove_row(m_selected_entry->db_table->get_table_name(), index_list))
         {
            m_selected_entry->data_source->row_removed(index_list);
            save_db();
         }
      } break;
   }

   // Reset the confirmation code. Probably not necessary but *might* prevent incorrect actions if the dialog
   // is displayed without properly setting the code first
   m_confirm_code = CC_None;
}


void GDDatabaseEditor::show_dialog(const DialogSettings& settings)
{
   if (!m_dialog_main.dialog)
   {
      build_main_dialog();
   }
   else
   {
      hide_dialog();
   }
   settings.contents->show();

   m_dialog_main.dialog->set_flag(godot::Window::FLAG_RESIZE_DISABLED, !settings.resizable);
   m_dialog_main.dialog->set_title(settings.title);

   if (settings.ok_text.is_empty())
   {
      m_dialog_main.btok->hide();
   }
   else
   {
      m_dialog_main.btok->show();
      m_dialog_main.btok->set_text(settings.ok_text);
   }

   /// FIXME: Deal with localization
   const godot::String ctext = settings.cancel_text.is_empty() ? "Cancel" : settings.cancel_text;
   m_dialog_main.btcancel->set_text(ctext);

   const godot::Size2i minsize = m_dialog_main.dialog->get_contents_minimum_size();
   godot::Size2i sz;

   // The +12 here is to take into account the fact that the VBox holding everything is using offset = 6 on each side
   sz.width = settings.dlg_size.width == 0 ? minsize.width + 12: settings.dlg_size.width;
   sz.height = settings.dlg_size.height == 0 ? minsize.height + 12 : settings.dlg_size.height;
   m_dialog_main.dialog->set_size(sz);
   
   m_dialog_main.current_visible = settings.contents;
   m_dialog_main.dialog->popup_centered();
}


void GDDatabaseEditor::hide_dialog()
{
   if (m_dialog_main.dialog->is_visible())
   {
      m_dialog_main.dialog->hide();
   }
   if (m_dialog_main.current_visible)
   {
      m_dialog_main.current_visible->hide();
      m_dialog_main.current_visible = nullptr;
   }
}


void GDDatabaseEditor::show_message(const godot::String& msg)
{
   /// FIXME: Deal with localization
   DialogSettings settings;
   settings.contents = m_dialog_message.vbox;
   settings.title = "Alert!";
   settings.resizable = false;
   settings.cancel_text = "Close";
   m_dialog_message.label->set_text(msg);
   
   show_dialog(settings);
}


bool GDDatabaseEditor::can_create_column()
{
   /// FIXME: Deal with localization
   const godot::String& colname = m_dialog_addcol.txt_colname->get_text();

   if (colname.is_empty())
   {
      m_dialog_addcol.lbl_colname_err->set_text("Column name can't be empty");
      return false;
   }

   if (colname.to_lower() == "id")
   {
      m_dialog_addcol.lbl_colname_err->set_text(godot::vformat("Column name '%s' is reserved.", colname));
      return false;
   }

   if (m_selected_entry->db_table->has_column(colname))
   {
      const godot::StringName tbl_name = m_selected_entry->db_table->get_table_name();
      m_dialog_addcol.lbl_colname_err->set_text(godot::vformat("Table '%s' already contains a column named '%s'", tbl_name, colname));
      return false;
   }

   const int type_id = m_dialog_addcol.opt_type->get_selected_id();
   const bool external = (type_id >= 500 && type_id < 600);
   if (external && m_dialog_addcol.opt_other_table->get_item_count() == 0)
   {
      // Selected to reference another table, but there is none available to be referenced. So, new column can't be created
      // The list of "other tables" should already be displaying that no table has been found so no need (hopefully) to set
      // an error message here
      return false;
   }

   // If here then the column can be created
   m_dialog_addcol.lbl_colname_err->set_text("");

   return true;
}


void GDDatabaseEditor::save_db()
{
   if (!m_db.is_valid())
   {
      // Should not be here. If so, it's a bug!
      WARN_PRINT("Called save_db() but there is no valid Database open!");
      return;
   }

   const godot::String path = m_db->get_path();
   if (!m_db->save(path, true))
   {
      // There is a chance the save_db() function is called when a recently closed dialog is still "registered" as the current exclusive one.
      // To prevent error messages defer this call
      callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("Failed to save database file\n%s", path));
   }
}


void GDDatabaseEditor::on_table_selection(const godot::Ref<DBETableEntry>& entry)
{
   if (m_selected_entry == entry) { return; }

   godot::Ref<DBETableEntry> previous = m_selected_entry;
   m_selected_entry = entry;

   clear_column_checkboxes();

   if (m_selected_entry.is_valid())
   {
      m_selected_entry->ui_table->queue_redraw();
      m_right.tabular->set_data_source(m_selected_entry->data_source);
      update_ui();

      build_column_checkboxes();

      if (!m_layout.restoring)
      {
         m_layout.db_layout["last_table"] = m_selected_entry->db_table->get_table_name();
         const int64_t res_id = godot::ResourceLoader::get_singleton()->get_resource_uid(m_db->get_path());
         // This next line is probably not necessary if all the setup has been correctly done. Yet, because there is a chance
         // this setup hasn't been correctly done, adding it to prevent errors
         m_layout.master_data[res_id] = m_layout.db_layout;
      }

      const godot::Dictionary width = m_selected_entry->layout.get("width", godot::Dictionary());
      
      // Save current "restoring" flag state
      const bool restoring = m_layout.restoring;
      // Set the restoring flag to true so when setting column widths the layout saving is not triggered.
      m_layout.restoring = true;

      const int64_t ccount = m_selected_entry->data_source->get_column_count();
      for (int64_t i = 0; i < ccount; i++)
      {
         TabularDataSource::ColumnInfo info;
         m_selected_entry->data_source->fill_column_info(i, info);

         const int32_t w = width.get(info.title, -1);
         if (w != -1)
         {
            m_right.tabular->set_column_width(info.title, w);
         }
      }

      // Set the restoring flag to its "original" value prior to calling this function
      m_layout.restoring = restoring;
   }
   else
   {
      m_right.tabular->set_data_source(nullptr);
   }

   if (previous.is_valid())
   {
      previous->ui_table->queue_redraw();
   }

   // The table filtering does not hide the selected table, even if it doesn't match the criteria. Since a new table has been selected,
   // reapply the filtering to ensure the correct hiding criteria is used
   const godot::String tbl_filter = m_left.txt_filter->get_text();
   if (!tbl_filter.is_empty())
   {
      on_table_filtering(tbl_filter);
   }

   if (!m_layout.restoring)
   {
      // Since not restoring layout, save the "last table"
      m_layout.saver.call();
   }
}


void GDDatabaseEditor::draw_table_entry(const godot::Ref<DBETableEntry>& entry)
{
   if (entry == m_selected_entry && m_entry_style_selected.is_valid())
   {
      entry->ui_table->draw_style_box(m_entry_style_selected, godot::Rect2(godot::Vector2(), entry->ui_table->get_size()));
   }
}


void GDDatabaseEditor::on_dbinfo_clicked()
{
   if (!m_db.is_valid())
   {
      return;
   }

   const godot::Dictionary dbinfo = m_db->get_db_info();

   godot::String code = godot::vformat("[b]Database: %s[/b]\n", dbinfo.get("name", ""));
   code += godot::vformat("Table data (count = %d)\n", dbinfo.get("table_count", 0));

   const godot::String tformatter = "\n[b]%s[/b] (%s) - [i]%s ID[/i]\n    Column count: %d | Row count: %d\n";
   const godot::Array table_data = dbinfo.get("table_data", godot::Array());
   const int64_t tsize = table_data.size();
   for (int64_t i = 0; i < tsize; i++)
   {
      const godot::Dictionary t = table_data[i];
      const godot::Array refs = t.get("references", godot::Array());
      const godot::Array refd = t.get("referenced_by", godot::Array());

      code += godot::vformat(tformatter, t.get("name", ""), t.get("path", ""), t.get("id_type", 0), t.get("column_count", 0), t.get("row_count", 0));
      code += godot::vformat("    References: %s\n", refs);
      code += godot::vformat("    Referenced by: %s\n", refd);
   }

   /// FIXME: Deal with localization
   /// FIXME: Dialog size should be persisted, defaulting to 460 x 320
   m_dialog_dbinfo.info->set_text(code);
   DialogSettings settings;
   settings.contents = m_dialog_dbinfo.vbox;
   settings.title = "Database Info";
   settings.resizable = true;
   settings.cancel_text = "Close";
   settings.dlg_size = godot::Size2i(460, 320);
   show_dialog(settings);
}


void GDDatabaseEditor::on_opendb_clicked()
{
   if (!m_dlg_openclose) { create_openclose_dialog(); }
   m_fdlg_type = FDT_LoadDB;

   /// FIXME: Assign last used "open directory" - that is, persist the directory to be used in the FileDialog
   m_dlg_openclose->set_file_mode(godot::FileDialog::FILE_MODE_OPEN_FILE);

   /// FIXME: Deal with localization
   m_dlg_openclose->set_title("Open Database");
   m_dlg_openclose->popup_centered();
}


void GDDatabaseEditor::on_createdb_clicked()
{
   if (!m_dlg_openclose) { create_openclose_dialog(); }
   m_fdlg_type = FDT_SaveDB;
   
   /// FIXME: Assign last used "save directory" - that is, persist the directory to be used in the FileDialog

   m_dlg_openclose->set_file_mode(godot::FileDialog::FILE_MODE_SAVE_FILE);

   /// FIXME: Deal with localization
   m_dlg_openclose->set_title("Create Database");
   m_dlg_openclose->popup_centered();
}


void GDDatabaseEditor::on_export_clicked()
{
   if (!m_db.is_valid())
   {
      return;
   }

   DialogSettings settings;
   settings.contents = m_dialog_export.vbox;
   settings.title = "Export Database";
   settings.resizable = false;
   settings.dlg_size.width = 480;
   settings.ok_text = "Export";

   show_dialog(settings);
   m_dialog_main.btok->set_disabled(m_dialog_export.fd_picker->get_current_path().is_empty());
}


void GDDatabaseEditor::on_createtable_clicked()
{
   const godot::Dictionary ntable = m_layout.db_layout.get("ntable", godot::Dictionary());
   const bool embed = ntable.get("embed", true);
   const bool string_id = ntable.get("string_id", true);
   const bool editable = ntable.get("editable_id", false);

   if (!m_layout.db_layout.has("ntable"))
   {
      // Not necessary *if* everything is properly set. That is, no bugs in the code, however this might prevent errors
      m_layout.db_layout["ntable"] = ntable;
   }

   // First ensure the main input boxes are empty
   m_dialog_newtable.txt_tablename->set_text("");
   m_dialog_newtable.txt_filename->set_text("");
   m_dialog_newtable.txt_filename->set_placeholder("");
   m_dialog_newtable.txt_tablename->call_deferred("grab_focus");

   m_dialog_newtable.chk_embed->set_pressed(embed);
   m_dialog_newtable.chk_string->set_pressed(string_id);
   m_dialog_newtable.chk_integer->set_pressed(!string_id);
   m_dialog_newtable.chk_editable->set_pressed(editable);

   // Build dialog settings them show it
   DialogSettings settings;
   settings.contents = m_dialog_newtable.vbox;
   settings.title = "Create Table";
   settings.resizable = false;
   settings.dlg_size.width = 480;
   settings.ok_text = "Create";

   show_dialog(settings);
   m_dialog_main.btok->set_disabled(true);
}


void GDDatabaseEditor::on_addcol_clicked()
{
   on_insert_column_requested(-1);
}


void GDDatabaseEditor::on_insert_row(int64_t at)
{
   const godot::StringName tbl_name = m_selected_entry->db_table->get_table_name();

   const int64_t added = m_db->table_insert_row(tbl_name, godot::Dictionary(), at);

   if (added >= 0)
   {
      m_selected_entry->data_source->row_added(added);
      save_db();
   }
   else
   {
      WARN_PRINT("Failed to insert new row");
   }
}


void GDDatabaseEditor::on_patchmenu_showing()
{
   godot::PopupMenu* mnu = m_right.mbt_tpatch->get_popup();

   const bool has_table = m_selected_entry.is_valid();
   //const bool has_patch = has_table ? m_selected_entry->data_source->has_patch() : false;
   const bool has_patch = false;

   mnu->set_item_disabled(mnu->get_item_index(PMID_Create), !has_table);
   mnu->set_item_disabled(mnu->get_item_index(PMID_Open), !has_table);
   mnu->set_item_disabled(mnu->get_item_index(PMID_Close), !has_patch);
}


void GDDatabaseEditor::on_patchmenu_selected(int id)
{
   ERR_FAIL_COND(!m_selected_entry.is_valid());
   if (!m_dlg_openclose) { create_openclose_dialog(); }
   switch (id)
   {
      case PMID_Create:
      {
         // Display SAVE dialog, setting things to indicate that upon "OK" is pressed it should create a new DBTablePatch
         m_fdlg_type = FDT_SavePatch;
         m_dlg_openclose->set_title("Create Table Patch");
         m_dlg_openclose->set_file_mode(godot::FileDialog::FILE_MODE_SAVE_FILE);
         m_dlg_openclose->popup_centered();
      } break;

      case PMID_Open:
      {
         // Display LOAD dialog, setting things to indicate that open "OK" is pressed it should open a DBTablePatch
         m_fdlg_type = FDT_LoadPatch;
         m_dlg_openclose->set_title("Open Table Patch");
         m_dlg_openclose->set_file_mode(godot::FileDialog::FILE_MODE_OPEN_FILE);
         m_dlg_openclose->popup_centered();
      } break;

      case PMID_Close:
      {
         // Close a loaded DBTablePatch
         //if (m_selected_entry->data_source->has_patch())
         //{
         //   m_selected_entry->data_source->set_patch(nullptr);

         //   update_ui();
         //}
      } break;
   }
}


void GDDatabaseEditor::on_dialog_ok_clicked()
{
   godot::VBoxContainer* which = m_dialog_main.current_visible;
   hide_dialog();

   if (which == m_dialog_newtable.vbox)
   {
      // Save the layout stuff - that is, the optional stuff set in the dialog
      godot::Dictionary ntable;
      ntable["embed"] = m_dialog_newtable.chk_embed->is_pressed();
      ntable["string_id"] = m_dialog_newtable.chk_string->is_pressed();
      ntable["editable_id"] = m_dialog_newtable.chk_editable->is_pressed();

      m_layout.db_layout["ntable"] = ntable;
      m_layout.saver.call();

      add_new_table();
   }
   else if (which == m_dialog_message.vbox)
   {
      confirm_action();
   }
   else if (which == m_dialog_tablerename.vbox)
   {
      // The "OK" button should be disabled in case the input box is empty or with a name of an already existing table.
      // Should those still be checked? That is, in theory code should only reach this if said input holds valid value.
      rename_selected_table(m_dialog_tablerename.txt_newname->get_text());
   }
   else if (which == m_dialog_addcol.vbox)
   {
      add_column();
   }
   else if (which == m_dialog_export.vbox)
   {
      godot::String indent;
      if (m_dialog_export.mnu_indent_type->get_selected() == 0)
      {
         indent = "\t";
      }
      else if (m_dialog_export.spin_amount->get_value() > 0)
      {
         const int count = m_dialog_export.spin_amount->get_value();
         indent = godot::String(" ").repeat(count);
      }
      m_db->export_to_json(m_dialog_export.fd_picker->get_current_path(), m_dialog_export.chk_monolithic->is_pressed(), indent);
   }
}


void GDDatabaseEditor::on_table_filtering(const godot::String& txt)
{
   const godot::String lower = txt.to_lower();
   for (godot::HashMap<godot::StringName, godot::Ref<DBETableEntry>>::Iterator iter = m_table_entry.begin(); iter != m_table_entry.end(); ++iter)
   {
      const godot::String lname = iter->value->db_table->get_table_name().to_lower();
      bool visible = false;
      if (lower.is_empty() || lname.contains(lower) || m_selected_entry == iter->value)
      {
         visible = true;
      }

      iter->value->ui_table->set_visible(visible);
   }
}


void GDDatabaseEditor::on_value_filter_toggled(bool on)
{
   m_right.filter_box->set_visible(on);
}


void GDDatabaseEditor::on_row_filtering(const godot::String& txt)
{
   if (!m_selected_entry.is_valid()) { return; }

   const bool sensitive = m_right.chk_case_sensitive->is_pressed();
   const bool regex = m_right.chk_regex->is_pressed();
//   m_selected_entry->data_source->pre_filter();
   if (regex)
   {
      m_selected_entry->data_source->set_filter_regex(txt, m_right.filter_exclude);
   }
   else
   {
      m_selected_entry->data_source->set_filter(txt, sensitive, m_right.filter_exclude);
   }
//   m_selected_entry->data_source->post_filter();
}


void GDDatabaseEditor::on_regex_toggled(bool on)
{
   on_row_filtering(m_right.txt_filter->get_text());
}


void GDDatabaseEditor::on_column_checkbox_toggled(bool on)
{
   // Way easier to simply rebuild the list
   m_right.filter_exclude.clear();

   for (godot::KeyValue<godot::String, godot::CheckBox*>& pair : m_right.column_checkbox)
   {
      if (!pair.value->is_pressed())
      {
         m_right.filter_exclude.append(pair.key);
      }
   }

   on_row_filtering(m_right.txt_filter->get_text());
}


void GDDatabaseEditor::on_newtable_name_changed(const godot::String& txt)
{
   m_dialog_main.btok->set_disabled(txt.is_empty() || m_db->has_table(txt));
   m_dialog_newtable.txt_filename->set_placeholder(txt);
}


void GDDatabaseEditor::on_newtable_embed_toggled(bool on)
{
   const godot::String placeholder = on ? "" : m_dialog_newtable.txt_tablename->get_text();

   m_dialog_newtable.txt_filename->set_editable(!on);
   m_dialog_newtable.txt_filename->set_placeholder(placeholder);
}


void GDDatabaseEditor::on_table_rename_input(const godot::String& txt)
{
   // Empty text, DB already has new name or text is equal to the old one
   m_dialog_main.btok->set_disabled(txt.is_empty() || m_db->has_table(txt) || m_selected_entry->db_table->get_table_name() == txt);
}


void GDDatabaseEditor::on_column_value_type_selected(int index)
{
   const int type_id = m_dialog_addcol.opt_type->get_item_id(index);

   const bool changeable = type_id >= 1000;
   const bool external = (type_id >= 500 && type_id < 600);

   // Regardless if selecting a type referencing another table or not, clear the list of tables
   godot::PopupMenu* pop = m_dialog_addcol.opt_other_table->get_popup();
   pop->clear();
   pop->set_size(godot::Vector2());
   m_dialog_addcol.opt_other_table->select(-1);

   // And assume the other table list will be disabled
   bool other_list_disabled = true;


   if (!changeable)
   {
      const godot::String tpstr = m_dialog_addcol.opt_type->get_item_text(index);
      m_dialog_addcol.lbl_vtype_warn->set_text(godot::vformat("Caution: A column of '%s' cannot have its type changed after it's created.", tpstr));
   }
   else
   {
      m_dialog_addcol.lbl_vtype_warn->set_text("");
   }

   if (external)
   {
      godot::PackedStringArray entries;
      m_db->fill_external_candidates_for(type_id, m_selected_entry->db_table->get_table_name(), entries);

      for (godot::String entry : entries)
      {
         pop->add_item(entry);
      }

      if (entries.size() > 0)
      {
         m_dialog_addcol.opt_other_table->select(0);
         other_list_disabled = false;
      }
      else
      {
         m_dialog_addcol.opt_other_table->select(-1);
         m_dialog_addcol.opt_other_table->set_text("**No valid table found**");
      }
   }

   m_dialog_addcol.opt_other_table->set_disabled(other_list_disabled);
   m_dialog_main.btok->set_disabled(!can_create_column());
}



void GDDatabaseEditor::on_mouse_enter_entry(const godot::Ref<DBETableEntry>& entry)
{
   m_entry_under_mouse = entry;
}


void GDDatabaseEditor::on_mouse_exit_entry(const godot::Ref<DBETableEntry>& entry)
{
   if (m_entry_under_mouse == entry)
   {
      m_entry_under_mouse = godot::Ref<DBETableEntry>();
   }
}


void GDDatabaseEditor::on_table_list_input(const godot::Ref<godot::InputEvent>& evt)
{
   godot::Ref<godot::InputEventMouseButton> mb = evt;
   if (mb.is_valid())
   {
      if (mb->is_pressed() && m_entry_under_mouse.is_valid() && mb->get_button_index() == godot::MouseButton::MOUSE_BUTTON_LEFT)
      {
         // Unselect cells as this will force any pending change to be committed
         m_right.tabular->unselect_all_cells();
         // Yet defer the call to make sure any changes will be done before assigning the new table
         callable_mp(this, &GDDatabaseEditor::on_table_selection).call_deferred(m_entry_under_mouse);
      }
   }
}


void GDDatabaseEditor::on_entry_rename_clicked(const godot::Ref<DBETableEntry>& entry)
{
   if (entry == m_selected_entry)
   {
      const godot::String oname = entry->db_table->get_table_name();

      m_dialog_tablerename.lbl_info->set_text(godot::vformat("Rename '%s' to:", oname));
      m_dialog_tablerename.txt_newname->set_text("");
      m_dialog_tablerename.txt_newname->call_deferred("grab_focus");

      DialogSettings settings;
      settings.contents = m_dialog_tablerename.vbox;
      settings.title = "Rename table";
      settings.resizable = false;
      settings.dlg_size.width = 460;
      settings.ok_text = "Rename";
      show_dialog(settings);
      m_dialog_main.btok->set_disabled(true);
   }
   else
   {
      // Unselect cells as this will force any pending change to be committed
      m_right.tabular->unselect_all_cells();
      // Yet defer the call to make sure any changes will be done before assigning the new table
      callable_mp(this, &GDDatabaseEditor::on_table_selection).call_deferred(entry);
   }
}


void GDDatabaseEditor::on_entry_remove_clicked(const godot::Ref<DBETableEntry>& entry)
{
   if (entry == m_selected_entry)
   {
      const godot::PackedStringArray reflist = entry->db_table->get_referenced_by_list();

      if (reflist.size() > 0)
      {
         godot::String msg = godot::vformat("Table '%s' is referenced by the following table(s), thus cannot be removed:", entry->db_table->get_table_name());
         const int64_t sz = reflist.size();
         for (int64_t i = 0; i < sz; i++)
         {
            msg += godot::vformat("\n- %s", reflist[i]);
         }

         show_message(msg);
         return;
      }

      const godot::String cmsg = godot::vformat("Do you really want to remove '%s' table?\nThis can't be undone!", entry->db_table->get_table_name());
      m_dialog_message.label->set_text(cmsg);
      m_confirm_code = CC_RemoveTable;

      DialogSettings settings;
      settings.contents = m_dialog_message.vbox;
      settings.title = "Confirm";
      settings.resizable = false;
      settings.ok_text = "Yes";
      settings.cancel_text = "No";
      
      show_dialog(settings);
      
      m_dialog_main.btok->set_disabled(false);
   }
   else
   {
      // Unselect cells as this will force any pending change to be committed
      m_right.tabular->unselect_all_cells();
      // Yet defer the call to make sure any changes will be done before assigning the new table
      callable_mp(this, &GDDatabaseEditor::on_table_selection).call_deferred(entry);
   }
}


void GDDatabaseEditor::on_newcolumn_name_changed(const godot::String& txt)
{

   m_dialog_main.btok->set_disabled(!can_create_column());
}


void GDDatabaseEditor::on_export_path_selected(const godot::String& path)
{
   m_dialog_main.btok->set_disabled(path.is_empty());
}


void GDDatabaseEditor::on_export_indent_id_selected(int64_t id)
{
   // ID = 1 is the "spaces". The other option is "tab", which doesn't make any sense to keep amount enabled
   m_dialog_export.spin_amount->set_editable(id == 1);
}


void GDDatabaseEditor::on_file_selected(const godot::String& path)
{
   godot::Ref<GDDatabase> db;

   /*if (m_dlg_openclose->get_file_mode() == godot::FileDialog::FILE_MODE_OPEN_FILE)
   {
      godot::Ref<GDDatabase> res = godot::ResourceLoader::get_singleton()->load(path);
      if (res.is_valid())
      {
         db = res;
      }
      else
      {
         // The FileDialog is still currently the "exclusive" window. Defer the call to show the message as an immediate attempt
         // will result in error messages."
         callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("The provided file\n'%s'\nis not a GDDatabase", path));
      }
   }
   else if (m_dlg_openclose->get_file_mode() == godot::FileDialog::FILE_MODE_SAVE_FILE)
   {
      godot::Ref<GDDatabase> newdb;
      newdb.instantiate();

      if (godot::ResourceSaver::get_singleton()->save(newdb, path) == godot::Error::OK)
      {
         db = godot::ResourceLoader::get_singleton()->load(path);
      }
      else
      {
         callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("Failed to create new GDDatabase file at\n%s", path));
      }
   }*/
   switch (m_fdlg_type)
   {
      case FDT_LoadDB:
      {
         godot::Ref<GDDatabase> res = godot::ResourceLoader::get_singleton()->load(path);
         if (res.is_valid())
         {
            db = res;
         }
         else
         {
            // The FileDialog is still currently the "exclusive" window. Defer the call to show the message as an immediate attempt
            // will result in error messages."
            callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("The selected file\n'%s'\nis not a GDDatabase", path));
         }
      } break;

      case FDT_SaveDB:
      {
         godot::Ref<GDDatabase> newdb;
         newdb.instantiate();

         if (godot::ResourceSaver::get_singleton()->save(newdb, path) == godot::Error::OK)
         {
            db = godot::ResourceLoader::get_singleton()->load(path);
         }
         else
         {
            callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("Failed to create new GDDatabase file at\n%s", path));
         }
      } break;

      case FDT_LoadPatch:
      {
         //ERR_FAIL_COND(!m_selected_entry.is_valid());
         //godot::Ref<DBTablePatch> res = godot::ResourceLoader::get_singleton()->load(path);

         //if (res.is_valid())
         //{
         //   if (m_selected_entry->db_table->get_table_id() == res->get_target_id())
         //   {
         //      m_selected_entry->data_source->set_patch(res);

         //   }
         //   else
         //   {
         //      const godot::String tbl_name = m_selected_entry->db_table->get_name();
         //      callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("File '%s' is a valid patch but not for the selected table '%s'", path, tbl_name));
         //   }
         //}
         //else
         //{
         //   callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("The selected file\n'%s'is not a DBTablePatch", path));
         //}

         //update_ui();

      } break;

      case FDT_SavePatch:
      {
         //ERR_FAIL_COND(!m_selected_entry.is_valid());
         //godot::Ref<DBTablePatch> patch = memnew(DBTablePatch(m_selected_entry->db_table->get_table_id()));

         //if (godot::ResourceSaver::get_singleton()->save(patch, path) == godot::Error::OK)
         //{
         //   m_selected_entry->data_source->set_patch(godot::ResourceLoader::get_singleton()->load(path));
         //}
         //else
         //{
         //   callable_mp(this, &GDDatabaseEditor::show_message).call_deferred(godot::vformat("Failed to create new DBTablePatch file at\n%s", path));
         //}

         //update_ui();

      } break;
   }

   m_fdlg_type = FDT_None;

   if (db.is_valid())
   {
      edit(db);
   }
}


void GDDatabaseEditor::on_dialog_input(const godot::Ref<godot::InputEvent>& event)
{
   godot::Ref<godot::InputEventKey> key = event;
   if (key.is_valid() && key->is_pressed())
   {
      if (key->is_action("ui_cancel"))
      {
         hide_dialog();
      }
      else if (key->is_action("ui_text_submit"))
      {
         if (m_dialog_main.btok->is_visible() && !m_dialog_main.btok->is_disabled())
         {
            on_dialog_ok_clicked();
         }
      }
   }
}


void GDDatabaseEditor::on_insert_column_requested(int64_t at_index)
{
   m_dialog_addcol.at_index = at_index - 1;

   m_dialog_addcol.txt_colname->set_text("");
   m_dialog_addcol.txt_colname->call_deferred("grab_focus");

   const int index = m_dialog_addcol.opt_type->get_item_index(DBTable::VT_String);
   m_dialog_addcol.opt_type->select(index);

   // String type is selected by default, so disable the "other table" reference.
   m_dialog_addcol.opt_other_table->get_popup()->clear();
   m_dialog_addcol.opt_other_table->set_disabled(true);
   m_dialog_addcol.opt_other_table->set_text("");
   m_dialog_addcol.lbl_vtype_warn->set_text("");

   DialogSettings settings;
   settings.contents = m_dialog_addcol.vbox;
   /// FIXME: Deal with localization
   settings.title = godot::vformat("Add column to '%s'", m_selected_entry->db_table->get_table_name());
   settings.resizable = false;
   settings.dlg_size.width = 510;
   settings.ok_text = "Create";

   show_dialog(settings);
   m_dialog_main.btok->set_disabled(!can_create_column());
}


void GDDatabaseEditor::on_column_rename_requested(int64_t column_index, const godot::String& title)
{
   // Assume rename will not be rejected
   bool rejected = false;
   godot::String old_name;

   if (column_index == 0 || m_selected_entry->db_table->has_column(title))
   {
      rejected = true;
   }
   else
   {
      // The data source creates a "fake column" to hold the row IDs. So subtract one from the column index
      // so the "real column" is dealt with
      const godot::StringName table_name = m_selected_entry->db_table->get_table_name();
      old_name = m_selected_entry->db_table->get_column_title(column_index - 1);
      rejected = !m_db->table_rename_column(table_name, column_index - 1, title);
   }

   if (!rejected)
   {
      godot::CheckBox* chk = m_right.column_checkbox[old_name];
      chk->set_text(title);
      m_right.column_checkbox.erase(old_name);
      m_right.column_checkbox[title] = chk;

      save_db();

      godot::Dictionary cwidth = m_selected_entry->layout.get("width", godot::Dictionary());
      if (cwidth.has(old_name))
      {
         ExtPackUtils::change_dictionary_key(cwidth, old_name, title);
         m_layout.saver.call();
      }
   }

   m_selected_entry->data_source->column_renamed(column_index, rejected);
}


void GDDatabaseEditor::on_column_remove_requested(int64_t column_index)
{
   if (column_index == 0)
   {
      return;
   }

   DialogSettings settings;
   settings.contents = m_dialog_message.vbox;
   settings.title = "Confirm column removal";
   settings.ok_text = "Yes";
   settings.cancel_text = "No";

   // Data source creates a "fake column" to hold the row IDs. Subtracting 1 from the index to compensate
   const godot::String colname = m_selected_entry->db_table->get_column_title(column_index - 1);
   m_dialog_message.label->set_text(godot::vformat("Do you really want to remove '%s' column?\nThis can't be undone!", colname));
   m_dialog_message.data = column_index - 1;

   m_confirm_code = CC_RemoveColumn;

   show_dialog(settings);
}


void GDDatabaseEditor::on_column_move_requested(int64_t from, int64_t to)
{
   const godot::StringName table_name = m_selected_entry->db_table->get_table_name();

   // Data source creates a "fake column" to hold the row IDs. Subtracting 1 from the index to compensate
   if (m_db->table_move_column(table_name, from - 1, to - 1))
   {
      m_selected_entry->data_source->column_moved(from, to);
      save_db();
   }
}

void GDDatabaseEditor::on_column_type_change_requested(int64_t column_index, int to_type)
{
   const godot::StringName table_name = m_selected_entry->db_table->get_table_name();
   const DBTable::ValueType type = (DBTable::ValueType)to_type;
   // Data source creates a "fake column" to hold the row IDs. Subtracting 1 from the index to compensate
   if (m_db->table_change_column_value_type(table_name, column_index - 1, type))
   {
      m_selected_entry->data_source->column_type_changed(column_index);
      save_db();
   }
}


void GDDatabaseEditor::on_column_resized(const godot::String& title, int width)
{
   if (m_layout.restoring || !m_selected_entry.is_valid())
   {
      return;
   }

   godot::Dictionary layout_width = m_selected_entry->layout.get("width", godot::Dictionary());
   layout_width[title] = width;

   // This next line is not exactly necessary *if* everything is properly setup. To prevent errors in case something
   // is not, add it
   m_selected_entry->layout["width"] = layout_width;
   m_layout.saver.call();
}


void GDDatabaseEditor::on_value_change_requested(int64_t column_index, int64_t row_index, const godot::Variant& value)
{
   const godot::StringName tbl_name = m_selected_entry->db_table->get_table_name();

   // Assume no change will be done
   bool changed = false;

   if (column_index == 0)
   {
      // If the table id type is Integer, incoming value might by a float inside the Variant, simply because the
      // "numeric editor" uses a Control that internally uses floats. So, must do this verification and proper conversion here
      if (m_selected_entry->db_table->get_id_type() == godot::Variant::STRING)
      {
         const godot::String strval = value;
         changed = m_db->table_set_row_id(tbl_name, row_index, strval);
      }  
      else
      {
         const int64_t intval = value;
         changed = m_db->table_set_row_id(tbl_name, row_index, intval);
      }
   }
   else
   {
      // Subtracting 1 from the column_index to take the ID column into account, which is a "fake" column
      // shown within the UI but not exactly counted within the DBTable
      changed = m_db->table_set_cell_value(tbl_name, column_index - 1, row_index, value);
   }

   if (changed)
   {
      save_db();
   }

   // Relay the change (or rejection) into the data source
   m_selected_entry->data_source->value_changed(column_index, row_index, value, !changed);
}


void GDDatabaseEditor::on_row_remove_requested(const godot::Array& index_list)
{
   godot::String txt = "";

   const int64_t lsize = index_list.size();

   if (lsize == 0) { return; }

   // It becomes way more useful to display Row number rather than row index in the UI. So, iterate through entries and
   // add one into each one. Also, incoming list is in inverse order, so to make things easier reverse it
   godot::Array rownum;
   for (int64_t i = lsize - 1; i >= 0; i--)
   {
      int64_t index = index_list[i];
      rownum.append(index + 1);
   }

   if (lsize > 1)
   {
      txt = godot::vformat("Do you really want to remove selected rows from the database?\n%s", rownum);
   }
   else if (lsize == 1)
   {
      txt = godot::vformat("Do you really want to remove row %d from the database?", rownum[0]);
   }

   if (txt.is_empty())
   {
      return;
   }

   DialogSettings settings;
   settings.contents = m_dialog_message.vbox;
   settings.title = "Confirm row removal";
   settings.ok_text = "Yes";
   settings.cancel_text = "No";

   m_dialog_message.label->set_text(txt);
   m_dialog_message.data = index_list;

   m_confirm_code = CC_RemoveRow;

   show_dialog(settings);
}


void GDDatabaseEditor::on_row_move_requested(int64_t from, int64_t to)
{
   const godot::StringName table_name = m_selected_entry->db_table->get_table_name();
   if (m_db->table_move_row(table_name, from, to))
   {
      m_selected_entry->data_source->row_moved(from, to);
      save_db();
   }
}


void GDDatabaseEditor::on_row_sort_requested(int64_t column_index, bool ascending)
{
   const godot::StringName table_name = m_selected_entry->db_table->get_table_name();
   bool changed = false;
   if (column_index == 0)
   {
      changed = m_db->table_sort_rows_by_id(table_name, ascending);
   }
   else
   {
      // The data source creates a "fake column" to display the row IDs. So compensante the column index by
      // subtracting 1 from it
      changed = m_db->table_sort_rows(table_name, column_index - 1, ascending);
   }

   if (changed)
   {
      m_selected_entry->data_source->rows_sorted();
      save_db();
   }
}


void GDDatabaseEditor::debug_draw(godot::Control* ctrl, const godot::Color& cl)
{
   ctrl->draw_rect(godot::Rect2(godot::Vector2(), ctrl->get_size()), cl);
}


void GDDatabaseEditor::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         // Connect event handlers
         m_top_bar.bt_dbinfo->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_dbinfo_clicked));
         m_top_bar.bt_opendb->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_opendb_clicked));
         m_top_bar.bt_createdb->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_createdb_clicked));
         m_top_bar.bt_export->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_export_clicked));

         m_left.bt_filter->connect("toggled", callable_mp((godot::CanvasItem*)m_left.txt_filter, &godot::LineEdit::set_visible));
         m_left.txt_filter->connect("text_changed", callable_mp(this, &GDDatabaseEditor::on_table_filtering));
         m_left.bt_add->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_createtable_clicked));
         m_left.vbox->connect("gui_input", callable_mp(this, &GDDatabaseEditor::on_table_list_input));

         m_right.bt_filter->connect("toggled", callable_mp(this, &GDDatabaseEditor::on_value_filter_toggled));
         m_right.bt_addcol->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_addcol_clicked));
         m_right.bt_addrow->connect("pressed", callable_mp(this, &GDDatabaseEditor::on_insert_row).bind((int64_t)-1));
//         m_right.mbt_tpatch->connect("about_to_popup", callable_mp(this, &GDDatabaseEditor::on_patchmenu_showing));
//         m_right.mbt_tpatch->get_popup()->connect("id_pressed", callable_mp(this, &GDDatabaseEditor::on_patchmenu_selected));
         m_right.txt_filter->connect("text_changed", callable_mp(this, &GDDatabaseEditor::on_row_filtering));
         m_right.chk_regex->connect("toggled", callable_mp(this, &GDDatabaseEditor::on_regex_toggled));
         m_right.tabular->connect("insert_column_requested", callable_mp(this, &GDDatabaseEditor::on_insert_column_requested));
         m_right.tabular->connect("column_rename_requested", callable_mp(this, &GDDatabaseEditor::on_column_rename_requested));
         m_right.tabular->connect("column_remove_requested", callable_mp(this, &GDDatabaseEditor::on_column_remove_requested));
         m_right.tabular->connect("column_move_requested", callable_mp(this, &GDDatabaseEditor::on_column_move_requested));
         m_right.tabular->connect("column_type_change_requested", callable_mp(this, &GDDatabaseEditor::on_column_type_change_requested));
         m_right.tabular->connect("column_resized", callable_mp(this, &GDDatabaseEditor::on_column_resized));
         m_right.tabular->connect("value_change_requested", callable_mp(this, &GDDatabaseEditor::on_value_change_requested));
         m_right.tabular->connect("insert_row_requested", callable_mp(this, &GDDatabaseEditor::on_insert_row));
         m_right.tabular->connect("row_remove_requested", callable_mp(this, &GDDatabaseEditor::on_row_remove_requested));
         m_right.tabular->connect("row_move_requested", callable_mp(this, &GDDatabaseEditor::on_row_move_requested));
         m_right.tabular->connect("row_sort_requested", callable_mp(this, &GDDatabaseEditor::on_row_sort_requested));
      } break;

      case NOTIFICATION_THEME_CHANGED:
      {
         check_style();
      } break;

      case NOTIFICATION_ENTER_TREE:
      {
         build_msg_dialog();
         build_dbinfo_dialog();
         build_opencreate_dialog();
         build_newtable_dialog();
         build_tablerename_dialog();
         build_addcol_dialog();
         build_export_dialog();
      } break;


      case NOTIFICATION_EXIT_TREE:
      {
         if (!m_dialog_main.dialog)
         {
            // If here then none of the dialog contols have been actually added into the tree.
            // Manually delete them
            memdelete(m_dialog_message.vbox);
            memdelete(m_dialog_dbinfo.vbox);
            memdelete(m_dialog_newtable.vbox);
            memdelete(m_dialog_tablerename.vbox);
            memdelete(m_dialog_addcol.vbox);
            memdelete(m_dialog_export.vbox);

            m_dialog_message.vbox = nullptr;
            m_dialog_dbinfo.vbox = nullptr;
            m_dialog_newtable.vbox = nullptr;
            m_dialog_tablerename.vbox = nullptr;
            m_dialog_addcol.vbox = nullptr;
            m_dialog_export.vbox = nullptr;
         }
      } break;
   }
}


bool GDDatabaseEditor::_can_drop_data(const godot::Vector2& at, const godot::Variant& data) const
{
   if (!m_db.is_valid()) { return false; }

   // This rect has position relative to its parent, which is a ScrollContainer. In turn the ScrollContainer
   // is added into a Panel, which is added into a VBoxContainer, which is added into a HSplitContainer.
   // Incoming position is local to the HSplitContainer's parent, which is the instance of GDDatabaseEditor
   // (read: this class). So must convert the obtained rect's position to be relative to the main editor
   godot::Rect2 valid_drop_rect = m_left.vbox->get_rect();
   valid_drop_rect.position = m_left.vbox->get_global_position() - get_global_position();

   if (!valid_drop_rect.has_point(at)) { return false; }

   // From the file tree inside the Godot editor, the data should be a dictionary with the following entries:
   // "type" (string): "files"
   // "files" (array): array of of paths to the files being dragged
   // "from": (object): Pointer to Control node from where the file has been dragged
   // That said, the relevant entries here are "type" and "files". If type is different from "files" then most
   // likely the dragged thing is of no interest. Then, the "files" array should contain the list of dragged
   // resources. In this case, scan each one and reject is a single one is not a DBTable. Otherwise can return
   // true. Or perhaps allow if there is at least one table?
   if (data.get_type() != godot::Variant::DICTIONARY) { return false; }


   // For easier querying from dictionary
   const godot::Dictionary ddata = data;

   const godot::String type = ddata.get("type", "");
   if (type != "files") { return false; }

   // Assume dragged data is valid
   bool is_valid = true;
   const godot::Array files = ddata.get("files", godot::Array());
   const int64_t fsize = files.size();
   for (int64_t i = 0; i < fsize; i++)
   {
      /// TODO: Check if there is a better way of verifying if the dragged data is valid, other than first loading the resource

      const godot::String fpath = files[i];
      godot::Ref<DBTable> tbl = godot::ResourceLoader::get_singleton()->load(fpath);
      if (!tbl.is_valid())
      {
         // Well, got a resource that is not a DBTable. So, mark as invalid dragged data
         is_valid = false;
         break;
      }

      if (m_db->has_table(tbl->get_table_name()))
      {
         // Ok, the resource is a DBTable, but the Database already contains a table with that name.
         // Filter out...
         is_valid = false;
         break;
      }
   }

   return is_valid;
}

void GDDatabaseEditor::_drop_data(const godot::Vector2& at, const godot::Variant& data)
{
   // In theory invalid 'data' has already been filtered out by the _can_drop_data(), so not checking it
   godot::String err = "";
   const godot::Dictionary ddata = data;
   const godot::Array files = ddata.get("files", godot::Array());
   const int64_t fsize = files.size();
   for (int64_t i = 0; i < fsize; i++)
   {
      const godot::String fpath = files[i];
      godot::Ref<DBTable> table = godot::ResourceLoader::get_singleton()->load(fpath);

      if (!m_db->add_table_res(table))
      {
         err += godot::vformat("- %s", table->get_table_name());
      }
      else
      {
         add_table(table, false);
         save_db();
         sort_table_list();
      }
   }

   if (!err.is_empty())
   {
      show_message(godot::vformat("While dropping table resources, failed to add the following into the Database:\n%s", err));
   }
}


void GDDatabaseEditor::edit(const godot::Ref<GDDatabase>& db)
{
   if (m_db == db) { return; }

   m_db = db;

   // Clear all table entries from the table list
   {
      const int64_t size = m_left.vbox->get_child_count();
      for (int64_t i = 0; i < size; i++)
      {
         m_left.vbox->get_child(i)->queue_free();
      }
   }

   if (db.is_valid())
   {
      const int64_t res_id = godot::ResourceLoader::get_singleton()->get_resource_uid(db->get_path());

      if (!m_layout.master_data.has(res_id))
      {
         godot::Dictionary ntable;
         ntable["embed"] = m_dialog_newtable.chk_embed ? m_dialog_newtable.chk_embed->is_pressed() : true;
         ntable["string_id"] = m_dialog_newtable.chk_string ? m_dialog_newtable.chk_string->is_pressed() : true;
         ntable["editable_id"] = m_dialog_newtable.chk_editable ? m_dialog_newtable.chk_editable->is_pressed() : false;

         godot::Dictionary dblayout;
         dblayout["last_table"] = godot::StringName("");
         dblayout["ntable"] = ntable;
         dblayout["table"] = godot::Dictionary();
         m_layout.master_data[res_id] = dblayout;
      }

      m_layout.db_layout = m_layout.master_data[res_id];
   }

   const godot::PackedStringArray table_list = m_db.is_valid() ? m_db->get_table_list() : godot::PackedStringArray();

   const godot::StringName last_table = m_layout.db_layout.get("last_table", "");

   bool selected = false;

   // Build the list of tables
   {
      const int64_t size = table_list.size();
      for (int64_t i = 0; i < size; i++)
      {
         godot::Ref<DBTable> table = m_db->get_table(table_list[i]);
         const bool must_select = (!selected && (last_table.is_empty() || table->get_table_name() == last_table));
         
         add_table(table, must_select);

         selected = selected || must_select;
      }
   }

   if (selected)
   {
      sort_table_list();
   }
   else
   {
      // If here then no table has been selected. In this case most likely the incoming DB is not valid or it's empty.
      // If there ws a previous database open, must ensure the data is cleared within the TabularBox
      on_table_selection(nullptr);
   }


   if (!m_layout.restoring)
   {
      // This will ensure the "last edited" database is saved.
      m_layout.saver.call();
   }


   update_ui();
}


void GDDatabaseEditor::set_window_layout(const godot::Ref<godot::ConfigFile>& layout)
{
   const godot::String category = "GDDBPlugin";
   m_layout.restoring = true;

   const int64_t last_db_id = layout->get_value(category, "last_open", -1);

   m_layout.master_data = layout->get_value(category, "layout_data", godot::Dictionary());
   const int32_t split_offset = layout->get_value(category, "splitter", 0);

   if (godot::ResourceUID::get_singleton()->has_id(last_db_id))
   {
      const godot::String path = godot::ResourceUID::get_singleton()->get_id_path(last_db_id);
      if (!path.is_empty() && godot::ResourceLoader::get_singleton()->exists(path))
      {
         godot::Ref<GDDatabase> db = godot::ResourceLoader::get_singleton()->load(path);
         if (db.is_valid())
         {
            edit(db);
         }
      }
   }

   m_layout.restoring = false;
}


void GDDatabaseEditor::get_window_layout(const godot::Ref<godot::ConfigFile>& layout)
{
   const godot::String category = "GDDBPlugin";

   // What must be saved:
   // - The offset of the vertical splitter that separates column list from the TabularBox
   // - Last used "new table settings" (embedded/extra file, string/integer ID type)
   // - *Maybe* list of columsn to be excluded in row filtering, in a "per table basis"
   // - Last selected table, in a "per database basis"
   // - Widths of the columns, in a "per table basis"
   // - *Maybe toggled state of the filter buttons
   // - Last edited database
   // Important thing to note is that all of those must be saved for each database.

   // Store the "last db" as its UID, which allows mapping to it even if the resource file is renamed or even moved within the project
   const int64_t last_db = m_db.is_valid() ? godot::ResourceLoader::get_singleton()->get_resource_uid(m_db->get_path()) : -1;
   const int32_t split_offset = m_hsplit->get_split_offset();

   layout->set_value(category, "last_open", last_db);
   layout->set_value(category, "layout_data", m_layout.master_data);       // this includes column width data
   layout->set_value(category, "splitter", split_offset);
}


GDDatabaseEditor::GDDatabaseEditor()
{
   m_confirm_code = CC_None;
   m_tbox_theme.instantiate();
   m_layout.restoring = false;

   m_dlg_openclose = nullptr;
   m_fdlg_type = FDT_None;

   /// FIXME: Deal with localization so this can be translated
   m_lbl_style = Internals::make_flat_stylebox(godot::Color(), 2, 2, 2, 2, 2.0, false, 1);
   m_lbl_style->set_border_color(godot::Color(0.1, 0.1, 0.15, 1.0));

   m_tex_magnify = Internals::texture_from_base64(ICON_MAGNIFY);


   /// Build the top "toolbar". There is no need to keep a reference to the PanelContainer holding the widgets
   godot::PanelContainer* top_bar = memnew(godot::PanelContainer);
   add_child(top_bar);

   {
      godot::HBoxContainer* hbox = memnew(godot::HBoxContainer);
      top_bar->add_child(hbox);

      // When clicked this should display information regarding the loaded Database. That said, if not editing
      // any then this button should be disabled
      m_top_bar.bt_dbinfo = memnew(godot::Button);
      m_top_bar.bt_dbinfo->set_text("Database file");
      m_top_bar.bt_dbinfo->set_disabled(true);
      hbox->add_child(m_top_bar.bt_dbinfo);

      // This label is meant to dispaly the full path of the loaded database
      m_top_bar.lbl_dbfile = memnew(godot::Label);
      m_top_bar.lbl_dbfile->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_top_bar.lbl_dbfile->add_theme_stylebox_override("normal", m_lbl_style);
      hbox->add_child(m_top_bar.lbl_dbfile);

      // Button used to open a database. In other words, clicking it should bring the file dialog meant to choose
      // a database.
      m_top_bar.bt_opendb = memnew(godot::Button);
      m_top_bar.bt_opendb->set_text("Open DB");
      hbox->add_child(m_top_bar.bt_opendb);

      // Then this should create a Database
      m_top_bar.bt_createdb = memnew(godot::Button);
      m_top_bar.bt_createdb->set_text("Create DB");
      hbox->add_child(m_top_bar.bt_createdb);

      // The button to bring export dialog
      m_top_bar.bt_export = memnew(godot::Button);
      m_top_bar.bt_export->set_text("Export");
      hbox->add_child(m_top_bar.bt_export);
   }

   /// Build the "main area" of the editor. This is a PanelContainer that will fill the remaining space and will hold
   // a "two side" divider, which is basically a split container
   godot::PanelContainer* main_container = memnew(godot::PanelContainer);
   main_container->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   main_container->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
   main_container->set_mouse_filter(MOUSE_FILTER_IGNORE);
   add_child(main_container);

   m_hsplit = memnew(godot::HSplitContainer);
   m_hsplit->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_hsplit->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
   m_hsplit->set_mouse_filter(MOUSE_FILTER_PASS);
   m_hsplit->set_dragger_visibility(godot::SplitContainer::DRAGGER_VISIBLE);
   main_container->add_child(m_hsplit);

   /// Build the Left section - which is meant to display the list of tables within the Database
   {
      godot::VBoxContainer* vbox = memnew(godot::VBoxContainer);
      m_hsplit->add_child(vbox);
      vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
      vbox->set_custom_minimum_size(godot::Vector2(200, 0));

      // Little "bar" with label and button. The label is just to make it clear that this is the table list while the
      // button is used to add a new table into the datbase
      godot::HBoxContainer* hbox = memnew(godot::HBoxContainer);
      hbox->set_h_size_flags(SIZE_EXPAND_FILL);
      vbox->add_child(hbox);

      godot::Label* lbl_tables = memnew(godot::Label);
      lbl_tables->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      lbl_tables->set_v_size_flags(godot::Control::SIZE_SHRINK_CENTER);
      lbl_tables->set_text("Tables");
      hbox->add_child(lbl_tables);

      m_left.bt_filter = memnew(godot::Button);
      m_left.bt_filter->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
      m_left.bt_filter->set_button_icon(m_tex_magnify);
      m_left.bt_filter->set_toggle_mode(true);
      hbox->add_child(m_left.bt_filter);

      m_left.bt_add = memnew(godot::Button);
      m_left.bt_add->set_text("+");
      m_left.bt_add->set_disabled(true);
      hbox->add_child(m_left.bt_add);

      // The box that will be used to filter table list
      m_left.txt_filter = memnew(godot::LineEdit);
      m_left.txt_filter->set_placeholder("Filter table...");
      m_left.txt_filter->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_left.txt_filter->set_clear_button_enabled(true);
      m_left.txt_filter->hide();
      vbox->add_child(m_left.txt_filter);


      // Background panel that should help "highlight" the list of tables
      godot::Panel* back_panel = memnew(godot::Panel);
      back_panel->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      back_panel->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
      back_panel->set_mouse_filter(MOUSE_FILTER_IGNORE);
      vbox->add_child(back_panel);

      godot::ScrollContainer* scroller = memnew(godot::ScrollContainer);
      scroller->set_anchor_and_offset(godot::Side::SIDE_LEFT, 0, 2);
      scroller->set_anchor_and_offset(godot::Side::SIDE_TOP, 0, 2);
      scroller->set_anchor_and_offset(godot::Side::SIDE_RIGHT, 1, -2);
      scroller->set_anchor_and_offset(godot::Side::SIDE_BOTTOM, 1, -2);
      back_panel->add_child(scroller);

      m_left.vbox = memnew(godot::VBoxContainer);
      m_left.vbox->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_left.vbox->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
      scroller->add_child(m_left.vbox);
   }

   /// Build the right section - this will hold the cells of the selected table
   {
      godot::VBoxContainer* vbox = memnew(godot::VBoxContainer);
      m_hsplit->add_child(vbox);

      godot::HBoxContainer* hbox = memnew(godot::HBoxContainer);
      vbox->add_child(hbox);

      m_right.lbl_tablename = memnew(godot::Label);
      m_right.lbl_tablename->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_right.lbl_tablename->set_v_size_flags(godot::Control::SIZE_SHRINK_CENTER);
      m_right.lbl_tablename->add_theme_stylebox_override("normal", m_lbl_style);
      hbox->add_child(m_right.lbl_tablename);

      m_right.bt_filter = memnew(godot::Button);
      m_right.bt_filter->set_button_icon(m_tex_magnify);
      m_right.bt_filter->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
      m_right.bt_filter->set_toggle_mode(true);
      hbox->add_child(m_right.bt_filter);

      m_right.bt_addcol = memnew(godot::Button);
      m_right.bt_addcol->set_text("Add column");
      m_right.bt_addcol->set_disabled(true);
      hbox->add_child(m_right.bt_addcol);
      
      m_right.bt_addrow = memnew(godot::Button);
      m_right.bt_addrow->set_text("Add row");
      m_right.bt_addrow->set_disabled(true);
      hbox->add_child(m_right.bt_addrow);

//      m_right.mbt_tpatch = memnew(godot::MenuButton);
//      m_right.mbt_tpatch->set_text("Patch");
//      m_right.mbt_tpatch->set_flat(false);
//      m_right.mbt_tpatch->get_popup()->add_item("Create Patch...", PMID_Create);
//      m_right.mbt_tpatch->get_popup()->add_item("Load Patch...", PMID_Open);
//      m_right.mbt_tpatch->get_popup()->add_item("Close Patch", PMID_Close);
//      hbox->add_child(m_right.mbt_tpatch);

      m_right.filter_box = memnew(godot::VBoxContainer);
      m_right.filter_box->hide();
      m_right.filter_box->set_h_size_flags(SIZE_EXPAND_FILL);
      vbox->add_child(m_right.filter_box);

      godot::HBoxContainer* filterhb = memnew(godot::HBoxContainer);
      filterhb->set_h_size_flags(SIZE_EXPAND_FILL);
      m_right.filter_box->add_child(filterhb);

      m_right.txt_filter = memnew(godot::LineEdit);
      m_right.txt_filter->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_right.txt_filter->set_placeholder("Filter values...");
      m_right.txt_filter->set_clear_button_enabled(true);
      filterhb->add_child(m_right.txt_filter);

      m_right.chk_case_sensitive = memnew(godot::CheckBox);
      m_right.chk_case_sensitive->set_text("Case sensitive");
      filterhb->add_child(m_right.chk_case_sensitive);

      m_right.chk_regex = memnew(godot::CheckBox);
      m_right.chk_regex->set_text("RegEx");
      filterhb->add_child(m_right.chk_regex);

      m_right.hf_column = memnew(godot::HFlowContainer);
      m_right.hf_column->set_h_size_flags(SIZE_EXPAND_FILL);
      m_right.filter_box->add_child(m_right.hf_column);

      m_right.tabular = memnew(TabularBox);
      m_right.tabular->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_right.tabular->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
      m_right.tabular->set_autosave_source(false);
      m_right.tabular->set_auto_handle_remove_row(false);
      m_right.tabular->set_auto_handle_remove_column(false);
      m_right.tabular->set_auto_handle_col_insertion(false);
      m_right.tabular->set_auto_handle_col_rename(false);
      m_right.tabular->set_auto_handle_col_move(false);
      m_right.tabular->set_auto_handle_col_type_change(false);
      m_right.tabular->set_auto_handle_row_insertion(false);
      m_right.tabular->set_auto_handle_row_move(false);
      m_right.tabular->set_auto_handle_row_sort(false);
      m_right.tabular->set_title_horizontal_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
      m_right.tabular->set_theme(m_tbox_theme);
      vbox->add_child(m_right.tabular);
   }

   // Some of the buttons are meant to be square. This is the attempt to make them so
   {
      const godot::Vector2 btminsz = m_left.bt_add->get_combined_minimum_size();
      const int btsz = godot::Math::max<int>(btminsz.x, btminsz.y);
      const godot::Size2i btsize(btsz, btsz);
      
      m_left.bt_filter->set_custom_minimum_size(btsize);
      m_left.bt_add->set_custom_minimum_size(btsize);
      m_right.bt_filter->set_custom_minimum_size(btsize);
   }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBEditorPlugin
void DBEditorPlugin::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_ENTER_TREE:
      {
         //m_left.bt_filter->connect("toggled", callable_mp((godot::CanvasItem*)m_left.txt_filter, &godot::LineEdit::set_visible));
         m_editor = memnew(GDDatabaseEditor);
         m_editor->set_anchors_and_offsets_preset(godot::Control::LayoutPreset::PRESET_FULL_RECT);
         m_editor->set_v_size_flags(godot::Control::SIZE_EXPAND_FILL);
         m_editor->set_h_size_flags(godot::Control::SIZE_EXPAND_FILL);

         get_editor_interface()->get_editor_main_screen()->add_child(m_editor);

         _make_visible(false);

         m_editor->set_layout_saver(callable_mp((godot::EditorPlugin*)this, &godot::EditorPlugin::queue_save_layout));
      } break;

      case NOTIFICATION_EXIT_TREE:
      {
         if (m_editor)
         {
            m_editor->queue_free();
            m_editor = nullptr;
         }
      } break;
   }
}


bool DBEditorPlugin::_has_main_screen() const
{
   return true;
}


bool DBEditorPlugin::_handles(Object* obj) const
{
   const GDDatabase* database = godot::Object::cast_to<GDDatabase>(obj);
   return (database != nullptr);
}


void DBEditorPlugin::_edit(Object* obj)
{
   m_editor->edit(godot::Ref<GDDatabase>(obj));
}


void DBEditorPlugin::_make_visible(bool visible)
{
   if (m_editor)
   {
      m_editor->set_visible(visible);
   }
}


godot::String DBEditorPlugin::_get_plugin_name() const
{
   return "Database";
}


godot::Ref<godot::Texture2D> DBEditorPlugin::_get_plugin_icon() const
{
   return m_db_icon;
}


void DBEditorPlugin::_set_window_layout(const godot::Ref<godot::ConfigFile>& configuration)
{
   if (m_editor)
   {
      m_editor->set_window_layout(configuration);
   }
}


void DBEditorPlugin::_get_window_layout(const godot::Ref<godot::ConfigFile>& configuration)
{
   if (m_editor)
   {
      m_editor->get_window_layout(configuration);
   }
}



DBEditorPlugin::DBEditorPlugin()
{
   m_db_icon = Internals::texture_from_base64(ICON_DATABASE);
   m_editor = nullptr;
}

#endif  //EDITOR_DISABLED  
#endif  //DATABASE_DISABLED
