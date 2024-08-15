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

#include "database.h"

#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include <godot_cpp/variant/utility_functions.hpp>



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GDDatabase
void GDDatabase::check_table_setup()
{
   godot::Array table_list = m_table.values();
   godot::Array name_list = m_table.keys();

   const int64_t tb_count = m_table.size();

   m_intid_list.clear();
   m_strid_list.clear();

   // First go through all tables and clear the "referenced by list" in all of them.
   for (int64_t i = 0; i < tb_count; i++)
   {
      godot::Ref<DBTable> table = table_list[i];
      table->clear_referencer();
   }

   // Then rebuild the "links"
   for (int64_t i = 0; i < tb_count; i++)
   {
      const godot::String tbname = name_list[i];
      godot::Ref<DBTable> table = table_list[i];

      if (table->get_id_type() == godot::Variant::INT)
      {
         m_intid_list.append(tbname);
      }
      else if (table->get_id_type() == godot::Variant::STRING)
      {
         m_strid_list.append(tbname);
      }

      godot::PackedStringArray reflist = table->get_reference_list();
      const int64_t refsize = reflist.size();
      for (int64_t r = 0; r < refsize; r++)
      {
         const godot::String refname = reflist[r];
         godot::Ref<DBTable> referenced = get_table(refname);
         
         if (referenced.is_valid())
         {
            referenced->add_referencer(tbname);
         }
      }
   }
}


void GDDatabase::expand_row_cell(const godot::Ref<DBTable>& table, godot::Dictionary& out_row) const
{
   // In the obtained dictionary:
   // key = name of referenced table
   // value = column title referencing a key in the referenced table
   const godot::Dictionary reflist = table->get_reference_data();
   const godot::Array klist = reflist.keys();
   const int64_t ksize = klist.size();
   for (int64_t i = 0; i < ksize; i++)
   {
      // Name of the referenced table
      const godot::String ref_table = klist[i];

      // Name of the column referencing another table
      const godot::String referencing = reflist[ref_table];
      
      // ID of the row within the other table
      const godot::Variant ref_id = out_row.get(referencing, nullptr);

      // Get the data from the other table and replace within the row data.
      // NOTE: It would be really easy to simply call get_row_from(), however it might call this function back. Depending on
      // how the references are done it's possible to result in an infinite loop, so instead manually gather the data and stop
      // there. In other words, expand just once
      godot::Ref<DBTable> other_table = get_table(ref_table);
      if (other_table.is_valid() && ref_id.get_type() != godot::Variant::NIL)
      {
         const godot::Dictionary row = other_table->get_row(ref_id);
         if (!row.is_empty())
         {
            out_row[referencing] = row;
         }
      }
   }

}



