# Copyright (c) 2022 Yuri Sarudiansky
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

@tool
class_name TabularDataSourceCustom
extends TabularDataSource


#######################################################################################################################
### Signals and definitions
enum ValueType {
	VT_STRING,
	VT_BOOL,
	VT_INT,
	VT_FLOAT,
	VT_TEXTURE,
	VT_MULTILINE_STRING,
	VT_DAMAGE_TYPE,
}

const type_to_class: Dictionary = {
	ValueType.VT_STRING: "TabularBoxCellString",
	ValueType.VT_BOOL: "TabularBoxCellBool",
	ValueType.VT_INT: "TabularBoxCellInteger",
	ValueType.VT_FLOAT: "TabularBoxCellFloat",
	ValueType.VT_TEXTURE: "TabularBoxCellTexture",
	ValueType.VT_MULTILINE_STRING: "TabularBoxCellMultilineString",
	# This is for a custom cell and must be implemented
	ValueType.VT_DAMAGE_TYPE: "TabularBoxCellDropdown",
}

enum DamageType {
	DT_None,        # Mostly to have a "default value"
	DT_Physical,
	DT_Fire,
	DT_Cold,
	DT_Lightning,
	DT_Poison,
	DT_Arcane,
}

#######################################################################################################################
### "Public" properties
# Each dictionary should hold the following fields:
# - title: Column title
# - type: Value type
# - flags: setting flags from the ColumnFlags enum
@export var column_list: Array[Dictionary]

# Each dictionary should hold column title as "key" with the corresponding cell value assigned to it
@export var row_list: Array[Dictionary]

#######################################################################################################################
### "Public" functions


#######################################################################################################################
### "Private" definitions


#######################################################################################################################
### "Private" properties
const _alphabet: String = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

#######################################################################################################################
### "Private" functions
func _generate_title() -> String:
	var attempt_count: int = 0
	var done: bool = false
	
	while !done:
		for l: String in _alphabet:
			var attempt: String = "%s%s" % [l, str(attempt_count) if attempt_count > 0 else ""]
			if !_has_column(attempt):
				return attempt
		
		# If still here, all characters have been iterated through
		attempt_count += 1
		
		# Put a hard limit on the number of attempts
		if attempt_count == 15:
			done = true
	
	return ""


func _get_damage_type_string(type: DamageType) -> String:
	match type:
		DamageType.DT_Physical:
			return "Physical"
		DamageType.DT_Fire:
			return "Fire"
		DamageType.DT_Cold:
			return "Cold"
		DamageType.DT_Lightning:
			return "Lightning"
		DamageType.DT_Poison:
			return "Poison"
		DamageType.DT_Arcane:
			return "Arcane"
	
	return "None"


func _convert_to_string(from_type: int, value: Variant) -> String:
	var ret: String = ""
	
	match from_type:
		ValueType.VT_BOOL:
			var bval: bool = value
			if bval:
				ret = "True"
			else:
				ret = "False"
		
		ValueType.VT_INT:
			ret = "%d" % value
		
		ValueType.VT_FLOAT:
			ret = "%f" % value
		
		ValueType.VT_DAMAGE_TYPE:
			# We will deal with this conversion later, after implementing the custom cell
			var dtv: DamageType = value
			ret = _get_damage_type_string(dtv)
		
		_:
			# Texture and multi-line string are already stored as strings, so no conversion is needed here
			ret = value
	
	return ret


func _convert_to_bool(from_type: int, value: Variant) -> bool:
	var ret: bool = false
	
	match from_type:
		ValueType.VT_STRING:
			var strval: String = value
			if strval.to_lower() == "true":
				ret = true
		
		ValueType.VT_INT, ValueType.VT_FLOAT:
			if value > 0:
				ret = true
		
		# No sense in dealing with the other possible source types
	
	return ret


