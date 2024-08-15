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

#ifndef TABULAR_BOX_DISABLED

/// FIXME: If the very first column is "fixed", do not allow the second column to swap places with the first one

#include "data_source_simple.h"

#include <godot_cpp/variant/utility_functions.hpp>


void TabularDataSourceSimple::build_index()
{
   m_column_index.clear();

   const int32_t ccount = m_column_list.size();
   for (int32_t i = 0; i < ccount; i++)
   {
      godot::Dictionary col = m_column_list[i];
      m_column_index[col.get("title", "")] = m_column_list[i];
   }
}


godot::String TabularDataSourceSimple::get_free_column_title() const
{
   const godot::String base = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
   const godot::String formatter = "%s%s";

   // Attempt to find a title 100 times
   for (int32_t attempt = 0; attempt < 100; attempt++)
   {
      for (int i = 0; i < base.length(); i++)
      {
         // For some reason base[i] is resulting in a number rather than the actual character! Because of that
         // using base.substr()
         godot::String test;
         if (attempt > 0)
         {
            test = godot::vformat(formatter, base.substr(i, 1), attempt);
         }
         else
         {
            test = godot::vformat(formatter, base.substr(i, 1), "");
         }

         if (!has_column(test))
         {
            return test;
         }
      }
   }

   return "";
}


godot::Variant TabularDataSourceSimple::get_default_value_for_type(ValueType type) const
{
   switch (type)
   {
      case VT_String: return godot::String();
      case VT_Bool: return false;
      case VT_Integer: return (int64_t)0;
      case VT_Float: return 0.0f;
      case VT_Texture: return godot::String();      // Texture is stored as path to the resource
      case VT_MultiLineString: return godot::String();
   }

   return godot::Variant();
}


godot::String TabularDataSourceSimple::get_cell_class(ValueType type) const
{
   switch (type)
   {
      case VT_String:
      {
         return "TabularBoxCellString";
      } break;

      case VT_Bool:
      {
         return "TabularBoxCellBool";
      } break;

      case VT_Integer:
      {
         return "TabularBoxCellInteger";
      } break;

      case VT_Float:
      {
         return "TabularBoxCellFloat";
      } break;

      case VT_Texture:
      {
         return "TabularBoxCellTexture";
      } break;

      case VT_MultiLineString:
      {
         return "TabularBoxCellMultilineString";
      } break;
   }

   // If here then type is either invalid or something new. Default to CellString to avoid errors
   return "TabularBoxCellString";
}



godot::Variant TabularDataSourceSimple::convert(const godot::Variant& val, int32_t to_type) const
{
   switch (to_type)
   {
      case VT_String:
      case VT_MultiLineString:
      {
         // From something to String
         if (val.get_type() != godot::Variant::STRING)
         {
            return godot::vformat("%s", val);
         }
      } break;

      case VT_Bool:
      {
         // From something to boolean
         switch (val.get_type())
         {
            // Nothing to do here. Although, it should not be here
            case godot::Variant::BOOL: {} break;

            case godot::Variant::STRING:
            {
               // Value is String but converting to boolean
               const godot::String str = ((godot::String)val).to_lower();
               return str == "true" || str == "yes" || str == "enabled" ?  true : false;
            } break;

            case godot::Variant::INT:
            {
               // Value is integer but converting to boolean
               return ((int)val) > 0;
            } break;

            default:
            {
               return false;
            } break;
         }
      } break;

      case VT_Integer:
      {
         // From something to integer
         switch (val.get_type())
         {
            // Nothing to do here. Although, it should not be here
            case godot::Variant::INT: {} break;

            case godot::Variant::FLOAT:
            {
               return (int)val;
            } break;

            case godot::Variant::STRING:
            {
               const godot::String str = (godot::String)val;
               if (str.is_valid_int())
               {
                  return str.to_int();
               }
               else if (str.is_valid_float())
               {
                  return (int)str.to_float();
               }
               else
               {
                  return (int)0;
               }
            } break;

            case godot::Variant::BOOL:
            {
               return ((bool)val) == true ? (int)1 : (int)0;
            } break;

            default:
            {
               return (int)0;
            }
         }
      } break;

      case VT_Float:
      {
         // From something to float
         switch (val.get_type())
         {
            case godot::Variant::FLOAT: {} break;

            case godot::Variant::INT:
            {
               return (float)val;
            } break;

            case godot::Variant::STRING:
            {
               const godot::String str = (godot::String)val;
               if (str.is_valid_float())
               {
                  return str.to_float();
               }
               else if (str.is_valid_int())
               {
                  // probably never reaching here but just to ensure
                  return (float)str.to_int();
               }
               else
               {
                  return 0.0f;
               }
            } break;

            default:
            {
               return 0.0f;
            }
         }
      } break;

      case VT_Texture:
      {
         if (val.get_type() == godot::Variant::STRING)
         {
            const godot::String str = (godot::String)val;
            if (str.is_absolute_path())
            {
               return str;
            }
            return "";
         }
         else
         {
            return "";
         }
      } break;
   }

   return val;
}



