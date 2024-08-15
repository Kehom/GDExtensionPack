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

#ifndef _kehother_database_editor_datasource_h_included
#define _kehother_database_editor_datasource_h_included 1

/// This data source implementation is used to bridge the GDDatabase resource and the TabularBox
/// The DataSource itself is not needed outside of editor builds as its sole purpose is to "tell"
/// the TabularBox what must be displayed. The data itself is stored within the GDDatabase resource

#ifndef DATABASE_DISABLED
#ifndef EDITOR_DISABLED

#include "../database.h"
#include "../dbtable.h"
//#include "../dbtablepatch.h"
#include "../../../ui/tabular_box/data_source.h"

// This is a TabularBox data source specialized for the Database plugin.
// The data source is meant to provide the data from a single DBTable instance.
// And if a DBTablePatch is assigned, this must mediate everything. More specifically:
// - The data given to the TabularBox must be the "patched" one
// - Changes must occur in the "DBTablePatch" instead of the "DBTable"

class DBEditorDataSource : public TabularDataSource
{
   GDCLASS(DBEditorDataSource, TabularDataSource);
private:
   /// Internal variables
   godot::Dictionary m_type_list;
   godot::Ref<GDDatabase> m_db;
   godot::Ref<DBTable> m_table;
//   godot::Ref<DBTablePatch> m_patch;

   // This will hold a duplicated o the m_table, but patched. Indeed this will lead to more RAM requirements, but
   // will make things a lot easier to provide the correct data to the TabularBox
   godot::Ref<DBTable> m_patched_table;

   /// Exposed variables

   /// Internal functions
   godot::String get_cell_class(DBTable::ValueType type) const;
   uint32_t get_column_flags(DBTable::ValueType type) const;
   godot::Dictionary get_extra_settings(DBTable::ValueType type, int64_t column_index) const;

   /// Event handlers

protected:
   static void _bind_methods() {}
public:
   /// Overrides
   virtual godot::Dictionary get_type_list() const override { return m_type_list; }
   virtual bool has_column(const godot::String& title) const override;
   virtual int64_t get_column_count() const override;
   virtual godot::Dictionary get_column_info(int64_t index) const override;
   virtual int64_t get_row_count() const override;
   virtual godot::Dictionary get_row(int64_t index) const override;
   virtual godot::Variant get_value(int64_t column, int64_t row) const override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void setup(const godot::Ref<GDDatabase>& db, const godot::Ref<DBTable>& table);

   godot::Ref<DBTable> get_table() const { return m_table; }

//   void set_patch(const godot::Ref<DBTablePatch>& patch);
//   bool has_patch() const { return m_patch.is_valid(); }
//   godot::Ref<DBTablePatch> get_patch() const { return m_patch; }
//   godot::String get_patch_filename() const;

   godot::String get_table_name() const;

//   void pre_filter() { m_expand_row = true; }
//   void post_filter() { m_expand_row = false; }

   // The editor plugin takes over the process of dealing with changes done within the TabularBox. Once that is
   // dealt with, the TabularBox must be notified back. The next functions are meant to do that.
   void column_added(int at);
   void column_removed(int from);
   void column_renamed(int index, bool rejected);
   void column_moved(int from, int to);
   void column_type_changed(int column_index);
   void row_added(int at);
   void row_removed(const godot::Array& list);
   void row_moved(int from, int to);
   void value_changed(int column_index, int row_index, const godot::Variant& value, bool rejected);
   void rows_sorted();



   DBEditorDataSource();
};



#endif  //EDITOR_DISABLED
#endif  //DATABASE_DISABLED 

#endif   // _kehother_database_editor_datasource_h_included

