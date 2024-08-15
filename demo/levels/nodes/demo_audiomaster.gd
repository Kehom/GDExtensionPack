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
# Sound effects are small enought to be preloaded. Store the AudioStream resources within an array for easier random
# playback of those SFX
const _sfx_list: Array[AudioStream] = [
	preload("res://resources/sfx/impactmining_000.wav"),
	preload("res://resources/sfx/impactmining_001.wav"),
	preload("res://resources/sfx/impactmining_002.wav"),
	preload("res://resources/sfx/impactmining_003.wav"),
	preload("res://resources/sfx/impactmining_004.wav"),
]

# For the music tracks, hold their paths instead. When requesting to playback, using this to load the resource
const _music_tracks: Array[String] = [
	"res://resources/music/action_strike.ogg",
	"res://resources/music/dreams_of_vain.ogg",
	"res://resources/music/epic_boss_battle.ogg",
]

#######################################################################################################################
### "Private" properties
@onready var _amaster: AudioMaster = ($audio_master as AudioMaster)

@onready var _spin_master: SpinSlider = ($ui/expanel/vbsettings/hmaster/sp_vol as SpinSlider)
@onready var _spin_music: SpinSlider = ($ui/expanel/vbsettings/hmusic/sp_vol as SpinSlider)
@onready var _spin_sfxnormal: SpinSlider = ($ui/expanel/vbsettings/hsfx/sp_vol as SpinSlider)
@onready var _spin_sfx2d: SpinSlider = ($ui/expanel/vbsettings/hsfx2D/sp_vol as SpinSlider)
@onready var _spin_sfx3d: SpinSlider = ($ui/expanel/vbsettings/hsfx3D/sp_vol as SpinSlider)
@onready var _spin_ui: SpinSlider = ($ui/expanel/vbsettings/hui/sp_vol as SpinSlider)

@onready var _lbl_info: Label = ($ui/expanel/pnl_info/lbl_info as Label)
@onready var _spin_fade: SpinSlider = ($ui/pnl_bottom/hbox/sp_fade as SpinSlider)
@onready var _progress: ProgressBar = ($ui/pnl_bottom/hbox/bp_palybackpos as ProgressBar)

# Index of currently active music stream player. If -1 then no music is playing. This is required in order to add
# cross fading when switching music
# This index is directly obtained from the AudioMaster. It could easily be associated with the index of the Music
# to be played, however separating that on purpose. Reasons:
# 1 - The first is obviously that it's not necessary to associate "node index" with "music index"
# 2 - The second is that depending on the amount of audio tracks it's obviously not a good idea to perform that kind of
#   association. So, the code in here is meant to show one way to playback music, add cross fading and keep two different
#   indexing in use. One for the node performing the playback and the other telling which file is playing
var _currently_active: int = -1

# This is the "music index" currently playing. See more detailed description above (for the _currently_active)
var _playing_music_index: int = -1

# Cache the duration of the music currently being played
var _active_duration: float = 0.0

# Flag telling if user is attempting to seek playback position
var _seeking: bool = false



#######################################################################################################################
### "Private" functions
func _populate_device_list() -> void:
	var opt: OptionButton = ($ui/expanel/vbsettings/opt_device as OptionButton)
	
	var devlist: PackedStringArray = AudioServer.get_output_device_list()
	for dev: String in devlist:
		opt.add_item(dev)


func _update_volumes() -> void:
	_spin_master.set_value(_amaster.get_bus_volume_percent("Master"))
	_spin_music.set_value(_amaster.get_bus_volume_percent("Music"))
	_spin_sfxnormal.set_value(_amaster.get_bus_volume_percent("SFX"))
	_spin_sfx2d.set_value(_amaster.get_bus_volume_percent("SFX2D"))
	_spin_sfx3d.set_value(_amaster.get_bus_volume_percent("SFX3D"))
	_spin_ui.set_value(_amaster.get_bus_volume_percent("UI"))


func _play_music(track_index: int) -> void:
	_currently_active = _amaster.get_available_player_index("Music")
	_playing_music_index = track_index
	
	var audio: AudioStream = load(_music_tracks[track_index])
	
	_amaster.play_audio("Music", audio, _currently_active, _spin_fade.get_value())
	_active_duration = audio.get_length()
	
	_on_info_updater_timeout()


func _release_button(index: int) -> void:
	# This function exists only because there is a button for each available music track. Obviously this is not exactly
	# very practical when the avaialble music files becomes any larger than this.
	if (index == -1):
		return
	
	match index:
		0:
			($ui/pnl_bottom/hbox/bt_music1 as Button).set_pressed_no_signal(false)
		1:
			($ui/pnl_bottom/hbox/bt_music2 as Button).set_pressed_no_signal(false)
		2:
			($ui/pnl_bottom/hbox/bt_music3 as Button).set_pressed_no_signal(false)


