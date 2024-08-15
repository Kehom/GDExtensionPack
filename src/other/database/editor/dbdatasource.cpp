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
#ifndef EDITOR_DISABLED

#include "dbdatasource.h"
#include "../../../internal.h"
#include "../../../extpackutils.h"

#include <godot_cpp/variant/utility_functions.hpp>


godot::String DBEditorDataSource::get_cell_class(DBTable::ValueType type) const
{
   // A HashMap from ValueType to String would have been way more convenient. However the switch
   // provides fallthrough for easier cell class sharing
   switch (type)
   {
      case DBTable::VT_UniqueString:
      case DBTable::VT_String:
      {
         return "TabularBoxCellString";
      } break;

      case DBTable::VT_UniqueInteger:
      case DBTable::VT_Integer:
      {
         return "TabularBoxCellInteger";
      } break;

      case DBTable::VT_LockedUniqueString:
      case DBTable::VT_LockedUniqueInteger:
      {
         return "DBTabularCellLockedRowID";
      } break;

      case DBTable::VT_ExternalString:
      case DBTable::VT_ExternalInteger:
      {
         return "DBTabularCellExternalID";
      } break;

      //case DBTable::VT_ExtStringArray:
      //case DBTable::VT_ExtIntArray:
      //{
      //   return "ExternalIDArrayNotImplementedYet";
      //} break;

      case DBTable::VT_RandomWeight:
      {
         return "DBTabularCellRandomWeight";
      } break;

      case DBTable::VT_Bool:
      {
         return "TabularBoxCellBool";
      } break;

      case DBTable::VT_Float:
      {
         return "TabularBoxCellFloat";
      } break;

      case DBTable::VT_Texture:
      {
         return "TabularBoxCellTexture";
      } break;

      case DBTable::VT_Audio:
      {
         return "DBTabularCellAudio";
      } break;

      case DBTable::VT_GenericRes:
      {
         return "DBTabularCellGenericResource";
      } break;

      case DBTable::VT_Color:
      {
         return "DBTabularCellColor";
      } break;

      case DBTable::VT_MultiLineString:
      {
         return "TabularBoxCellMultilineString";
      } break;


      case DBTable::VT_StringArray:
      {
         return "DBTabularCellArray";
      } break;

      case DBTable::VT_IntegerArray:
      {
         return "DBTabularCellArrayInteger";
      } break;

      case DBTable::VT_FloatArray:
      {
         return "DBTabularCellArrayFloat";
      } break;

      case DBTable::VT_TextureArray:
      {
         return "DBTabularCellArrayTexture";
      } break;

      case DBTable::VT_AudioArray:
      {
         return "DBTabularCellArrayAudio";
      } break;

      case DBTable::VT_GenericResArray:
      {
         return "DBTabularCellArrayGenericRes";
      } break;

      case DBTable::VT_ColorArray:
      {
         return "DBTabularCellArrayColor";
      } break;
   }

   return "TabularBoxCellString";
}


uint32_t DBEditorDataSource::get_column_flags(DBTable::ValueType type) const
{
   uint32_t ret = TabularDataSource::AllowTitleEdit | TabularDataSource::AllowMenu | TabularDataSource::AllowResize | TabularDataSource::AllowSorting |
      TabularDataSource::AllowMove | TabularDataSource::ValueChangeSignal;
   
   if (type >= 1000)
   {
      ret |= TabularDataSource::AllowTypeChange;
   }

   return ret;
}


godot::Dictionary DBEditorDataSource::get_extra_settings(DBTable::ValueType type, int64_t column_index) const
{
   godot::Dictionary ret;
   switch (type)
   {
      case DBTable::VT_RandomWeight:
      {
         ret["table"] = m_table;
         ret["min_value"] = 0.0;
      } break;

      case DBTable::VT_ExternalInteger:
      case DBTable::VT_ExternalString:
      {
         const godot::String cname = m_table->get_column_title(column_index);
         const godot::StringName referenced = m_table->get_referenced_by_column(cname);
         ret["table"] = m_db->get_table(referenced);
      } break;
   }

   return ret;
}


