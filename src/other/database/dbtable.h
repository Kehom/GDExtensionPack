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

#ifndef _kehother_dbtable_h_included
#define _kehother_dbtable_h_included 1

#ifndef DATABASE_DISABLED


#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/classes/resource_format_loader.hpp>
#include <godot_cpp/classes/resource_format_saver.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/variant.hpp>

//class DBTablePatch;

class DBTable : public godot::Resource
{
   friend class GDDatabase;
   GDCLASS(DBTable, godot::Resource);
   //typedef godot::HashMap<godot::Variant, godot::Dictionary, godot::VariantHasher, godot::VariantComparator> ColumnIndexType;
   //typedef godot::HashMap<godot::String, godot::Dictionary, godot::VariantHasher, godot::VariantComparator> ColumnIndexType;
   typedef godot::HashMap<godot::String, godot::Dictionary> ColumnIndexType;
   typedef godot::HashMap<godot::Variant, godot::Dictionary, godot::VariantHasher, godot::VariantComparator> RowIndexType;
public:
   typedef godot::HashSet<godot::Variant, godot::VariantHasher, godot::VariantComparator> ValueSetType;

   // For organization purposes some entries are skipping values. This is certainly not necessary but well...
   // That said, this is the type code used to store columns and their expected stored values
   enum ValueType
   {
      VT_Invalid,

      // A column with UniqueString or UniqueInteger is meant to hold unique values of either string or integer
      // The Locked* versions are not meant to be changed after created
      VT_UniqueString,
      VT_UniqueInteger,
      /// NOTE: The LockedUniqueString and LockedUniqueInteger are not exposed. Those are meant exclusively for when the
      /// table locks row IDs and is merely for internal usage. Ideally no "normal column" should have any of these two
      /// types assigned to it.
      VT_LockedUniqueString,
      VT_LockedUniqueInteger,

      // External string or external integer is basically telling that the column will hold IDs pointing to another
      // table within the database. If that table has IDs of integer types, then the column must hold ExternalInteger
      VT_ExternalString = 500,
      VT_ExternalInteger,

      // Does it make sense to add those two types?
      //VT_ExtStringArray,       // External string array
      //VT_ExtIntArray,          // External integer array


      // This is for a column that should automatically generate random weight accumulation values
      VT_RandomWeight = 600,

      // For "normal" usage. Columns of types bellow here will be able to change to other types as long as the
      // target type is not one of the above options
      VT_String = 1000,
      VT_Bool,
      VT_Integer,
      VT_Float,
      VT_Texture,
      VT_Audio,
      VT_GenericRes,
      VT_Color,
      VT_MultiLineString,


      VT_StringArray = 1500,
      VT_IntegerArray,
      VT_FloatArray,
      VT_TextureArray,
      VT_AudioArray,
      VT_GenericResArray,
      VT_ColorArray,
   };

   // When creating a child Table, this specifies the editing level of the values that are in the parent Tables.
   // Those are used as bit masks
   enum EditLevel
   {
      // Cells that exist in the parent table can be edited
      EL_ValueEditing = 1,

      // Rows in parent table can be marked for "removal". The original resource will not be changed, but the child table will
      // contain that telling that those rows should be removed when running "merge into master"
      EL_AllowRowRemoval = 1 << 1,

      // Allow new columns to be added. Original resource will not be changed, but data will be appended when running the
      // "merge into master" process
      //EL_AllowNewColumn = 1 << 2,

      // This specific mask is not meant to be exposed. Nevertheless a single parent can contain this flag. This indicates which
      // parent table is the "master".
      EL_Master = 1 << 16,
   };

   // When the owning Database needs to deal with column (adding or querying info), this struct will be used.
   // The exposed interface will deal with Dictionary, but the database should "translate" into/from an instance
   // of this struct.
   // While most fields here might seem like a duplication of what is in the ColumnData, at closer inspection there
   // are some differences that justify having both ColumnInfo and ColumnData structs.
   struct ColumnInfo
   {
      ValueType type;

      godot::String external_table;        // *IF* this column is pointing to another table, hold the name here

