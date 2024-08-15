# Copyright (c) 2024 Yuri Sarudiansky
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


extends Node2D


#######################################################################################################################
### Signals and definitions


#######################################################################################################################
### "Public" properties


#######################################################################################################################
### "Public" functions


#######################################################################################################################
### "Private" definitions


#######################################################################################################################
### "Private" properties
#@onready var _tbox: TabularBox = ($ui/tbox as TabularBox)
@onready var _tbox: TabularBox = ($ui/tabular_box)

#######################################################################################################################
### "Private" functions


#######################################################################################################################
### Event handlers
func _on_txt_filter_text_changed(new_text: String) -> void:
	var tbox: TabularBox = ($ui/tabular_box as TabularBox)
	if (!tbox.data_source):
		return
	
	tbox.data_source.set_filter(new_text, false)


func _on_opt_title_align_item_selected(index: int) -> void:
	var align: HorizontalAlignment = index as HorizontalAlignment
	_tbox.title_horizontal_alignment = align


func _on_opt_rnum_align_item_selected(index: int) -> void:
	var align: HorizontalAlignment = index as HorizontalAlignment
	_tbox.row_numbers_alignment = align


func _on_chk_showrnum_toggled(toggled_on: bool) -> void:
	_tbox.show_row_numbers = toggled_on


func _on_chk_showchks_toggled(toggled_on: bool) -> void:
	_tbox.show_row_checkboxes = toggled_on


func _on_chk_allowctxmenu_toggled(toggled_on: bool) -> void:
	_tbox.allow_context_menu = toggled_on


func _on_bt_save_pressed() -> void:
	var tbox: TabularBox = ($ui/tabular_box as TabularBox)
	
	if (!tbox.data_source):
		return
	
	if (ResourceSaver.save(tbox.data_source) == OK):
		print("Saved the data source!")
	else:
		print("Failed to save the data source!")


func _on_bt_back_pressed() -> void:
	@warning_ignore("return_value_discarded")
	get_tree().change_scene_to_file("res://levels/entry.tscn")

#######################################################################################################################
### Overrides

func _ready() -> void:
	
	pass
