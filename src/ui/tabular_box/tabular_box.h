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

#ifndef _kehui_tabular_box_h_included
#define _kehui_tabular_box_h_included 1

#ifndef TABULAR_BOX_DISABLED

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

#include "data_source.h"
#include "tabular_header.h"
#include "tbox_theme_cache.h"


// Forward declares
class TabularBoxCell;
class TabularBoxCellRowNumber;

namespace godot
{
   class FileDialog;
   class HBoxContainer;
   class HScrollBar;
   class InputEventMouseButton;
   class InputEventMouseMotion;
   class MenuButton;
   class PopupMenu;
   class Script;
   class VBoxContainer;
   class VScrollBar;
}


/// TODO: Means for DataSource to add extra context menu options, in a "per column" basis (in other words, based on value type)
/// TODO: Try to squeeze a little bit more space within column headers to incorporate a "sort" button
class TabularBox : public godot::Control
{
   GDCLASS(TabularBox, godot::Control);
public:
   enum NextCellMode
   {
      NCM_Disabled,       // No automatic "edit next cell"
      NCM_Row,            // Editing "next cell" will be the one in the row bellow
      NCM_Column,         // Editing "next cell" will be the one on the next column
   };
private:
   // Although the boolean settings will be stored as a single integer through several bit flags,
   // the masks are not needed for public access. Thus those are here in the private section
   enum FlagSettings
   {
//      AutoEditNextRow =            1,        // If enabled, committing a cell change will move the cursor (focus) to the next row
      AutoHandleRemoveRow =        1 <<  1,  // If not set then remove_row_request signal will be emitted when attempting to remove a row
      AutoHandleRemoveColumn =     1 <<  2,  // If not set then remove_column_request signal will be emitted when attempting to remove a column
      ShowRowNumbers =             1 <<  3,  // Display row numbers
      ShowCheckboxes =             1 <<  4,  // Display row checkboxes to allow multiple selection
      AutosaveSource =             1 <<  5,  // If set and the provided data source is a file, then it will be automatically saved when modified
      AutoHandleColumnInsertion =  1 <<  6,  // If not set then the insert_column_request signal will be emitted when selecting "insert column" on UI
      AutoHandleRowInsertion =     1 <<  7,  // If not set then the insert_row_request signal will be emitted when selecting "insert row" on UI
      AutoHandleColumnRename =     1 <<  8,  // If not set then column_rename_request will be emitted when attempting to change a column title
      AutoHandleColumnMove =       1 <<  9,  // If not set then column_move_requested will be emitted when attempting to reorder a column
      AutoHandleColumnTypeChange = 1 << 10,  // If not set then column_type_change_requested will be emitted when attempting to change a column value type
      AutoHandleRowMove =          1 << 11,  // If not set then row_move_request will be emitted when attempting to reorder a row
      AutoHandleRowSort =          1 << 12,  // If not set the nrow_sort_request will be emitted when attempting to sort rows by a column
      AllowContextMenu =           1 << 13,  // If not set then right clicking will not display a context menu
      AutoHideVScrollBar =         1 << 14,  // If not set then vertical scrollbar will always be visible
      AutoHideHScrollBar =         1 << 15,  // If not set then horizontal scrollbar will always be visible

      FSDefault = AutoHandleRemoveRow | AutoHandleRemoveColumn | ShowRowNumbers | ShowCheckboxes | AutosaveSource | AutoHandleColumnInsertion |
            AutoHandleRowInsertion | AutoHandleColumnRename | AutoHandleColumnMove | AutoHandleColumnTypeChange | AutoHandleRowMove | AutoHandleRowSort | AllowContextMenu |
            AutoHideVScrollBar | AutoHideHScrollBar
   };

   // The context menu (shown when right clicking) will display several items. The next enum is meant
   // to provide the IDs to those menu entries. No need to expose them.
   enum ContextMenuEntry
   {
      AppendColumn, RemoveColumn, InsertColumnBefore, InsertColumnAfter, MoveColumnLeft, MoveColumnRight,
      SortAscending, SortDescending,
      AppendRow, RemoveRow, InsertRowAbove, InsertRowBellow, MoveRowUp, MoveRowDown,
      ToggleSelected
   };

   enum RowMenuID
   {
      SelectAll, DeselectAll, InvertSelection, DeleteSelected,
   };

   struct RowData
   {
      // Hold cells matching column order
      godot::Vector<TabularBoxCell*> cell;

