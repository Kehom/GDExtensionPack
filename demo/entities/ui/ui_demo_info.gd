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

@tool
extends MarginContainer


#######################################################################################################################
### Signals and definitions


#######################################################################################################################
### "Public" properties
@export_file("*.tscn") var scene: String = ""
@export var no_load: bool = false:
	set(val):
		no_load = val
		if (is_inside_tree()):
			($vbox/bt_load as Button).visible = !no_load

@export var title: String = "":
	set(val):
		title = val
		if (is_inside_tree()):
			(%lbl_title as Label).text = val

@export_multiline var description: String = "":
	set(val):
		description = val
		if (is_inside_tree()):
			(%lbl_description as Label).text = val

#######################################################################################################################
### "Public" functions


#######################################################################################################################
### "Private" definitions


#######################################################################################################################
### "Private" properties


#######################################################################################################################
### "Private" functions


#######################################################################################################################
### Event handlers
func _on_bt_load_pressed() -> void:
	if (!is_inside_tree()):
		# This should not happen... but...
		return
	
	if (scene.is_empty()):
		return
	
	@warning_ignore("return_value_discarded")
	get_tree().change_scene_to_file(scene)


#######################################################################################################################
### Overrides
func _enter_tree() -> void:
	(%lbl_title as Label).text = title
	(%lbl_description as Label).text = description
	($vbox/bt_load as Button).visible = !no_load
