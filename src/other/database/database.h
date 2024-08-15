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

#ifndef _kehother_database_h_included
#define _kehother_database_h_included 1

#ifndef DATABASE_DISABLED

#include "dbtable.h"

#include <godot_cpp/classes/resource.hpp>

/// Regarding the storage of the data. There are two options, which are shown bellow with "pros" and "cons".
/// 1) Use godot::Dictionary and godot::Array to store the data.
///    Pros:
///    - Resource saving and loading into/from files is "automatic", no need to implement anything besides the
///      resource itself.
///    - Because of the "automatic" saving/loading, leveraging the "export text resource to binary" feature is
///      also automatic, without requiring additional code for that to work.
///    Cons:
///    - Because these containers are actually dealing with godot::Variant, type safety is somewhat lost
///    - Dealing with Dictionary instead of Structs means that validity of the data must be verified
///    - Attempting to add custom file extension to the custom resources simply "kill" automatic save/load. The
///      thing is, in the core there is RES_BASE_EXTENSION macro, which allows custom extensions for whatever
///      the resource is. Unfortunately it's not exposed to GDExtension
/// 2) Use godot::Vector<>, godot::HashMap<> and godot::HashSet<> containers.
///    Pros
///    - Validity of the data becomes way simpler to deal with. Even though converting incoming scripted data
///      still requires those verifications.
///    - Because manual save/load has to be implemented, custom file extension somewhat comes as a "bonus" since
///      it is part of the exact same system (ResourceFormatLoader and ResourceFormatSaver)
///    Cons:
///    - Saving/Loading to/from file must be manually done through ResourceFormatLoader and ResourceFormatSaver.
///    - Leveraging text to binary format when exporting means that yet another system must be implemented, which
///      is creating an EditorExportPlugin.
///
/// OK, so based on this, the "type safety" as well as "data verification" is something that will be needed
/// regardless of the internal containers. Dealing with strong typed containers internally *might* bring more
/// performance because there wont be so much need to unwrap the variants. But still, deciding solely because of
/// this is not exactly a good idea, specially without proper profiling.
/// Yet the prospect of having a custom file extension is something that would be very desired, specially to help
/// organize project files. Unfortunately there is a high cost in GDExtension to add this kind of support. That cost
/// comes in the requirement to implement custom resource saver and loader. Of course, taking such path makes the
/// usage of strong typed containers an obvious choice, regardless of any *possible* performance gains.
/// Yet, if following with option (2), there is still another requirement in order to fulfill all desired features,
/// which is to implement the EditorExportPlugin. The "cost" becomes even higher considering the low amount of information
/// regarding this system. My initial attempts to make the plugin work simply failed, so the implementation here
/// is going with option (1)



class GDDatabase : public godot::Resource
{
   GDCLASS(GDDatabase, godot::Resource);
private:
   /// Internal variables
   // Key = StringName holding DBTable name
   // Value = Instance of DBTable. More specifically, a Ref<DBTable>
   // Using Dictionary to take advantage of automatic Resource save/load system
   godot::Dictionary m_table;
   
   // The next two containers are dynamically generated and not stored. They are mostly used to help keep internal structure of
   // the DB consistent (as well as to provide some data to external code). The first is used to store table names that use
   // String as unique ID. The second is used to store table names that use Integer as unique ID
   godot::Vector<godot::StringName> m_strid_list;
   godot::Vector<godot::StringName> m_intid_list;

   /// Exposed variables

   /// Internal functions
   void check_table_setup();

   // When retrieving a row, it's possible to "expand" data from a referenced table. This function performs this expansion
   void expand_row_cell(const godot::Ref<DBTable>& table, godot::Dictionary& out_row) const;

   /// Event handlers

protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* out_list) const;
   bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   static void _bind_methods();
