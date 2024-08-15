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
var _pivot_yrotation: float = -1.5            # Negative value for clockwise rotation
var _pivot_zrotation: float = 45.0

var _cube_xrotation: float = 2.0

var _pivot_zstate: float = 0.0
var _pivot_zdir: float = 1.0

var _simulate_replication: Callable = Callable(self, "_simulate_replication_9bits")

@onready var _slpivy: SpinSlider = ($ui/expanel/settings/sl_pivyrotation as SpinSlider)
@onready var _slpivz: SpinSlider = ($ui/expanel/settings/sl_pivzrotation as SpinSlider)
@onready var _slcubex: SpinSlider = ($ui/expanel/settings/sl_cubexrotation as SpinSlider)

var _rng: RandomNumberGenerator = RandomNumberGenerator.new()

var _original1: float = 0
var _original2: float = 0
var _original3: float = 0

var _original4: Quaternion = Quaternion()
var _original5: Quaternion = Quaternion()


#######################################################################################################################
### "Private" functions
func _calculate_error(value: float, expected: float) -> float:
	return abs((expected - value) / value)


func _euler_to_quat(rx: float, ry: float, rz: float) -> Quaternion:
	var roll: float = deg_to_rad(rx)
	var pitch: float = deg_to_rad(ry)
	var yaw: float = deg_to_rad(rz)
	
	var cr: float = cos(roll * 0.5)
	var sr: float = sin(roll * 0.5)
	var cp: float = cos(pitch * 0.5)
	var sp: float = sin(pitch * 0.5)
	var cy: float = cos(yaw * 0.5)
	var sy: float = sin(yaw * 0.5)
	
	var w: float = cr * cp * cy + sr * sp * sy
	var x: float = sr * cp * cy - cr * sp * sy
	var y: float = cr * sp * cy + sr * cp * sy
	var z: float = cr * cp * sy - sr * sp * cy
	
	
	return Quaternion(x, y, z, w)

func _get_quat_diff(value: Quaternion, expected: Quaternion) -> Quaternion:
	# Rotation quaternions might have signals flipped. Flip the entire quaternion if the first component don't match
	# signals
	if ((value.x > 0 && expected.x < 0) || (value.x < 0 && expected.x > 0)):
		expected = expected * -1
	
	var dx: float = _calculate_error(value.x, expected.x)
	var dy: float = _calculate_error(value.y, expected.y)
	var dz: float = _calculate_error(value.z, expected.z)
	var dw: float = _calculate_error(value.w, expected.w)
	
	
	return Quaternion(dx, dy, dz, dw)



func _simulate_replication_9bits() -> void:
	# First take pivot orientation as a quaternion
	var pivq: Quaternion = Quaternion(($source as Node3D).transform.basis)
	
	# Then quantize using the 9 bits version
	var pivc: int = Quantize.compress_rquat_9bits(pivq)
	
	# Restore the quaternion (as it would be done on a remote machine, for example)
	var restpivq: Quaternion = Quantize.restore_rquat_9bits(pivc)
	
	# Apply the replicated pivot
	($replicated as Node3D).transform.basis = Basis(restpivq)
	
	# Now take the Cube orientation as a Quaternion
	var srccubeq: Quaternion = Quaternion(($source/cube as MeshInstance3D).transform.basis)
	
	# Quantize it using the 9 bits version
	var srccubec: int = Quantize.compress_rquat_9bits(srccubeq)
	
	# Perform the restoration as if it was a remote machine
	var restcubeq: Quaternion = Quantize.restore_rquat_9bits(srccubec)
	
	# Finally apply the restored data into the "replicated" cube
	($replicated/cube as MeshInstance3D).transform.basis = Basis(restcubeq)



func _simulate_replication_10bits() -> void:
	var pivq: Quaternion = Quaternion(($source as Node3D).transform.basis)
	var pivc: int = Quantize.compress_rquat_10bits(pivq)
	var restpivq: Quaternion = Quantize.restore_rquat_10bits(pivc)
	($replicated as Node3D).transform.basis = Basis(restpivq)
	
	var srccubeq: Quaternion = Quaternion(($source/cube as MeshInstance3D).transform.basis)
	var srccubec: int = Quantize.compress_rquat_10bits(srccubeq)
	var restcubeq: Quaternion = Quantize.restore_rquat_10bits(srccubec)
	($replicated/cube as MeshInstance3D).transform.basis = Basis(restcubeq)


