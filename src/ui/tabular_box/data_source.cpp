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

#include "data_source.h"

#include <godot_cpp/classes/reg_ex.hpp>
#include <godot_cpp/classes/reg_ex_match.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/variant/utility_functions.hpp>


void TabularDataSource::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("sort_by_column", "column_index", "ascending"), &TabularDataSource::sort_by_column);
   ClassDB::bind_method(D_METHOD("is_filtering"), &TabularDataSource::is_filtering);
   ClassDB::bind_method(D_METHOD("set_filter", "keyword", "case_sensitive", "exclude_columns"), &TabularDataSource::set_filter, DEFVAL(godot::PackedStringArray()));
   ClassDB::bind_method(D_METHOD("set_filter_regex", "expression", "exclude_columns"), &TabularDataSource::set_filter_regex, DEFVAL(godot::PackedStringArray()));
   ClassDB::bind_method(D_METHOD("clear_filter"), &TabularDataSource::clear_filter);
   ClassDB::bind_method(D_METHOD("is_filtered", "row_index"), &TabularDataSource::is_filtered);

   ClassDB::bind_method(D_METHOD("notify_new_column", "index"), &TabularDataSource::notify_new_column);
   ClassDB::bind_method(D_METHOD("notify_column_removed", "index"), &TabularDataSource::notify_column_removed);
   ClassDB::bind_method(D_METHOD("notify_column_moved", "from", "to"), &TabularDataSource::notify_column_moved);
   ClassDB::bind_method(D_METHOD("notify_column_renamed", "index"), &TabularDataSource::notify_column_renamed);
   ClassDB::bind_method(D_METHOD("notify_column_rename_rejected", "index"), &TabularDataSource::notify_column_rename_rejected);
   ClassDB::bind_method(D_METHOD("notify_new_row", "index"), &TabularDataSource::notify_new_row);
   ClassDB::bind_method(D_METHOD("notify_row_removed", "index"), &TabularDataSource::notify_row_removed);
   ClassDB::bind_method(D_METHOD("notify_row_moved", "from", "to"), &TabularDataSource::notify_row_moved);
   ClassDB::bind_method(D_METHOD("notify_value_changed", "column", "row", "new_value"), &TabularDataSource::notify_value_changed);
   ClassDB::bind_method(D_METHOD("notify_value_change_rejected", "column", "row"), &TabularDataSource::notify_value_change_rejected);
   ClassDB::bind_method(D_METHOD("notify_type_changed", "column"), &TabularDataSource::notify_type_changed);
   ClassDB::bind_method(D_METHOD("notify_sorted"), &TabularDataSource::notify_sorted);

   ClassDB::bind_method(D_METHOD("has_column", "title"), &TabularDataSource::has_column);
   ClassDB::bind_method(D_METHOD("get_column_count"), &TabularDataSource::get_column_count);
   ClassDB::bind_method(D_METHOD("get_column_info", "index"), &TabularDataSource::get_column_info);
   ClassDB::bind_method(D_METHOD("insert_column", "title", "type", "index", "info", "extra"), &TabularDataSource::insert_column, DEFVAL(godot::Dictionary()), DEFVAL(godot::Dictionary()));
   ClassDB::bind_method(D_METHOD("remove_column", "index"), &TabularDataSource::remove_column);
   ClassDB::bind_method(D_METHOD("rename_column", "index", "new_title"), &TabularDataSource::rename_column);
   ClassDB::bind_method(D_METHOD("change_column_value_type", "index", "new_type"), &TabularDataSource::change_column_value_type);
   ClassDB::bind_method(D_METHOD("move_column", "from", "to"), &TabularDataSource::move_column);

   ClassDB::bind_method(D_METHOD("get_row_count"), &TabularDataSource::get_row_count);
   ClassDB::bind_method(D_METHOD("insert_row", "values", "index"), &TabularDataSource::insert_row, DEFVAL(-1));
   ClassDB::bind_method(D_METHOD("remove_row", "index"), &TabularDataSource::remove_row);
   ClassDB::bind_method(D_METHOD("move_row", "from", "to"), &TabularDataSource::move_row);
   ClassDB::bind_method(D_METHOD("get_value", "column_index", "row_index"), &TabularDataSource::get_value);
   ClassDB::bind_method(D_METHOD("set_value", "column_index", "row_index", "value"), &TabularDataSource::set_value);
   ClassDB::bind_method(D_METHOD("get_row", "row_index"), &TabularDataSource::get_row);
   

   ClassDB::bind_method(D_METHOD("dump_data"), &TabularDataSource::dump_data);


   GDVIRTUAL_BIND(_get_type_list);
   GDVIRTUAL_BIND(_has_column, "title");
   GDVIRTUAL_BIND(_get_column_count);
   GDVIRTUAL_BIND(_get_column_info, "index");
   GDVIRTUAL_BIND(_insert_column, "title", "type", "index", "info", "extra");
   GDVIRTUAL_BIND(_remove_column, "index");
   GDVIRTUAL_BIND(_rename_column, "index", "new_title");
   GDVIRTUAL_BIND(_change_column_value_type, "index", "new_type");
   GDVIRTUAL_BIND(_move_column, "from", "to");
   GDVIRTUAL_BIND(_get_row_count);
   GDVIRTUAL_BIND(_insert_row, "values", "index");
   GDVIRTUAL_BIND(_remove_row, "index");
   GDVIRTUAL_BIND(_move_row, "from", "to");
   GDVIRTUAL_BIND(_get_value, "column_index", "row_index");
   GDVIRTUAL_BIND(_set_value, "column_index", "row_index", "value");
   GDVIRTUAL_BIND(_get_row, "index");
   GDVIRTUAL_BIND(_sort_by_column, "column_index", "ascending");
   GDVIRTUAL_BIND(_dump_data);

   BIND_ENUM_CONSTANT(AllowTitleEdit);
   BIND_ENUM_CONSTANT(AllowMenu);
   BIND_ENUM_CONSTANT(AllowResize);
   BIND_ENUM_CONSTANT(AllowTypeChange);
   BIND_ENUM_CONSTANT(AllowSorting);
   BIND_ENUM_CONSTANT(AllowMove);
   BIND_ENUM_CONSTANT(ValueChangeSignal);

   BIND_ENUM_CONSTANT(AlwaysVisible);
   BIND_ENUM_CONSTANT(ShowOnMouseOver);
   BIND_ENUM_CONSTANT(AlwaysHidden);


   ADD_SIGNAL(MethodInfo("column_inserted", PropertyInfo(Variant::INT, "at_index")));
   ADD_SIGNAL(MethodInfo("column_removed", PropertyInfo(Variant::INT, "index")));
   ADD_SIGNAL(MethodInfo("column_moved", PropertyInfo(Variant::INT, "from"), PropertyInfo(Variant::INT, "to")));
   ADD_SIGNAL(MethodInfo("column_renamed", PropertyInfo(Variant::INT, "index")));
   ADD_SIGNAL(MethodInfo("column_rename_rejected", PropertyInfo(Variant::INT, "index")));
   ADD_SIGNAL(MethodInfo("row_inserted", PropertyInfo(Variant::INT, "at_index")));
   ADD_SIGNAL(MethodInfo("row_removed", PropertyInfo(Variant::INT, "index")));
   ADD_SIGNAL(MethodInfo("row_moved", PropertyInfo(Variant::INT, "from"), PropertyInfo(Variant::INT, "to")));
   ADD_SIGNAL(MethodInfo("value_changed", PropertyInfo(Variant::INT, "column"), PropertyInfo(Variant::INT, "row"), PropertyInfo(Variant::NIL, "new_value")));
   ADD_SIGNAL(MethodInfo("value_change_rejected", PropertyInfo(Variant::INT, "column"), PropertyInfo(Variant::INT, "row")));
   ADD_SIGNAL(MethodInfo("type_changed", PropertyInfo(Variant::INT, "column")));
   ADD_SIGNAL(MethodInfo("data_sorting_changed", PropertyInfo(Variant::INT, "column")));
   ADD_SIGNAL(MethodInfo("filter_changed"));
}