func _convert_to_integer(from_type: int, value: Variant) -> int:
	var ret: int = 0
	
	match from_type:
		ValueType.VT_STRING, ValueType.VT_MULTILINE_STRING:
			var strval: String = value
			if strval.is_valid_int():
				ret = strval.to_int()
			elif strval.is_valid_float():
				ret = int(strval.to_float())
		
		ValueType.VT_BOOL:
			var bval: bool = value
			if bval:
				ret = 1
		
		ValueType.VT_FLOAT:
			var fval: float = value
			ret = int(fval)
		
		ValueType.VT_DAMAGE_TYPE:
			var dt: DamageType = value
			ret = dt
	
	return ret


func _convert_to_float(from_type: int, value: Variant) -> float:
	var ret: float = 0.0
	
	match from_type:
		ValueType.VT_STRING, ValueType.VT_MULTILINE_STRING:
			var strval: String = value
			if strval.is_valid_int():
				ret = float(strval.to_int())
			elif strval.is_valid_float():
				ret = strval.to_float()
		
		ValueType.VT_BOOL:
			var bval: bool = value
			if bval:
				ret = 1.0
		
		ValueType.VT_INT:
			var ival: int = value
			ret = float(ival)
	
	return ret


func _convert_to_texture(from_type: int, value: Variant) -> String:
	var ret: String = ""
	
	match from_type:
		ValueType.VT_STRING, ValueType.VT_MULTILINE_STRING:
			var strval: String = value
			if strval.is_absolute_path() || strval.is_relative_path():
				ret = strval
	
	return ret


func _convert_to_multil_string(from_type: int, value: Variant) -> String:
	var ret: String = ""
	
	match from_type:
		ValueType.VT_STRING, ValueType.VT_INT, ValueType.VT_FLOAT, ValueType.VT_TEXTURE:
			ret = value
		
		ValueType.VT_BOOL:
			var bval: bool = value;
			if bval:
				ret = "True"
			else:
				ret = "False"
		
		ValueType.VT_DAMAGE_TYPE:
			var dt: DamageType = value
			ret = _get_damage_type_string(dt)
	
	return ret


func _convert_to_damage_type(from_type: int, value: Variant) -> DamageType:
	var ret: DamageType = DamageType.DT_None
	
	match from_type:
		ValueType.VT_STRING, ValueType.VT_MULTILINE_STRING:
			var strval: String = value
			match strval:
				"Physical":
					ret = DamageType.DT_Physical
				"Fire":
					ret = DamageType.DT_Fire
				"Cold":
					ret = DamageType.DT_Cold
				"Lightning":
					ret = DamageType.DT_Lightning
				"Poison":
					ret = DamageType.DT_Poison
				"Arcane":
					ret = DamageType.DT_Arcane
		
		ValueType.VT_INT:
			var ival: int = value
			match ival:
				DamageType.DT_Physical:
					ret = DamageType.DT_Physical
				DamageType.DT_Fire:
					ret = DamageType.DT_Fire
				DamageType.DT_Cold:
					ret = DamageType.DT_Cold
				DamageType.DT_Lightning:
					ret = DamageType.DT_Lightning
				DamageType.DT_Poison:
					ret = DamageType.DT_Poison
				DamageType.DT_Arcane:
					ret = DamageType.DT_Arcane
	
	return ret


func _get_default_value_for(type: int) -> Variant:
	var ret: Variant
	match type:
		ValueType.VT_STRING, ValueType.VT_MULTILINE_STRING, ValueType.VT_TEXTURE:
			ret = ""
		ValueType.VT_BOOL:
			ret = false
		ValueType.VT_INT:
			ret = int(0)
		ValueType.VT_FLOAT:
			ret = 0.0
		ValueType.VT_DAMAGE_TYPE:
			ret = DamageType.DT_Physical
	
	return ret


#######################################################################################################################
### Event handlers