func _simulate_replication_15_bits() -> void:
	var pivq: Quaternion = Quaternion(($source as Node3D).transform.basis)
	var pivc: PackedInt32Array = Quantize.compress_rquat_15bits(pivq)
	var restpivq: Quaternion = Quantize.restore_rquat_15bits(pivc[0], pivc[1])
	($replicated as Node3D).transform.basis = Basis(restpivq)
	
	var srccubeq: Quaternion = Quaternion(($source/cube as MeshInstance3D).transform.basis)
	var srccubec: PackedInt32Array = Quantize.compress_rquat_15bits(srccubeq)
	var restcubeq: Quaternion = Quantize.restore_rquat_15bits(srccubec[0], srccubec[1])
	($replicated/cube as MeshInstance3D).transform.basis = Basis(restcubeq)


func _update_grid() -> void:
	var num_bits: int = int(($ui/panel/hbox/sp_bits as SpinSlider).get_value())
	
	var quantized1: int = Quantize.quantize_unit_float(_original1, num_bits)
	var restored1: float = Quantize.restore_unit_float(quantized1, num_bits)
	var diff1: float = _calculate_error(restored1, _original1)
	
	($ui/panel/grid/lbl_original1 as Label).text = "%s" % _original1
	($ui/panel/grid/lbl_quantized1 as Label).text = "%s" % quantized1
	($ui/panel/grid/lbl_restored1 as Label).text = "%s" % restored1
	($ui/panel/grid/lbl_difference1 as Label).text = "%s" % diff1
	
	var quantized2: int = Quantize.quantize_unit_float(_original2, num_bits)
	var restored2: float = Quantize.restore_unit_float(quantized2, num_bits)
	#var diff2: float = abs(restored2 - _original2)
	var diff2: float = _calculate_error(restored2, _original2)
	
	($ui/panel/grid/lbl_original2 as Label).text = "%s" % _original2
	($ui/panel/grid/lbl_quantized2 as Label).text = "%s" % quantized2
	($ui/panel/grid/lbl_restored2 as Label).text = "%s" % restored2
	($ui/panel/grid/lbl_difference2 as Label).text = "%s" % diff2
	
	var quantized3: int = Quantize.quantize_unit_float(_original3, num_bits)
	var restored3: float = Quantize.restore_unit_float(quantized3, num_bits)
	var diff3: float = abs(restored3 - _original3)
	
	($ui/panel/grid/lbl_original3 as Label).text = "%s" % _original3
	($ui/panel/grid/lbl_quantized3 as Label).text = "%s" % quantized3
	($ui/panel/grid/lbl_restored3 as Label).text = "%s" % restored3
	($ui/panel/grid/lbl_difference3 as Label).text = "%s" % diff3
	
	var quantized4: Dictionary = Quantize.compress_rotation_quat(_original4, num_bits)
	var restored4: Quaternion = Quantize.restore_rotation_quat(quantized4, num_bits)
	var diff4: Quaternion = _get_quat_diff(restored4, _original4)
	
	($ui/panel/grid/lbl_original4 as Label).text = "Quat.x: %s\nQuat.y: %s\nQuat.z: %s\nQuat.w: %s" % [_original4.x, _original4.y, _original4.z, _original4.w]
	($ui/panel/grid/lbl_quantized4 as Label).text = "A: %s\nB: %s\nC: %s\nDropped: %s" % [quantized4.a, quantized4.b, quantized4.c, quantized4.index]
	($ui/panel/grid/lbl_restored4 as Label).text = "Quat.x: %s\nQuat.y: %s\nQuat.z: %s\nQuat.w: %s" % [restored4.x, restored4.y, restored4.z, restored4.w]
	($ui/panel/grid/lbl_difference4 as Label).text = "x: %s\ny: %s\nz: %s\nw: %s" % [diff4.x, diff4.y, diff4.z, diff4.w]
	
	
	#var quantized5: int = Quantize.compress_rquat_9bits(_original5)
	var quantized5: Dictionary = Quantize.compress_rotation_quat(_original5, num_bits)
	#var restored5: Quaternion = Quantize.restore_rquat_9bits(quantized5)
	var restored5: Quaternion = Quantize.restore_rotation_quat(quantized5, num_bits)
	var diff5: Quaternion = _get_quat_diff(restored5, _original5)
	
	($ui/panel/grid/lbl_original5 as Label).text = "Quat.x: %s\nQuat.y: %s\nQuat.z: %s\nQuat.w: %s" % [_original5.x, _original5.y, _original5.z, _original5.w]
	($ui/panel/grid/lbl_quantized5 as Label).text = "A: %s\nB: %s\nC: %s\nDropped: %s" % [quantized5.a, quantized5.b, quantized5.c, quantized5.index]
	($ui/panel/grid/lbl_restored5 as Label).text = "Quat.x: %s\nQuat.y: %s\nQuat.z: %s\nQuat.w: %s" % [restored5.x, restored5.y, restored5.z, restored5.w]
	($ui/panel/grid/lbl_difference5 as Label).text = "x: %s\ny: %s\nz: %s\nw: %s" % [diff5.x, diff5.y, diff5.z, diff5.w]