void GDDatabase::_get_property_list(godot::List<godot::PropertyInfo>* out_list) const
{
   using namespace godot;
   
   // "Mark" the internal Dictionary to be stored, but don't expose it
   out_list->push_back(PropertyInfo(Variant::DICTIONARY, "db_table", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
}

bool GDDatabase::_set(const godot::StringName& prop_name, const godot::Variant& value)
{
   if (prop_name == godot::StringName("db_table"))
   {
      m_table = value;
      check_table_setup();
      return true;
   }
   return false;
}

bool GDDatabase::_get(const godot::StringName& prop_name, godot::Variant& out_value) const
{
   if (prop_name == godot::StringName("db_table"))
   {
      out_value = m_table;
      return true;
   }
   return false;
}


void GDDatabase::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("save", "path", "include_tables"), &GDDatabase::save);
   ClassDB::bind_method(D_METHOD("has_table", "table_name"), &GDDatabase::has_table);
   ClassDB::bind_method(D_METHOD("add_table", "table_name", "int_id", "locked_id"), &GDDatabase::add_table, DEFVAL(false));
   ClassDB::bind_method(D_METHOD("add_table_resource", "table"), &GDDatabase::add_table_res);
   ClassDB::bind_method(D_METHOD("rename_table", "from", "to"), &GDDatabase::rename_table);
   ClassDB::bind_method(D_METHOD("get_table_list"), &GDDatabase::get_table_list);
   ClassDB::bind_method(D_METHOD("get_table", "table_name"), &GDDatabase::get_table);
   ClassDB::bind_method(D_METHOD("remove_table", "table_name"), &GDDatabase::remove_table);
   ClassDB::bind_method(D_METHOD("table_has_column", "table_name", "column_title"), &GDDatabase::table_has_column);
   ClassDB::bind_method(D_METHOD("table_insert_column", "table_name", "column_title", "type", "index", "external"), &GDDatabase::table_insert_column, DEFVAL(-1), DEFVAL(""));
   ClassDB::bind_method(D_METHOD("table_rename_column", "table_name", "column_index", "new_title"), &GDDatabase::table_rename_column);
   ClassDB::bind_method(D_METHOD("table_remove_column", "table_name", "column_index"), &GDDatabase::table_remove_column);
   ClassDB::bind_method(D_METHOD("table_move_column", "table_name", "from", "to"), &GDDatabase::table_move_column);
   ClassDB::bind_method(D_METHOD("table_has_random_setup", "table_name"), &GDDatabase::table_has_random_setup);
   ClassDB::bind_method(D_METHOD("table_insert_row", "table_name", "values", "index"), &GDDatabase::table_insert_row);
   ClassDB::bind_method(D_METHOD("move_row", "table_name", "from", "to"), &GDDatabase::table_move_row);
   ClassDB::bind_method(D_METHOD("remove_row", "table_name", "index_list"), &GDDatabase::table_remove_row);
   ClassDB::bind_method(D_METHOD("table_set_row_id", "table_name", "row_index", "new_id"), &GDDatabase::table_set_row_id);
   ClassDB::bind_method(D_METHOD("table_sort_rows_by_id", "table_name", "ascending"), &GDDatabase::table_sort_rows_by_id, DEFVAL(true));
   ClassDB::bind_method(D_METHOD("table_sort_rows", "table_name", "column_index", "ascending"), &GDDatabase::table_sort_rows);
   ClassDB::bind_method(D_METHOD("table_set_cell_value", "table_name", "column_index", "row_index", "value"), &GDDatabase::table_set_cell_value);
   ClassDB::bind_method(D_METHOD("get_row_from", "table_name", "id", "expand"), &GDDatabase::get_row_from, DEFVAL(false));
   ClassDB::bind_method(D_METHOD("get_row_from_by_index", "table_name", "index", "expand"), &GDDatabase::get_row_from_by_index);
   ClassDB::bind_method(D_METHOD("randomly_pick_row", "table_name", "expand", "random_number_generator"), &GDDatabase::randomly_pick_row, DEFVAL(nullptr));
   ClassDB::bind_method(D_METHOD("is_table_referenced_by", "table_name", "other_table"), &GDDatabase::is_table_referenced_by);
   ClassDB::bind_method(D_METHOD("export_to_json", "directory", "monolithic", "indent"), &GDDatabase::export_to_json, DEFVAL(""));
}


bool GDDatabase::save(const godot::String& path, bool include_tables)
{
   godot::String p = path;
   if (p.is_empty())
   {
      p = get_path();
   }

   if (include_tables)
   {
      const godot::Array k = m_table.keys();
      const int ks = k.size();
      for (int i = 0; i < ks; i++)
      {
         godot::Ref<DBTable> table = m_table[k[i]];

         const godot::String tpath = table->get_path();
         if (tpath.begins_with("res://") && tpath.find("::") == -1)
         {
            godot::ResourceSaver::get_singleton()->save(table, tpath, godot::ResourceSaver::FLAG_REPLACE_SUBRESOURCE_PATHS);
         }
      }
   }

   if (!p.begins_with("res://") || p.find("::") != -1)
   {
      return false;
   }

   return (godot::ResourceSaver::get_singleton()->save(this, p, godot::ResourceSaver::FLAG_REPLACE_SUBRESOURCE_PATHS) == godot::OK);
}