#######################################################################################################################
### Overrides
func _get_type_list() -> Dictionary:
	return {
		ValueType.VT_STRING: "String",
		ValueType.VT_BOOL: "Bool",
		ValueType.VT_INT: "Integer",
		ValueType.VT_FLOAT: "Float",
		ValueType.VT_TEXTURE: "Texture",
		ValueType.VT_MULTILINE_STRING: "Multi-line String",
		ValueType.VT_DAMAGE_TYPE: "Damage Type",
	}


func _has_column(title: String) -> bool:
	for col: Dictionary in column_list:
		if col["title"] == title:
			return true
	
	return false


func _get_column_count() -> int:
	return column_list.size()


func _get_column_info(index: int) -> Dictionary:
	if (index < 0 || index >= column_list.size()):
		# In here perhaps print an error message?
		return {}
	
	var column: Dictionary = column_list[index]
	
	var type: ValueType = column["type"]
	var extra: Dictionary = {}
	
	if type == ValueType.VT_DAMAGE_TYPE:
		extra["damage_type"] = [
			{ "type_id": DamageType.DT_None, "name": _get_damage_type_string(DamageType.DT_None) },
			{ "type_id": DamageType.DT_Physical, "name": _get_damage_type_string(DamageType.DT_Physical) },
			{ "type_id": DamageType.DT_Fire, "name": _get_damage_type_string(DamageType.DT_Fire)  },
			{ "type_id": DamageType.DT_Cold, "name": _get_damage_type_string(DamageType.DT_Cold) },
			{ "type_id": DamageType.DT_Lightning, "name": _get_damage_type_string(DamageType.DT_Lightning) },
			{ "type_id": DamageType.DT_Poison, "name": _get_damage_type_string(DamageType.DT_Poison) },
		]
	
	return {
		"title": column["title"],
		"type_code": type,
		"cell_class": type_to_class[type],
		"extra_settings": extra,
	}


func _insert_column(title: String, type: int, index: int, _info: Dictionary, _extra: Dictionary) -> void:
	if title.is_empty():
		title = _generate_title()
		
		if title.is_empty():
			# Perhaps an error message could be printed here?
			return
	else:
		if _has_column(title):
			# Perhaps an error message?
			return
	
	if index < 0 || index > column_list.size():
		# Perhaps an error message here?
		index = column_list.size()
	
	if type < 0:
		type = ValueType.VT_STRING
	
	var column: Dictionary = {
		"title": title,
		"type": type,
	}
	
	@warning_ignore("return_value_discarded")
	column_list.insert(index, column)
	
	# Update rows to contain the new column
	for row: Dictionary in row_list:
		row[title] = _get_default_value_for(type)
	
	notify_new_column(index)


func _remove_column(index: int) -> void:
	if index < 0 || index >= column_list.size():
		return
	
	var column: Dictionary = column_list[index]
	var title: String = column["title"]
	
	# Update the rows
	for row: Dictionary in row_list:
		@warning_ignore("return_value_discarded")
		row.erase(title)
	
	column_list.remove_at(index)
	
	notify_column_removed(index)


func _rename_column(index: int, new_title: String) -> void:
	if index < 0 || index >= column_list.size():
		return
	
	if _has_column(new_title):
		# New title already exists
		# An improvement here could be done, by only notifying about the rejection if the incoming column index
		# is different from the already existing column
		notify_column_rename_rejected(index)
		return
	
	var column: Dictionary = column_list[index]
	var old_title: String = column["title"]
	
	# Column can be renamed, so update existing rows
	for row: Dictionary in row_list:
		ExtPackUtils.change_dictionary_key(row, old_title, new_title)
	
	column["title"] = new_title
	notify_column_renamed(index)


