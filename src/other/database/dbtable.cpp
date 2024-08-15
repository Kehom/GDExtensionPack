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

#include "dbtable.h"
//#include "dbtablepatch.h"

#include <godot_cpp/classes/json.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTable::Sorter
bool DBTable::Sorter::ascending_color(const godot::Dictionary& a, const godot::Dictionary& b) const
{
   const godot::Color ca = a[column];
   const godot::Color cb = b[column];

   if (ca.get_h() < cb.get_h())
   {
      return true;
   }
   else if (ca.get_h() == cb.get_h())
   {
      if (ca.get_s() < cb.get_s())
      {
         return true;
      }
      else if (ca.get_s() == cb.get_s())
      {
         if (ca.get_v() < cb.get_v())
         {
            return true;
         }
      }
   }
   return false;
}


bool DBTable::Sorter::descending_color(const godot::Dictionary& a, const godot::Dictionary& b) const
{
   const godot::Color ca = a[column];
   const godot::Color cb = b[column];

   if (cb.get_h() < ca.get_h())
   {
      return true;
   }
   else if (cb.get_h() == ca.get_h())
   {
      if (cb.get_s() < ca.get_s())
      {
         return true;
      }
      else if (cb.get_s() == ca.get_s())
      {
         if (cb.get_v() < ca.get_v())
         {
            return true;
         }
      }
   }
   return false;
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTable
void DBTable::column_set_external(const godot::String& other, godot::Dictionary& out_column)
{
   if (other.is_empty())
   {
      if (out_column.has("extid"))
      {
         out_column.erase("extid");
      }
   }
   else
   {
      out_column["extid"] = other;
   }
}


bool DBTable::require_unique_values(const godot::Dictionary& column) const
{
   const ValueType vt = column_get_type(column);

   return (vt == VT_UniqueInteger || vt == VT_UniqueString);
}


bool DBTable::is_unique(const godot::Dictionary& column, const godot::Variant& value) const
{
#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_V_MSG(!require_unique_values(column), false, "Testing if value is unique, however relevant column does not require unique values.");
#endif

   const godot::String ctitle = column_get_title(column);

   const ValueSetType& uset = m_unique_indexing[ctitle];

   return (uset.has(value));
}


void DBTable::set_used(const godot::Dictionary& column, const godot::Variant& value)
{
#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_MSG(!require_unique_values(column), "Attempting to mark a value as 'used' in a column that does not require unique values.");
#endif

   const godot::String ctitle = column_get_title(column);

   ValueSetType& uset = m_unique_indexing[ctitle];

   uset.insert(value);
}


void DBTable::unique_changed(const godot::Dictionary& column, const godot::Variant& from_value, const godot::Variant& to_value)
{
#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_MSG(!require_unique_values(column), "Attempting to update internal state related to unique values on a table that does not require that.");
   ERR_FAIL_COND_MSG(from_value == to_value, "Changing unique value, however both old and new values are equal");
   ERR_FAIL_COND_MSG(!is_unique(column, to_value), "Changing unique value, however new one is not unique");
#endif

   CRASH_COND(!false);

   const godot::String ctitle = column_get_title(column);

   ValueSetType& uset = m_unique_indexing[ctitle];

   uset.erase(from_value);
   uset.insert(to_value);
}


void DBTable::set_auto_weight(const godot::String& title, bool is_new)
{
   if (title.is_empty())
   {
      m_rand_weight.column = "";
      m_rand_weight.acc_weight.clear();
      m_rand_weight.acc_weight.resize(0);
   }
   else
   {
      if (!m_rand_weight.column.is_empty() && is_new)
      {
         // There is already a column setup for automatic random weights. 
         return;
      }

      m_rand_weight.column = title;

      if (is_new)
      {
         const int64_t rcount = get_row_count();

         m_rand_weight.total_weight = (double)rcount;
         m_rand_weight.acc_weight.resize(rcount);

         // godot::Vector<> does not provide non const access through [] operator
         double* acc_weight = m_rand_weight.acc_weight.ptrw();

         double acc = 1.0;
         for (int64_t i = 0; i < rcount; i++)
         {
            acc_weight[i] = acc;
            acc += 1.0;
         }
      }
   }
}


void DBTable::calculate_weights()
{
   if (m_rand_weight.column.is_empty())
   {
      // Well... there is no random weight column in this table. Nothing to do here
      return;
   }

   const int64_t rcount = get_row_count();

   if (m_rand_weight.acc_weight.size() != rcount)
   {
      m_rand_weight.acc_weight.resize(rcount);
   }

   // godot::Vector<> does not provide non const access through [] operator. Obtain raw pointer to
   // allow [] usage
   double* acc_weight = m_rand_weight.acc_weight.ptrw();
   double acc = 0.0;

   for (int64_t i = 0; i < rcount; i++)
   {
      const godot::Dictionary row = m_row_list[i];

      const double w = row.get(m_rand_weight.column, 0.0);
      acc += w;

      acc_weight[i] = acc;
   }

   m_rand_weight.total_weight = acc;
}


void DBTable::build_column_index()
{
   m_column_index.clear();
   const int64_t ccount = m_column_array.size();

   for (int64_t i = 0; i < ccount; i++)
   {
      const godot::Dictionary column = m_column_array[i];
      const godot::String title = column_get_title(column);
   #ifdef DEBUG_ENABLED
      ERR_FAIL_COND_MSG(title.is_empty(), "Building column index, however retrieved a column without a title!");
   #endif
      m_column_index[title] = column;

      const ValueType type = column_get_type(column);
      if (type == VT_RandomWeight)
      {
         set_auto_weight(title, true);
      }
   }
}


void DBTable::build_unique_index()
{
   const int64_t ccount = m_column_array.size();
   for (int64_t i = 0; i < ccount; i++)
   {
      const godot::Dictionary column = m_column_array[i];
      if (!require_unique_values(column))
      {
         // This column does not require unique values, so skip it
         continue;
      }

      const godot::String title = column_get_title(column);
   #ifdef DEBUG_ENABLED
      ERR_FAIL_COND_MSG(title.is_empty(), "Building unique values indexing, however retrieved a column without a title!");
   #endif

      if (!m_unique_indexing.has(title))
      {
         m_unique_indexing[title] = ValueSetType();
      }

      ValueSetType& uset = m_unique_indexing[title];
      uset.clear();

      const int64_t rcount = get_row_count();
      for (int64_t i = 0; i < rcount; i++)
      {
         const godot::Dictionary row = m_row_list[i];
         const godot::Variant value = row.get(title, nullptr);
         uset.insert(value);
      }
   }
}


void DBTable::build_row_index()
{
   m_row_index.clear();
   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      const godot::Dictionary row = m_row_list[i];
      const godot::Variant id = row.get("id", nullptr);
   #ifdef DEBUG_ENABLED
      ERR_FAIL_COND_MSG(id.get_type() == godot::Variant::NIL, "Building row index, however found a row without ID");
      ERR_FAIL_COND_MSG(id.get_type() != m_id_type, "Building row indexing, however got row ID with type that doesn't match that of the table");
   #endif
      m_row_index[id] = row;
   }
}