void TabularDataSource::sort_by_column(int64_t column_index, bool ascending)
{
   // Get all rows that are currently selected. Those will be used to attempt to restore selection
   // after sorting
   // This will be used as a Set. Using a Dictionary because godot:RBSet (or godot::HashMap) does not like holding Dictionaries
   godot::Dictionary sel_row;
   

   for (godot::RBSet<int64_t>::Iterator iter = m_selected.begin(); iter != m_selected.end(); ++iter)
   {
      const godot::Dictionary row = get_row(*iter);
      sel_row[row] = 0;        // Value is irrelevant
   }

   m_selected.clear();

   if (native_sort_by_column(column_index, ascending))
   {
      const int64_t rcount = get_row_count();
      for (int64_t ri = 0; ri < rcount; ri++)
      {
         const godot::Dictionary row = get_row(ri);
         if (sel_row.has(row))
         {
            m_selected.insert(ri);
         }
      }

      notify_sorted(column_index);
   }
}


void TabularDataSource::set_filter(const godot::String& keyword, bool case_sensitive, const godot::PackedStringArray& exclude_columns)
{
   m_filtered.clear();

   if (keyword.is_empty())
   {
      emit_signal("filter_changed");
      return;
   }

   m_is_filtering = true;

   // Build a set ouf of the exclude column array to make things easier when iterating. More specifically to make easier to
   // query if a specific column is meant to be excluded or not
   godot::RBSet<godot::String> exclude;
   for (const godot::String& col : exclude_columns)
   {
      exclude.insert(col);
   }

   const godot::String word = case_sensitive ? keyword : keyword.to_lower();

   const int64_t ccount = get_column_count();
   const int64_t rcount = get_row_count();
   for (int64_t r = 0; r < rcount; r++)
   {
      // Assume this row will be filtered out
      bool filtered = true;
      const godot::Dictionary row = get_row(r);

      for (int64_t c = 0; c < ccount; c++)
      {
         ColumnInfo info;
         fill_column_info(c, info);

         if (exclude.has(info.title)) { continue; }

         godot::String value = row.get(info.title, "");
         if (value.is_empty()) { continue; }

         if (!case_sensitive)
         {
            value = value.to_lower();
         }

         if (value.contains(word))
         {
            filtered = false;
            break;
         }
      }

      if (filtered)
      {
         m_filtered.insert(r);
      }
   }

   m_is_filtering = false;

   emit_signal("filter_changed");
}


