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

#ifndef _kehother_database_editor_dbplugin_h_included
#define _kehother_database_editor_dbplugin_h_included 1

#ifndef DATABASE_DISABLED
#ifndef EDITOR_DISABLED

#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

#include <godot_cpp/templates/hash_map.hpp>

#include "dbdatasource.h"

namespace godot
{
   class AcceptDialog;
   class Button;
   class CheckBox;
   class ConfirmationDialog;
   class FileDialog;
   class HBoxContainer;
   class HFlowContainer;
   class HSplitContainer;
   class InputEvent;
   class Label;
   class LineEdit;
   class MenuButton;
   class OptionButton;
   class Panel;
   class PanelContainer;
   class RichTextLabel;
   class SpinBox;
   class StyleBox;
   class Texture2D;
   class Window;
}


class DBTable;
class FileDirPicker;
class GDDatabase;
class TabularBox;


/// Well, the original intention was to make this a struct, thus all variables are in the public section.
/// Nevertheless, making this as a RefCounted object just so instances can be bound to Callable objects.
/// The alternative would be to derive from CallableCustom.
class DBETableEntry : public godot::RefCounted
{
   GDCLASS(DBETableEntry, godot::RefCounted);
private:
   /// Internal variables

   /// Exposed variables

   /// Internal functions

   /// Event handlers

protected:
   static void _bind_methods() {}
public:
   godot::Ref<DBEditorDataSource> data_source;
   godot::Ref<DBTable> db_table;

   godot::PanelContainer* ui_table;
   godot::Button* bt_table_name;

   // In the GDDatabaseEditor there is "master Dictionary" used to leverage Godot's layout save/load system. In its declaration
   // there is explanation of what is stuffed into it. This Dictionary is the "inner most" one
   godot::Dictionary layout;
   
   /// Overrides

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   DBETableEntry() : ui_table(nullptr), bt_table_name(nullptr) {}
};


// In here a work around is required in order to request the Editor to trigger layout saving. In the core code it's done
// by using EditorNode::get_singleton()->save_editor_layout_delayed(). Unfortunately EditorNode is not exposed to scripting
// nor GDExtension. So this class uses a Callable that will have the owning EditorPlugin's function assigned to it. More specifically,
// the "queue_save_layout()", which will then trigger the desired layout saving. The Callable will be set by the plugin when
// instancing this class.
class GDDatabaseEditor : public godot::VBoxContainer
{
   GDCLASS(GDDatabaseEditor, godot::VBoxContainer);
private:
   // When the confirmation dialog opens, a "code" will be set to indicate which action is being confirmed
   // This allows the reuse of a single ConfirmationDialog instance. The "action code" is defined by this enum
   enum ConfirmCode
   {
      CC_None,
      CC_RemoveTable,

      CC_RemoveColumn,
      CC_RemoveRow,
   };

   enum PatchMenuID
   {
      PMID_Create,
      PMID_Open,
      PMID_Close,
   };

   enum FileDialogType
   {
      FDT_None,         // Just to initialize internal data
      FDT_LoadDB,
      FDT_SaveDB,
      FDT_LoadPatch,
      FDT_SavePatch,
   };

   // This uses one Window to display several types of dialogs instead of creating one Window for each dialog type.
   // To make things easier, a function is used to display the dialog with the relevant settings. An instance of this
   // struct is used to provide such settings.
   struct DialogSettings
   {
      godot::VBoxContainer* contents;
      godot::String title;
      bool resizable;
      godot::Size2i dlg_size;
      godot::String ok_text;      // If not provided button will be hidden
      godot::String cancel_text;  // If not provided "Cancel" will be used

      DialogSettings() :
         contents(nullptr),
         resizable(false),
         cancel_text("Cancel")
      {}
   };

   /// Internal variables
   // This will be applied to a few labels in the UI
   godot::Ref<godot::StyleBoxFlat> m_lbl_style;

   // Use several structs to help group internal variable/controls/things based on purpose - and section within the UI
   struct
   {
      // The "top bar" should contain:
      // - A button that once pressed displays a dialog containing information about the edited Database
      // - A label that shows the full path to the edited database
      // - Button to open a database
      // - Button to create a database
      // - Button to bring export dialog
      godot::Button* bt_dbinfo;
      godot::Label* lbl_dbfile;
      godot::Button* bt_opendb;
      godot::Button* bt_createdb;
      godot::Button* bt_export;
   } m_top_bar;

   godot::HSplitContainer* m_hsplit;