func _change_column_value_type(index: int, new_type: int) -> void:
	if index < 0 || index >= column_list.size():
		return
	
	var vt: ValueType = column_list[index]["type"]
	if vt == new_type:
		# "New type" matches current one. No need to do anything else so bail
		return
	
	# This will be called during the row iteration to perform the conversion. The actual function depends on the "new_type"
	var converter: Callable
	match new_type:
		ValueType.VT_STRING:
			converter = Callable(self, "_convert_to_string")
		ValueType.VT_BOOL:
			converter = Callable(self, "_convert_to_bool")
		ValueType.VT_INT:
			converter = Callable(self, "_convert_to_integer")
		ValueType.VT_FLOAT:
			converter = Callable(self, "_convert_to_float")
		ValueType.VT_TEXTURE:
			converter = Callable(self, "_convert_to_texture")
		ValueType.VT_MULTILINE_STRING:
			converter = Callable(self, "_convert_to_multil_string")
		ValueType.VT_DAMAGE_TYPE:
			converter = Callable(self, "_convert_to_damage_type")
	
	if !converter.is_valid():
		return
	
	column_list[index]["type"] = new_type
	
	var title: String = column_list[index]["title"]
	for row: Dictionary in row_list:
		var stored: Variant = row[title]
		var converted: Variant = converter.call(vt, stored)
		row[title] = converted
	
	notify_type_changed(index)


func _move_column(from: int, to: int) -> void:
	if from == to:
		return
	
	if from < 0 || from >= column_list.size():
		return
	
	if to < 0 || to >= column_list.size():
		return
	
	var column: Dictionary = column_list[from]
	column_list.remove_at(from)
	
	@warning_ignore("return_value_discarded")
	column_list.insert(to, column)
	
	notify_column_moved(from, to)


func _get_row_count() -> int:
	return row_list.size()


func _insert_row(values: Dictionary, index: int) -> void:
	if index < 0 || index > row_list.size():
		index = row_list.size()
	
	var ccount: int = column_list.size()
	var new_row: Dictionary
	
	for c: int in ccount:
		var title: String = column_list[c]["title"]
		
		var value: Variant = values.get(title)
		if !value:
			var type: ValueType = column_list[c].get("type", ValueType.VT_STRING)
			value = _get_default_value_for(type)
		
		new_row[title] = value
	
	@warning_ignore("return_value_discarded")
	row_list.insert(index, new_row)
	
	notify_new_row(index)


func _remove_row(index: int) -> void:
	if index < 0 || index >= row_list.size():
		return
	
	row_list.remove_at(index)
	notify_row_removed(index)


func _move_row(from: int, to: int) -> void:
	if from == to:
		return
	
	if from < 0 || from >= row_list.size():
		return
	
	if to < 0 || to >= row_list.size():
		return
	
	var row: Dictionary = row_list[from];
	row_list.remove_at(from)
	
	@warning_ignore("return_value_discarded")
	row_list.insert(to, row)
	
	notify_row_moved(from, to)


func _get_value(column_index: int, row_index: int) -> Variant:
	if column_index < 0 || column_index >= column_list.size():
		return null
	
	if row_index < 0 || row_index >= row_list.size():
		return null
	
	var title: String = column_list[column_index]["title"]
	return row_list[row_index].get(title, "")


func _set_value(column_index: int, row_index: int, value: Variant) -> void:
	if column_index < 0 || column_index >= column_list.size():
		return
	
	if row_index < 0 || row_index >= row_list.size():
		return
	
	var title: String = column_list[column_index]["title"]
	row_list[row_index][title] = value
	
	notify_value_changed(column_index, row_index, value)


# This is the original function
#func _get_row(index: int) -> Dictionary:
#	assert(index >= 0 && index < row_list.size(), "Attempting to retrieve row data, but incoming index is not valid")
#	return row_list[index].duplicate()

# Modified to take filtering into account
func _get_row(index: int) -> Dictionary:
	assert(index >= 0 && index < row_list.size(), "Attempting to retrieve row data, but incoming index is not valid")
	
	var ret: Dictionary = row_list[index].duplicate()
	if (is_filtering()):
		for cinfo: Dictionary in column_list:
			if (cinfo["type"] == ValueType.VT_DAMAGE_TYPE):
				var cname: String = cinfo["title"]
				
				var dt: DamageType = ret[cname]
				ret[cname] = _get_damage_type_string(dt)
	
	return ret