void TabularDataSource::set_filter_regex(const godot::String& expression, const godot::PackedStringArray& exclude_columns)
{
   m_filtered.clear();

   if (expression.is_empty())
   {
      emit_signal("filter_changed");
      return;
   }

   m_is_filtering = true;

   godot::RBSet<godot::String> exclude;
   for (const godot::String& col : exclude_columns)
   {
      exclude.insert(col);
   }

   godot::Ref<godot::RegEx> regex;
   regex.instantiate();
   if (regex->compile(expression) != godot::Error::OK)
   {
      // According to the documentation details of the error are printed to standard output
      emit_signal("filter_changed");
      return;
   }

   const int64_t ccount = get_column_count();
   const int64_t rcount = get_row_count();
   for (int64_t r = 0; r < rcount; r++)
   {
      // Assume this row will be filtered out
      bool filtered = true;
      const godot::Dictionary row = get_row(r);

      for (int64_t c = 0; c < ccount; c++)
      {
         ColumnInfo info;
         fill_column_info(c, info);

         if (exclude.has(info.title)) { continue; }

         godot::String value = row.get(info.title, "");
         if (value.is_empty()) { continue; }

         godot::Ref<godot::RegExMatch> match = regex->search(value);

         if (!match.is_valid()) { continue; }

         
         // Is this enough to properly tell if there was a match?
         if (match->get_start() >= 0 && match->get_end() >= 0)
         {
            filtered = false;
            break;
         }
      }

      if (filtered)
      {
         m_filtered.insert(r);
      }
   }


   m_is_filtering = false;
   emit_signal("filter_changed");
}