      // Map from column title into the cell of this row
      godot::HashMap<godot::String, TabularBoxCell*> cell_map;

      // This is the UI Control holding instances of TabularBoxCell
      godot::HBoxContainer* box;

      // Cache the minimum height here. By doing so, there is another benefit. When the row's minimum height
      // changes it become possible to calculate the "delta" and perform internal updates without having to
      // iterate through every single row
      int32_t height;

      RowData() : box(nullptr), height(0) {}
   };

   // Used to determine minimum column header width
   static const int32_t MIN_TITLE_CHARS = 2;

   /// Internal variables
   TBoxThemeCache m_theme_cache;

   // Indeed internal variable. Although this is indirectly exposed through several functions. Well, individual
   // bits of this variable are exposed
   uint32_t m_setting_flags;

   // When right clicking, display a context menu. This holds it
   godot::PopupMenu* m_context_menu;

   // A submenu will show supported values types. This holds the submenu
   godot::PopupMenu* m_value_type_menu;

   // Cache minimum sizes
   struct
   {
      // Minimum size just for the column title
      godot::Size2i title_size;

      // Minimum size for the column move buttons
      godot::Size2i title_buttons;

      // Minimum size required for the row_number margins
      godot::Size2i row_num_margins;

      // Minimum size required to render row number
      godot::Size2i row_number;
      
      // Minimum size required to render row checkbox
      godot::Size2i row_checkbox;

      // The final row number size. The width takes checkbox and number display into account. Height considers the maximum value
      // between checkbox and number string. On both cases margins are taken into account
      godot::Size2i final_rownum;
   } m_min_size;

   // Row number area
   struct
   {
      godot::Vector<TabularBoxCellRowNumber*> array;

      // Holds a MenuButton - while also being used to draw the Header style box
      godot::Control* menu_area;

      // This button will provide a popup menu specific for dealing with row selection
      godot::MenuButton* bt_menu;

      // The box that will hold each "cell" meant to display row number/checkbox
      godot::VBoxContainer* box;

      // This will "clip" contents of the box.
      godot::Control* area;

      // Horizontal alignment of the drawn numbers within the "cells"
      godot::HorizontalAlignment align;
   } m_rownum_data;

   struct
   {
      // While the TabularBoxHeader instances can be retrieved from the 'box' bellow, this array makes
      // iteration easier - that is, no need to cast every single object retrieved from box->get_child()
      godot::Vector<TabularBoxHeader*> array;

      // And this map helps query a specific header by its column title
      godot::HashMap<godot::String, TabularBoxHeader*> map;

      // Instances of TabularBoxHeader will be added into this box
      godot::HBoxContainer* box;

      // This will hold the HBoxContainer that is declared above. Its purpose is to help clip column headers when
      // dealing with scrolling. Without this things become rather difficult to deal with when theme styling uses
      // transparency.
      godot::Control* area;


      // This flag is set whenever "manually" resizing a header through code. This is meant to prevent emitting the
      // column resized signal
      bool manually_resizing;
   } m_header_data;

   // Cell area
   struct
   {
      // This should match row order
      godot::Vector<RowData> row;

      // A row is an instance of HBoxContainer holding instances of TabularBoxCell. Place each HBox's row into this
      // VBoxContainer.
      godot::VBoxContainer* box;

      // This holds the VBoxContainer and serves as a "clipper" for the cells. Without this scrolling becomes rather
      // difficult to deal with when theme styling uses transparency.
      godot::Control* area;

      // When mouse moves over a row, this will be set
      godot::HBoxContainer* row_under_mouse;

      // Cache total size required for the cells
      godot::Size2i total_size;

      // The 'available height' is necessary in order to properly limit the column sizers' height.
      int32_t available_height;
   } m_cell_data;

   // Keep track of selected cell. For the upkeep to correctly work, information regarding which column and row must
   // be stored. This also provides means to implement the "auto edit next row" feature
   struct 
   {
      // While it's relatively easy to obtain the cell from its column and row indices, also holding a direct pointer to
      // it makes things even easier to deal with.
      TabularBoxCell* cell;
   } m_selected;


   // If a specific cell requires FileDialog, uses this one instead of adding one for each cell instance or for each column
   struct
   {
      godot::FileDialog* dialog;
      godot::Callable selected_handler;
   } m_file_dialog;

   // A cell implementation might need additional Controls to be added (and shared between the other cells of the same type).
   // For that a "shared" system is used. Yet, those Controls require a common parent Control node. This is it and will be
   // provided to the cell through the "share_with()" AND "set_extra_parent()". In other words, both functions will be called
   godot::Control* m_extra_parent;