public:
   enum ExportMode
   {
      EXPORT_MONOLITHIC, EXPORT_INHERIT, EXPORT_FILE_PER_TABLE,
   };

   /// Overrides

   /// Exposed virtual functions

   /// Exposed functions
   bool save(const godot::String& path, bool include_tables);

   bool has_table(const godot::StringName& table_name) const { return m_table.has(table_name); }

   // If int_id is false then the added table will use Strings as row IDs instead of integers
   // If the table is created, true will be returned. Also, it will be embedded
   bool add_table(const godot::StringName& table_name, bool int_id, bool locked_id = false);


   // Given a DBTable resource, attempt to add it as a table managed by this database
   bool add_table_res(const godot::Ref<DBTable>& table);

   

   // Renames a table, from 'from' name into 'to' name
   // Returns true if there was any change at all
   bool rename_table(const godot::StringName& from, const godot::StringName& to);

   // Obtain an array containing the names of all tables within this database
   godot::PackedStringArray get_table_list() const { return m_table.keys(); }

   // Retrieve a table instance given its name
   godot::Ref<DBTable> get_table(const godot::StringName& table_name) const { return m_table.get(table_name, godot::Ref<DBTable>()); }

   // Removes a table from the database, given its name. Attention, this cannot be undone!
   bool remove_table(const godot::StringName& table_name);

   // Returns true if the table 'table_name' contains a column with the name 'title'
   bool table_has_column(const godot::StringName& table_name, const godot::String& title) const;

   // Given a table name and column title insert a new column into that table. The column will be inserted into the specified index.
   // If it's -1 then the column will be appended. If external is not empty it should be the name of another table that already exists
   // in the database.
   // Returns the index of the new table, -1 if no column has been added
   int64_t table_insert_column(const godot::StringName& table_name, const godot::String& column_title, DBTable::ValueType type, int index, const godot::String& external);

   // Given a table name and a column index, rename that column into a new name
   // Returns true if a change happened
   bool table_rename_column(const godot::StringName& table_name, int64_t column_index, const godot::String& to);

   // Given a column index, remove it from the specified table. Returns true if something changed
   // Warning: this can't be undone
   bool table_remove_column(const godot::StringName& table_name, int64_t column_index);

   // Given a table name, move a column from 'from' index into 'to' index.
   // Returns true if something has actually changed
   bool table_move_column(const godot::StringName& table_name, int64_t from, int64_t to);

   // Change the value type of a column within a table, given its name. Returns true if something changed
   bool table_change_column_value_type(const godot::StringName& table_name, int64_t column_index, DBTable::ValueType to_type);

   // Given a table name, return true if the corresponding table has the random weight system setup within it
   bool table_has_random_setup(const godot::StringName& table_name) const;

   // Insert a row into the given column. The dictionary containing the values should be "key = column title" -> "corresponding value".
   // If the index is negative then the row will be appended. Returns the index where the row has been inserted. -1 if error
   int64_t table_insert_row(const godot::StringName& table_name, const godot::Dictionary& values, int64_t index);

   // Move a row from 'from' index into 'to' index, in the specified table. Returns true if something actually changed
   bool table_move_row(const godot::StringName& table_name, int64_t from, int64_t to);

   // Given a column name and a list of indices, removes all rows matching the index list
   // Returns true if something actually changed
   bool table_remove_row(const godot::StringName& table_name, const godot::PackedInt64Array& index_list);

   // Change the ID of a row at the given index in the specified table name. Returns true if something actually changed
   bool table_set_row_id(const godot::StringName& table_name, int64_t row_index, const godot::Variant& new_id);

   // Sort rows of the specified table by the ID
   bool table_sort_rows_by_id(const godot::StringName& table_name, bool ascending = true);

   // Sort the rows of the specified table by the specified column index
   bool table_sort_rows(const godot::StringName& table_name, int64_t column_index, bool ascending = true);

   // Change the value of the cell in the specified table. Returns true if something changed.
   bool table_set_cell_value(const godot::StringName& table_name, int64_t column_index, int64_t row_index, const godot::Variant& value);


   // Given a table name and row id, retrieve row data. If expand is set to true and the table contains columns referencing other tables,
   // then the referenced data will be expanded and added as fields of an inner Dictionary of the same name of the column referencing
   // the data. That said, the returned Dictionary should be formatted as "key = column_title" -> "cell value". It will be empty if
   // there was an error retrieving the data
   godot::Dictionary get_row_from(const godot::StringName& table_name, const godot::Variant& id, bool expand) const;

   // Instead of retrieving a row by its ID, use the index within the column.
   godot::Dictionary get_row_from_by_index(const godot::StringName& table_name, int64_t index, bool expand) const;

   // Randomly pick a row from the specified table. If that column contains random weight setup, that will be used, otherwise a
   // random index will be used
   // A custom random number generator can be provided, which gives more control over seed and state
   godot::Dictionary randomly_pick_row(const godot::StringName& table_name, bool expand, const godot::Ref<godot::RandomNumberGenerator>& rng = godot::Ref<godot::RandomNumberGenerator>()) const;


   // Returns true if the given table name is referenced by the other table name
   bool is_table_referenced_by(const godot::StringName& table_name, const godot::StringName& other_table) const;


   