godot::String DBTable::make_random_string(int32_t length)
{
   // Code in here is based on the algorithm found at 'core/ui/resource_uid.cpp' in the Core Godot Engine source
   static const char chars[] = "abcdefghijklmnopqrstuvwxyz0123456789";
   static const uint32_t base = sizeof(chars) / sizeof(chars[0]);

   bool done = false;
   godot::String txt;

   while (txt.length() < length)
   {
      uint32_t id = m_rand_gen->randi();

      while (id && txt.length() < length)
      {
         uint32_t c = id % base;
         txt = godot::String::chr(chars[c]) + txt;

         id /= base;
      }
   }

   return txt;
}


// For the next two functions, it would be possible to use godot::Crypto to generate the IDs. However those
// values are meant to be used as row IDs, unique only to this table. Absolutely no need for cryptography strength.
// So relying on the "ordinary" random number generator. It's good enough for this purpose anyway.
int64_t DBTable::generate_int_id()
{
   int64_t ret = m_rand_gen->randi();

   while (ret < 0 || m_row_index.has(ret))
   {
      ret = m_rand_gen->randi();
   }

   return ret;
}


godot::String DBTable::generate_str_id()
{
   godot::String ret = make_random_string(16);
   while (m_row_index.has(ret))
   {
      ret = make_random_string(16);
   }

   return ret;
}


void DBTable::add_row(const godot::Variant& id, const godot::Dictionary& values, int64_t index)
{
   if (index < 0 || index > m_row_list.size())
   {
      index = m_row_list.size();
   }

   godot::Dictionary row;
   row["id"] = id;

   // Assume there is a column with random weights and the new row does not contain a value for it
   double rweight = 1.0;

   const int64_t ccount = m_column_array.size();
   for (int64_t i = 0; i < ccount; i++)
   {
      const godot::Dictionary column = m_column_array[i];
      const godot::String title = column_get_title(column);

   #ifdef DEBUG_ENABLED
      ERR_FAIL_COND_MSG(title.is_empty(), "While inserting a new row, retrieved a column without a title!");
   #endif

      godot::Variant value = values.get(title, nullptr);

      if (value.get_type() == godot::Variant::NIL)
      {
         value = get_default_value(column);
      }
      else
      {
         if (require_unique_values(column) && !is_unique(column, value))
         {
            /// NOTE: Should a warning be pushed instead of silently generating a valid unique value?
            value = get_default_value(column);
         }
      }

      row[title] = value;

      if (title == m_rand_weight.column)
      {
         rweight = value;
      }
   }

   if (index < m_row_list.size())
   {
      m_row_list.insert(index, row);

      // The row has been inserted, which basically affects all subsequent accumulated weights (if any).
      // Rebuild everything (although it would be better to recalculated only the ones from this new row
      // and subsequent ones)
      calculate_weights();
   }
   else
   {
      m_row_list.append(row);
      
      // Since the row has been appended, updating the weights is simpler - if there are any weights that is
      if (!m_rand_weight.column.is_empty())
      {
         const double nacc = m_rand_weight.total_weight + rweight;

         m_rand_weight.acc_weight.append(nacc);

         m_rand_weight.total_weight = nacc;
      }
   }

   m_row_index[id] = row;
}


godot::Variant DBTable::get_default_value(const godot::Dictionary& column)
{
   godot::Variant ret;

   const ValueType type = column_get_type(column);
   const godot::String title = column_get_title(column);

#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_V_MSG(type == VT_Invalid, nullptr, "Attempting to get default value for a column, however its value type is invalid");
   ERR_FAIL_COND_V_MSG(title.is_empty(), nullptr, "Attempting to get default value for a column, however it does not have a title");
#endif

   switch (type)
   {
      case VT_UniqueInteger:
      {
         //godot::HashSet<godot::Variant>& uset = m_unique_indexing[title];
         ValueSetType& uset = m_unique_indexing[title];

         // Seek an available value by incrementing. A random number might probably be more efficient when the amount
         // of rows is big. Perhaps use some sort of threshold? If bellow it iterate otherwise pick a random number?
         int64_t attempt = 0;


         while (uset.has(attempt) && attempt >= 0)
         {
            attempt++;
         }

         if (attempt >= 0)
         {
            ret = attempt;
         }
      } break;

      case VT_UniqueString:
      {
         ValueSetType& uset = m_unique_indexing[title];

         int i = 0;
         godot::String rstr = make_random_string(16);
         godot::String attempt = godot::vformat("%s-%02d-%s", title, i, rstr);
         
         bool done = !uset.has(attempt);

         while (!done)
         {
            i = 0;

            if (uset.has(attempt))
            {
               while (i < 100)
               {
                  i++;
                  attempt = godot::vformat("%s-%02d-%s", title, i, rstr);
               }
            }

            done = !uset.has(attempt);

            if (!done)
            {
               // If here, previous random string has been used 100 times!
               // Generate another string and attempt again.
               rstr = make_random_string(16);
            }
         }

         ret = attempt;
      } break;

      case VT_ExternalString:
      case VT_String:
      case VT_Texture:
      case VT_Audio:
      case VT_GenericRes:
      case VT_MultiLineString:
      {
         // Texture, Audio and GenericResource are stored as paths to their specific files. In other words, strings
         ret = godot::String("");
      } break;

      case VT_ExternalInteger:
      {
         ret = (int64_t)-1;
      } break;

      case VT_RandomWeight:
      {
         ret = 1.0f;
      } break;

      case VT_Bool:
      {
         // The "cast" here is almost certainly not needed, however this is just to make absolutely sure Variant::BOOL is used
         ret = (bool)false;
      } break;

      case VT_Integer:
      {
         ret = (int)0;
      } break;

      case VT_Float:
      {
         ret = 0.0;
      } break;

      case VT_Color:
      {
         ret = godot::Color(0.0, 0.0, 0.0, 1.0);
      } break;
      
      case VT_StringArray:
      case VT_TextureArray:
      case VT_AudioArray:
      case VT_GenericResArray:
      {
         ret = godot::PackedStringArray();
      } break;

      case VT_IntegerArray:
      {
         ret = godot::PackedInt64Array();
      } break;

      case VT_FloatArray:
      {
         ret = godot::PackedFloat64Array();
      } break;

      case VT_ColorArray:
      {
         ret = godot::PackedColorArray();
      } break;


      default:
      {
         ret = "-";
      } break;
   }

   return ret;
}