   /// Exposed variables
   godot::Ref<TabularDataSource> m_data_source;

   // Determine the horizontal alignment of the column header titles
   godot::HorizontalAlignment m_title_halign;

   // Scroll bars
   godot::HScrollBar* m_hor_bar;
   godot::VScrollBar* m_ver_bar;

   // Determine the behavior of "edit next cell" after committing the value
   NextCellMode m_next_cell_mode;



   /// Internal functions
   // Scan script classes and fill m_cell_script with those that inherit from TabularBoxCell
   void scan_scripted_cells();

   // Save the data within the data source file
   void save_data_source();

   // Based on the styling, calculate (and cache) some minimum sizes that will be applied into add children
   void calculate_min_sizes();

   // This is meant to calculate the total size of the children, which should be used to determine scrolling and
   // how column width sizers will be setup. This will use the data calculated (and saved) within the calculate_min_sizes()
   void calculate_sizes();

   // Calculating the available height must be done on certain situations separate from the 'calculate_sizes()'. Or some other
   // functions. As an example, when horizontal scrollbar visibility changes, there is no need to recalculate sizes but the
   // available height must be recalculated otherwise the column sizers will have incorrect heights.
   void calculate_available_height();

   // The width of the rownum "cells" might change based on exposed variables (properties, whatever). Instead of recalculating
   // all internal sizes, use this function just for the width.
   void calculate_rownum_width();

   // Make children consistent in their visuals
   void apply_styling();

   // Deal with the scroll bars - show/hide if necessary as well as deal with the offset caused by the scrolling
   // This returns true if the visibility of one of the bars has changed
   bool check_scroll_bars();

   // Properly "distribute" children
   void apply_layout();


   // Request to bring the FileDialog (create it if it's not created yet). This is the "internal function" that performs some
   // extra setup that will help differentiate the call done from external TabularBox code or from an owned cell
   void internal_request_fdialog(const godot::String& title, const godot::PackedStringArray& filters, const godot::Callable& handler, const godot::String& dir);

   

   // Update odd/even state of rows. Not relying on child index because filtering might hide rows
   void set_rows_odd_even(bool request_redraw);

   // Ensure column header is properly set. Mostly their sizing
   void setup_header(TabularBoxHeader* header);

   // Create a TabularBoxHeader instance to represent a column within the UI
   void create_header(const TabularDataSource::ColumnInfo& info, int64_t index);

   // Remove a TabularBoxHeader from the UI
   void remove_header(int64_t index);

   // Create a TabularBoxCell instance based on the editor type specified by the assigned data source.
   void create_cell(const TabularDataSource::ColumnInfo& info, const godot::Variant& value, RowData& row, int64_t column_index);

   // Create a full row UI representation
   void create_row(const godot::Dictionary& values, int64_t index);

   // Remove a full row UI representation
   void remove_row(int64_t index);

   // Makes the incoming TabularBoxCell as the currently selected one. If null then ensure no cell is selected.
   void select_cell(TabularBoxCell* cell);


   // Remove all columns and reset everything to initial state
   void clear();


   // This function is used connect or disconnect event handlers into/from the assigned data source. Handling those events allows
   // the TabularBox to keep updated
   void check_ds_signals(bool disconnect);

   // Goes through all rows and make sure the "rownumber" selected box is in sync with the data source
   void check_row_selection();

   // Find the column that is currently under the mouse cursor and return its index. -1 if there is no column
   int64_t get_column_under_mouse(const godot::Vector2& mouse_pos) const;



   // Handles right click - build context menu and all
   void handle_right_click(int64_t column_index, int64_t row_index);

   // Handles mouse button events
   void handle_mouse_button(const godot::Ref<godot::InputEventMouseButton>& evt);

   // Handles mouse motion events
   //void handle_mouse_motion(const godot::Ref<godot::InputEventMouseMotion>& evt);


   /// Event handlers
   // Handles the even given when the file dialog is closed without selecting a file (cancelling that is). This must disconnect
   // the one shot event handler if there is one
   void on_file_dialog_closed();

   // Handles the changes in the scroll bars
   void on_scroll_value_changed(float value);

   // Handles the "menu selection" event
   void on_context_menu_id_selected(int32_t id);

   // And the value type menu selection event
   void on_value_type_id_selected(int32_t id);

   // Column header has been resized. Listen to this event so cells of that column can match the new width
   void on_header_resized(TabularBoxHeader* header);