func _generate_random_data() -> void:
	_original1 = _rng.randf()
	_original2 = _rng.randf()
	_original3 = _rng.randf()
	
	var rx4: int = _rng.randi_range(-360, 360)
	var ry4: int = _rng.randi_range(-360, 360)
	var rz4: int = _rng.randi_range(-360, 360)
	_original4 = _euler_to_quat(rx4, ry4, rz4)
	
	var rx5: int = _rng.randi_range(-360, 360)
	var ry5: int = _rng.randi_range(-360, 360)
	var rz5: int = _rng.randi_range(-360, 360)
	_original5 = _euler_to_quat(rx5, ry5, rz5)
	
	_update_grid()




#######################################################################################################################
### Event handlers
func _on_sl_pivyrotation_value_changed(value: float) -> void:
	_pivot_yrotation = value

func _on_sl_pivzrotation_value_changed(value: float) -> void:
	_pivot_zrotation = value

func _on_sl_cubexrotation_value_changed(value: float) -> void:
	_cube_xrotation = value


func _on_chk_9_bits_toggled(toggled_on: bool) -> void:
	if (toggled_on):
		_simulate_replication = Callable(self, "_simulate_replication_9bits")

func _on_chk_10_bits_toggled(toggled_on: bool) -> void:
	if (toggled_on):
		_simulate_replication = Callable(self, "_simulate_replication_10bits")

func _on_chk_15_bits_toggled(toggled_on: bool) -> void:
	if (toggled_on):
		_simulate_replication = Callable(self, "_simulate_replication_15_bits")



func _on_chk_random_data_toggled(toggled_on: bool) -> void:
	($ui/panel as Panel).visible = toggled_on


func _on_sp_seed_value_changed(value: float) -> void:
	_rng.seed = int(value)
	_generate_random_data()


func _on_bt_randomize_pressed() -> void:
	_rng.randomize()
	($ui/panel/hbox/sp_seed as SpinSlider).set_value(_rng.seed)
	_generate_random_data()


func _on_sp_bits_value_changed(_value: float) -> void:
	_update_grid()


func _on_bt_back_pressed() -> void:
	@warning_ignore("return_value_discarded")
	get_tree().change_scene_to_file("res://levels/entry.tscn")


#######################################################################################################################
### Overrides
func _physics_process(dt: float) -> void:
	_pivot_zstate += _pivot_zrotation * dt * _pivot_zdir
	
	if (_pivot_zstate >= 45.0):
		_pivot_zdir = -1.0
	elif (_pivot_zstate <= -45.0):
		_pivot_zdir = 1.0
	
	($source as Node3D).rotation.y += _pivot_yrotation * dt
	($source as Node3D).rotation.z = deg_to_rad(_pivot_zstate)
	($source/cube as MeshInstance3D).rotation.x += _cube_xrotation * dt
	
	_simulate_replication.call()
	
	var src_basis: Basis = ($source/cube as MeshInstance3D).basis
	var cpy_basis: Basis = ($replicated/cube as MeshInstance3D).basis
	
	var diffx: Vector3 = abs(cpy_basis.x - src_basis.x)
	var diffy: Vector3 = abs(cpy_basis.y - src_basis.y)
	var diffz: Vector3 = abs(cpy_basis.z - src_basis.z)
	
	($ui/expanel/info/lbl_basisdiff_x as Label).text = "X:\n  %s\n  %s\n  %s" % [diffx.x, diffx.y, diffx.z]
	($ui/expanel/info/lbl_basisdiff_y as Label).text = "Y:\n  %s\n  %s\n  %s" % [diffy.x, diffy.y, diffy.z]
	($ui/expanel/info/lbl_basisdiff_z as Label).text = "Z:\n  %s\n  %s\n  %s" % [diffz.x, diffz.y, diffz.z]



func _ready() -> void:
	_slpivy.set_value(_pivot_yrotation)
	_slpivz.set_value(_pivot_zrotation)
	_slcubex.set_value(_cube_xrotation)
	
	($ui/panel as Panel).visible = ($ui/expanel/settings/chk_random_data as CheckBox).button_pressed
	_rng.seed = int(($ui/panel/hbox/sp_seed as SpinSlider).get_value())
	_generate_random_data()