   struct
   {
      // The left section of the "main area" is meant to display tables in the database and should contain:
      // - A button to toggle the filter area (this filter is specific to table list)
      // - A button to pop the dialog to create a new table
      // - Text input to allow filtering list of tables
      // - A container to hold the list of tables - which are instances of DBTableEntry (declared above this class)
      godot::Button* bt_filter;
      godot::Button* bt_add;
      godot::LineEdit* txt_filter;
      godot::VBoxContainer* vbox;
   } m_left;

   struct
   {
      // The right section of the "main area" is meant to display the data corresponding to the selected table and should contain:
      // - A label to display the name of the currently selected table
      // - Button to toggle row filtering area
      // - Button to pop dialog to add new column into the table
      // - Button to add new row into the table
      // - Menu button allowing to create, load or close a table patch.
      // - Container to hold the filtering area
      // - Text input to be used as the row filter criteria
      // - Checkbox indicating if the filter should be case sensitive or not
      // - Checkbox that if checked means the typed value should be used as a RegEx expression
      // - HFlowContainer to hold a list of checkboxes, each one corresponding to a column of the selected table. Those are meant to allow
      //   enabling/disabling certain columns to be used during the filtering
      // - TabularBox to display and allow editing the data within the selected table
      godot::Label* lbl_tablename;
      godot::Button* bt_filter;
      godot::Button* bt_addcol;
      godot::Button* bt_addrow;
      godot::MenuButton* mbt_tpatch;
      godot::VBoxContainer* filter_box;
      godot::LineEdit* txt_filter;
      godot::CheckBox* chk_case_sensitive;
      godot::CheckBox* chk_regex;

      godot::HFlowContainer* hf_column;
      TabularBox* tabular;

      // From column name into its corresponding checkbox. Using a hashmap so table renaming upkeep becomes very simple
      godot::HashMap<godot::String, godot::CheckBox*> column_checkbox;

      // Cache the list of "excluded columns" to be applied when filtering rows
      godot::PackedStringArray filter_exclude;
   } m_right;


   // This style will be applied to root of each table entry (which is a PanelContainer)
   godot::Ref<godot::StyleBox> m_entry_style_normal;

   // Cache the "focus style". This will be used to highlight the selected table entry
   godot::Ref<godot::StyleBox> m_entry_style_selected;

   // Key is table name
   godot::HashMap<godot::StringName, godot::Ref<DBETableEntry>> m_table_entry;

   // This is the edited Database
   godot::Ref<GDDatabase> m_db;

   // Cache here which table is selected - obviously if there is one
   godot::Ref<DBETableEntry> m_selected_entry;

   // Cache which table entry has the mouse cursor over it - obviously if there is any under the mouse
   godot::Ref<DBETableEntry> m_entry_under_mouse;

   // This should be set before displaying the confirmation dialog. This allows for a single instance to be used
   // as well as a single event handler function
   ConfirmCode m_confirm_code;

   // This will be applied to the TabularBox and is meant primarily to make its visual consistent with Editor styling
   godot::Ref<godot::Theme> m_tbox_theme;

   // At least two buttons will use the "magnifying glass" icon. So hold the texture here
   godot::Ref<godot::Texture2D> m_tex_magnify;


   // Instead of creating custom dialogs for the various requirements of this Editor, hold the contents within
   // specific Structs. Then when a dialog is required, take the contents and stuff them into an instance of the
   // godot::Window. This one is specific to display simple messages, such as errors.
   struct
   {
      godot::VBoxContainer* vbox;
      godot::Label* label;
      godot::Variant data;
   } m_dialog_message;

   // This one will display detailed information related to the edited database, such as list of tables as well as
   // column and row counts for each table. Another information shown is the relational aspect of existing tables,
   // that is, which tables are referencing others.
   struct
   {
      godot::VBoxContainer* vbox;
      godot::RichTextLabel* info;
   } m_dialog_dbinfo;

   // Dialog used to create a new table within the edited database
   struct
   {
      godot::VBoxContainer* vbox;
      
      // Line: table name
      godot::LineEdit* txt_tablename;

      // Line: File name
      godot::LineEdit* txt_filename;
      godot::CheckBox* chk_embed;

      // Line: ID Type
      godot::CheckBox* chk_integer;
      godot::CheckBox* chk_string;
      godot::CheckBox* chk_editable;
   } m_dialog_newtable;

   // Allow renaming a table - this dialog is used primarily to "block" names that can't be used, such as a name that is already used
   struct
   {
      godot::VBoxContainer* vbox;
      godot::Label* lbl_info;     // Display name of the table to be renamed
      godot::LineEdit* txt_newname;
   } m_dialog_tablerename;