godot::String DBTable::convert_to_string(const godot::Variant& value) const
{
   return godot::String(value);
}


bool DBTable::convert_to_bool(const godot::Variant& value) const
{
   bool ret = false;
   switch (value.get_type())
   {
      case godot::Variant::BOOL:
      {
         ret = value;
      } break;
      case godot::Variant::STRING:
      {
         godot::String val = value;
         val = val.to_lower();

         /// FIXME: Use localization system to properly deal with other languages
         if (val == "true" || val == "yes" || val == "enabled")
         {
            ret = true;
         }
      } break;
      case godot::Variant::INT:
      {
         const int val = value;
         ret = val > 0;
      } break;
   }

   return ret;
}


int64_t DBTable::convert_to_int(const godot::Variant& value) const
{
   int64_t ret = 0;

   switch (value.get_type())
   {
      case godot::Variant::INT:
      {
         ret = value;
      } break;

      case godot::Variant::FLOAT:
      {
         const double val = value;
         ret = (int64_t)godot::Math::round(val);
      } break;

      case godot::Variant::STRING:
      {
         const godot::String val = value;
         if (val.is_valid_int())
         {
            ret = val.to_int();
         }
         else if (val.is_valid_float())
         {
            const double fval = val.to_float();
            ret = (int64_t)godot::Math::round(fval);
         }
      } break;
   }

   return ret;
}


double DBTable::convert_to_double(const godot::Variant& value) const
{
   double ret = 0.0;

   switch (value.get_type())
   {
      case godot::Variant::FLOAT:
      case godot::Variant::INT:
      {
         ret = value;
      } break;

      case godot::Variant::STRING:
      {
         const godot::String strval = value;
         if (strval.is_valid_int())
         {
            ret = strval.to_int();
         }
         else if (strval.is_valid_float())
         {
            ret = strval.to_float();
         }
      } break;
   }

   return ret;
}


godot::String DBTable::convert_to_path(const godot::Variant& value) const
{
   if (value.get_type() == godot::Variant::STRING)
   {
      const godot::String strval = value;

      if (strval.is_absolute_path())
      {
         // The value is a String and a path. It may not be pointing to something valid, but this is still a path
         return strval;
      }
   }

   return "";
}


godot::Color DBTable::convert_to_color(const godot::Variant& value) const
{
   godot::Color ret = godot::Color(0.0, 0.0, 0.0, 1.0);

   if (value.get_type() == godot::Variant::STRING)
   {
      const godot::String strval = value;
      ret = godot::Color::from_string(strval, godot::Color(0.0, 0.0, 0.0, 1.0));
   }
   else if (value.get_type() == godot::Variant::COLOR)
   {
      // Ideally should never come here - but this reduces chances of error
      ret = value;
   }

   return ret;
}


void DBTable::convert_value_type(const godot::String& column_title, ValueType to_type)
{
   /// NOTE: In here not using any of the Unique* types because those are not meant to be changed after creation
   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      godot::Dictionary row = m_row_list[i];
      godot::Variant value = row[column_title];
      if (value.get_type() == godot::Variant::NIL)
      {
         continue;
      }

      /// FIXME: Add the various "array" types
      switch (to_type)
      {
         case VT_String:
         {
            row[column_title] = convert_to_string(value);
         } break;

         case VT_Bool:
         {
            row[column_title] = convert_to_bool(value);
         } break;

         case VT_Integer:
         {
            row[column_title] = convert_to_int(value);
         } break;

         case VT_Float:
         {
            row[column_title] = convert_to_double(value);
         } break;

         case VT_Texture:
         case VT_Audio:
         case VT_GenericRes:
         {
            row[column_title] = convert_to_path(value);
         } break;

         case VT_Color:
         {
            row[column_title] = convert_to_color(value);
         }
      }
   }
}


godot::Dictionary DBTable::make_dictionary_ui_entry(ValueType type, const godot::String& ui)
{
   godot::Dictionary ret;
   ret["code"] = type;
   ret["ui"] = ui;
   return ret;
}


void DBTable::referenced_table_renamed(const godot::String& from, const godot::String& to)
{
   // REMEMBER: The m_ref_table holds referenced table name as key and the referencing column name as value
   const godot::String column_name = m_ref_table.get(from, "");
   if (column_name.is_empty())
   {
      return;
   }

   ColumnIndexType::Iterator column_it = m_column_index.find(column_name);

   if (column_it == m_column_index.end())
   {
      return;
   }

   m_ref_table[to] = column_name;
   m_ref_table.erase(from);
   
   column_it->value["extid"] = to;
}


void DBTable::referenced_row_id_changed(const godot::String& other_table, const godot::Variant& id_from, const godot::Variant& id_to)
{
   // Column referencing the other table is stored within the m_reftable container, keyed by the other table's name
   const godot::String column_name = m_ref_table.get(other_table, "");
   if (column_name.is_empty())
   {
      return;
   }

   // Go through all cells and update the changed ID
   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      godot::Dictionary row = m_row_list[i];
      if (row[column_name] == id_from)
      {
         row[column_name] = id_to;
      }
   }
}


void DBTable::referenced_rows_removed(const godot::String& other_table, const ValueSetType& id_list)
{
   // Column referencing the other table is stored within the m_ref_table container, keyed by the other table's name
   const godot::String column_name = m_ref_table.get(other_table, "");
   if (column_name.is_empty())
   {
      return;
   }

   godot::Dictionary column = m_column_index[column_name];
   const godot::Variant def_val = get_default_value(column);

   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      godot::Dictionary row = m_row_list[i];
      const godot::Variant stored = row.get(column_name, nullptr);

      if (id_list.has(stored))
      {
         row[column_name] = def_val;
      }
   }
}


