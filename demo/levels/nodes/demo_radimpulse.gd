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
var _radius2d: int = 60

var _radius3d: float = 1.0

# This will be used to flag if the 3D scene was seen at least one time. If so, no need to attempt to reactivate the balls
var _seen3d: bool = false

#######################################################################################################################
### "Private" functions
func _set_radius2d(val: int) -> void:
	_radius2d = clampi(val, 32, 500)
	
	# Calculate the scale to be set within the "visual" of the RadialImpulse2D. The texture is 40x40 so dividing the
	# requested radius by 20 should give the appropriate scale value
	var s: float = float(_radius2d) / 20.0
	
	($setup2d/radimpulse/visual as Sprite2D).scale = Vector2(s, s)
	($setup2d/radimpulse as RadialImpulse2D).radius = _radius2d


func _set_radius3d(val: float) -> void:
	_radius3d = clampf(val, 0.2, 5.0)
	
	var sphere: SphereMesh = ($setup3d/radimpulse/visual as MeshInstance3D).mesh
	sphere.radius = _radius3d
	sphere.height = _radius3d * 2.0
	($setup3d/radimpulse as RadialImpulse3D).radius = _radius3d


func _handle_input2d(evt: InputEvent) -> void:
	var mb: InputEventMouseButton = evt as InputEventMouseButton
	if (mb && mb.is_pressed()):
		match mb.button_index:
			MOUSE_BUTTON_LEFT:
				($setup2d/radimpulse as RadialImpulse2D).apply_impulse()
			
			MOUSE_BUTTON_WHEEL_UP:
				_set_radius2d(_radius2d + 5)
			
			MOUSE_BUTTON_WHEEL_DOWN:
				_set_radius2d(_radius2d - 5)
	
	var mm: InputEventMouseMotion = evt as InputEventMouseMotion
	if (mm):
		if (($setup2d/rect_helper as Control).get_rect().has_point(mm.global_position)):
			($setup2d/radimpulse as RadialImpulse2D).global_position = mm.global_position


func _handle_input3d(evt: InputEvent) -> void:
	var mb: InputEventMouseButton = evt as InputEventMouseButton
	if (mb && mb.is_pressed()):
		match mb.button_index:
			MOUSE_BUTTON_LEFT:
				($setup3d/radimpulse as RadialImpulse3D).apply_impulse()
			
			MOUSE_BUTTON_WHEEL_UP:
				_set_radius3d(_radius3d + 0.1)
			
			MOUSE_BUTTON_WHEEL_DOWN:
				_set_radius3d(_radius3d - 0.1)
	
	var mm: InputEventMouseMotion = evt as InputEventMouseMotion
	if (mm):
		# Perform a raycast from "mouse position" and attempt to find the "floor" bellow. Position the RadialImpulse3D at
		# that location
		var cam: Camera3D = ($setup3d/env/cam as Camera3D)
		var ray_from: Vector3 = cam.project_ray_origin(mm.global_position)
		var ray_dir: Vector3 = cam.project_ray_normal(mm.global_position)
		var ray_to: Vector3 = ray_from + (ray_dir * 10000.0)
		
		var ray_params: PhysicsRayQueryParameters3D = PhysicsRayQueryParameters3D.new()
		ray_params.from = ray_from
		ray_params.to = ray_to
		ray_params.collision_mask = 2
		
		var hit: Dictionary = get_world_3d().direct_space_state.intersect_ray(ray_params)
		if (hit.size() > 0):
			var rimp: RadialImpulse3D = ($setup3d/radimpulse as RadialImpulse3D)
			
			var fpos: Vector3 = hit.position
			fpos.y = 0
			rimp.global_transform.origin = fpos


func _set_3d_sleeping_state(sleep: bool) -> void:
	for ball: RigidBody3D in ($setup3d/rigid as Node3D).get_children():
		ball.sleeping = sleep
		ball.freeze = sleep



#######################################################################################################################
### Event handlers
func _on_chk_2d_toggled(toggled_on: bool) -> void:
	($setup2d as Node2D).visible = toggled_on


func _on_opt_falloff_2d_item_selected(index: int) -> void:
	match index:
		RadialImpulse2D.CONSTANT:
			($setup2d/radimpulse as RadialImpulse2D).falloff = RadialImpulse2D.CONSTANT
		RadialImpulse2D.LINEAR:
			($setup2d/radimpulse as RadialImpulse2D).falloff = RadialImpulse2D.LINEAR


func _on_sp_impulse_2d_value_changed(value: float) -> void:
	($setup2d/radimpulse as RadialImpulse2D).force = value


func _on_chk_3d_toggled(toggled_on: bool) -> void:
	($setup3d as Node3D).visible = toggled_on
	
	if (!_seen3d):
		_seen3d = true
		_set_3d_sleeping_state(false)


func _on_opt_falloff_3d_item_selected(index: int) -> void:
	match index:
		RadialImpulse3D.CONSTANT:
			($setup3d/radimpulse as RadialImpulse3D).falloff = RadialImpulse3D.CONSTANT
		RadialImpulse3D.LINEAR:
			($setup3d/radimpulse as RadialImpulse3D).falloff = RadialImpulse3D.LINEAR


func _on_sp_impulse_3d_value_changed(value: float) -> void:
	($setup3d/radimpulse as RadialImpulse3D).force = value




func _on_bt_back_pressed() -> void:
	@warning_ignore("return_value_discarded")
	get_tree().change_scene_to_file("res://levels/entry.tscn")


#######################################################################################################################
### Overrides
func _unhandled_input(evt: InputEvent) -> void:
	if (($setup2d as Node2D).visible):
		_handle_input2d(evt)
	
	if (($setup3d as Node3D).visible):
		_handle_input3d(evt)



func _ready() -> void:
	_set_radius2d(60)
	
	# Ensure 2D is visible by default, regardless of what is shown in the editor
	($setup2d as Node2D).visible = true
	($setup3d as Node3D).visible = false
	
	# Begin with all 3D balls sleeping - so when changing into 3D view the falling can be seen
	_set_3d_sleeping_state(true)