void TabularDataSource::clear_filter()
{
   m_filtered.clear();
   emit_signal("filter_changed");
}


bool TabularDataSource::is_filtered(int64_t row_index) const
{
   return m_filtered.has(row_index);
}


void TabularDataSource::notify_new_column(int64_t index)
{
   emit_signal("column_inserted", index);
}

void TabularDataSource::notify_column_removed(int64_t index)
{
   emit_signal("column_removed", index);
}

void TabularDataSource::notify_column_moved(int64_t from, int64_t to)
{
   emit_signal("column_moved", from, to);
}

void TabularDataSource::notify_column_renamed(int64_t index)
{
   emit_signal("column_renamed", index);
}

void TabularDataSource::notify_column_rename_rejected(int64_t index)
{
   emit_signal("column_rename_rejected", index);
}

void TabularDataSource::notify_new_row(int64_t index)
{
   // A new row has been inserted at 'index'. This means that selected rows have been shifted by one. Update the m_selected
   godot::Vector<int64_t> reselect;
   const int64_t rcount = get_row_count();
   for (int64_t i = index; i < rcount; i++)
   {
      if (m_selected.has(i))
      {
         m_selected.erase(i);
         reselect.append(i+1);
      }
   }

   for (int64_t r : reselect)
   {
      m_selected.insert(r);
   }

   emit_signal("row_inserted", index);
}

void TabularDataSource::notify_row_removed(int64_t index)
{
   // A row has been removed from 'index'. This means that selected rows have been shifted by one. Update the m_selected
   godot::Vector<int64_t> reselect;
   const int64_t rcount = get_row_count();
   for (int64_t i = index + 1; i < rcount; i++)
   {
      if (m_selected.has(i))
      {
         m_selected.erase(i);
         reselect.append(i-1);
      }
   }

   for (int64_t r : reselect)
   {
      m_selected.insert(r);
   }

   emit_signal("row_removed", index);
}

void TabularDataSource::notify_row_moved(int64_t from, int64_t to)
{
   emit_signal("row_moved", from, to);
}

void TabularDataSource::notify_value_changed(int64_t column, int64_t row, const godot::Variant& new_value)
{
   emit_signal("value_changed", column, row, new_value);
}

void TabularDataSource::notify_value_change_rejected(int64_t column, int64_t row)
{
   emit_signal("value_change_rejected", column, row);
}

void TabularDataSource::notify_type_changed(int64_t column)
{
      emit_signal("type_changed", column);
}

void TabularDataSource::notify_sorted(int64_t column)
{
   emit_signal("data_sorting_changed", column);
}


bool TabularDataSource::has_column(const godot::String& title) const
{
   bool ret;
   if (GDVIRTUAL_REQUIRED_CALL(_has_column, title, ret))
   {
      return ret;
   }
   return false;
}

int64_t TabularDataSource::get_column_count() const
{
   int64_t ret;
   if (GDVIRTUAL_REQUIRED_CALL(_get_column_count, ret))
   {
      return ret;
   }
   return -1;
}

godot::Dictionary TabularDataSource::get_column_info(int64_t index) const
{
   godot::Dictionary ret;
   if (GDVIRTUAL_REQUIRED_CALL(_get_column_info, index, ret))
   {
      return ret;
   }

   return godot::Dictionary();
}


int64_t TabularDataSource::get_row_count() const
{
   int64_t ret;
   if (GDVIRTUAL_REQUIRED_CALL(_get_row_count, ret))
   {
      return ret;
   }
   return -1;
}


void TabularDataSource::insert_row(const godot::Dictionary& values, int64_t index)
{
   GDVIRTUAL_REQUIRED_CALL(_insert_row, values, index);
}


