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
class_name TabularBoxCellDropdown
extends TabularBoxCell


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
var _optbutton: OptionButton = OptionButton.new()

#######################################################################################################################
### "Private" functions


#######################################################################################################################
### Event handlers
func _on_item_selected(index: int) -> void:
	var id: int = _optbutton.get_item_id(index)
	notify_value_changed(id)



#######################################################################################################################
### Overrides
func _check_theme() -> void:
	var im_left: float = get_internal_margin_left()
	var im_top: float = get_internal_margin_top()
	var im_right: float = get_internal_margin_right()
	var im_bottom: float = get_internal_margin_bottom()
	
	apply_button_style(_optbutton)
	var btheight: float = get_button_height()
	var half_btheight: float = btheight * 0.5
	
	_optbutton.set_offset(SIDE_LEFT, im_left)
	_optbutton.set_offset(SIDE_TOP, -half_btheight)
	_optbutton.set_offset(SIDE_RIGHT, -im_right)
	_optbutton.set_offset(SIDE_BOTTOM, half_btheight)
	
	set_min_height(int(im_top + btheight + im_bottom))


func _apply_extra_settings(extra_settings: Dictionary) -> void:
	var type_array: Array = extra_settings.get("damage_type", Array())
	
	for type: Dictionary in type_array:
		var tpname: String = type["name"]
		var tpid: int = type["type_id"]
		
		_optbutton.add_item(tpname, tpid)


func _assign_value(value: Variant) -> void:
	var id: int = value
	var index: int = _optbutton.get_item_index(id)
	
	_optbutton.selected = index


func _init() -> void:
	add_child(_optbutton)
	_optbutton.mouse_filter = Control.MOUSE_FILTER_PASS
	
	_optbutton.set_anchor(SIDE_LEFT, 0)
	_optbutton.set_anchor(SIDE_TOP, 0.5)
	_optbutton.set_anchor(SIDE_RIGHT, 1)
	_optbutton.set_anchor(SIDE_BOTTOM, 0.5)
	
	@warning_ignore("return_value_discarded")
	_optbutton.connect("item_selected", Callable(self, "_on_item_selected"))