      int64_t index;
   };

private:
   // When sorting rows an instance of this will be created
   // One thing to note here is that godot::Callable requires the object to have the get_instance_id() function, which should
   // return an unsigned integer of 64 bits. When an instance of this struct is created the ID value will be assigned from
   // this resource itself, like 'sorter.id = get_instance_id()'.
   // An alternative here is to use CallableCustom, however it requires several other functions to be implemented.
   struct Sorter
   {
      godot::String column;
      uint64_t inst_id;

      bool ascending(const godot::Dictionary& a, const godot::Dictionary& b) const
      {
         return a[column] < b[column];
      }

      bool descending(const godot::Dictionary& a, const godot::Dictionary& b) const
      {
         return b[column] < a[column];
      }

      bool ascending_color(const godot::Dictionary& a, const godot::Dictionary& b) const;
      bool descending_color(const godot::Dictionary& a, const godot::Dictionary& b) const;


      uint64_t get_instance_id() const { return inst_id; }


      Sorter(const godot::String& col = "id", uint64_t iid = 0) : column(col), inst_id(iid) {}
   };

   /// Internal variables
   // The DBTablePatch needs means to verify that its target is indeed "this table". Godot offers ResourceUID system, which
   // would have been perfect for this task, specially because it would also provide means to reach the target resource (this
   // table) even if the file is renamed or moved into another directory. Unfortunately it doesn't fully work. The problem is
   // that resource IDs are not carried over through exported projects (https://github.com/godotengine/godot/issues/75617).
   // The ResourceUID can still be used *if* an ExportPlugin is created. In it the UIDs should all be converted into the final
   // paths. In here going with a much simpler approach, which is to generate a "table id" that can't be modified. Indeed,
   // resilience to file rename/directory change is lost when compared to the export plugin + ResourceUID.
   godot::String m_table_id;


   // "Reaching" the table through the Database is done by its name.
   godot::StringName m_table_name;

   // The list of columns on this table. Each entry is a Dictionary with the following format:
   // - name (String): Column title
   // - value_type (Int): type code of values stored within cells of this column. This is from the ValueType enum
   // - unique_values (Bool): if true then the values within this column should not be allowed to be repeated
   godot::Array m_column_array;

   
   // This is dynamically built (that is, not stored) and is meant to help with queries. Nevertheless, this maps from
   // column title into its Dictionary representation
   ColumnIndexType m_column_index;

   // This can be only TYPE_INT or TYPE_STRING. This should not be changed after the table is created. Nevertheless,
   // it basically determines the ID type for each row in this table
   int16_t m_id_type;

   // If this is true then row IDs of this table cannot be changed once created. Ideally the state of this flag should
   // not be changed after table creation
   bool m_locked_id;

   // Each entry is a Dictionary containing:
   // - id: The ID of the row, which is either a String or an Integer
   // - [column_name]: Corresponding value. There will be one for each column
   godot::Array m_row_list;

   // This will be dynamically generated. The key is the row id and will directly point to the same corresponding row
   // entry Dictionary that is held within the m_row_list
   //godot::HashMap<godot::Variant, godot::Dictionary> m_row_index;
   RowIndexType m_row_index;

   // Another data meant to be dynamically generated. Columns that are meant to hold unique values will generate sets
   // within this container. The idea is to make things easier to ensure the values are indeed unique within those
   // columns. This will result if faster verifications at the expense of using more memory.
   // That said, the Map points from column name into Set. The Set holds the values of the column.
   godot::HashMap<godot::StringName, ValueSetType> m_unique_indexing;


   // Each time a column meant to reference another table is created, an entry in this Dictionary will be created. The
   // key is the name of the referenced table. The value is the name of the column referencing the table. This container
   // will be stored
   godot::Dictionary m_ref_table;

   // This will be filled by the owning database. After filled, this will hold the list of tables referencing this one.
   godot::HashSet<godot::StringName> m_referencer;

   // Also set when loading the table. This holds information related to the random weights.
   struct RandomWeight
   {
      // The title of the column assigned to hold the weights. Empty if none
      godot::String column;

      // The total weight -sum
      double total_weight;

      // Each row needs its accumulation, which is based on the sum of the previous rows. Randomly picking something
      // from the table means iterating through this specific array
      godot::Vector<double> acc_weight;
   } m_rand_weight;

   /// NOTE: Perhaps find a way to share a single instance between tables of the same Database?
   godot::Ref<godot::RandomNumberGenerator> m_rand_gen;



   /// Exposed variables

