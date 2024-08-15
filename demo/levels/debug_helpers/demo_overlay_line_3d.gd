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


extends Node3D


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
@onready var _bt_valign_top: Button = ($ui/expanel/settings/hbox_valign/bt_top as Button)
@onready var _bt_valign_center: Button = ($ui/expanel/settings/hbox_valign/bt_center as Button)
@onready var _bt_valign_bottom: Button = ($ui/expanel/settings/hbox_valign/bt_bottom as Button)
@onready var _bt_halign_left: Button = ($ui/expanel/settings/hbox_halign/bt_left as Button)
@onready var _bt_halign_center: Button = ($ui/expanel/settings/hbox_halign/bt_center as Button)
@onready var _bt_halign_right: Button = ($ui/expanel/settings/hbox_halign/bt_right as Button)


#######################################################################################################################
### "Private" functions


#######################################################################################################################
### Event handlers
func _on_chk_overlay_toggled(toggled_on: bool) -> void:
	OverlayInfo.visible = toggled_on


func _on_chk_nontimed_toggled(toggled_on: bool) -> void:
	if (!toggled_on):
		OverlayInfo.remove_label("fps")
		OverlayInfo.remove_label("physicsfps")
		OverlayInfo.remove_label("winsize");
		OverlayInfo.remove_label("winpos")


func _on_bt_top_toggledv(toggled_on: bool) -> void:
	if (toggled_on):
		# OK, actively attempting to set vertical align to top
		OverlayInfo.set_vertical_align_top()
		_bt_valign_center.set_pressed_no_signal(false)
		_bt_valign_bottom.set_pressed_no_signal(false)
	
	else:
		# If here then attempting to directly toggle the button (by clicking). Don't allow it
		_bt_valign_top.set_pressed_no_signal(true)


func _on_bt_center_toggledv(toggled_on: bool) -> void:
	if (toggled_on):
		OverlayInfo.set_vertical_align_center()
		_bt_valign_top.set_pressed_no_signal(false)
		_bt_valign_bottom.set_pressed_no_signal(false)
	
	else:
		_bt_valign_center.set_pressed_no_signal(true)


func _on_bt_bottom_toggledv(toggled_on: bool) -> void:
	if (toggled_on):
		OverlayInfo.set_vertical_align_bottom()
		_bt_valign_top.set_pressed_no_signal(false)
		_bt_valign_center.set_pressed_no_signal(false)
	
	else:
		_bt_valign_bottom.set_pressed_no_signal(false)


func _on_bt_left_toggledh(toggled_on: bool) -> void:
	if (toggled_on):
		OverlayInfo.set_horizontal_align_left()
		_bt_halign_center.set_pressed_no_signal(false)
		_bt_halign_right.set_pressed_no_signal(false)
	
	else:
		_bt_halign_left.set_pressed_no_signal(true)


func _on_bt_center_toggledh(toggled_on: bool) -> void:
	if (toggled_on):
		OverlayInfo.set_horizontal_align_center()
		_bt_halign_left.set_pressed_no_signal(false)
		_bt_halign_right.set_pressed_no_signal(false)
	
	else:
		_bt_halign_center.set_pressed_no_signal(true)


func _on_bt_right_toggledh(toggled_on: bool) -> void:
	if (toggled_on):
		OverlayInfo.set_horizontal_align_right()
		_bt_halign_left.set_pressed_no_signal(false)
		_bt_halign_center.set_pressed_no_signal(false)
	
	else:
		_bt_halign_right.set_pressed_no_signal(true)


func _on_bt_gdversion_pressed() -> void:
	var duration: float = ($ui/expanel/settings/hbox_duration/sp_lblduration as SpinSlider).get_value()
	var vstr: String = Engine.get_version_info().string
	OverlayInfo.add_timed_label(vstr, duration)


func _on_bt_randomnum_pressed() -> void:
	var rnd: int = randi()
	var duration: float = ($ui/expanel/settings/hbox_duration/sp_lblduration as SpinSlider).get_value()
	OverlayInfo.add_timed_label("%s" % rnd, duration)


func _on_chk_line_3d_enabled_toggled(toggled_on: bool) -> void:
	Line3D.enabled = toggled_on
	if (!toggled_on):
		Line3D.clear_lines()


func _on_bt_back_pressed() -> void:
	@warning_ignore("return_value_discarded")
	get_tree().change_scene_to_file("res://levels/entry.tscn")


#######################################################################################################################
### Overrides
func _physics_process(dt: float) -> void:
	if (($ui/expanel/settings/chk_nontimed as CheckBox).button_pressed && ($ui/expanel/settings/chk_overlay as CheckBox).button_pressed):
		# Show frames per second
		OverlayInfo.set_label("fps", "FPS: %s" % Engine.get_frames_per_second())
		# Show number of physics iterations per second
		OverlayInfo.set_label("physicsfps", "Physics %s/s" % Engine.physics_ticks_per_second)
		# Show Window size
		OverlayInfo.set_label("winsize", "Window size: %s" % get_window().size)
		# Show window position
		OverlayInfo.set_label("winpos", "Window position: %s" % get_window().position)
	
	var rspeed: float = ($ui/expanel/settings/hbox_speed/sp_spherespeed as SpinSlider).get_value()
	
	# Before rotating the pivots, take current positions to create a vector indicating the movement direction
	# of the spheres. Those vectors will be drawn as "one frame" ones.
	var p00: Vector3 = ($pivot1/point as MeshInstance3D).global_position
	var p10: Vector3 = ($pivot2/point as MeshInstance3D).global_position
	
	($pivot1 as Node3D).rotation.x += rspeed * dt
	($pivot2 as Node3D).rotation.x -= rspeed * dt
	
	var p0: Vector3 = ($pivot1/point as MeshInstance3D).global_position
	var p1: Vector3 = ($pivot2/point as MeshInstance3D).global_position
	
	var line_duration: float = ($ui/expanel/settings/hbox_lineduration/sp_lineduration as SpinSlider).get_value()
	Line3D.add_timed_line(p0, p1, line_duration, Color(0.85, 0.1, 0.1, 1.0))
	
	# The idea is to display the direction vectors with 1.5 length
	var dir1: Vector3 = (p0 - p00).normalized() * 1.5
	var dir2: Vector3 = (p1 - p10).normalized() * 1.5
	
	#Line3D.add_from_vector(p0, dir1, Color(0.1, 0.85, 0.1, 1.0))
	Line3D.add_from_vector(p0, dir1, Color.YELLOW)
	#Line3D.add_from_vector(p1, dir2, Color(0.1, 0.1, 0.85, 1.0))
	Line3D.add_from_vector(p1, dir2, Color.BLUE)
	#Color.RED


func _enter_tree() -> void:
	randomize()
	OverlayInfo.theme = load("res://resources/other/ui.theme")
	OverlayInfo.show()
	OverlayInfo.set_horizontal_align_right()
	OverlayInfo.set_vertical_align_top()


func _exit_tree() -> void:
	OverlayInfo.clear_labels()
	OverlayInfo.hide()
