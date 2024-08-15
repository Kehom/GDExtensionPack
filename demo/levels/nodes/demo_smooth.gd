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
var _original_physics_fps: int = 60
var _original_jitter: float = 0.5
var _original_state: Dictionary
var _original_vsync: int = DisplayServer.VSYNC_ENABLED

@onready var _chk_useteleport: CheckBox = ($ui/expanel/settings/chk_useteleport as CheckBox)

#######################################################################################################################
### "Private" functions


#######################################################################################################################
### Event handlers
func _on_reset_normal3d(node: Smooth3D, t: Transform3D) -> void:
	if (_chk_useteleport.button_pressed):
		node.teleport_to(t)

func _on_reset_auto3d(node: AutoInterpolate, t: Transform3D) -> void:
	if (_chk_useteleport.button_pressed):
		node.teleport_to3d(t)


func _on_reset_normal2d(node: Smooth2D, t: Transform2D) -> void:
	if (_chk_useteleport.button_pressed):
		node.teleport_to(t)

func _on_reset_auto2d(node: AutoInterpolate, t: Transform2D) -> void:
	if (_chk_useteleport.button_pressed):
		node.teleport_to2d(t)



func _on_sl_physicsfps_value_changed(value: float) -> void:
	Engine.physics_ticks_per_second = int(value)


func _on_opt_vsync_item_selected(index: int) -> void:
	var opt: OptionButton = ($ui/expanel/settings/opt_vsync as OptionButton)
	var txt: String = opt.get_item_text(index)
	match txt:
		"Disabled":
			DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_DISABLED)
		"Enabled":
			DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_ENABLED)
		"Adaptive":
			DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_ADAPTIVE)
		"Mailbox":
			DisplayServer.window_set_vsync_mode(DisplayServer.VSYNC_MAILBOX)


func _on_chk_showghost_toggled(toggled_on: bool) -> void:
	($s2d/smooth/ghost as Sprite2D).visible = toggled_on
	($s2d/auto/ghost as Sprite2D).visible = toggled_on
	
	($s3d/smooth/ghost as MeshInstance3D).visible = toggled_on
	($s3d/auto/ghost as MeshInstance3D).visible = toggled_on


func _on_chk_show_2d_toggled(toggled_on: bool) -> void:
	($s2d as Node2D).visible = toggled_on

func _on_chk_show_3d_toggled(toggled_on: bool) -> void:
	($s3d as Node3D).visible = toggled_on


func _on_chk_enable_2d_toggled(toggled_on: bool) -> void:
	var s2d: Smooth2D = ($s2d/smooth/smoother as Smooth2D)
	s2d.interpolate_translation = Smooth2D.Full if toggled_on else Smooth2D.Snap
	s2d.interpolate_orientation = Smooth2D.Full if toggled_on else Smooth2D.Snap
	
	var a2d: AutoInterpolate = ($s2d/auto/smoother as AutoInterpolate)
	a2d.interpolate_translation = AutoInterpolate.Full if toggled_on else AutoInterpolate.Snap
	a2d.interpolate_orientation = AutoInterpolate.Full if toggled_on else AutoInterpolate.Snap


func _on_chk_enable_3d_toggled(toggled_on: bool) -> void:
	var s3d: Smooth3D = ($s3d/smooth/smoother as Smooth3D)
	s3d.interpolate_translation =  Smooth3D.Full if toggled_on else Smooth3D.Snap
	s3d.interpolate_orientation = Smooth3D.Full if toggled_on else Smooth3D.Snap
	
	var a3d: AutoInterpolate = ($s3d/auto/smoother as AutoInterpolate)
	a3d.interpolate_translation = AutoInterpolate.Full if toggled_on else AutoInterpolate.Snap
	a3d.interpolate_orientation = AutoInterpolate.Full if toggled_on else AutoInterpolate.Snap


func _on_bt_replay_pressed() -> void:
	# Wake Objects up
	($s2d/normal as RigidB2D).sleeping = false
	($s2d/smooth as RigidB2D).sleeping = false
	($s2d/auto as RigidB2D).sleeping = false
	($s3d/normal as RigidB3D).sleeping = false
	($s3d/smooth as RigidB3D).sleeping = false
	($s3d/auto as RigidB3D).sleeping = false
	
	# Reset 2D objects
	var nt2d: Transform2D = _original_state.normal2d
	var st2d: Transform2D = _original_state.smooth2d
	var at2d: Transform2D = _original_state.auto2d
	
	var nt3d: Transform3D = _original_state.normal3d
	var st3d: Transform3D = _original_state.smooth3d
	var at3d: Transform3D = _original_state.auto3d
	
	($s2d/normal as RigidB2D).reset_to(nt2d)
	($s2d/smooth as RigidB2D).reset_to(st2d)
	($s2d/auto as RigidB2D).reset_to(at2d)
	
	($s3d/normal as RigidB3D).reset_to(nt3d)
	($s3d/smooth as RigidB3D).reset_to(st3d)
	($s3d/auto as RigidB3D).reset_to(at3d)



func _on_bt_back_pressed() -> void:
	@warning_ignore("return_value_discarded")
	get_tree().change_scene_to_file("res://levels/entry.tscn")


#######################################################################################################################
### Overrides
func _enter_tree() -> void:
	_original_physics_fps = Engine.physics_ticks_per_second
	Engine.physics_ticks_per_second = int(($ui/expanel/settings/sl_physicsfps as SpinSlider).get_value())
	
	_original_jitter = Engine.physics_jitter_fix
	Engine.physics_jitter_fix = 0.0
	
	_original_vsync = DisplayServer.window_get_vsync_mode()



func _exit_tree() -> void:
	Engine.physics_ticks_per_second = _original_physics_fps
	Engine.physics_jitter_fix = _original_jitter
	
	DisplayServer.window_set_vsync_mode(_original_vsync)


func _ready() -> void:
	_original_state = {
		# The 3D nodes
		"normal3d": ($s3d/normal as RigidB3D).global_transform,
		"smooth3d": ($s3d/smooth as RigidB3D).global_transform,
		"auto3d": ($s3d/auto as RigidB3D).global_transform,
		
		# The 2D nodes
		"normal2d": ($s2d/normal as RigidB2D).global_transform,
		"smooth2d": ($s2d/smooth as RigidB2D).global_transform,
		"auto2d": ($s2d/auto as RigidB2D).global_transform,
	}
	
	@warning_ignore("return_value_discarded")
	($s3d/normal as RigidB3D).connect("reset_done", Callable(self, "_on_reset_normal3d").bind(($s3d/smooth/smoother as Smooth3D), _original_state.smooth3d))
	
	@warning_ignore("return_value_discarded")
	($s3d/auto as RigidB3D).connect("reset_done", Callable(self, "_on_reset_auto3d").bind(($s3d/auto/smoother as AutoInterpolate), _original_state.auto3d))
	
	@warning_ignore("return_value_discarded")
	($s2d/smooth as RigidB2D).connect("reset_done", Callable(self, "_on_reset_normal2d").bind(($s2d/smooth/smoother as Smooth2D), _original_state.smooth2d))
	
	@warning_ignore("return_value_discarded")
	($s2d/auto as RigidB2D).connect("reset_done", Callable(self, "_on_reset_auto2d").bind(($s2d/auto/smoother as AutoInterpolate), _original_state.auto2d))