   /// Internal functions
   // Because Dictionaries don't provide "contracts" regarding what is stored, add some few functions to deal with the values
   // that are expected to be inside Dictionaries representing columns
   void column_set_title(const godot::String& t, godot::Dictionary& out_column) { out_column["title"] = t; }
   godot::String column_get_title(const godot::Dictionary& column) const { return column.get("title", ""); }
   void column_set_type(ValueType type, godot::Dictionary& out_column) { out_column["value_type"] = type; }
   ValueType column_get_type(const godot::Dictionary& column) const { return ((ValueType)(int)column.get("value_type", VT_Invalid)); }
   void column_set_external(const godot::String& other, godot::Dictionary& out_column);
   godot::String column_get_external(const godot::Dictionary& column) const { return column.get("extid", ""); }

   // This function is meant to serve as a "shortcut" to verify if a given column requires unique values or not.
   bool require_unique_values(const godot::Dictionary& column) const;

   // Helps verify if the given value is unique within the provided column. In other words, this returns true if the
   // value does not exist in the column.
   bool is_unique(const godot::Dictionary& column, const godot::Variant& value) const;

   // When adding values into a column that requires unique values, this function should be used to help with the upkeep
   void set_used(const godot::Dictionary& column, const godot::Variant& value);

   // When changing values in a column that requires unique values, this function should be used to update internal state
   void unique_changed(const godot::Dictionary& column, const godot::Variant& from_value, const godot::Variant& to_value);

   // Either set or remove a column meant to automatically deal with random weights. Basically if the provided column
   // title is empty then it will cleared, otherwise it will take the initial setup and set the total to the default
   void set_auto_weight(const godot::String& title, bool is_new);

   // This should be called whenever a change occurs in the random weight system
   // Whenever rows are reordered, new rows are added and so on, this function must be used to update
   void calculate_weights();

   // (Re)build the column indexing
   void build_column_index();

   // (Re)build the unique values indexing
   void build_unique_index();

   // (Re)build the row indexing. That is, from row ID into row data
   void build_row_index();

   // Generate a random string. Remember this does *not* have cryptography strength.
   godot::String make_random_string(int32_t length);

   // Create a valid (not used) integer ID
   int64_t generate_int_id();

   // Create a valid (not used) String ID
   godot::String generate_str_id();

   // This performs the task of inserting the row data into the table and the process assumes the incoming dictionary already
   // contains the ID
   void add_row(const godot::Variant& id, const godot::Dictionary& values, int64_t index);


   // Given a column, return a "default" value for it, which should be used whenever inserting a new row
   // IMPORTANT: This should not allow 'null' to be returned as it will disrupt the internal system that
   // relies on setting meta values on each cell (the TabularBox Control). The thing is, if the value is
   // null then the meta will actually be removed and it will cause errors
   godot::Variant get_default_value(const godot::Dictionary& column);

   // Take a Variant value and return a String
   godot::String convert_to_string(const godot::Variant& value) const;

   // Take a Variant value and return a Boolean
   bool convert_to_bool(const godot::Variant& value) const;

   // Take a Variant value and return an integer
   int64_t convert_to_int(const godot::Variant& value) const;

   // Take a Variant value and return a double
   double convert_to_double(const godot::Variant& value) const;

   // Take a Variant value and return a String compatible with values meant to be stored on VT_Texture, VT_Audio or VT_GenericResource columns
   godot::String convert_to_path(const godot::Variant& value) const;

   // Take a Variant value and return a Color
   godot::Color convert_to_color(const godot::Variant& value) const;

   // Convert the value type of a column into another
   void convert_value_type(const godot::String& column_title, ValueType to_type);

   // Just a "shortcut" function meant to build a Dictionary containing the information that will be appended into the data meant for UI usage
   static godot::Dictionary make_dictionary_ui_entry(ValueType type, const godot::String& ui);


   // The owning database will call this when a referenced table has been renamed
   void referenced_table_renamed(const godot::String& from, const godot::String& to);

   // The owning database will call this whenever upkeep must be performed
   void clear_referencer() { m_referencer.clear(); }

   // Called by the owning database
   void add_referencer(const godot::String& table_name) { m_referencer.insert(table_name); }

   // Called by the owning database
   void remove_referencer(const godot::String& table_name) { m_referencer.erase(table_name); }