void TabularDataSourceSimple::_get_property_list(godot::List<godot::PropertyInfo>* list) const
{
   using namespace godot;

   list->push_back(PropertyInfo(Variant::ARRAY, "column_list", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   list->push_back(PropertyInfo(Variant::ARRAY, "row_list", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
}


bool TabularDataSourceSimple::_set(const godot::StringName& prop_name, const godot::Variant& value)
{
   // Assume the property is an expected one
   bool ret = true;
   if (prop_name == godot::StringName("column_list"))
   {
      m_column_list = value;
      build_index();
   }
   else if (prop_name == godot::StringName("row_list"))
   {
      m_row_list = value;
   }
   else
   {
      ret = false;
   }

   return ret;
}

bool TabularDataSourceSimple::_get(const godot::StringName& prop_name, godot::Variant& out_value) const
{
   bool ret = true;
   if (prop_name == godot::StringName("column_list"))
   {
      out_value = m_column_list;
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


godot::Dictionary TabularDataSourceSimple::get_type_list() const
{
   godot::Dictionary ret;
   ret[VT_String] = godot::String("String");
   ret[VT_MultiLineString] = godot::String("Multi-line String");
   ret[VT_Bool] = godot::String("Bool");
   ret[VT_Integer] = godot::String("Integer");
   ret[VT_Float] = godot::String("Float");
   ret[VT_Texture] = godot::String("Texture");

   return ret;
}


godot::Dictionary TabularDataSourceSimple::get_column_info(int64_t index) const
{
   ERR_FAIL_INDEX_V_MSG(index, m_column_list.size(), godot::Dictionary(), "Attempting to get column info but incoming index is out of bounds");
   const godot::Dictionary col = m_column_list[index];
   godot::Dictionary ret;

   const int32_t tp = col.get("type", (int32_t)VT_String);
   const int32_t flags = col.get("flags", -1);
   const int32_t move_mode = col.get("move_mode", -1);

   ret["title"] = col.get("title", "");
   ret["type_code"] = tp;
   ret["cell_class"] = get_cell_class((ValueType)tp);
   ret["extra_settings"] = col.get("extra_settings", godot::Dictionary());

   if (flags != -1)
   {
      ret["flags"] = flags;
   }

   if (move_mode != -1)
   {
      ret["move_mode"] = move_mode;
   }

   return ret;
}


void TabularDataSourceSimple::insert_column(const godot::String& title, int32_t type, int64_t index, const godot::Dictionary& info, const godot::Dictionary& extra)
{
   godot::String the_title;
   if (title.is_empty())
   {
      the_title = get_free_column_title();
      if (the_title.is_empty())
      {
         WARN_PRINT("Attempting to insert a column but automatic title \"generation\" couldn't find any available.");
         return;
      }
   }
   else
   {
      the_title = title;
   }

   if (has_column(the_title))
   {
      WARN_PRINT(godot::vformat("Attempting to insert a column but requested title '%s' is already used.", the_title));
      return;
   }

   if (type < 0)
   {
      type = (int32_t)VT_String;
   }

   if (index < 0 || index > m_column_list.size())
   {
      // If the index is outside of the boundaries, set the value to be the last one, which will effectively
      // append the new column into the data
      index = m_column_list.size();
   }

   const int32_t flags = info.get("flags", _Default);
   const int32_t move_buttons_mode = info.get("move_mode", AlwaysVisible);

   godot::Dictionary data;
   data["title"] = the_title;
   data["type"] = type;

   if (flags != _Default)
   {
      data["flags"] = flags;
   }

   if (move_buttons_mode != AlwaysVisible)
   {
      data["move_mode"] = move_buttons_mode;
   }

   if (!extra.is_empty())
   {
      data["extra_settings"] = extra;
   }


   /*if (index < m_column_list.size())
   {
      m_column_list.insert(index, data);
   }
   else
   {
      m_column_list.append(data);
   }*/
   // At the top of the function index is ensured to be either smaller than size() or equal, which are the requirements
   // for the .insert() function
   m_column_list.insert(index, data);
   

   const int32_t rcount = m_row_list.size();
   for (int32_t i = 0; i < rcount; i++)
   {
      // Damn, this line is ugly!
      ((godot::Dictionary)m_row_list[i])[the_title] = get_default_value_for_type((ValueType)type);
   }

   build_index();
   notify_new_column(index);
}


void TabularDataSourceSimple::remove_column(int64_t index)
{
   ERR_FAIL_INDEX_MSG(index, m_column_list.size(), "Attempting to remove column but incoming index is out of bounds");

   const godot::Dictionary data = m_column_list[index];
   const godot::String title = data.get("title", "");
   
   m_column_index.erase(title);
   m_column_list.remove_at(index);

   const int32_t rcount = m_row_list.size();
   for (int32_t i = 0; i < rcount; i++)
   {
      ((godot::Dictionary)m_row_list[i]).erase(title);
   }

   notify_column_removed(index);
}


void TabularDataSourceSimple::rename_column(int64_t index, const godot::String& new_title)
{
   ERR_FAIL_INDEX_MSG(index, m_column_list.size(), "Attempting to rename column but incoming index is out of bounds");

   if (has_column(new_title))
   {
      // Incoming "new title" already exists. If it is for a different column emit a signal telling that the
      // change has been rejected
      const godot::String existing = ((godot::Dictionary)m_column_list[index]).get("title", "");

      if (existing != new_title)
      {
         notify_column_rename_rejected(index);
      }

      return;
   }

   godot::Dictionary data = m_column_list[index];
   const godot::String otitle = data.get("title", "");

   // Update "column_list" data to the new title
   data["title"] = new_title;

   // To update the column indexing, first add the new entry into it
   m_column_index[new_title] = m_column_list[index];

   // Then remove the old one
   m_column_index.erase(otitle);

   // Update each row
   const int32_t rcount = m_row_list.size();
   for (int32_t i = 0; i < rcount; i++)
   {
      godot::Dictionary row = m_row_list[i];

      // Retrieve the value of the renamed column
      const godot::Variant theval = row.get(otitle, nullptr);

      // Create the entry for the new title
      row[new_title] = theval;

      // Then remove the old one
      row.erase(otitle);
   }

   notify_column_renamed(index);
}


void TabularDataSourceSimple::change_column_value_type(int64_t index, int32_t new_type)
{
   ERR_FAIL_INDEX_MSG(index, m_column_list.size(), "Attempting to change column type but incoming column index is out of bounds.");

   godot::Dictionary data = m_column_list[index];
   const int32_t otype = data.get("type", -1);
   if (otype == new_type)
   {
      return;
   }

   const godot::String cell_class = get_cell_class((ValueType)new_type);
   data["type"] = new_type;


   const godot::String title = ((godot::Dictionary)m_column_list[index]).get("title", "");

   // Now must update rows
   const int32_t rcount = m_row_list.size();
   for (int32_t i = 0; i < rcount; i++)
   {
      const godot::Variant value = ((godot::Dictionary)m_row_list[i]).get(title, nullptr);
      ((godot::Dictionary)m_row_list[i])[title] = convert(value, new_type);
   }

   notify_type_changed(index);
}


void TabularDataSourceSimple::move_column(int64_t from, int64_t to)
{
   if (from == to)
   {
      return;
   }

   if (from < 0 || from >= m_column_list.size())
   {
      return;
   }
   if (to < 0 || to >= m_column_list.size())
   {
      return;
   }

   const godot::Dictionary entry_from = m_column_list[from];
   m_column_list.remove_at(from);

   // At this point 'to' is either "valid index" or equal to the current m_column_list.size()
   // Those are the requirements for the array.insert()
   m_column_list.insert(to, entry_from);

   /*if (to == m_column_list.size())
   {
      // Because the "from" was temporarily removed from the list it is possible the "to" is pointing past the
      // last array element. In this case just append the column back
      m_column_list.append(entry_from);
   }
   else
   {
      // Otherwise just insert the column back into the desired index
      m_column_list.insert(to, entry_from);
   }*/

   // The column indexing directly points into the Dictionary data, so no need to update it
   // Yet, emit the signal telling that a column has been moved
   notify_column_moved(from, to);
}


void TabularDataSourceSimple::insert_row(const godot::Dictionary& values, int64_t index)
{
   if (index < 0 || index > m_row_list.size())
   {
      index = m_row_list.size();
   }

   const int32_t ccount = m_column_list.size();

   if (ccount == 0)
   {
      return;
   }

   godot::Dictionary nrow;

   for (int32_t i = 0; i < ccount; i++)
   {
      const godot::String title = ((godot::Dictionary)m_column_list[i]).get("title", "");

      godot::Variant value = values.get(title, nullptr);
      if (value.get_type() == godot::Variant::NIL)
      {
         const int32_t type = ((godot::Dictionary)m_column_list[i]).get("type", VT_String);
         value = get_default_value_for_type((ValueType)type);
      }

      nrow[title] = value;
   }

   /*if (index < m_row_list.size())
   {
      m_row_list.insert(index, nrow);
   }
   else
   {
      m_row_list.append(nrow);
   }*/
   // As per the array requirements, index must be a within bounds or equal to the current size. At the top of this function
   // this requirement is ensured.
   m_row_list.insert(index, nrow);

   notify_new_row(index);
}


void TabularDataSourceSimple::remove_row(int64_t index)
{
   if (index < 0 || index >= m_row_list.size())
   {
      return;
   }

   m_row_list.remove_at(index);
   notify_row_removed(index);
}


void TabularDataSourceSimple::move_row(int64_t from, int64_t to)
{
   if (from == to)
   {
      return;
   }

   if (from < 0 || from >= m_row_list.size())
   {
      return;
   }
   if (to < 0 || to >= m_row_list.size())
   {
      return;
   }

   const godot::Dictionary row = m_row_list[from];
   m_row_list.remove_at(from);
   m_row_list.insert(to, row);
   notify_row_moved(from, to);
}


godot::Dictionary TabularDataSourceSimple::get_row(int64_t index) const
{
   ERR_FAIL_INDEX_V_MSG(index, m_row_list.size(), godot::Dictionary(), "Attempting to retrieve row, but requested index is out of bounds");

   return m_row_list[index];
}


godot::Variant TabularDataSourceSimple::get_value(int64_t column, int64_t row) const
{
   if (column < 0 || column >= m_column_list.size()) { return ""; }
   if (row < 0 || row >= m_row_list.size()) { return ""; }

   const godot::String title = ((godot::Dictionary)m_column_list[column]).get("title", "");
   
   return ((godot::Dictionary)m_row_list[row]).get(title, godot::String(""));
}


void TabularDataSourceSimple::set_value(int64_t column, int64_t row, const godot::Variant& value)
{
   if (column < 0 || column >= m_column_list.size()) { return; }
   if (row < 0 || row >= m_row_list.size()) { return; }

   const godot::String title = ((godot::Dictionary)m_column_list[column]).get("title", "");
   ((godot::Dictionary)m_row_list[row])[title] = value;

   notify_value_changed(column, row, value);
}


bool TabularDataSourceSimple::native_sort_by_column(int64_t column, bool ascending)
{
   if (column < 0 || column >= m_column_list.size()) { return false; }

   Sorter sorter;
   sorter.column = ((godot::Dictionary)m_column_list[column]).get("title", "");
   sorter.id = get_instance_id();

   godot::Callable custom = callable_mp(&sorter, ascending ? &Sorter::ascending : &Sorter::descending);
   m_row_list.sort_custom(custom);

   //notify_sorted(column);
   return true;
}


void TabularDataSourceSimple::dump_data() const
{
   godot::UtilityFunctions::print(godot::vformat("Column List:\n%s\n\nRow List:\n%s", m_column_list, m_row_list));
}



#endif