func _stop_music() -> void:
	if (_currently_active == -1):
		return
	
	_amaster.stop("Music", _currently_active, _spin_fade.get_value())
	_release_button(_playing_music_index)
	_playing_music_index = -1
	_currently_active = -1


func _cross_fade(to_audio_index: int) -> void:
	if (_playing_music_index == to_audio_index):
		return
	
	_stop_music()
	_play_music(to_audio_index)



#######################################################################################################################
### Event handlers
func _on_audio_master_playback_finished(bus_name: String, player_index: int) -> void:
	if (bus_name == "Music" && _currently_active == player_index):
		_currently_active = -1
		_release_button(_playing_music_index)
		_playing_music_index = -1




func _on_info_updater_timeout() -> void:
	### Build "debug info" which is set in a label within the second page of the expandable panel
	var dbginfo: Array = _amaster.get_details_info()
	var baseinfo: String = "- %s (%s)\n   Player count: %s/%s\n   Available: %d\n   Playing: %d"
	
	var txt: String = ""
	for ainfo: Dictionary in dbginfo:
		if (!txt.is_empty()):
			txt += "\n"
		
		txt += baseinfo % [ainfo.bus, ainfo.type, ainfo.player_count, ainfo.max_count, ainfo.available, ainfo.playing]
	
	_lbl_info.text = txt
	
	### Update the music progress bar
	if (_currently_active != -1):
		var cpos: float = _amaster.get_playback_position("Music", _currently_active)
		var percent: float = cpos / _active_duration
		_progress.value = percent
	
	else:
		_progress.value = 0.0



func _on_opt_device_item_selected(index: int) -> void:
	var opt: OptionButton = ($ui/expanel/vbsettings/opt_device as OptionButton)
	
	AudioServer.output_device = opt.get_item_text(index)

func _on_volume_changed(value: float, busname: String) -> void:
	_amaster.set_bus_volume_percent(busname, value)





func _on_sfxhelper_source_chosen(global_pos: Vector2, relative: Vector2) -> void:
	var opttype: OptionButton = ($ui/expanel/vbsettings/opt_sfxtype as OptionButton)
	
	var which: int = randi() % _sfx_list.size()
	
	match (opttype.selected):
		0:
			# Normal
			_amaster.play_audio("SFX", _sfx_list[which])
		
		1:
			var extra: Dictionary = {
				"position": global_pos,
				#"area_mask": 1,
				#"attenuation": 1.0,
				"max_distance": 500.0,        # This is roughly the distance from the edge of the "area" to the "listener"
				#"max_polyphony": 1,
				#"panning_strength": 1.0,
				#"pitch_scale": 1.0,
			}
			
			_amaster.play_audio("SFX2D", _sfx_list[which], -1, 0.0, extra)
		
		2:
			# 3D
			var pos3d: Vector3 = Vector3(relative.x, 1.0, relative.y)
			
			# All of the commented options are with their default values
			var extra: Dictionary = {
				"position": pos3d,
				#"area_mask": 1,
				#"attenuation_filter_cutoff_hz": 5000.0,
				#"attenuation_filter_db": -24.0,
				#"attenuation_model": AudioStreamPlayer3D.ATTENUATION_INVERSE_DISTANCE,
				#"doppler_tracking": AudioStreamPlayer3D.DOPPLER_TRACKING_DISABLED,
				#"emission_angle_degrees": 45.0,
				#"emission_angle_enabled": false,
				#"emission_angle_filter_attenuation_db": -12.0,
				#"max_db": 3.0,
				"max_distance": 6.0,
				#"max_polyphony": 1,
				#"panning_strength": 1.0,
				#"pitch_scale": 1.0,
				#"unit_size": 10.0,
			}
			
			_amaster.play_audio("SFX3D", _sfx_list[which], -1, 0.0, extra)



func _on_bt_music_1_toggled(toggled_on: int) -> void:
	if (toggled_on):
		_cross_fade(0)
	else:
		_stop_music()

func _on_bt_music_2_toggled(toggled_on: int) -> void:
	if (toggled_on):
		_cross_fade(1)
	else:
		_stop_music()

func _on_bt_music_3_toggled(toggled_on: int) -> void:
	if (toggled_on):
		_cross_fade(2)
	else:
		_stop_music()


