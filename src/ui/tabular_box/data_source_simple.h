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

#ifndef _kehui_data_source_simple_h_included
#define _kehui_data_source_simple_h_included 1

#ifndef TABULAR_BOX_DISABLED

#include "data_source.h"

#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_map.hpp>

// In here I have opted to not use an internal struct to store column data. Moreover, instead of using godot::Vector<>
// godot::Array is in use here. The reason? Take advantage of the already existing serialization features of Godot.
// Indeed I do potentially loose some performance, however the advantages of having an already tested system to deal
// with saving and loading data are the defining factors here. The thing is, the features provided by Godot also have
// text vs binary data. By simply changing a project setting, when exporting the game/app text resources are converted
// into binary data for faster loading. This brings the nice feature of having resources compatible with version control
// systems while also providing faster loading when exported. Another advantage is that when converted to binary data
// those resources use less storage space. So... yeah.
// Note that to perform the storage the internal containers will be "registered" but not exposed to the editor. That will
// be done through the _get_property_list(), _get() and _set() functions.
class TabularDataSourceSimple : public TabularDataSource
{
   GDCLASS(TabularDataSourceSimple, TabularDataSource);
private:
   enum ValueType
   {
      VT_String, VT_Bool, VT_Integer, VT_Float, VT_Texture, VT_MultiLineString,
      VT_Invalid = 50000,
   };

   
   // godot::Array provides a custom_sort() function, which receives a Callable. When requesting to sort rows an instance
   // of this struct will be created, which will hold the title of the column. Then a callable with be created pointing to
   // either ascending() or descending() function of the created object.
   // One thing to note here is that godot::Callable requires the object to have the get_instance_id() function, which should
   // return an unsigned integer of 64 bits. When an instance of this struct is created the ID value will be assigned from
   // this data source itself, like 'sorter.id = get_instance_id()'.
   // An alternative is to use CallableCustom, but its setup is a bit more complex that this simple struct
   struct Sorter
   {
      godot::String column;        // The title of the column, that is
      uint64_t id;
      
      bool ascending(const godot::Dictionary& a, const godot::Dictionary& b) { return a[column] < b[column]; }
      bool descending(const godot::Dictionary& a, const godot::Dictionary& b) { return b[column] < a[column]; }

      uint64_t get_instance_id() const { return id; }
   };

   /// Internal variables
   // Each column in here is a Dictionary containing two fields:
   // title: godot::String holding the title/name of the column
   // type: Integer corresponding to the const values within ValueType enum
   // This is "marked" for storage through the _get_property_list()
   godot::Array m_column_list;

   // The "indexing" of the columns is used to make queries easier instead of using linear searches. This index
   // does not need storage so it can use HashMap with "strong" typing
   godot::HashMap<godot::String, godot::Dictionary> m_column_index;

   // The row data works like this:
   // Each entry is a Dictionary. Each of those will contain a number of entries equal to the columns. Basically
   // "column_title": "value"
   // This is "marked" for storage through the _get_property_list()
   godot::Array m_row_list;

   /// Exposed variables

   /// Internal functions
   void build_index();

   godot::String get_free_column_title() const;

   godot::Variant get_default_value_for_type(ValueType type) const;

   godot::String get_cell_class(ValueType type) const;

   godot::Variant convert(const godot::Variant& val, int32_t to_type) const;

   /// Event handlers

protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* out_list) const;
   bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   //void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual godot::Dictionary get_type_list() const override;
   virtual bool has_column(const godot::String& title) const override { return m_column_index.has(title); }
   virtual int64_t get_column_count() const override { return m_column_list.size(); }
   virtual godot::Dictionary get_column_info(int64_t index) const override;
   virtual void insert_column(const godot::String& title, int32_t type, int64_t index, const godot::Dictionary& info, const godot::Dictionary& extra) override;
   virtual void remove_column(int64_t index) override;
   virtual void rename_column(int64_t index, const godot::String& new_title) override;
   virtual void change_column_value_type(int64_t index, int32_t new_type) override;
   virtual void move_column(int64_t from, int64_t to) override;

   virtual int64_t get_row_count() const override { return m_row_list.size(); }
   virtual void insert_row(const godot::Dictionary& values, int64_t index) override;
   virtual void remove_row(int64_t index) override;
   virtual void move_row(int64_t from, int64_t to) override;
   virtual godot::Dictionary get_row(int64_t index) const override;
   virtual godot::Variant get_value(int64_t column, int64_t row) const override;
   virtual void set_value(int64_t column, int64_t row, const godot::Variant& value) override;
   virtual bool native_sort_by_column(int64_t column, bool ascending) override;
   virtual void dump_data() const override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   TabularDataSourceSimple() {}
};

#endif   // TABULAR_BOX_DISABLED

#endif  //_kehui_data_source_simple_h_included
