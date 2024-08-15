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

#ifndef _kehui_tabular_data_source_h_included
#define _kehui_tabular_data_source_h_included 1

#ifndef TABULAR_BOX_DISABLED


#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/resource.hpp>

#include <godot_cpp/core/gdvirtual.gen.inc>

#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/rb_set.hpp>
#include <godot_cpp/templates/vector.hpp>

#include <godot_cpp/variant/variant.hpp>



class TabularDataSource : public godot::Resource
{
   GDCLASS(TabularDataSource, godot::Resource);
public:
   enum ColumnFlags
   {
      AllowTitleEdit =          1,
      AllowMenu =         1 <<  1,
      AllowResize =       1 <<  2,
      AllowTypeChange =   1 <<  4,
      AllowSorting =      1 <<  5,
      AllowMove =         1 <<  6,
      ValueChangeSignal = 1 << 10,

      _Default = AllowTitleEdit | AllowMenu | AllowResize | AllowTypeChange | AllowSorting | AllowMove,
   };

   enum MoveColumnButtons
   {
      AlwaysVisible, ShowOnMouseOver, AlwaysHidden,
   };
private:
   /// Internal variables
   // Index of each selected row will be added into this set
   godot::RBSet<int64_t> m_selected;

   // Index list of rows that should be filtered out
   godot::RBSet<int64_t> m_filtered;

   // If true then a filter is being processed - this might be useful depending on the data source implementation.
   bool m_is_filtering;


   /// Exposed variables

   /// Internal functions

   /// Event handlers

protected:
   static void _bind_methods();
public:
   struct ColumnInfo
   {
      godot::String title;
      godot::String cell_class;
      int32_t type_code;
      uint32_t flags;
      MoveColumnButtons move_bt_mode;
      godot::Dictionary extra_settings;

      bool operator == (const ColumnInfo& other) const
      {
         return (title == other.title && 
            cell_class == other.cell_class &&
            type_code == other.type_code &&
            flags == other.flags &&
            move_bt_mode == other.move_bt_mode && 
            extra_settings == other.extra_settings);
      }
   };

   /// Overrides


   /// Exposed virtual functions
   // This is meant to return a Dictionary containing the list of supported value types. The Dictionary should have
   // integer keys and string values. Basically the 'key' is a code representing a type while the string is the name
   // (as shown in UI) of that type. Note that the code can be anything as is entirely up to each data source implementation.
   GDVIRTUAL0RC(godot::Dictionary, _get_type_list);

   // Must return true if the given column title exists within the data source.
   GDVIRTUAL1RC(bool, _has_column, const godot::String&);

   // This should return how many columns the data source is currently holding
   GDVIRTUAL0RC(int64_t, _get_column_count);

   // Given a column index, this function should return a Dictionary containing information related to that column.
   // With the exception of 'title', entries are optional and if not provided will default to the value shown bellow:
   // 1 - title: String -> Must match the title of the column
   // 2 - type_code: int -> Internal custom value type code. Will default to 0 if not given
   // 3 - flags: int -> flag settings based on the enum TabularBoxColumn:::FlagSEttings. Will default to FSDefault
   GDVIRTUAL1RC(godot::Dictionary, _get_column_info, int64_t);
   
   // The TabularBox expects this function to be able to deal with default values (empty title, type = -1...)
   // Also, when index is -1 it assumes the new column is meant to be appended into the list
   // The 'extra' Dictionary can be used to provide extra settings if the custom data source requires it.
   GDVIRTUAL5(_insert_column, const godot::String&, int32_t, int64_t, const godot::Dictionary&, const godot::Dictionary&);

   // This should remove the column at the given index
   GDVIRTUAL1(_remove_column, int64_t);

   // This should rename the column at the given index to the provided new title. Checking if the new title is valid or
   // not is meant to be done by the derived classes. If it's rejected remember to notify so the proper signal is emitted
   GDVIRTUAL2(_rename_column, int64_t, const godot::String&);

   // This must change the value type of the given column index into "new_type". Again, type codes are completely
   // implementation based.
   GDVIRTUAL2(_change_column_value_type, int64_t, int32_t);

   // Move a column from 'from' index into 'to' index. In other words, this is meant to change the column order
   GDVIRTUAL2(_move_column, int64_t, int64_t);


   // Must return the number of rows currently stored within the data source
   GDVIRTUAL0RC(int64_t, _get_row_count);

   // Used to insert a new row at the given index. If it's negative then it's expected that the new row should be appended
   // The incoming Dictionary holds "column title" as key with their respective values
   GDVIRTUAL2(_insert_row, const godot::Dictionary&, int64_t);

   // This should remove the row from the given index
   GDVIRTUAL1(_remove_row, int64_t);

   // Move a row from 'from' index into 'to' index. In other words, this is meant to change the row order.
   GDVIRTUAL2(_move_row, int64_t, int64_t);