   // The headers can have move_left and move_right buttons, allowing to reorder columns. The next two functions
   // handle the clicked events of those two buttons, move left and move right respectively
   void on_header_move_left(TabularBoxHeader* header);
   void on_header_move_right(TabularBoxHeader* header);

   void on_mouse_enter_row(godot::HBoxContainer* row);
   void on_mouse_leave_row(godot::HBoxContainer* row);

   // When requesting to move a row up this will be called
   void on_move_up(int64_t which_row);

   // Same thing when attempting to move row down
   void on_move_down(int64_t which_row);


   // The main row Control is an instance of HBoxContainer. When its minimum size is changed, an event is emitted
   // This function handles that event and it should update sizing data.
   void on_row_min_size_changed(godot::HBoxContainer* row);

   // The RowNumber "cell" sends an event whenever its selected state is changed through input. This handles said event
   // and update the data source to reflect the change
   void on_row_selected_changed(int64_t row_index, bool selected);


   // The column header will emit a signal whenever there is an attempt to change a column title. This will handle
   // that event.
   void on_header_renamed(const godot::String& new_title, TabularBoxHeader* header);


   // The Cell should send an event whenever the user changes its value. This handles that event
   void on_cell_value_changed(const godot::Variant& new_value, TabularBoxHeader* header, godot::HBoxContainer* row);

   // A Cell can emit an event requesting to select the next one. This function handles that event
   void on_cell_select_next(TabularBoxCell* cell);

   // A cell can request to be unselected through an event. This handles said request
   void on_cell_unselect();


   // This one will directly handle event given by assigned data source - which will deal with column inserted
   void on_column_inserted(int64_t index);

   // This one will directly handle event given by assigned data source - which will deal with column removed
   void on_column_removed(int64_t index);

   // This one will directly handle event given by assigned data source - which will deal with column moved
   void on_column_moved(int64_t from, int64_t to);

   // The column title has been changed within the data source. This handles the event and must update the UI
   void on_column_title_changed(int64_t index);

   // The column title change has been rejected. This handles the event and must update the UI
   void on_column_title_change_rejected(int64_t index);

   // This one will directly handle event given by assigned data source - which will deal with column value type changed
   void on_type_changed(int64_t column);


   // This one will directly handle event given by assigned data source - which will deal with new row insertion
   void on_row_inserted(int64_t index);

   // This one will directly handle event given by assigned data source - which will deal with removed row
   void on_row_removed(int64_t index);

   // This one will directly handle event given by assigned data source - which will deal with moved row
   void on_row_moved(int64_t from, int64_t to);

   // This is triggered by the data source and if here either the column does not require external dealing with the
   // new value or it was already confirmed
   void on_value_changed(int64_t column, int64_t row, const godot::Variant& value);

   // If a value change has been rejected...
   void on_value_change_rejected(int64_t column, int64_t row);

   // This one will directly handle event given by assigned data source - which will deal with row sorting
   void on_sorted(int64_t column);

   // Primarily meant to draw the header style box - but for the "area" holding the row_number menu button
   // If such style is meant to be drawn by another Control, the function is already here
   void on_draw_header(godot::Control* ctrl);

   // When the row menu is about to popup, this will handle the event that is given.
   void on_row_menu_popup();

   // Handles the event given when selecting an item from the 'row menu'
   void on_rownum_id_selected(int32_t id);


   // If a filter is applied (or removed) into the data source, an event will be triggered. This function handles
   // it. In here the UI rows should reflect the filter state
   void on_filter_changed();
   