   // And allow creating a new column in the selected table of the edited database. This dialog provides means to specify which value
   // type should be used. And if the type is a "foreign key", then allow choosing from which other table to take values from.
   struct
   {
      godot::VBoxContainer* vbox;

      // Line(s): Column name
      godot::LineEdit* txt_colname;
      godot::Label* lbl_colname_err;     // Help telling why column name is not valid

      // Line(s): Value Type
      godot::OptionButton* opt_type;
      godot::Label* lbl_vtype_warn;      // Warning when value type can't be changed after column is created

      // Line: External ID
      godot::OptionButton* opt_other_table;

      // At which index should the column be inserted
      int64_t at_index;
   } m_dialog_addcol;

   // Dialog meant to provide export options. Monolithic vs one file per table. A directory where the file(s) will be saved. And the
   // indent used in the final JSON
   struct
   {
      godot::VBoxContainer* vbox;

      // Line: checkbox to enable monolithic export (should be on by default)
      godot::CheckBox* chk_monolithic;

      // Line: output directory
      FileDirPicker* fd_picker;

      // Line: Indent, Tab vs Space (with number of spaces)
      godot::OptionButton* mnu_indent_type;
      godot::SpinBox* spin_amount;
   } m_dialog_export;

   // This is the base of the dialog window itself
   struct DialogMain
   {
      godot::Window* dialog;

      godot::Button* btok;
      godot::Button* btcancel;

      // Hold some state - which should help deal with upkeep
      godot::VBoxContainer* current_visible;

      DialogMain() :
         dialog(nullptr),
         btok(nullptr),
         btcancel(nullptr),
         current_visible(nullptr) {}

   } m_dialog_main;

   godot::FileDialog* m_dlg_openclose;
   FileDialogType m_fdlg_type;

   /// Holds data related to the layout, save/load and cache some things to make upkeep easier
   struct
   {
      // Flag to help prevent infinite loops when restoring layout
      bool restoring;

      // There is an explanation for the reason this callable exists at the declaration of the class
      godot::Callable saver;

      // This is the "master" Dictionary that holds all the layout data that must be associated with a
      // specific database. The reason to use Dictionary instead of a strong typed map container is that
      // it's supported out of the box by the layout saving/loading mechanism in Godot.
      // All that said, this outer dictionary has String as key (database path), and will hold inner
      // dictionaries, one for each database that has been edited by the editor in the current project.
      // This first "level" inner dictionary contains:
      // - last_table: StringName indicating the name of the last selected table of this DB
      // - ntable: Dictionary containing information to restore settings in the "new table" dialog
      // - table: Dictionary containing all the layout data for each existing table in the database, where
      //   each key is a table name
      // The "table" most inner dictionary is described bellow (this is cached in the DBETableEntry):
      // - width: Dictionary where each entry is key = column name, value = width of that column
      // - fexclude: Array holding list of columns that should be excluded in the row filtering of the table
      godot::Dictionary master_data;

      // When a database is "edited", cache its Dictionary entry in here to make updating easier
      // Remember that this Dictionary is saved into "master_data" keyed by the path to the database
      // This dictionary also holds several other Dictionaries keyed by "table". Those are cached inside the
      // corresponding DBETableEntry instance
      godot::Dictionary db_layout;
   } m_layout;




   /// Exposed variables

   /// Internal functions
   void check_style();

   void build_msg_dialog();
   void build_dbinfo_dialog();
   void build_opencreate_dialog();
   void build_newtable_dialog();
   void build_tablerename_dialog();
   void build_addcol_dialog();
   void build_export_dialog();
   void build_main_dialog();

   void create_openclose_dialog();

   void update_ui();

   // This one is meant to create the UI representation of an existing DBTable
   void add_table(const godot::Ref<DBTable>& table, bool select);

   // Sorts the list of tables ascending order, by table name. This does not change any stored data, only the order in which
   // table entries are displayed in the UI
   void sort_table_list();

   // This is meant to create a new DBTable owned by currently edited database. The specifies of the table are
   // expected to be set within the dialog
   void add_new_table();

   // The row filtering offers the option to exclude columns from the search. This is done through a checkbox for each available
   // column in the selected table. This means that the checkboxes must reflect the table selection. Moreover, renaming a table
   // must also update the corresponding checkbox. The next two functions are used to build and delete the checkboxes
   void build_column_checkboxes();
   void clear_column_checkboxes();

   // Renames the selected table into the provided new name
   void rename_selected_table(const godot::String& into);

   // Add a column into the selected table. All of the details of the new column will be retrieved from the AddColumn
   // dialog. In other words, this function expects to be called just after confirming action through said dialog
   void add_column();