bool DBEditorDataSource::has_column(const godot::String& title) const
{
   if (title.to_lower() == "id") { return true; }

   return m_table->has_column(title);
}


int64_t DBEditorDataSource::get_column_count() const
{
   // Add one for the "id" column
   return m_table->get_column_count() + 1;
}


godot::Dictionary DBEditorDataSource::get_column_info(int64_t index) const
{
   godot::Dictionary ret;

   if (index == 0)
   {
      // This is the ID column
      const int id_type = m_table->get_id_type();
      DBTable::ValueType type = DBTable::VT_Invalid;
      godot::Dictionary extra;

      if (id_type == godot::Variant::INT)
      {
         type = m_table->is_row_id_locked() ? DBTable::VT_LockedUniqueInteger : DBTable::VT_UniqueInteger;
         // Even if using locked UniqueInteger, there is no harm in providing the extra 'min_value' setting
         extra["min_value"] = 0;
      }
      else if (id_type == godot::Variant::STRING)
      {
         type = m_table->is_row_id_locked() ? DBTable::VT_LockedUniqueString : DBTable::VT_UniqueString;
      }

      ERR_FAIL_COND_V_MSG(type == DBTable::VT_Invalid, ret, "While getting column info, found a table reporting invalid ID type.");


      ret["title"] = "ID";
      ret["type_code"] = type;
      ret["cell_class"] = get_cell_class(type);
      ret["flags"] = TabularDataSource::AllowResize | TabularDataSource::AllowSorting | TabularDataSource::ValueChangeSignal;
      ret["move_mode"] = TabularDataSource::AlwaysHidden;
      ret["extra_settings"] = extra;
   }
   else
   {
      const godot::Dictionary column = m_table->get_column_by_index(index - 1);
      const DBTable::ValueType type = (DBTable::ValueType)((int)column.get("value_type", DBTable::VT_Invalid));

      ret["title"] = column.get("title", "");
      ret["type_code"] = type;
      ret["cell_class"] = get_cell_class(type);
      ret["flags"] = get_column_flags(type);
      ret["move_mode"] = TabularDataSource::AlwaysVisible;
      ret["extra_settings"] = get_extra_settings(type, index - 1);
   }

   return ret;
}


int64_t DBEditorDataSource::get_row_count() const
{
   //return m_patched_table.is_valid() ? m_patched_table->get_row_count() : m_table->get_row_count();
   return m_table->get_row_count();
}


godot::Dictionary DBEditorDataSource::get_row(int64_t index) const
{
   godot::Dictionary ret;
   
//   if (m_patched_table.is_valid())
//   {
      
//   }
//   else
   {
      ret = m_db->get_row_from_by_index(m_table->get_table_name(), index, is_filtering());
   }

   // The "ID" is stored in lower case, however the TabularBox obtains the "column" title in full upper case.
   // For display purposes it is more useful the "upper case" thing. However to properly display the value the
   // returned row must also update the 'id' to uppercase otherwise TabularBox will fail to properly obtain the
   // ID within the UI. Data wont be changed on the DBTable because the obtained Dictionary is already duplicated.
   ExtPackUtils::change_dictionary_key(ret, "id", "ID");

   return ret;
}


godot::Variant DBEditorDataSource::get_value(int64_t column, int64_t row) const
{
   if (column == 0)
   {
      return m_table->get_row_id(row);
   }

   // Again the "ID" column does not exist in the DBTable, so must "shift" the index
   const godot::Dictionary col = m_table->get_column_by_index(column - 1);
   const godot::String title = col.get("title", "");
   const godot::Dictionary r = m_table->get_row_by_index(row);

   return r.get(title, nullptr);
}