   // Given column and row indices, this should return the value of a single cell stored within the data source
   GDVIRTUAL2RC(godot::Variant, _get_value, int64_t, int64_t);

   // Given column and row indices, this should change the value of a single cell stored within the data source
   GDVIRTUAL3(_set_value, int64_t, int64_t, const godot::Variant&);

   // This is expected to return a Dictionary containing column title as *key* with the corresponding value assigned into it.
   // In other words, all values of a single row should be part of the returned dictionary.
   GDVIRTUAL1RC(godot::Dictionary, _get_row, int64_t);

   // This is an optional override and if not implemented then sorting will not occur. That said, this function is expected
   // to sort rows based on the given column index and the incoming 'ascending' flag
   GDVIRTUAL2(_sort_by_column, int64_t, bool);

   // Another optional override. This can be used to help debug implementations and will not be directly called by the TabularBox
   GDVIRTUAL0C(_dump_data);



   /// Exposed functions
   void sort_by_column(int64_t column_index, bool ascending);


   bool is_filtering() const { return m_is_filtering; }
   void set_filter(const godot::String& keyword, bool case_sensitive, const godot::PackedStringArray& exclude_columns);
   void set_filter_regex(const godot::String& expression, const godot::PackedStringArray& exclude_columns);
   void clear_filter();
   bool is_filtered(int64_t row_index) const;


   // Derived classes can (and should) call the following functions in order to notify (through signals) the owning TabularBox
   // about changes that might occur within the data source.
   void notify_new_column(int64_t index);
   void notify_column_removed(int64_t index);
   void notify_column_moved(int64_t from, int64_t to);
   void notify_column_renamed(int64_t index);
   void notify_column_rename_rejected(int64_t index);
   void notify_new_row(int64_t index);
   void notify_row_removed(int64_t index);
   void notify_row_moved(int64_t from, int64_t to);
   void notify_value_changed(int64_t column, int64_t row, const godot::Variant& new_value);
   void notify_value_change_rejected(int64_t column, int64_t row);
   void notify_type_changed(int64_t column);
   void notify_sorted(int64_t column);

   virtual bool has_column(const godot::String& title) const;
   virtual int64_t get_column_count() const;
   virtual godot::Dictionary get_column_info(int64_t index) const;
   virtual void insert_column(const godot::String& title, int32_t type, int64_t index, const godot::Dictionary& info = godot::Dictionary(), const godot::Dictionary& extra = godot::Dictionary())
   {
      GDVIRTUAL_REQUIRED_CALL(_insert_column, title, type, index, info, extra);
   }
   virtual void remove_column(int64_t index) { GDVIRTUAL_REQUIRED_CALL(_remove_column, index); }
   virtual void rename_column(int64_t index, const godot::String& new_title) { GDVIRTUAL_REQUIRED_CALL(_rename_column, index, new_title); }
   virtual void change_column_value_type(int64_t index, int32_t new_type) { GDVIRTUAL_REQUIRED_CALL(_change_column_value_type, index, new_type); }
   virtual void move_column(int64_t from, int64_t to) { GDVIRTUAL_REQUIRED_CALL(_move_column, from, to); }

   virtual int64_t get_row_count() const;
   virtual void insert_row(const godot::Dictionary& values, int64_t index);
   virtual void remove_row(int64_t index);
   virtual void move_row(int64_t from, int64_t to);
   virtual godot::Variant get_value(int64_t column_index, int64_t row_index) const;
   virtual void set_value(int64_t column_index, int64_t row_index, const godot::Variant& value) { GDVIRTUAL_REQUIRED_CALL(_set_value, column_index, row_index, value); }
   virtual godot::Dictionary get_row(int64_t index) const;

   virtual bool native_sort_by_column(int64_t column_index, bool ascending);
   
   virtual void dump_data() const { GDVIRTUAL_CALL(_dump_data); }

   /// Setters/Getters


   /// Public non exposed functions
   // This is meant to make things easier for C++ implementations to deal with column info. Instead of using dictionaries,
   // use the inner struct ColumnInfo
   void fill_column_info(int64_t index, ColumnInfo& out_data);

   // Type list might not be useful outside of the TabularBox itself so not exposing this
   virtual godot::Dictionary get_type_list() const;


   void set_row_selected(int64_t row_index, bool selected);
   bool is_row_selected(int64_t row_index) const;
   bool has_selected_row() const;
   int64_t get_selected_count() const;
   void invert_row_selection();
   void select_all_rows();
   void deselect_all_rows();
   void delete_selected_rows();
   godot::Array get_selected_list() const;
   

   TabularDataSource();
};

VARIANT_ENUM_CAST(TabularDataSource::ColumnFlags);
VARIANT_ENUM_CAST(TabularDataSource::MoveColumnButtons);

#endif  // TABULAR_BOX_DISABLED

#endif   //_kehui_tabular_data_source_h_included
