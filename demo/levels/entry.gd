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


#######################################################################################################################
### "Private" functions
func _hide_all_from(p: Control) -> void:
	for ctrl: Control in p.get_children():
		ctrl.visible = false

#######################################################################################################################
### Event handlers
func _on_bt_info_toggled(toggled_on: bool) -> void:
	($ui/panel/info_main as Control).visible = toggled_on


func _on_bt_overlayinfo_toggled(toggled_on: bool) -> void:
	($ui/panel/dbghelpers/ui_demo_info as Control).visible = toggled_on


func _on_bt_quantize_toggled(toggled_on: bool) -> void:
	($ui/panel/general/info_quantize as Control).visible = toggled_on


func _on_bt_audiomaster_toggled(toggled_on: bool) -> void:
	($ui/panel/nodes/info_audiomaster as Control).visible = toggled_on


func _on_bt_rimpulse_toggled(toggled_on: bool) -> void:
	($ui/panel/nodes/info_radimpulse as Control).visible = toggled_on


func _on_bt_smooth_toggled(toggled_on: bool) -> void:
	($ui/panel/nodes/info_smooth as Control).visible = toggled_on


func _on_bt_tabularbox_toggled(toggled_on: bool) -> void:
	($ui/panel/ui/info_tabularbox as Control).visible = toggled_on


func _on_bt_quit_pressed() -> void:
	get_tree().quit()


#######################################################################################################################
### Overrides
func _ready() -> void:
	# Ensure the "Entry" page is visible and all others are hidden, regardless of how things are setup in the editor
	($ui/panel/info_main as Control).visible = true
	
	_hide_all_from(($ui/panel/dbghelpers as Control))
	_hide_all_from(($ui/panel/general as Control))
	_hide_all_from(($ui/panel/nodes as Control))
	_hide_all_from(($ui/panel/ui) as Control)