bool GDDatabase::add_table(const godot::StringName& table_name, bool int_id, bool locked_id)
{
   if (has_table(table_name)) { return false; }

   const int id_type = int_id ? godot::Variant::INT : godot::Variant::STRING;

   godot::Ref<DBTable> table = godot::Ref<DBTable>(memnew(DBTable(id_type, table_name, locked_id)));
   
   return add_table_res(table);
}


bool GDDatabase::add_table_res(const godot::Ref<DBTable>& table)
{
   ERR_FAIL_COND_V_MSG(!table.is_valid(), false, "Cannot insert an invalid DBTable resource.");

   if (has_table(table->get_table_name())) { return false; }

   m_table[table->get_table_name()] = table;

   if (table->get_id_type() == godot::Variant::INT)
   {
      m_intid_list.append(table->get_table_name());
   }
   else
   {
      m_strid_list.append(table->get_table_name());
   }

   return true;
}


bool GDDatabase::rename_table(const godot::StringName& from, const godot::StringName& to)
{
   if (from == to) { return true; }
   if (m_table.has(to))
   {
      // Target name already exists. Bail
      return false;
   }

   godot::Ref<DBTable> table = get_table(from);
   if (!table.is_valid())
   {
      return false;
   }

   m_table[to] = table;
   m_table.erase(from);

   table->change_name(to);

   godot::PackedStringArray refs = table->get_referenced_by_list();
   const int64_t rs = refs.size();
   for (int64_t i = 0; i < rs; i++)
   {
      godot::Ref<DBTable> reftable = m_table[refs[i]];
      reftable->referenced_table_renamed(from, to);
   }

   check_table_setup();


   return true;
}


bool GDDatabase::remove_table(const godot::StringName& table_name)
{
   godot::Ref<DBTable> table = get_table(table_name);

   if (!table.is_valid())
   {
      return false;
   }

   if (table->get_referenced_by_list().size() > 0)
   {
      // This table is referenced by others. Bail because upkeep in this case isn't very fun
      return false;
   }

   if (table->get_id_type() == godot::Variant::INT)
   {
      const int idx = m_intid_list.find(table_name);
      if (idx != -1)
      {
         m_intid_list.remove_at(idx);
      }
   }
   else if (table->get_id_type() == godot::Variant::STRING)
   {
      const int idx = m_strid_list.find(table_name);
      if (idx != -1)
      {
         m_strid_list.remove_at(idx);
      }
   }

   m_table.erase(table_name);

   return true;
}


bool GDDatabase::table_has_column(const godot::StringName& table_name, const godot::String& title) const
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   return table->has_column(title);
}


int64_t GDDatabase::table_insert_column(const godot::StringName& table_name, const godot::String& column_title, DBTable::ValueType type, int index, const godot::String& external)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return -1;
   }

   if (table->has_column(column_title))
   {
      return -1;
   }

   // Assume external system will indeed be used
   godot::String ext = external;
   if (type == DBTable::VT_ExternalString || type == DBTable::VT_ExternalInteger)
   {
      if (ext.is_empty() || !m_table.has(ext))
      {
         if (ext.is_empty())
         {
            WARN_PRINT(godot::vformat("Requesting to link a table from '%s', however no external table has been specified.", table_name));
         }
         else
         {
            WARN_PRINT(godot::vformat("Requesting to link a table from '%s', however '%s' doesn't seem to exist in the Database.", table_name, ext));
         }
         return -1;
      }

      if (table->is_referenced_by(ext))
      {
         // This reference already exists
         WARN_PRINT(godot::vformat("Requesting to link a table from '%s' to '%s', however this reference already exists.", table_name, ext));
         return -1;
      }
   }
   else
   {
      ext = "";
   }

   DBTable::ColumnInfo cinfo;
   cinfo.type = type;
   cinfo.index = index;
   cinfo.external_table = ext;

   int64_t ret = table->add_column(column_title, cinfo);

   if (ret >= 0 && !ext.is_empty())
   {
      check_table_setup();
   }

   return ret;
}


bool GDDatabase::table_rename_column(const godot::StringName& table_name, int64_t column_index, const godot::String& to)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   if (table->has_column(to))
   {
      return false;
   }

   return table->rename_column(column_index, to);
}