void TabularDataSource::remove_row(int64_t index)
{
   GDVIRTUAL_REQUIRED_CALL(_remove_row, index);
}


void TabularDataSource::move_row(int64_t from, int64_t to)
{
   GDVIRTUAL_REQUIRED_CALL(_move_row, from, to);
}


godot::Variant TabularDataSource::get_value(int64_t column_index, int64_t row_index) const
{
   godot::Variant ret;
   if (GDVIRTUAL_REQUIRED_CALL(_get_value, column_index, row_index, ret))
   {
      return ret;
   }
   return godot::Variant();
}


godot::Dictionary TabularDataSource::get_row(int64_t index) const
{
   godot::Dictionary ret;
   if (GDVIRTUAL_REQUIRED_CALL(_get_row, index, ret))
   {
      return ret;
   }
   return godot::Dictionary();
}


bool TabularDataSource::native_sort_by_column(int64_t column_index, bool ascending)
{
   return GDVIRTUAL_CALL(_sort_by_column, column_index, ascending);
}


void TabularDataSource::fill_column_info(int64_t index, ColumnInfo& out_data)
{
   godot::Dictionary info = get_column_info(index);
   
   out_data.title = info.get("title", "Invalid Title");
   out_data.flags = info.get("flags", (int32_t)_Default);
   out_data.type_code = info.get("type_code", (int64_t)0);
   out_data.cell_class = info.get("cell_class", "TabularBoxCellString");
   // Yeah, very fun line this one...
   out_data.move_bt_mode = (MoveColumnButtons)((int)info.get("move_mode", AlwaysVisible));
   out_data.extra_settings = info.get("extra_settings", godot::Dictionary());
}




godot::Dictionary TabularDataSource::get_type_list() const
{
   godot::Dictionary ret;
   if (GDVIRTUAL_REQUIRED_CALL(_get_type_list, ret))
   {
      return ret;
   }
   return godot::Dictionary();
}



/*void TabularDataSource::column_ui_created(TabularBoxColumn* column)
{
   GDVIRTUAL_CALL(_column_ui_created, column);
}*/


void TabularDataSource::set_row_selected(int64_t row_index, bool selected)
{
   if (row_index < 0 || row_index >= get_row_count()) { return; }

   if (selected)
   {
      m_selected.insert(row_index);
   }
   else
   {
      if (m_selected.has(row_index))
      {
         m_selected.erase(row_index);
      }
   }
}


bool TabularDataSource::is_row_selected(int64_t row_index) const
{
   return m_selected.has(row_index);
}


bool TabularDataSource::has_selected_row() const
{
   return m_selected.size() > 0;
}


int64_t TabularDataSource::get_selected_count() const
{
   return m_selected.size();
}


void TabularDataSource::invert_row_selection()
{
   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      if (m_selected.has(i))
      {
         m_selected.erase(i);
      }
      else
      {
         m_selected.insert(i);
      }
   }
}


void TabularDataSource::select_all_rows()
{
   const int64_t rcount = get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      m_selected.insert(i);
   }
}


void TabularDataSource::deselect_all_rows()
{
   m_selected.clear();
}


void TabularDataSource::delete_selected_rows()
{
   // The list of selected rows is stored in a RBSet. Internally it uses Red Black binary tree. This means that in
   // theory normal iteration should be in ascending order of indices.
   for (godot::RBSet<int64_t>::Iterator iter = m_selected.back(); iter != nullptr; --iter)
   {
      remove_row(*iter);
   }
   m_selected.clear();
}


godot::Array TabularDataSource::get_selected_list() const
{
   // This function is meant primarily for the signal emitted by the TabularBox requesting to delete rows when there are
   // multiple selected ones.
   godot::Array ret;
   for (godot::RBSet<int64_t>::ConstIterator iter = m_selected.back(); iter != nullptr; --iter)
   {
      ret.append(*iter);
   }

   return ret;
}


TabularDataSource::TabularDataSource()
{
   m_is_filtering = false;
}


#endif