//   void merge_database(const godot::Ref<GDDatabase>& other_db, bool allow_new_tables, const godot::PackedStringArray& block_overwrite, const godot::PackedStringArray& replace_list);

   // If monolithic all data will be stored in a single file that matches the Database resource file, placed at the specified directory.
   // Otherwise each table will generate a file being named after the corresponding table.
   void export_to_json(const godot::String& directory, bool monolithic, const godot::String& indent = "") const;

   // Build a Dictionary containing information related to the Database. This might be useful for debugging
   godot::Dictionary get_db_info() const;

   /// Setters/Getters


   /// Public non exposed functions
   // Build a list of possible tables that can be referenced by the given table name. This is meant mostly for UI
   void fill_external_candidates_for(int type, const godot::StringName& table_name, godot::PackedStringArray& out_list) const;


   // This is not exposed mostly because it's primarily meant to be a "helper" for the editor, which deals with patched
   // tables without applying into the "main table"
   godot::Dictionary get_row_from_patched(const godot::Ref<DBTable>& patched, int64_t index, bool expand) const;


   //void set_table(const godot::Dictionary table) { m_table = table; }
   //godot::Dictionary get_table() const { return m_table; }


   GDDatabase();
};



/*class ResourceFormatLoaderGDDatabase : public godot::ResourceFormatLoader
{
   // This must be registered within the ClassDB, otherwise it will be completely ignored by the ResourceSaver singleton!
   // But for the registration to work, this GDCLASS() macro must be used. This macro also enforces the creation of _bind_methods()
   GDCLASS(ResourceFormatLoaderGDDatabase, godot::ResourceFormatLoader);
protected:
   static void _bind_methods() {}
public:
   virtual godot::PackedStringArray _get_recognized_extensions() const override;
   virtual bool _handles_type(const godot::StringName& type) const override;
   virtual godot::String _get_resource_type(const godot::String& path) const override;
   virtual godot::Variant _load(const godot::String& path, const godot::String& original_path, bool use_sub_threads, int32_t cache_mode) const override;

   ResourceFormatLoaderGDDatabase() {}
};

class ResourceFormatSaverGDDatabase : public godot::ResourceFormatSaver
{
   // This must be registered within the ClassDB, otherwise it will be completely ignored by the ResourceSaver singleton!
   // But for the registration to work, this GDCLASS() macro must be used. This macro also enforces the creation of _bind_methods()
   GDCLASS(ResourceFormatSaverGDDatabase, godot::ResourceFormatSaver);
protected:
   static void _bind_methods() {}
public:
   virtual godot::Error _save(const godot::Ref<godot::Resource>& resource, const godot::String& path, uint32_t flags) override;
   virtual godot::PackedStringArray _get_recognized_extensions(const godot::Ref<godot::Resource>& resource) const override;
   virtual bool _recognize(const godot::Ref<godot::Resource>& resource) const override;


   ResourceFormatSaverGDDatabase() {}
};*/




#endif  //DATABASE_DISABLED

#endif   // _kehother_database_h_included