bool GDDatabase::table_remove_column(const godot::StringName& table_name, int64_t column_index)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   DBTable::ColumnInfo cinfo;
   if (!table->fill_column_info(column_index, cinfo))
   {
      return false;
   }

   const bool ret = table->remove_column(column_index);

   if (ret && !cinfo.external_table.is_empty())
   {
      // The removed column is referencing another table. Rebuild the internal 'linking'
      check_table_setup();
   }

   return ret;
}


bool GDDatabase::table_move_column(const godot::StringName& table_name, int64_t from, int64_t to)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   return table->move_column(from, to);
}


bool GDDatabase::table_change_column_value_type(const godot::StringName& table_name, int64_t column_index, DBTable::ValueType to_type)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   return table->change_column_value_type(column_index, to_type);
}


bool GDDatabase::table_has_random_setup(const godot::StringName& table_name) const
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   return table->has_random_weight_column();
}


int64_t GDDatabase::table_insert_row(const godot::StringName& table_name, const godot::Dictionary& values, int64_t index)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return -1;
   }

   return table->add_row(values, index);
}

bool GDDatabase::table_move_row(const godot::StringName& table_name, int64_t from, int64_t to)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   return table->move_row(from, to);
}


bool GDDatabase::table_remove_row(const godot::StringName& table_name, const godot::PackedInt64Array& index_list)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   // The index list must be sorted and iterated in reverse order. To ensure it's indeed sorted, copy it then perform the operation
   godot::PackedInt64Array ilist = index_list;
   ilist.sort();

   // If another table is referencing 'table_name' then that other must be updated as some of its cell may be pointing to the
   // row(s) being removed. The way this will happen here follows:
   // - For each row to be removed, its ID will be stored within a Set
   // - Once every row is removed, use the list of tables referencing the 'table_name' and 'notify' each one that the rows within the
   //   provided Set have been removed
   // - The referencing table should then go through each cell in the corresponding column checking if the value is within the provided
   //   Set and, if so, clear the stored value.
   godot::PackedStringArray reflist = table->get_referenced_by_list();
   
   // The Set holding removed rows. Since row ID can be either String or Integer, holding IDs as godot::Variant
   DBTable::ValueSetType rem_set;

   for (int64_t i = ilist.size() - 1; i >= 0; i--)
   {
      // There is no need to fill the rem_set if there is no referencing table
      if (reflist.size() > 0)
      {
         rem_set.insert(ilist[i]);
      }

      table->remove_row_by_index(ilist[i]);
   }

   if (rem_set.size() > 0)
   {
      const int64_t rs = reflist.size();
      for (int64_t i = 0; i < rs; i++)
      {
         godot::Ref<DBTable> reftable = get_table(reflist[i]);
         reftable->referenced_rows_removed(table_name, rem_set);
      }
   }

   return true;
}


bool GDDatabase::table_set_row_id(const godot::StringName& table_name, int64_t row_index, const godot::Variant& new_id)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   // First hold current ID as it may be necessary to update referencing tables (if any)
   const godot::Variant& old_id = table->get_row_id(row_index);

   const bool ret = table->set_row_id(row_index, new_id);

   if (ret)
   {
      // A row id has changed. Must verify if there is another table that references this row ID and, if so,
      // update those references to the new value
      godot::PackedStringArray reflist = table->get_referenced_by_list();
      const int64_t rs = reflist.size();
      for (int64_t i = 0; i < rs; i++)
      {
         godot::Ref<DBTable> reftable = get_table(reflist[i]);

         if (!reftable.is_valid())
         {
            WARN_PRINT(godot::vformat("Table '%s' reports it's referenced by '%s', which was not found in the database.", table_name, reflist[i]));
            continue;
         }

         reftable->referenced_row_id_changed(table_name, old_id, new_id);
      }
   }

   return ret;
}


bool GDDatabase::table_sort_rows_by_id(const godot::StringName& table_name, bool ascending)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   table->sort_by_id(ascending);
   return true;
}


bool GDDatabase::table_sort_rows(const godot::StringName& table_name, int64_t column_index, bool ascending)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   return table->sort_by_column(column_index, ascending);
}