   // Used as "draw" event handler during development to help debug positioning of the internal controls.
   void on_debug_draw(godot::Control* ctrl, const godot::Color& color);


protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* out_list) const;
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   godot::Vector2 _get_minimum_size() const { return godot::Vector2(120, 85); }       // "override"

   virtual void _gui_input(const godot::Ref<godot::InputEvent>& evt) override;

   virtual void _unhandled_input(const godot::Ref<godot::InputEvent>& evt) override;

   /// Exposed virtual functions

   /// Exposed functions
   void set_column_width(const godot::String& title, int32_t width);
   int32_t get_column_width(const godot::String& title) const;

   // Well, still debating if this should be exposed or not. While not done within the _bind_methods(), keeping it here
   // until it's decided
   void unselect_all_cells();

   /// Setters/Getters
   void set_data_source(const godot::Ref<TabularDataSource>& data_source);
   godot::Ref<TabularDataSource> get_data_source() const { return m_data_source; }

   godot::HorizontalAlignment get_title_horizontal_alignment() const { return m_title_halign; }
   void set_title_horizontal_alignment(godot::HorizontalAlignment align);

   bool get_allow_context_menu() const { return Internals::is_flag_set(AllowContextMenu, m_setting_flags); }
   void set_allow_context_menu(bool enabled) { Internals::set_flag(enabled, AllowContextMenu, m_setting_flags); }

   NextCellMode get_next_cell_mode() const { return m_next_cell_mode; }
   void set_next_cell_mode(NextCellMode mode) { m_next_cell_mode = mode; }

   bool get_auto_handle_remove_row() const { return Internals::is_flag_set(AutoHandleRemoveRow, m_setting_flags); }
   void set_auto_handle_remove_row(bool enabled) { Internals::set_flag(enabled, AutoHandleRemoveRow, m_setting_flags); }

   bool get_auto_handle_remove_column() const { return Internals::is_flag_set(AutoHandleRemoveColumn, m_setting_flags); }
   void set_auto_handle_remove_column(bool enabled) { Internals::set_flag(enabled, AutoHandleRemoveColumn, m_setting_flags); }

   bool get_show_row_numbers() const { return Internals::is_flag_set(ShowRowNumbers, m_setting_flags); }
   void set_show_row_numbers(bool enabled);

   godot::HorizontalAlignment get_row_numbers_alignment() const;
   void set_row_numbers_alignment(godot::HorizontalAlignment align);

   bool get_show_row_checkboxes() const { return Internals::is_flag_set(ShowCheckboxes, m_setting_flags); }
   void set_show_row_checkboxes(bool enabled);

   bool get_autosave_source() const { return Internals::is_flag_set(AutosaveSource, m_setting_flags); }
   void set_autosave_source(bool enabled) { Internals::set_flag(enabled, AutosaveSource, m_setting_flags); }

   bool get_auto_handle_col_insertion() const { return Internals::is_flag_set(AutoHandleColumnInsertion, m_setting_flags); }
   void set_auto_handle_col_insertion(bool enabled) { Internals::set_flag(enabled, AutoHandleColumnInsertion, m_setting_flags); }

   bool get_auto_handle_row_insertion() const { return Internals::is_flag_set(AutoHandleRowInsertion, m_setting_flags); }
   void set_auto_handle_row_insertion(bool enabled) { Internals::set_flag(enabled, AutoHandleRowInsertion, m_setting_flags); }

   bool get_auto_handle_col_rename() const { return Internals::is_flag_set(AutoHandleColumnRename, m_setting_flags); }
   void set_auto_handle_col_rename(bool enabled) { Internals::set_flag(enabled, AutoHandleColumnRename, m_setting_flags); }

   bool get_auto_handle_col_move() const { return Internals::is_flag_set(AutoHandleColumnMove, m_setting_flags); }
   void set_auto_handle_col_move(bool enabled) { Internals::set_flag(enabled, AutoHandleColumnMove, m_setting_flags); }

   bool get_auto_handle_col_type_change() const { return Internals::is_flag_set(AutoHandleColumnTypeChange, m_setting_flags); }
   void set_auto_handle_col_type_change(bool enabled) { Internals::set_flag(enabled, AutoHandleColumnTypeChange, m_setting_flags); }

   bool get_auto_handle_row_move() const { return Internals::is_flag_set(AutoHandleRowMove, m_setting_flags); }
   void set_auto_handle_row_move(bool enabled) { Internals::set_flag(enabled, AutoHandleRowMove, m_setting_flags); }

   bool get_auto_handle_row_sort() const { return Internals::is_flag_set(AutoHandleRowSort, m_setting_flags); }
   void set_auto_handle_row_sort(bool enabled) { Internals::set_flag(enabled, AutoHandleRowSort, m_setting_flags); }

   bool get_autohide_vscroll_bar() const { return Internals::is_flag_set(AutoHideVScrollBar, m_setting_flags); }
   void set_autohide_vscroll_bar(bool enable);

   bool get_autohide_hscroll_bar() const { return Internals::is_flag_set(AutoHideHScrollBar, m_setting_flags); }
   void set_autohide_hscroll_bar(bool enable);

   /// Public non exposed functions
   TabularBox();
};

VARIANT_ENUM_CAST(TabularBox::NextCellMode);



#endif     // TABULAR_BOX_DISABLED

#endif     // _kehui_tabular_box_h_included