func _on_bt_stop_pressed() -> void:
	if (_currently_active == -1):
		return
	
	($ui/pnl_bottom/hbox/bt_music1 as Button).button_pressed = false
	($ui/pnl_bottom/hbox/bt_music2 as Button).button_pressed = false
	($ui/pnl_bottom/hbox/bt_music3 as Button).button_pressed = false
	
	_on_info_updater_timeout()


func _on_button_mouse_over() -> void:
	# Several buttons have this single function as the event listener for mouse over. To be more specific:
	# - The button to go back into the main menu (Back -> $ui/expanel/vbsettings/bt_back)
	# - The three buttons to toggle music playback that are the bottom of the window
	#   * Action Strike -> $ui/pnl_bottom/hbox/bt_music1
	#   * Dreams of Vain -> $ui/pnl_bottom/hbox/bt_music2
	#   * Epic Boss Battle -> $ui/pnl_bottom/hbox/bt_music3
	# - The button to stop all music playback (Stop -> $ui/pnl_bottom/hbox/bt_stop)
	_amaster.load_and_play("UI", "res://resources/sfx/rollover2.wav")



func _on_bp_palybackpos_gui_input(evt: InputEvent) -> void:
	var seekp: float = -1.0
	
	var mb: InputEventMouseButton = evt as InputEventMouseButton
	if (mb):
		if (mb.button_index == MOUSE_BUTTON_LEFT):
			if (mb.pressed):
				seekp = clampf(mb.position.x / _progress.size.x, 0.0, 1.0)
				_seeking = true
			else:
				_seeking = false
	
	var mm: InputEventMouseMotion = evt as InputEventMouseMotion
	if (mm && _seeking):
		seekp = clampf(mm.position.x / _progress.size.x, 0.0, 1.0)
	
	if (seekp >= 0.0 && _currently_active != -1):
		_amaster.set_playback_position("Music", _currently_active, seekp * _active_duration)


func _on_sfxhelper_resized() -> void:
	var lpos: Vector2 = ($ui/sfxhelper as SoundFXHelper).get_listener_global_position()
	($ui/sfxhelper/listener2d as AudioListener2D).global_position = lpos


func _on_bt_back_pressed() -> void:
	_amaster.stop_all()
	
	@warning_ignore("return_value_discarded")
	get_tree().change_scene_to_file("res://levels/entry.tscn")


#######################################################################################################################
### Overrides
func _enter_tree() -> void:
	_on_sfxhelper_resized()
	
	@warning_ignore("return_value_discarded")
	($ui/sfxhelper as SoundFXHelper).connect("resized", Callable(self, "_on_sfxhelper_resized"))


func _exit_tree() -> void:
	pass


func _ready() -> void:
	_populate_device_list()
	_update_volumes()
	
	# It's possible to configure the AudioMaster both through the Inspector or by code. The commented lines bellow perform the
	# exact same configuration that is done in the Inspector.
	#_amaster.set_player_type("SFX2D", AudioMaster.Player2D)
	#_amaster.set_player_type("SFX3D", AudioMaster.Player3D)
	
	# Set maximum amount of stream player nodes for each bus
	#_amaster.set_maximum_players("Music", 2)
	#_amaster.set_maximum_players("SFX", 16)
	#_amaster.set_maximum_players("SFX2D", 16)
	#_amaster.set_maximum_players("SFX3D", 16)
	#_amaster.set_maximum_players("UI", 6)
	
	# Pre-allocate the two stream players for the Music bus. This is required since cross-fading will be used, which
	# needs explicit node index
	#_amaster.allocate_player("Music", 2)
	
	#_amaster.set_player_process_mode("UI", Node.PROCESS_MODE_ALWAYS)
	
	
	@warning_ignore("return_value_discarded")
	_spin_master.connect("value_changed", Callable(self, "_on_volume_changed").bind("Master"))
	
	@warning_ignore("return_value_discarded")
	_spin_music.connect("value_changed", Callable(self, "_on_volume_changed").bind("Music"))
	
	@warning_ignore("return_value_discarded")
	_spin_sfxnormal.connect("value_changed", Callable(self, "_on_volume_changed").bind("SFX"))
	
	@warning_ignore("return_value_discarded")
	_spin_sfx2d.connect("value_changed", Callable(self, "_on_volume_changed").bind("SFX2D"))
	
	@warning_ignore("return_value_discarded")
	_spin_sfx3d.connect("value_changed", Callable(self, "_on_volume_changed").bind("SFX3D"))
	
	@warning_ignore("return_value_discarded")
	_spin_ui.connect("value_changed", Callable(self, "_on_volume_changed").bind("UI"))
