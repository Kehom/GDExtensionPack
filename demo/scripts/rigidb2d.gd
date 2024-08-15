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

class_name RigidB2D
extends RigidBody2D

# This is just a little extension of the RigidBody2D that allows directly
# setting the state, which must be done through the _integrate_forces() function

#######################################################################################################################
### Signals and definitions
signal reset_done()

#######################################################################################################################
### "Public" properties


#######################################################################################################################
### "Public" functions
func reset_to(t: Transform2D) -> void:
	_transform = t
	_reset = true

#######################################################################################################################
### "Private" definitions


#######################################################################################################################
### "Private" properties
var _reset: bool = false
var _transform: Transform2D = Transform2D()
var _emit: bool = false

#######################################################################################################################
### "Private" functions


#######################################################################################################################
### Event handlers


#######################################################################################################################
### Overrides
func _integrate_forces(state: PhysicsDirectBodyState2D) -> void:
	if (_reset):
		_reset = false
		state.set_transform(_transform)
		state.sleeping = false
		state.angular_velocity = 0.0
		state.linear_velocity = Vector2()
		
		_emit = true
	
	## Integrate gravity
	state.linear_velocity.y += 98.0 * state.get_step()


func _process(_dt: float) -> void:
	if (_emit):
		reset_done.emit()
		_emit = false
