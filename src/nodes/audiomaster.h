/***
 * Copyright (c) 2024 Yuri Sarudiansky
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _kehgeneral_audiomaster_h_included
#define _kehgeneral_audiomaster_h_included 1

#include <godot_cpp/classes/node.hpp>

#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/templates/hash_set.hpp>
#include <godot_cpp/templates/hash_map.hpp>


namespace godot
{
   class AudioStream;
   class AudioStreamPlayer;
   class AudioStreamPlayer2D;
   class AudioStreamPlayer3D;
}

#ifndef AUDIO_MASTER_DISABLED

/// Provides a "pool" of audio stream players. With this there is no need to worry about node lifetime
/// when playing audio and removing visual representation nodes from the game.
/// Each configured Bus will generate a collection of stream players of the specified type (normal, 2D or 3D).
/// The amount of players can be configured and even allowed to be expanded in case all of them are in
/// use while also requesting to playback a new sound.
/// Another thing that this might make things easier is the fact that interaction is done through Bus name
/// rather than bus index like when directly using AudioServer
/// When requesting playback, 2D and 3D types can receive additional settings through a dictionary. In any case,
/// check the corresponding node documentation to know what each of the available settings do.
/// For the 2D type, the following settings can be added: position (must be global), area_mask, attenuation,
///    max_distance, max_polyphony, panning_strength, pitch_scale
/// For the 3D type, the following settings can be added: position (must be global), area_mask,
///    attenuation_filter_cutoff_hz, attenuation_filter_db, attenuation_model, doppler_tracking, emission_angle_degrees,
///    emission_angle_enabled, emission_angle_filter_attenuation_db, max_db, max_distance, max_polyphony,
///    panning_strength, pitch_scale, unit_size
class AudioMaster : public godot::Node
{
   GDCLASS(AudioMaster, godot::Node);
public:
   enum PlayerType { PlayerNormal, Player2D, Player3D };
private:
   // In order to allow "pre-configuration" to be done through the Inspector, some properties will be exposed through
   // _get_property_list. Since this is indirectly done there is no need to use Dictionary for storage. That said,
   // each audio bus should generate an instance of this struct
   struct BusPreConfig
   {
      // Store which type of stream player should be applied
      PlayerType type;

      // Maximum amount of stream players to be set
      int32_t max_players;

      // How many to pre-allocate
      int32_t pre_allocate;

      // Which audio volume to be applied - this is meant to be the percent value
      float volume;

      // The process mode
      ProcessMode process_mode;

      BusPreConfig() :
         type(PlayerNormal),
         max_players(32),
         pre_allocate(0),
         volume(0.5),
         process_mode(PROCESS_MODE_INHERIT)
      {}
   };


   // When AudioMaster enters the tree, it will query the AudioServer. At that point an instance of this PlayerData
   // will be created for each configured Bus.
   // This class holds information about each "pool" of stream players, effectively managing available nodes for
   // each existing Bus
   class PlayerData
   {
   private:
      // Indicate which type of StreamPlayer node will be generated
      PlayerType m_type;

      // Holds the index of the Bus corresponding to this PlayerData, as found within the AudioServer singleton
      int32_t m_bus_index;

      // Holds instances of the relevant AudioStreamPlayer* nodes. Unfortunately the only common parent is the
      // godot::Node itself, meaning that casts will be needed
      godot::Vector<godot::Node*> m_player;

      // This is the maximum amount of audio stream player nodes that will be created for this specified Bus.
      // If this is set to 0 then nodes will be dynamically created if necessary, without any imposed limit from
      // AudioMaster
      int32_t m_max_players;

      // Values in this set are indices to the m_player array and correspond to stream player nodes that are free
      // to be used.
      godot::HashSet<int32_t> m_available;

      // Stream Players must be added into the tree. This determines the parent.
      godot::Node* m_parent;

   public:
      void set_bus_index(int32_t i) { m_bus_index = i; }
      int32_t get_bus_index() const { return m_bus_index; }
      PlayerType get_player_type() const { return m_type; }
      void set_player_type(PlayerType type) { m_type = type; }

      void rename_parent(const godot::String& new_name);

      void set_process_mode(ProcessMode mode);

      int get_player_count() const { return m_player.size(); }

      int get_currently_playing_count() const { return m_player.size() - m_available.size(); }

      int get_available_player_count() const { return m_available.size(); }

      bool has_player_node_limit() const { return m_max_players > 0; }

      void set_max_players(int val);
      int get_max_players() const { return m_max_players; }

      // Assuming here is a player limit, returns how many new ones can be created
      int get_max_new() const { return m_max_players - m_player.size(); }

      // Returns true if a new stream player can be created
      bool can_create() const { return m_max_players > 0 ? m_player.size() < m_max_players : true; }

      // Create a new audio stream player
      void create(const godot::Callable& on_finished);

      // Remove all players
      void clear_players();

      // Return true if there is a player available to be used
      bool has_available_player() const { return m_available.size() > 0; }

      // Obtain the index of a player that is available to be used. -1 if none
      int get_free_player_index() const;

      template <class T>
      T* get_stream_player(int index, bool mark_used)
      {
         godot::Node* ret = m_player[index];

         if (mark_used)
         {
            m_available.erase(index);
         }

         return godot::Object::cast_to<T>(ret);
      }

      template <class T>
      T* get_stream_player(int index) const
      {
         godot::Node* ret = m_player[index];
         return godot::Object::cast_to<T>(ret);
      }
      
      // Marks a player as available to be played.
      void release_player(int index);

      // Returns true if the player at index is currently marked as "not free".
      bool is_used(int index) const { return !m_available.has(index); }

      // Returns true if the specified stream player is playing something
      bool is_playing(int index) const;

      // Fills the incoming array with the indices of players that are currently playing
      void fill_currently_playing(godot::Vector<int>& out_playing);

      // Return the "position" (in seconds") of the playback in the specified stream player
      double get_playback_position(int index) const;

      // Change the playback "position" (seconds) of the specified stream player
      void set_playback_position(int index, double position);


      PlayerData(godot::Node* parent = nullptr, int32_t index = -1) :
         m_type(PlayerType::PlayerNormal),
         m_parent(parent),
         m_bus_index(index),
         m_max_players(32)
      {}
   };

   // Several functions provide an argument that allows something to occur over the specified amount of seconds
   // Volume change, fade-in, fade-out... this task will be done within the "process" and will be based on reading
   // data from instances of this inner class. This should give all the required data to perform the changes over
   // the time.
   class Fader
   {
   private:
      // Which bus this thing corresponds to. While this could be easily gathered from the AudioServer, caching
      // here just for the simplicity it brings
      godot::String m_bus;

      // Index of the bus. Will be used when no stream player is provided
      int m_bus_index;

      // The index of the player node. Might be -1 if dealing with a bus effect rather than stream player
      int m_player_index;

      // How long this effect should last (in seconds)
      double m_duration;

      // How much has elapsed
      double m_elapsed;

      // Which type of stream player is used
      PlayerType m_type;

      // The stream player node. If this is nullptr then the effect is applied to the Bus rather than a stream player
      godot::Node* m_player;

      // If this is true then volume will go from low to high. Otherwise from high to low. Also, if this is false
      // (fade-out), then audio stream will be stopped when the effect ends
      bool m_fade_in;

      // Hold initial and end volume values so there wont be any need to use "if" at every single loop iteration
      double m_from_vol;
      double m_to_vol;

   public:
      const godot::String& get_bus_name() const { return m_bus; }
      int get_bus_index() const { return m_bus_index; }
      bool should_stop_on_end() const { return !m_fade_in; }
      int get_player_index() const { return m_player_index; }

      void start(double time, double from, double to);
      

      // Returns true if the fader has finished
      bool tick(double dt);

      void setup_player_data(int bus_index, int player_index, godot::Node* player, PlayerType type);

      Fader() :
         m_bus_index(-1),
         m_player_index(-1),
         m_duration(0.0),
         m_elapsed(0.0),
         m_type(PlayerType::PlayerNormal),
         m_player(nullptr),
         m_fade_in(false),
         m_from_vol(0.0),
         m_to_vol(0.0)
      {}
   };

   typedef godot::HashMap<godot::String, PlayerData> pdata_map_t;

   /// Internal variables
   // Maps bus name into its PlayerData instance
   godot::HashMap<godot::String, PlayerData> m_player_map;

   // Hold "over-time" operations data
   godot::Vector<Fader> m_to_process;


   // This will be indirectly exposed by using _get_property_list(). The intention here is to store "pre-configuration" of the stream
   // player pools, so it can be done directly through the Inspector.
   godot::HashMap<godot::String, BusPreConfig> m_preconfig;

   /// Exposed variables

   /// Internal functions
   void cleanup();

   // This can be seen as "enum to String"
   godot::String type_to_name(PlayerType type) const;

   void create_fader(int bus_index, int player_index, godot::Node* player, PlayerType type, double time, double from, double to);

   void handle_fader_list();

   // Performs extra setup of the AudioStreamPlayer, AudioStreamPlayer2D and AudioStreamPlayer3D
   void setup_player(godot::AudioStreamPlayer* player, const godot::Dictionary& extra);
   void setup_player(godot::AudioStreamPlayer2D* player, const godot::Dictionary& extra);
   void setup_player(godot::AudioStreamPlayer3D* player, const godot::Dictionary& extra);

   // Take data from m_preconfig and apply into PlayerData
   void apply_preconfig();

   /// Event handlers
   void on_playback_finished(const godot::String& bus, int index);

   void on_bus_layout_changed();
   void on_bus_renamed(int32_t index, const godot::StringName& from, const godot::StringName& to);

protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* list) const;
   bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides

   /// Exposed functions
   // Check within the AudioServer which audio buses are configured. Based on that properly builds internal data
   // to help deal with the audio players
   //void scan_audio_bus();

   // Obtain the volume of the given audio bus, in percent (range [0..1]) rather than DB
   // If a negative value is returned then something went wrong, most likely the specified bus was not found
   double get_bus_volume_percent(const godot::String& bus) const;

   // Set the volume of the given audio bus by specifying a percent value (range [0..1]). It will be automatically
   // converted into DB
   // The "during" parameter tells for how long (in seconds) the volume change should occur.
   void set_bus_volume_percent(const godot::String& bus, double new_vol, double during = 0.0);

   // Set the audio stream player type (normal, 2D or 3D) of node that will be created for the playback of audio
   // associated with given audio bus
   void set_player_type(const godot::String& bus, PlayerType type);

   // Sets the maximum amount of stream players for the specified bus name. If zero (or negative) is provided then
   // this system will not impose any limit into the maximum amount of audio players. That is, if attempting to play
   // an audio and there isn't any player available then a new one will be created.
   void set_maximum_players(const godot::String& bus, int val);
   
   // Pre generates num stream players for the specified bus. Note that the total amount of players will not exceed
   // the maximum amount that might have been previously configured
   void allocate_players(const godot::String& bus, int num);

   // Sets the "process_mode" of all stream players associated with the given bus name. This might be useful for a bus
   // dedicated to player UI sounds, for example. In this case it would be interesting to still allow those nodes to
   // play sounds even when the game is paused.
   void set_player_process_mode(const godot::String& bus, ProcessMode mode);

   // Plays and audio stream. Or at least attempt to. A bus name must be specified in order to find an appropriate player.
   // If index is negative then the first (in queue) free audio stream player will be used. Explicitly picking a player
   // (by specifying a positive index) will basically interrupt its playback if it's already playing something.
   // The extra settings Dictionary can be used to change several properties that are available within the various
   // AudioStreamPlayer* nodes.
   // When the playback naturally finishes the signal 'playback_finished' will be emitted
   void play_audio(const godot::String& bus, const godot::Ref<godot::AudioStream>& stream, int index = -1, double fade_time = 0.0, const godot::Dictionary& extra = godot::Dictionary());

   // Load an audio stream from a given resource path then request playback
   void load_and_play(const godot::String& bus, const godot::String& path, int index = -1, double fade_time = 0.0, const godot::Dictionary& extra = godot::Dictionary());


   // Returns true if the audio stream player at index within the specified bus name is currently "marked" as in use
   bool is_used(const godot::String& bus, int index) const;

   // Returns true if the audio stream player at index within the specified bus name is currently playing something
   bool is_playing(const godot::String& bus, int index) const;


   // Stops playback of the stream player at the specified index in the corresponding bus
   void stop(const godot::String& bus, int index, double fade_time = 0.0);

   // Stops playback of all audio within the specified bus.
   void stop_all_in_bus(const godot::String& bus, double fade_time = 0.0);

   // Stop playback of all audio stream within all audio buses
   void stop_all(double fade_time = 0.0);

   // Request the playback position (in seconds) of the specified stream player within the given bus
   double get_playback_position(const godot::String& bus, int player_index) const;

   // Change the playback position (in seconds) of the specified stream player and bus
   void set_playback_position(const godot::String& bus, int index, double position);

   // Retrieve the index of an available audio stream player node from the specified bus. Returns -1 if there isn't any
   int get_available_player_index(const godot::String& bus) const;


   // This is mostly to help debug things. Each entry in the returned Array is a Dictionary that corresponds to an existing audio bus
   // The entries in the dictionary are:
   // - "bus": bus name
   // - "player_count": number of audio stream player nodes for this bus
   // - "max_count": maximum amount of stream player nodes for this bus (0 if no restriction)
   // - "playing": how many of those nodes are currently playing
   // - "available": how many of those nodes are currently free
   // - "type": Which stream player type the nodes are
   godot::Array get_details_info() const;

   /// Setters/Getters

   /// Public non exposed functions
   AudioMaster();
};

VARIANT_ENUM_CAST(AudioMaster::PlayerType);



#endif  //AUDIO_MASTER_DISABLED

#endif  //_kehgeneral_audiomaster_h_included