bool GDDatabase::table_set_cell_value(const godot::StringName& table_name, int64_t column_index, int64_t row_index, const godot::Variant& value)
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return false;
   }

   return table->set_value_by_index(column_index, row_index, value);
}



godot::Dictionary GDDatabase::get_row_from(const godot::StringName& table_name, const godot::Variant& id, bool expand) const
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return godot::Dictionary();
   }

   godot::Dictionary ret = table->get_row(id);

   if (expand)
   {
      expand_row_cell(table, ret);
   }

   return ret;
}


godot::Dictionary GDDatabase::get_row_from_by_index(const godot::StringName& table_name, int64_t index, bool expand) const
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return godot::Dictionary();
   }

   godot::Dictionary ret = table->get_row_by_index(index);

   if (expand)
   {
      expand_row_cell(table, ret);
   }

   return ret;
}


godot::Dictionary GDDatabase::randomly_pick_row(const godot::StringName& table_name, bool expand, const godot::Ref<godot::RandomNumberGenerator>& rng) const
{
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return godot::Dictionary();
   }

   godot::Dictionary ret = table->pick_random_row(rng);

   if (expand)
   {
      expand_row_cell(table, ret);
   }

   return ret;
}


bool GDDatabase::is_table_referenced_by(const godot::StringName& table_name, const godot::StringName& other_table) const
{
   ERR_FAIL_COND_V_MSG(!has_table(table_name) || !has_table(other_table), false, "While attempting to check if a table references another, both must exist in the Database.");

   godot::Ref<DBTable> table = get_table(table_name);
   return table->is_referenced_by(other_table);
}


/*void GDDatabase::merge_database(const godot::Ref<GDDatabase>& other_db, bool allow_new_tables, const godot::PackedStringArray& block_overwrite, const godot::PackedStringArray& replace_list)
{
   ERR_FAIL_COND_MSG(!other_db.is_valid(), "Attempting to merge invalid Database into current one");

   /// Something to note here. Table "linking" is data stored within the tables. What this means is that tables that reference others
   /// should not pose a problem when performing the merge. The possible situations:
   ///    First the cases in which the incoming table already exists in the Database
   /// - First check if the existing table must be entirely replaced. If so, do it. Then make sure existing referenced tables are properly set (that is,
   ///   update the lists of "referencing tables").
   /// - Existing table should not be replaced. Simply attempt to merge the values from the incoming table, properly providing the flag telling if
   ///   values can be overwritten. There is nothing else to do since linking is already done
   ///
   ///    Then the cases in which the incoming table is new.
   /// - If "allow_new_tables" is false then simply skip it. Nothing else to do!
   /// - Otherwise simply add the table. Take the reference list of the new table. There are two possibilities here:
   ///   There is a reference to an already existing table. Since the incoming is a new table, there is no chance the existing one has information that
   ///   it's being referenced, so update it. Even there, the "referenced" list is mostly used to help determine when something can be deleted or not.
   ///   There is a reference to another table that does not exist. Nothing to do because the linking is most likely already done
   const godot::Array table_list = other_db->m_table.keys();
   const int64_t tcount = table_list.size();
   for (int64_t t = 0; t < tcount; t++)
   {
      const godot::StringName tname = table_list[t];

      godot::Ref<DBTable> table = get_table(tname);
      godot::Ref<DBTable> other_table = other_db->get_table(tname);

      if (table.is_valid())
      {
         if (replace_list.has(tname))
         {
            // While incoming table already exists, it's also meant to be replaced, do it.
            if (table->get_id_type() != other_table->get_id_type())
            {
               if (table->get_id_type() == godot::Variant::INT)
               {
                  m_intid_list.erase(table->get_table_name());
                  m_strid_list.append(table->get_table_name());
               }
               else
               {
                  m_intid_list.append(table->get_table_name());
                  m_strid_list.append(table->get_table_name());
               }
            }

            m_table[tname] = other_table;

            const godot::PackedStringArray reflist = other_table->get_reference_list();
            for (const godot::String& ref_name : reflist)
            {
               godot::Ref<DBTable> referenced = get_table(ref_name);
               if (referenced.is_valid() && !referenced->is_referenced_by(tname))
               {
                  referenced->add_referencer(tname);
               }
            }
         }
         else
         {
            // Attempt to merge values from incoming table into existing one
            table->merge_from_table(other_table, !block_overwrite.has(tname));
         }
      }
      else if (allow_new_tables)
      {
         // In here incoming table does not exist and the flag to allow new tables is enabled, so insert it
         add_table_res(other_table);

         const godot::PackedStringArray reflist = other_table->get_reference_list();

         for (const godot::String& ref_name : reflist)
         {
            godot::Ref<DBTable> referenced = get_table(ref_name);
            if (referenced.is_valid())
            {
               referenced->add_referencer(tname);
            }
         }
      }
   }
}*/