void DBTable::_get_property_list(godot::List<godot::PropertyInfo>* out_list) const
{
   using namespace godot;

   out_list->push_back(PropertyInfo(Variant::STRING, "table_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   out_list->push_back(PropertyInfo(Variant::STRING, "table_name", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   out_list->push_back(PropertyInfo(Variant::INT, "id_type", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   out_list->push_back(PropertyInfo(Variant::BOOL, "locked_row_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   out_list->push_back(PropertyInfo(Variant::ARRAY, "column", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   out_list->push_back(PropertyInfo(Variant::DICTIONARY, "ref_table", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   out_list->push_back(PropertyInfo(Variant::ARRAY, "row_list", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
}


bool DBTable::_set(const godot::StringName& prop_name, const godot::Variant& value)
{
   bool ret = true;

   if (prop_name == godot::StringName("table_id"))
   {
      m_table_id = value;
   }
   else if (prop_name == godot::StringName("table_name"))
   {
      m_table_name = value;
   }
   else if (prop_name == godot::StringName("id_type"))
   {
      m_id_type = value;
   }
   else if (prop_name == godot::StringName("locked_row_id"))
   {
      m_locked_id = value;
   }
   else if (prop_name == godot::StringName("column"))
   {
      m_column_array = value;
      build_column_index();
   }
   else if (prop_name == godot::StringName("ref_table"))
   {
      m_ref_table = value;
   }
   else if (prop_name == godot::StringName("row_list"))
   {
      m_row_list = value;
      build_unique_index();
      build_row_index();
      calculate_weights();
   }
   else
   {
      ret = false;
   }

   return ret;
}


bool DBTable::_get(const godot::StringName& prop_name, godot::Variant& out_value) const
{
   bool ret = true;

   if (prop_name == godot::StringName("table_id"))
   {
      out_value = m_table_id;
   }
   else if (prop_name == godot::StringName("table_name"))
   {
      out_value = m_table_name;
   }
   else if (prop_name == godot::StringName("id_type"))
   {
      out_value = m_id_type;
   }
   else if (prop_name == godot::StringName("locked_row_id"))
   {
      out_value = m_locked_id;
   }
   else if (prop_name == godot::StringName("column"))
   {
      out_value = m_column_array;
   }
   else if (prop_name == godot::StringName("ref_table"))
   {
      out_value = m_ref_table;
   }
   else if (prop_name == godot::StringName("row_list"))
   {
      out_value = m_row_list;
   }
   else
   {
      ret = false;
   }


   return ret;
}


void DBTable::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("get_table_id"), &DBTable::get_table_id);
   ClassDB::bind_method(D_METHOD("get_table_name"), &DBTable::get_table_name);
   ClassDB::bind_method(D_METHOD("get_id_type"), &DBTable::get_id_type);
   ClassDB::bind_method(D_METHOD("is_row_id_locked"), &DBTable::is_row_id_locked);
   ClassDB::bind_method(D_METHOD("get_column_count"), &DBTable::get_column_count);
   ClassDB::bind_method(D_METHOD("get_column_by_index", "index"), &DBTable::get_column_by_index);
   ClassDB::bind_method(D_METHOD("get_column_by_title", "title"), &DBTable::get_column_by_title);
   ClassDB::bind_method(D_METHOD("get_column_title", "column_index"), &DBTable::get_column_title);
   ClassDB::bind_method(D_METHOD("get_column_value_type", "column_title"), &DBTable::get_column_value_type);
   ClassDB::bind_method(D_METHOD("get_row_count"), &DBTable::get_row_count);
   ClassDB::bind_method(D_METHOD("has_column", "title"), &DBTable::has_column);
   ClassDB::bind_method(D_METHOD("has_random_weight_column"), &DBTable::has_random_weight_column);
   ClassDB::bind_method(D_METHOD("get_row", "row_id"), &DBTable::get_row);
   ClassDB::bind_method(D_METHOD("get_row_by_index", "row_index"), &DBTable::get_row_by_index);
   ClassDB::bind_method(D_METHOD("get_cell_value", "row_id", "column_title"), &DBTable::get_cell_value);
   ClassDB::bind_method(D_METHOD("pick_random_row", "random_number_generator"), &DBTable::pick_random_row, DEFVAL(nullptr));
   ClassDB::bind_method(D_METHOD("get_random_weight_column_title"), &DBTable::get_random_weight_column_title);
   ClassDB::bind_method(D_METHOD("get_row_accumulated_weight", "row_index"), &DBTable::get_row_accumulated_weight);
   ClassDB::bind_method(D_METHOD("get_row_probability", "row_index"), &DBTable::get_row_probability);
   ClassDB::bind_method(D_METHOD("get_total_weight_sum"), &DBTable::get_total_weight_sum);
   ClassDB::bind_method(D_METHOD("get_reference_list"), &DBTable::get_reference_list);
   ClassDB::bind_method(D_METHOD("get_referenced_by_list"), &DBTable::get_referenced_by_list);
   ClassDB::bind_method(D_METHOD("is_referencing", "other_table_name"), &DBTable::is_referencing);
   ClassDB::bind_method(D_METHOD("is_referenced_by", "other_table_name"), &DBTable::is_referenced_by);
   //ClassDB::bind_method(D_METHOD("merge_from_table", "other_table", "allow_overwrite"), &DBTable::merge_from_table);
   ClassDB::bind_method(D_METHOD("get_data_as_json", "indent"), &DBTable::get_data_as_json, DEFVAL(""));


   BIND_ENUM_CONSTANT(VT_UniqueString);
   BIND_ENUM_CONSTANT(VT_UniqueInteger);

   BIND_ENUM_CONSTANT(VT_ExternalString);
   BIND_ENUM_CONSTANT(VT_ExternalInteger);
   //BIND_ENUM_CONSTANT(VT_ExtStringArray);
   //BIND_ENUM_CONSTANT(VT_ExtIntArray);

   BIND_ENUM_CONSTANT(VT_RandomWeight);

   BIND_ENUM_CONSTANT(VT_String);
   BIND_ENUM_CONSTANT(VT_Bool);
   BIND_ENUM_CONSTANT(VT_Integer);
   BIND_ENUM_CONSTANT(VT_Float);
   BIND_ENUM_CONSTANT(VT_Texture);
   BIND_ENUM_CONSTANT(VT_Audio);
   BIND_ENUM_CONSTANT(VT_GenericRes);
   BIND_ENUM_CONSTANT(VT_Color);
   BIND_ENUM_CONSTANT(VT_MultiLineString);

   BIND_ENUM_CONSTANT(VT_StringArray);
   BIND_ENUM_CONSTANT(VT_IntegerArray);
   BIND_ENUM_CONSTANT(VT_FloatArray);
   BIND_ENUM_CONSTANT(VT_TextureArray);
   BIND_ENUM_CONSTANT(VT_AudioArray);
   BIND_ENUM_CONSTANT(VT_GenericResArray);
   BIND_ENUM_CONSTANT(VT_ColorArray);
}


godot::Dictionary DBTable::get_column_by_index(int64_t index) const
{
   godot::Dictionary ret = godot::Dictionary();

   if (index < 0 || index >= m_column_array.size())
   {
      // Invalid index, so return empty "column"
      return ret;
   }

   // While it would be possible to directly return the stored Dictionary representing the column, Dictionaries
   // are always passed by reference. To avoid having problems, copy the values into another dictionary
   const godot::Dictionary column = m_column_array[index];

   ret["title"] = column_get_title(column);
   ret["value_type"] = column_get_type(column);
   ret["extid"] = column_get_external(column);

   return ret;
}


godot::Dictionary DBTable::get_column_by_title(const godot::String& title) const
{
   godot::Dictionary ret = godot::Dictionary();

   ColumnIndexType::ConstIterator iter = m_column_index.find(title);
   if (iter != m_column_index.end())
   {
      const godot::Dictionary column = iter->value;

      ret["title"] = column_get_title(column);
      ret["value_type"] = column_get_type(column);
      ret["extid"] = column_get_external(column);
   }

   return ret;
}


godot::String DBTable::get_column_title(int64_t index) const
{
   if (index < 0 || index >= m_column_array.size())
   {
      return "";
   }

   const godot::Dictionary column = m_column_array[index];

   return column_get_title(column);
}


DBTable::ValueType DBTable::get_column_value_type(const godot::String& column_title) const
{
   ColumnIndexType::ConstIterator iter = m_column_index.find(column_title);
   if (iter == m_column_index.end())
   {
      return VT_Invalid;
   }

   return column_get_type(iter->value);
}


godot::Dictionary DBTable::get_row(const godot::Variant& id) const
{
   godot::Dictionary ret;

   // Because this function is exposed, this verification can't be removed from release builds
   ERR_FAIL_COND_V_MSG(id.get_type() != m_id_type, ret, godot::vformat("Attempting to retrieve row from table '%s', however ID types don't match.", m_table_name));

   // While it would be possible to directly return the stored row, Dictionaries are always passed as references.
   // To prevent possible errors due to external modifications, copy the row values into the 'ret' Dictionary
   RowIndexType::ConstIterator iter = m_row_index.find(id);
   if (iter != m_row_index.end())
   {
      const godot::Dictionary row = iter->value;
      ret = row.duplicate();
   }

   return ret;
}


godot::Dictionary DBTable::get_row_by_index(int64_t index) const
{
   godot::Dictionary ret;

   if (index < 0 || index >= m_row_list.size())
   {
      return ret;
   }

   // While it would be possible to directly return the stored row, Dictionaries are always passed as references.
   // To prevent possible errors due to external modifications, copy the row values into the 'ret' Dictionary
   const godot::Dictionary row = m_row_list[index];
   ret = row.duplicate();

   return ret;
}


godot::Variant DBTable::get_cell_value(const godot::Variant& id, const godot::String& column_title) const
{
   godot::Variant ret;

   ERR_FAIL_COND_V_MSG(id.get_type() != m_id_type, ret, godot::vformat("Attempting to retrieve cell from table '%s', column '%s', however provided ID type don't match the one of the table.", m_table_name, column_title));

   // While it would be possible to directly return the stored row, Dictionaries are always passed as references.
   // To prevent possible errors due to external modifications, copy the row values into the 'ret' Dictionary
   RowIndexType::ConstIterator iter = m_row_index.find(id);
   if (iter != m_row_index.end())
   {
      const godot::Dictionary row = iter->value;
      ret = row.get(column_title, nullptr);
   }

   return ret;
}


godot::Dictionary DBTable::pick_random_row(const godot::Ref<godot::RandomNumberGenerator>& rng) const
{
   godot::Dictionary ret;
   godot::Ref<godot::RandomNumberGenerator> randgen = rng.is_valid() ? rng : m_rand_gen;

   if (m_rand_weight.column.is_empty())
   {
      // No random weight system set. Pick a random index instead
      const int64_t index = randgen->randi_range(0, m_row_list.size());
      const godot::Dictionary row = m_row_list[index];
      ret = row.duplicate();
   }
   else
   {
      const double roll = randgen->randf_range(0.0, m_rand_weight.total_weight);

      const int64_t rcount = get_row_count();
      for (int64_t i = 0; i < rcount; i++)
      {
         const double acc = m_rand_weight.acc_weight[i];
         if (acc > roll)
         {
            const godot::Dictionary row = m_row_list[i];
            ret = row.duplicate();
            break;
         }
      }
   }

   return ret;
}


double DBTable::get_row_accumulated_weight(int64_t row_index) const
{
   if (row_index < 0 || row_index >= m_row_list.size() || m_rand_weight.acc_weight.size() == 0)
   {
      return -1.0;
   }

   return m_rand_weight.acc_weight[row_index];
}


double DBTable::get_row_probability(int64_t row_index) const
{
   if (row_index < 0 || row_index >= m_row_list.size() || m_rand_weight.acc_weight.size() == 0)
   {
      return -1.0;
   }

   const godot::Dictionary row = m_row_list[row_index];
   const double w = row.get(m_rand_weight.column, -1.0);

   if (w >= 0.0)
   {
      return w / m_rand_weight.total_weight;
   }

   return -1.0;
}


godot::PackedStringArray DBTable::get_referenced_by_list() const
{
   godot::PackedStringArray ret;

   for (godot::HashSet<godot::StringName>::Iterator iter = m_referencer.begin(); iter != m_referencer.end(); ++iter)
   {
      ret.push_back(*iter);
   }

   return ret;
}


//godot::Ref<DBTablePatch> DBTable::create_table_patch() const
//{
//   godot::Ref<DBTablePatch> ret = memnew(DBTablePatch(m_table_id));
//   return ret;
//}


//void DBTable::clean_patch(godot::Ref<DBTablePatch>& table_patch) const
//{

//}

   
//void DBTable::apply_patch(const godot::Ref<DBTablePatch>& table_patch)
//{
//   ERR_FAIL_COND_MSG(!table_patch.is_valid(), "Incoming table patch must be valid in order for it to be applied into the table.");

   // Table patch contains three containers:
   // - m_delta_mod -> holds rows that should already exist but are changed
   // - m_delta_pos -> holds new rows
   // - m_delta_neg -> holds rows that must be removed. This is a dictionary used as a Set, so values are irrelevant.
   // If the patch contains conflicting data, then the order in which those containers are processed *might* becomes important.
   // This means that ideally conflicting data should be removed from the patch
   // Also, the DBTablePatch has set DBTable as a friend class, so internal data can be directly accessed.

   // First remove rows
//   {
//      const int64_t count = table_patch->m_delta_neg.size();
//      for (int64_t r = 0; r < count; r++)
//      {
//         remove_row(table_patch->m_delta_neg[r]);
//      }
//   }

   // Build a temporary list of columns to more easily iterate through them when dealing with delta_mod
//   godot::Vector<godot::String> column_vec;
//   {
//      const int64_t ccount = m_column_array.size();
//      for (int64_t c = 0; c < ccount; c++)
//      {
//         const godot::Dictionary column = m_column_array[c];
//         const godot::String col_title = column_get_title(column);
//         column_vec.append(col_title);
//      }
//   }


   // Then modify
//   {
//      const int64_t rcount = table_patch->m_delta_mod.size();
//      const godot::Array rlist = table_patch->m_delta_mod.values();
//      for (int64_t r = 0; r < rcount; r++)
//      {
//         const godot::Dictionary prow = rlist[r];
//         const godot::Variant rid = prow.get("id", nullptr);

//         RowIndexType::Iterator riter = m_row_index.find(rid);
//         if (riter == m_row_index.end())
//         {
//            // Patch indicates a row to be modified, but table does not contain that row!
//            // Perhaps collect a list of warnings?
//            continue;
//         }

//         for (const godot::String& col_title : column_vec)
//         {
//            riter->value[col_title] = prow.get(col_title, "");
//         }
//      }
//   }

   // Finally add rows
//   {
//      const int64_t rcount = table_patch->m_delta_pos.size();
//      const godot::Array id_list = table_patch->m_delta_pos.keys();
//      for (int64_t r = 0; r < rcount; r++)
//      {
//         const godot::Variant id = id_list[r];
//         if (m_row_index.has(id))
//         {
            // This row is supposed to be new, not a modification!
            // Perhaps collect a list of warnings?
//            continue;
//         }

//         const godot::Dictionary info = table_patch->m_delta_pos[id];
//         const godot::Dictionary prow_data = info.get("row_data", godot::Dictionary());
//         const int64_t index = info.get("index", -1);

//         if (prow_data.is_empty())
//         {
            // Invalid row data!
            // Perhaps collect a list of warnings?
//            continue;
//         }
         
//         add_row(prow_data, index);
//      }
//   }
//}


godot::String DBTable::get_data_as_json(const godot::String& indent) const
{
   godot::Ref<godot::JSON> json;
   json.instantiate();

   return json->stringify(m_row_list, indent, false);
}


bool DBTable::fill_column_info(int64_t column_index, ColumnInfo& out_info) const
{
   if (column_index < 0 || column_index >= m_column_array.size())
   {
      return false;
   }

   const godot::Dictionary column = m_column_array[column_index];

   out_info.index = column_index;
   out_info.external_table = column_get_external(column);
   out_info.type = column_get_type(column);

   return true;
}


int64_t DBTable::add_column(const godot::String& title, const ColumnInfo& settings)
{
   if (title.to_lower() == "id")
   {
      // Do not allow any column title that resembles "ID". This is an "automatic column" that will be handled by the table
      // and owning database.
      return -1;
   }


   if (title.is_empty() || m_column_index.has(title))
   {
      return -1;
   }

   if (settings.type == VT_RandomWeight && has_random_weight_column())
   {
      // Only one random weight column per table is allowed
      return -1;
   }


   godot::String ext = "";
   if (settings.type == VT_ExternalInteger || settings.type == VT_ExternalString)
   {
      m_ref_table[settings.external_table] = title;
      ext = settings.external_table;
   }

   const int64_t index = (settings.index < 0 || settings.index >= m_column_array.size()) ? m_column_array.size() : settings.index;

   godot::Dictionary column;
   column_set_title(title, column);
   column_set_type(settings.type, column);
   column_set_external(ext, column);

   if (index < m_column_array.size())
   {
      m_column_array.insert(index, column);
   }
   else
   {
      m_column_array.append(column);
   }

   m_column_index[title] = column;

   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      const godot::Variant defval = get_default_value(column);
      godot::Dictionary row = m_row_list[i];
      row[title] = defval;

      if (require_unique_values(column))
      {
         set_used(column, defval);
      }
   }

   if (settings.type == VT_RandomWeight)
   {
      set_auto_weight(title, true);
   }
   
   return index;
}


bool DBTable::remove_column(int64_t column_index)
{
   if (column_index < 0 || column_index >= m_column_array.size())
   {
      return false;
   }

   godot::Dictionary column = m_column_array[column_index];
   const godot::String title = column_get_title(column);

   m_column_index.erase(title);
   m_column_array.remove_at(column_index);

   if (require_unique_values(column))
   {
      m_unique_indexing.erase(title);
   }

   const godot::String ext = column_get_external(column);
   if (!ext.is_empty())
   {
      m_ref_table.erase(ext);
   }

   if (m_rand_weight.column == title)
   {
      set_auto_weight("", false);
   }

   return true;
}


bool DBTable::rename_column(int64_t column_index, const godot::String& new_title)
{
   if (column_index < 0 || column_index >= m_column_array.size())
   {
      return false;
   }

   godot::Dictionary column = m_column_array[column_index];
   const godot::String old_name = column_get_title(column);

   if (old_name == new_title)
   {
      return false;
   }

   // Create the correct entry within the column indexing
   m_column_index[new_title] = column;

   // Remove the old entry
   m_column_index.erase(old_name);

   // The column itself must be updated
   column_set_title(new_title, column);

   // Must remove this column from each existing row
   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      godot::Dictionary row = m_row_list[i];
      row[new_title] = row[old_name];
      row.erase(old_name);
   }

   // Finally check if this column references another table. If that's the case must update the reference table list
   const godot::String ext = column_get_external(column);
   if (!ext.is_empty())
   {
      // Remember, the reference table holds the referenced table as key and the referencing column as value
      m_ref_table[ext] = new_title;
   }

   // If this column is the auto random weight, then must update the internal data related to it
   if (m_rand_weight.column == old_name)
   {
      m_rand_weight.column = new_title;
   }

   return true;
}


bool DBTable::move_column(int64_t from, int64_t to)
{
   if (from == to)
   {
      // Both indices are equal. Bail as there is nothing to do here
      return false;
   }

   if (from < 0 || from >= m_column_array.size())
   {
      // The original index is invalid. Bail
      return false;
   }
   if (to < 0 || to >= m_column_array.size())
   {
      // The destination index is invalid. Bail
      return false;
   }

   godot::Dictionary column = m_column_array[from];
   m_column_array.remove_at(from);

   if (to == m_column_array.size())
   {
      // Because the "from" was temporarily removed from the array it is possible the "to" is now pointing
      // past the array boundaries. Just append the column back in
      m_column_array.append(column);
   }
   else
   {
      m_column_array.insert(to, column);
   }

   // Because dictionaries are always passed by reference there is no need to update the column indexing since
   // it's directly points into the column dictionaries
   return true;
}


bool DBTable::change_column_value_type(int64_t column_index, ValueType to_type)
{
   if (column_index < 0 || column_index >= m_column_array.size())
   {
      return false;
   }

   godot::Dictionary column = m_column_array[column_index];
   const ValueType type = column_get_type(column);
   const godot::String title = column_get_title(column);

#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_V_MSG(type == VT_Invalid, false, "Attempting to change column value type but stored one is invalid");
   ERR_FAIL_COND_V_MSG(title.is_empty(), false, "Attempting to change column value type but stored name is invalid");
#endif

   if (type < 1000)
   {
      // 1000 is the first "ID" of value types that can be changed. Don't change anything that is bellow it
      return false;
   }

   column_set_type(to_type, column);
   convert_value_type(title, to_type);

   return true;
}


void DBTable::get_column_order(godot::PackedStringArray& out_array) const
{
   const int64_t ccount = m_column_array.size();
   for (int64_t i = 0; i < ccount; i++)
   {
      godot::Dictionary column = m_column_array[i];

      out_array.append(column_get_title(column));
   }
}


int64_t DBTable::add_row(const godot::Dictionary& values, int64_t index)
{
   if (index < 0 || index >= m_row_list.size())
   {
      // Incoming desired index is out of bounds. Setup so new row will be appended
      index = m_row_list.size();
   }

   godot::Variant nid;
   if (m_id_type == godot::Variant::INT)
   {
      nid = generate_int_id();
   }
   else if (m_id_type == godot::Variant::STRING)
   {
      nid = generate_str_id();
   }

   if (nid.get_type() != godot::Variant::INT && nid.get_type() != godot::Variant::STRING) { return -1; }

   add_row(nid, values, index);

   return index;
}


void DBTable::remove_row(const godot::Variant row_id)
{
   ERR_FAIL_COND_MSG(row_id.get_type() != get_id_type(), "Attempting to remove a row, but incoming ID type does not match that of the table.");

   RowIndexType::Iterator iter = m_row_index.find(row_id);
   if (iter == m_row_index.end())
   {
      // Perhaps an error message here?
      return;
   }

   const godot::Dictionary row = iter->value;
   m_row_list.erase(row);
   m_row_index.erase(row_id);

   godot::HashMap<godot::StringName, ValueSetType>::Iterator unique_iter;
   for (unique_iter = m_unique_indexing.begin(); unique_iter != m_unique_indexing.end(); ++unique_iter)
   {
      // iter->key is the Title of the column that holds unique values.
      // iter->value holds the Set that contains the list of values of the column
      // Must remove from the Set the value that has been removed
      const godot::Variant value = row.get(unique_iter->key, nullptr);
      unique_iter->value.erase(value);
   }

   calculate_weights();
}


void DBTable::remove_row_by_index(int64_t index)
{
   if (index < 0 || index >= m_row_list.size())
   {
      return;
   }

   godot::Dictionary row = m_row_list[index];
   const godot::Variant id = row.get("id", nullptr);

#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_MSG(id.get_type() == godot::Variant::NIL, "Attempting to remove a row, but its ID is invalid");
   ERR_FAIL_COND_MSG(id.get_type() != m_id_type, "Attempting to remove a row, but its stored ID type doesn't match that of the table.");
#endif

   m_row_list.remove_at(index);
   m_row_index.erase(id);

   godot::HashMap<godot::StringName, ValueSetType>::Iterator iter;
   for (iter = m_unique_indexing.begin(); iter != m_unique_indexing.end(); ++iter)
   {
      // iter->key is the Title of the column that holds unique values.
      // iter->value holds the Set that contains the list of values of the column
      // Must remove from the Set the value that has been removed
      const godot::Variant value = row.get(iter->key, nullptr);
      iter->value.erase(value);
   }

   calculate_weights();
}


bool DBTable::move_row(int64_t from, int64_t to)
{
   if (from == to)
   {
      return false;
   }
   if (from < 0 || from >= m_row_list.size())
   {
      return false;
   }
   if (to < 0 || to >= m_row_list.size())
   {
      return false;
   }

   const godot::Dictionary row = m_row_list[from];
   m_row_list.remove_at(from);
   m_row_list.insert(to, row);

   calculate_weights();

   return true;
}


godot::Variant DBTable::get_row_id(int64_t row_index) const
{
   if (row_index < 0 || row_index >= m_row_list.size())
   {
      return nullptr;
   }

   godot::Variant ret;
   const godot::Dictionary row = m_row_list[row_index];

   if (m_id_type == godot::Variant::INT)
   {
      const int64_t id = row.get("id", -1);
      ret = id;
   }
   else if (m_id_type == godot::Variant::STRING)
   {
      const godot::String id = row.get("id", "");
      ret = id;
   }

   return ret;
}


bool DBTable::set_row_id(int64_t row_index, const godot::Variant& new_id)
{
   if (m_locked_id) { return false; }

   ERR_FAIL_COND_V_MSG(new_id.get_type() != m_id_type, false, "While attempting to set the ID of a row, the type does not match that of the table.");

   if (row_index < 0 || row_index >= m_row_list.size())
   {
      // Row index out of bounds. Bail
      return false;
   }

   godot::Dictionary row = m_row_list[row_index];
   const godot::Variant old_id = row.get("id", nullptr);

#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_V_MSG(old_id.get_type() != m_id_type, false, "While attempting to set the ID of row, old stored one doesn't match the type set for the table.");
#endif

   if (old_id == new_id)
   {
      // New ID is the exact same of the one already set for this row. Bail
      return false;
   }

   if (m_row_index.has(new_id))
   {
      // New ID already exists in the table. Bail
      return false;
   }

   // Update the row
   row["id"] = new_id;

   // Update the row indexing
   m_row_index[new_id] = row;
   m_row_index.erase(old_id);

   return true;
}


bool DBTable::set_value(const godot::String& column_title, int64_t row_index, const godot::Variant& value)
{
   ColumnIndexType::Iterator col_iter = m_column_index.find(column_title);
   if (col_iter == m_column_index.end())
   {
      return false;
   }
   if (row_index < 0 || row_index >= m_row_list.size())
   {
      // Row index out of bounds
      return false;
   }

   const godot::String title = column_get_title(col_iter->value);
   godot::Dictionary row = m_row_list[row_index];
   const godot::Variant old_value = row.get(title, nullptr);

#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_V_MSG(title.is_empty(), false, "While attempting to set the value of a cell, the requested column does not have a title");
   ERR_FAIL_COND_V_MSG(old_value.get_type() == godot::Variant::NIL, false, "While attempting to set the value of a cell, previously stored value is invalid");
#endif

   if (old_value == value)
   {
      // NOTE: maybe return true?
      return false;
   }

   if (require_unique_values(col_iter->value))
   {
      if (!is_unique(col_iter->value, value))
      {
         return false;
      }

      unique_changed(col_iter->value, old_value, value);
   }


   // If the type is meant to hold an integer, incoming variant storing value might be holding a Float instead, because of the
   // cell editor. Make sure the stored value is indeed an integer rather than a float/double
   const ValueType type = column_get_type(col_iter->value);
   switch (type)
   {
      case VT_UniqueInteger:
      case VT_LockedUniqueInteger:
      case VT_ExternalInteger:
      case VT_Integer:
      {
         const int64_t ival = value;
         row[title] = ival;
      } break;

      //case VT_ExtIntArray:
      //case VT_IntegerArray:
      //{
         /// FIXME: Perform the same kind of conversion for the array types meant to hold integers
      //} break;

      default:
      {
         row[title] = value;
      } break;
   }

   if (m_rand_weight.column == title)
   {
      calculate_weights();
   }

   return true;
}


bool DBTable::set_value_by_index(int64_t column_index, int64_t row_index, const godot::Variant& value)
{
   if (column_index < 0 || column_index >= m_column_array.size())
   {
      return false;
   }

   const godot::Dictionary column = m_column_array[column_index];
   const godot::String title = column_get_title(column);
   return set_value(title, row_index, value);
}


void DBTable::sort_by_id(bool ascending)
{
   Sorter sorter("id", get_instance_id());
   if (ascending)
   {
      m_row_list.sort_custom(callable_mp(&sorter, &Sorter::ascending));
   }
   else
   {
      m_row_list.sort_custom(callable_mp(&sorter, &Sorter::descending));
   }
}


bool DBTable::sort_by_column(int64_t index, bool ascending)
{
   if (index < 0 || index >= m_column_array.size())
   {
      return false;
   }

   const godot::Dictionary column = m_column_array[index];
   const godot::String title = column_get_title(column);
   const ValueType type = column_get_type(column);

#ifdef DEBUG_ENABLED
   ERR_FAIL_COND_V_MSG(title.is_empty(), false, "While attempting to sort rows by specific column, its stored title is not valid");
   ERR_FAIL_COND_V_MSG(type == VT_Invalid, false, "While attempting to sort rows by specific column, its stored value type is not valid");
#endif

   Sorter sorter(title, get_instance_id());

   if (type == VT_Color)
   {
      if (ascending)
      {
         m_row_list.sort_custom(callable_mp(&sorter, &Sorter::ascending_color));
      }
      else
      {
         m_row_list.sort_custom(callable_mp(&sorter, &Sorter::descending_color));
      }
   }
   else
   {
      if (ascending)
      {
         m_row_list.sort_custom(callable_mp(&sorter, &Sorter::ascending));
      }
      else
      {
         m_row_list.sort_custom(callable_mp(&sorter, &Sorter::descending));
      }
   }
   
   return true;
}


godot::StringName DBTable::get_referenced_by_column(const godot::String& column_title) const
{
   ColumnIndexType::ConstIterator iter = m_column_index.find(column_title);

   if (iter == m_column_index.end())
   {
      return "";
   }

   return iter->value.get("extid", "");
}


void DBTable::build_type_list(godot::Vector<godot::Dictionary>&out_list)
{
   out_list.append(make_dictionary_ui_entry(VT_UniqueString, "Unique String"));
   out_list.append(make_dictionary_ui_entry(VT_UniqueInteger, "Unique Integer"));
   // Skip the two "locked unique *" as those are meant to be exclusively used for the ID
   
   out_list.append(make_dictionary_ui_entry(VT_ExternalString, "External String ID"));
   out_list.append(make_dictionary_ui_entry(VT_ExternalInteger, "External Integer ID"));
   //out_list.append(make_dictionary_ui_entry(VT_ExtStringArray, "External String Array"));
   //out_list.append(make_dictionary_ui_entry(VT_ExtIntArray, "External Integer Array"));
   
   out_list.append(make_dictionary_ui_entry(VT_RandomWeight, "Random Weight"));
   
   out_list.append(make_dictionary_ui_entry(VT_String, "String"));
   out_list.append(make_dictionary_ui_entry(VT_Bool, "Bool"));
   out_list.append(make_dictionary_ui_entry(VT_Integer, "Integer"));
   out_list.append(make_dictionary_ui_entry(VT_Float, "Float"));
   out_list.append(make_dictionary_ui_entry(VT_Texture, "Texture"));
   out_list.append(make_dictionary_ui_entry(VT_Audio, "Audio"));
   out_list.append(make_dictionary_ui_entry(VT_GenericRes, "Generic Resource"));
   out_list.append(make_dictionary_ui_entry(VT_Color, "Color"));
   out_list.append(make_dictionary_ui_entry(VT_MultiLineString, "Multi-line String"));
   
   out_list.append(make_dictionary_ui_entry(VT_StringArray, "String Array"));
   out_list.append(make_dictionary_ui_entry(VT_IntegerArray, "Integer Array"));
   out_list.append(make_dictionary_ui_entry(VT_FloatArray, "Float Array"));
   out_list.append(make_dictionary_ui_entry(VT_TextureArray, "Texture Array"));
   out_list.append(make_dictionary_ui_entry(VT_AudioArray, "Audio Array"));
   out_list.append(make_dictionary_ui_entry(VT_GenericResArray, "Generic Resource Array"));
   out_list.append(make_dictionary_ui_entry(VT_ColorArray, "Color Array"));
}


DBTable::DBTable(int id_type, const godot::StringName& name, bool locked_id)
{
   m_rand_gen.instantiate();
   m_rand_gen->randomize();

   m_table_id = make_random_string(12);

   if (id_type != godot::Variant::INT && id_type != godot::Variant::STRING)
   {
      id_type = godot::Variant::INT;
   }
   m_id_type = id_type;
   m_table_name = name;
   m_locked_id = locked_id;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceFormatLoaderDBTable






///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// ResourceFormatSaverDBTable



#endif  //DATABASE_DISABLED 