   // When a "message dialog" is closed, this function will be called. If it was opened with the intention of
   // confirming an action m_confirm_code should contain which action.
   void confirm_action();

   
   void show_dialog(const DialogSettings& settings);
   void hide_dialog();

   void show_message(const godot::String& msg);

   // When changing fields within the Add Column dialog, call this function to test if the function can be created or not
   bool can_create_column();

   void save_db();

   void on_table_selection(const godot::Ref<DBETableEntry>& entry);


   /// Event handlers
   void draw_table_entry(const godot::Ref<DBETableEntry>& entry);

   void on_dbinfo_clicked();
   void on_opendb_clicked();
   void on_createdb_clicked();
   void on_export_clicked();
   void on_createtable_clicked();
   void on_addcol_clicked();
   void on_insert_row(int64_t at);
   void on_patchmenu_showing();
   void on_patchmenu_selected(int id);
   void on_dialog_ok_clicked();

   void on_table_filtering(const godot::String& txt);

   void on_value_filter_toggled(bool on);

   void on_row_filtering(const godot::String& txt);
   void on_regex_toggled(bool on);
   void on_column_checkbox_toggled(bool on);

   void on_newtable_name_changed(const godot::String& txt);
   void on_newtable_embed_toggled(bool on);

   void on_table_rename_input(const godot::String& txt);

   void on_column_value_type_selected(int index);

   void on_mouse_enter_entry(const godot::Ref<DBETableEntry>& entry);
   void on_mouse_exit_entry(const godot::Ref<DBETableEntry>& entry);
   void on_table_list_input(const godot::Ref<godot::InputEvent>& evt);

   void on_entry_rename_clicked(const godot::Ref<DBETableEntry>& entry);
   void on_entry_remove_clicked(const godot::Ref<DBETableEntry>& entry);

   void on_newcolumn_name_changed(const godot::String& txt);

   void on_export_path_selected(const godot::String& path);
   void on_export_indent_id_selected(int64_t id);

   void on_file_selected(const godot::String& path);

   void on_dialog_input(const godot::Ref<godot::InputEvent>& event);

   // The internal TabularBox is set to not automatically deal with any changes. When this is the case, several events are emitted.
   // Bellow are the handlers for such events when exclusively doing that. Some functions are handling multiple events and are
   // declared above
   void on_insert_column_requested(int64_t at_index);
   void on_column_rename_requested(int64_t column_index, const godot::String& title);
   void on_column_remove_requested(int64_t column_index);
   void on_column_move_requested(int64_t from, int64_t to);
   void on_column_type_change_requested(int64_t column_index, int to_type);
   void on_column_resized(const godot::String& title, int width);
   void on_value_change_requested(int64_t column_index, int64_t row_index, const godot::Variant& value);
   void on_row_remove_requested(const godot::Array& index_list);
   void on_row_move_requested(int64_t from, int64_t to);
   void on_row_sort_requested(int64_t column_index, bool ascending);

   void debug_draw(godot::Control* ctrl, const godot::Color& cl);

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual bool _can_drop_data(const godot::Vector2& at, const godot::Variant& data) const override;
   virtual void _drop_data(const godot::Vector2& at, const godot::Variant& data) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void edit(const godot::Ref<GDDatabase>& db);

   void set_window_layout(const godot::Ref<godot::ConfigFile>& layout);
   void get_window_layout(const godot::Ref<godot::ConfigFile>& layout);

   void set_layout_saver(const godot::Callable& func) { m_layout.saver = func; }

   GDDatabaseEditor();
};




class DBEditorPlugin : public godot::EditorPlugin
{
   GDCLASS(DBEditorPlugin, godot::EditorPlugin);
private:
   /// Internal variables
   // This will hold the Database Icon, which will be shown in the editor
   godot::Ref<godot::Texture2D> m_db_icon;

   GDDatabaseEditor* m_editor;

   /// Exposed variables

   /// Internal functions

   /// Event handlers


protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual bool _has_main_screen() const override;
   virtual bool _handles(Object* obj) const override;
   virtual void _edit(Object* obj) override;
   virtual void _make_visible(bool visible) override;
   virtual godot::String _get_plugin_name() const override;
   virtual godot::Ref<godot::Texture2D> _get_plugin_icon() const;
   virtual void _set_window_layout(const godot::Ref<godot::ConfigFile>& configuration) override;
	virtual void _get_window_layout(const godot::Ref<godot::ConfigFile>& configuration) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   DBEditorPlugin();
};



#endif  //EDITOR_DISABLED
#endif  //DATABASE_DISABLED


#endif   // _kehother_database_editor_dbplugin_h_included