void GDDatabase::export_to_json(const godot::String& directory, bool monolithic, const godot::String& indent) const
{
   const godot::Array tbkeys = m_table.keys();
   const int64_t tcount = tbkeys.size();

   godot::Ref<godot::JSON> json;
   json.instantiate();

   if (monolithic)
   {
      // In this case everything is meant to be stored in a single .json file. So the "root" element should be an array
      godot::Array data;

      for (int64_t i = 0; i < tcount; i++)
      {
         godot::Ref<DBTable> table = m_table[tbkeys[i]];

         godot::Dictionary tbl_out;
         tbl_out["table"] = table->get_table_name();

         // GDDatabase is declared as friend of DBTable. Yes, I know the line bellow is not the ideal thing, but...
         tbl_out["rows"] = table->m_row_list;

         data.append(tbl_out);
      }

      godot::String fname = get_path().get_file();
      const godot::String extension = fname.get_extension();
      fname = fname.trim_suffix(extension) + "json";
      
      godot::Ref<godot::FileAccess> file = godot::FileAccess::open(directory.path_join(fname), godot::FileAccess::WRITE);

      if (file.is_valid() && file->is_open())
      {
         file->store_string(json->stringify(data, indent, false));
         file->close();
      }
   }
   else
   {
      // Each table generates one file, which will be '<table_name>.json'.

      for (int64_t i = 0; i < tcount; i++)
      {
         godot::Ref<DBTable> table = m_table[tbkeys[i]];

         godot::Dictionary out;
         out["table"] = table->get_table_name();
         out["rows"] = table->m_row_list;

         const godot::String fname = godot::vformat("%s.json", table->get_table_name());
         godot::Ref<godot::FileAccess> file = godot::FileAccess::open(directory.path_join(fname), godot::FileAccess::WRITE);

         if (file.is_valid() && file->is_open())
         {
            file->store_string(json->stringify(out, indent, false));
            file->close();
         }

      }
   }
}


godot::Dictionary GDDatabase::get_db_info() const
{
   godot::Dictionary ret;
   godot::Array table_data;

   const godot::Array k = m_table.keys();
   const int ks = k.size();
   for (int i = 0; i < ks; i++)
   {
      godot::Ref<DBTable> table = m_table[k[i]];
      godot::String path = table->get_path();

      if (!(path.begins_with("res://") && path.find("::") == -1))
      {
         path = "embedded";
      }

      godot::Dictionary tinfo;
      tinfo["name"] = table->get_table_name();
      tinfo["path"] = path;
      tinfo["id_type"] = table->get_id_type() == godot::Variant::INT ? "Integer" : "String";
      tinfo["column_count"] = table->get_column_count();
      tinfo["row_count"] = table->get_row_count();
      tinfo["references"] = table->get_reference_list();
      tinfo["referenced_by"] = table->get_referenced_by_list();

      table_data.append(tinfo);
   }
   

   ret["database"] = get_path();
   ret["table_count"] = m_table.size();
   ret["table_data"] = table_data;

   return ret;
}