   // Called by the owning database - whenever a row ID of a referenced table is changed
   void referenced_row_id_changed(const godot::String& other_table, const godot::Variant& id_from, const godot::Variant& id_to);

   // Owning database calls this whenever rows from a referenced table are removed
   void referenced_rows_removed(const godot::String& other_table, const ValueSetType& id_list);


   /// Event handlers

protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* out_list) const;
   bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   static void _bind_methods();
public:
   /// Overrides

   /// Exposed virtual functions

   /// Exposed functions
   godot::String get_table_id() const { return m_table_id; }

   godot::StringName get_table_name() const { return m_table_name; }

   // This should return either Variant::INT or Variant::STRING
   int get_id_type() const { return m_id_type; }

   // Returns true if this table has "locked IDs". In other words, if this returns true row IDs in this table cannot be changed after
   // entries are added
   bool is_row_id_locked() const { return m_locked_id; }

   // Retrieves how many columns are stored within this table
   int64_t get_column_count() const { return m_column_array.size(); }

   // Retrieve a Dictionary containing information about the Column at 'index'. If the index is invalid then an empty
   // dictionary will be returned. The entries of the dictionary are:
   // - title (String): the column title
   // - value_type (int): type code of values stored in the column, following ValueType enum
   // - extid (String): If not empty, name of another table, referenced by this column
   godot::Dictionary get_column_by_index(int64_t index) const;

   // Retrieve a Dictionary containing information about the Column given its name. The entries are the same as those
   // obtained by calling get_column_by_index()
   godot::Dictionary get_column_by_title(const godot::String& title) const;

   // Retrieve the title of a column given its index
   godot::String get_column_title(int64_t index) const;

   // Retrieve the value type of a given column
   ValueType get_column_value_type(const godot::String& column_title) const;

   //
   int64_t get_row_count() const { return m_row_list.size(); }

   // Returns true if there is a column with the given title
   bool has_column(const godot::String& title) const { return m_column_index.has(title); }

   // Returns true if this table has a column registered to hold automatic random weights
   bool has_random_weight_column() const { return !m_rand_weight.column.is_empty(); }
   
   // Given an ID, retrieve a row
   godot::Dictionary get_row(const godot::Variant& id) const;

   // Retrieve a row given its index
   godot::Dictionary get_row_by_index(int64_t index) const;

   // Given a row Id an da column title, retrieve the value of a single cell
   godot::Variant get_cell_value(const godot::Variant& id, const godot::String& column_title) const;

   // Pick a random row. If the random weight system is setup it will be used, otherwise a random index will be chosen
   godot::Dictionary pick_random_row(const godot::Ref<godot::RandomNumberGenerator>& rng = godot::Ref<godot::RandomNumberGenerator>()) const;

   // Retrieve the title of the column holding automatic random weight calculation, if any. Empty if it's not set
   godot::String get_random_weight_column_title() const { return m_rand_weight.column; }

   // Obtain the accumulated random weight of a specific row.
   double get_row_accumulated_weight(int64_t row_index) const;

   // Given a row index, obtain the probability of it being randomly picked
   double get_row_probability(int64_t row_index) const;

   // Obtain the total accumulated weight sum for this table (-1.0 if the system is not set)
   double get_total_weight_sum() const { return m_rand_weight.total_weight; }

   // Get the list of tables referenced by this one
   godot::PackedStringArray get_reference_list() const { return m_ref_table.keys(); }

   // Get the list of tables referencing this one
   godot::PackedStringArray get_referenced_by_list() const;

   // Returns true if this table references the given other table name
   bool is_referencing(const godot::StringName& other) const { return m_ref_table.has(other); }
   
   // Returns true if this table is referenced by the other provided table name
   bool is_referenced_by(const godot::StringName& other) const { return m_referencer.has(other); }

   // Create (and return) an instance of a DBTablePatch in which its target is this DBTable
   //godot::Ref<DBTablePatch> create_table_patch() const;

   // DBTablePatch instances are meant to contain information related to modified, new and removed rows. If the "master table"
   // is changed, the data in the DBTablePatch might become redundant or even invalid. On both cases it shouldn't pose that
   // much of a problem when applying the patch. Nevertheless, this function is used to help ensure the patch file is as clean
   // as possible. During editing the data source might not work correctly on non cleaned patches
   //void clean_patch(godot::Ref<DBTablePatch>& table_patch) const;

   // Attempt to apply the incoming DBTablePatch into this table. Returns true on success
   //void apply_patch(const godot::Ref<DBTablePatch>& table_patch);


   // "Converts" the stored data into a String in JSON format
   godot::String get_data_as_json(const godot::String& indent = "") const;
   
   /// Setters/Getters

   /// Public non exposed functions
   // This is not meant to be directly used (thus not exposed). Nevertheless, the owning database will call
   // this function whenever the user requests to rename a table
   void change_name(const godot::StringName& name) { m_table_name = name; }

   // Given a column index, fill the incoming ColumnInfo struct with the corresponding data. Returns false if
   // no valid data was actually added into the output
   bool fill_column_info(int64_t column_index, ColumnInfo& out_info) const;

   // Used to insert a new column - not exposing this to avoid having problems when/if tables have references
   // between them. Instead, use the owning GDDataBase to insert a new column, which will them call this.
   // Returns the index of the newly added column. If a negative value, then the column was not added.
   int64_t add_column(const godot::String& title, const ColumnInfo& settings);

   // Given a column index, remove it from the table. Be very careful because this can't be undone.
   // Returns true if the column was actually removed
   bool remove_column(int64_t column_index);

   // Given a column index, rename it into the given title.
   // Returns true if the column was actually renamed
   bool rename_column(int64_t column_index, const godot::String& new_title);

   // Move a column from index 'from' into 'to'.
   // Returns true if the column was actually moved
   bool move_column(int64_t from, int64_t to);

   // Given a column index, change the column value type into the specified one
   bool change_column_value_type(int64_t column_index, ValueType to_type);

   // Obtain an array containing the list of column names in the "display order"
   void get_column_order(godot::PackedStringArray& out_array) const;


   // Insert a row at the given index. ID will be automatically generated
   int64_t add_row(const godot::Dictionary& values, int64_t index);

   // Given a row ID, remove it from the table
   void remove_row(const godot::Variant row_id);

   // Remove a row given its index
   void remove_row_by_index(int64_t index);

   // Move a row from 'from' into 'to' index
   bool move_row(int64_t from, int64_t to);

   // Get the ID of the given row index
   godot::Variant get_row_id(int64_t row_index) const;

   // Change the ID of the specified row index
   bool set_row_id(int64_t row_index, const godot::Variant& new_id);

   // Given a column title and row index, change the cell value
   bool set_value(const godot::String& column_title, int64_t row_index, const godot::Variant& value);

   // Change the value of the specified cell
   bool set_value_by_index(int64_t column_index, int64_t row_index, const godot::Variant& value);

   // Sort rows by ID
   void sort_by_id(bool ascending);

   // Sort rows by the specified column index
   bool sort_by_column(int64_t index, bool ascending);


   // Returns the name of the table referenced by the given column name
   godot::StringName get_referenced_by_column(const godot::String& column_title) const;

   // Retrieve a copy of the reftable
   godot::Dictionary get_reference_data() const { return m_ref_table.duplicate(); }

   // Builds a "list" of value type entries meant to be displayed in UI
   static void build_type_list(godot::Vector<godot::Dictionary>&out_list);


   DBTable(int id_type = godot::Variant::INT, const godot::StringName& name = "", bool locked_id = false);
};


VARIANT_ENUM_CAST(DBTable::ValueType);


/*class ResourceFormatLoaderDBTable : public godot::ResourceFormatLoader
{
public:
   virtual godot::PackedStringArray _get_recognized_extensions() const override;
   virtual bool _handles_type(const godot::StringName &type) const override;
   virtual godot::String _get_resource_type(const godot::String &path) const override;
   virtual godot::Variant _load(const godot::String &path, const godot::String &original_path, bool use_sub_threads, int32_t cache_mode) const override;
};

class ResourceFormatSaverDBTable : public godot::ResourceFormatSaver
{
public:
   virtual godot::Error _save(const godot::Ref<godot::Resource> &resource, const godot::String &path, uint32_t flags) override;
   virtual godot::PackedStringArray _get_recognized_extensions(const godot::Ref<godot::Resource> &resource) const override;
   virtual bool _recognize(const godot::Ref<godot::Resource> &resource) const override;
};*/

#endif  //DATABASE_DISABLED


#endif   // _kehother_dbtable_h_included