void DBEditorDataSource::setup(const godot::Ref<GDDatabase>& db, const godot::Ref<DBTable>& table)
{
   m_db = db;
   m_table = table;
}


/*void DBEditorDataSource::set_patch(const godot::Ref<DBTablePatch>& patch)
{
   if(patch.is_valid())
   {
      if (m_table->get_table_id() == patch->get_target_id())
      {
         m_patch = patch;

         m_patched_table = m_table->duplicate();
         m_patched_table->apply_patch(m_patch);
      }
   }
   else
   {
      if (m_patch.is_valid())
      {
         m_patch.unref();
         m_patched_table.unref();
      }
   }
}


godot::String DBEditorDataSource::get_patch_filename() const
{
   return m_patch.is_valid() ? m_patch->get_path().get_file() : "";
}*/


godot::String DBEditorDataSource::get_table_name() const
{
   return m_table.is_valid() ? m_table->get_table_name() : "";
}


void DBEditorDataSource::column_added(int at)
{
   // Add 1 to the index because "0" is a "fake column" reserved for the ID.
   notify_new_column(at + 1);
}


void DBEditorDataSource::column_removed(int from)
{
   // The plugin is already dealing with the index "correction" (that is, taking ID "column" index into account)
   notify_column_removed(from);
}


void DBEditorDataSource::column_renamed(int index, bool rejected)
{
   if (rejected)
   {
      notify_column_rename_rejected(index);
   }
   else
   {
      notify_column_renamed(index);
   }
}


void DBEditorDataSource::column_moved(int from, int to)
{
   notify_column_moved(from, to);
}


void DBEditorDataSource::column_type_changed(int column_index)
{
   notify_type_changed(column_index);
}


void DBEditorDataSource::row_added(int at)
{
   notify_new_row(at);
}


void DBEditorDataSource::row_removed(const godot::Array& list)
{
   const int64_t size = list.size();
   for (int64_t i = 0; i < size; i++)
   {
      const int64_t index = list[i];
      notify_row_removed(index);
   }
}


void DBEditorDataSource::row_moved(int from, int to)
{
   notify_row_moved(from, to);
}


void DBEditorDataSource::value_changed(int column_index, int row_index, const godot::Variant& value, bool rejected)
{
   if (rejected)
   {
      notify_value_change_rejected(column_index, row_index);
   }
   else
   {
      notify_value_changed(column_index, row_index, value);
   }
}


void DBEditorDataSource::rows_sorted()
{
   //notify_sorted();
}


DBEditorDataSource::DBEditorDataSource()
{
   /// FIXME: Take localization into account
   // Add only the non unique types into the list. Those are the ones that will appear in the "value type" popup menu
   // when attempting to change value type of a right clicked column
   m_type_list[DBTable::VT_String] = "String";
   m_type_list[DBTable::VT_Bool] = "Bool";
   m_type_list[DBTable::VT_Integer] = "Integer";
   m_type_list[DBTable::VT_Float] = "Float";
   m_type_list[DBTable::VT_Texture] = "Texture";
   m_type_list[DBTable::VT_Audio] = "Audio";
   m_type_list[DBTable::VT_GenericRes] = "GenericResource";
   m_type_list[DBTable::VT_Color] = "Color";
   m_type_list[DBTable::VT_MultiLineString] = "MultiLineString";

   m_type_list[DBTable::VT_StringArray] = "StringArray";
   m_type_list[DBTable::VT_IntegerArray] = "IntegerArray";
   m_type_list[DBTable::VT_FloatArray] = "FloatArray";
   m_type_list[DBTable::VT_TextureArray] = "TextureArray";
   m_type_list[DBTable::VT_AudioArray] = "AudioArray";
   m_type_list[DBTable::VT_GenericResArray] = "GenericResourceArray";
   m_type_list[DBTable::VT_ColorArray] = "ColorArray";
}


#endif  //EDITOR_DISABLED
#endif  //DATABASE_DISABLED