void GDDatabase::fill_external_candidates_for(int type, const godot::StringName& table_name, godot::PackedStringArray& out_list) const
{
   godot::Ref<DBTable> table = get_table(table_name);

   if (!table.is_valid())
   {
      return;
   }

   const godot::Vector<godot::StringName>* list = nullptr;
   if (type == DBTable::VT_ExternalInteger)
   {
      list = &m_intid_list;
   }
   else if (type == DBTable::VT_ExternalString)
   {
      list = &m_strid_list;
   }

   if (!list)
   {
      return;
   }

   const int size = list->size();
   for (int i = 0; i < size; i++)
   {
      const godot::StringName current = list->get(i);
      if (current == table_name)
      {
         continue;
      }

      // Only add a candidate if it does not reference the "source table" itself and source table is not already
      // referencing the other table
      if (!table->is_referenced_by(current) && !table->is_referencing(current))
      {
         out_list.append(current);
      }
   }
}

/*
   godot::Ref<DBTable> table = get_table(table_name);
   if (!table.is_valid())
   {
      return godot::Dictionary();
   }

   godot::Dictionary ret = table->get_row_by_index(index);

   if (expand)
   {
      expand_row_cell(table, ret);
   }

   return ret;
*/
godot::Dictionary GDDatabase::get_row_from_patched(const godot::Ref<DBTable>& patched, int64_t index, bool expand) const
{
   ERR_FAIL_COND_V_MSG(!patched.is_valid(), godot::Dictionary(), "Cannot retrieve data from invalid patched table.");

   godot::Dictionary ret = patched->get_row_by_index(index);

   if (expand)
   {
      expand_row_cell(patched, ret);
   }

   return ret;
}


GDDatabase::GDDatabase()
{
   m_table = godot::Dictionary();
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceFormatLoaderGDDatabase
/*godot::PackedStringArray ResourceFormatLoaderGDDatabase::_get_recognized_extensions() const
{
   godot::PackedStringArray ret;
   ret.push_back("tgddb");       // Text format
   ret.push_back("gddb");        // Binary format
   return ret;
}

bool ResourceFormatLoaderGDDatabase::_handles_type(const godot::StringName& type) const
{
   return (type == godot::StringName("GDDatabase"));
}

godot::String ResourceFormatLoaderGDDatabase::_get_resource_type(const godot::String& path) const
{
   const godot::String exl = path.get_extension().to_lower();
   if (exl == "tgddb" || exl == "gddb")
   {
      return "GDDatabase";
   }

   return "";
}

godot::Variant ResourceFormatLoaderGDDatabase::_load(const godot::String& path, const godot::String& original_path, bool use_sub_threads, int32_t cache_mode) const
{
   const godot::String exl = path.get_extension().to_lower();

   godot::UtilityFunctions::print(godot::vformat("Loading GDDatabase. Is it in binary? %s", exl == "gddb"));

   return godot::Variant();
}*/



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceFormatSaverGDDatabase
/*godot::Error ResourceFormatSaverGDDatabase::_save(const godot::Ref<godot::Resource>& resource, const godot::String& path, uint32_t flags)
{
   godot::Error err;
   godot::Ref<godot::FileAccess> file = godot::FileAccess::open(path, godot::FileAccess::WRITE);
   err = godot::FileAccess::get_open_error();
   ERR_FAIL_COND_V_MSG(err, godot::Error::ERR_CANT_OPEN, "Cannot save database file '" + path + "'.");


   // Write "header" - title
   {
      godot::String title = "[gd_resource type=\"GDDatabase\" format=1]\n";

      file->store_string(title);
   }


   return godot::OK;
}

godot::PackedStringArray ResourceFormatSaverGDDatabase::_get_recognized_extensions(const godot::Ref<godot::Resource>& resource) const
{
   godot::PackedStringArray ret;
   godot::Ref<GDDatabase> db = resource;
   if (db.is_valid())
   {
      ret.push_back("tgddb");
      ret.push_back("gddb");
   }

   return ret;
}

bool ResourceFormatSaverGDDatabase::_recognize(const godot::Ref<godot::Resource>& resource) const
{
   //return resource->_get_extension_class_name() == "GDDatabase";
   // In several core implementations the return is basically `return resource->get_class_name() == "some_dame"`
   // Unfortunately "get_class_name()" does not exist in extension space. So...
   godot::Ref<GDDatabase> db = resource;
   return db.is_valid();
}*/





#endif  //DATABASE_DISABLED 
