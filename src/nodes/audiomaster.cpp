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

#include "audiomaster.h"

#ifndef AUDIO_MASTER_DISABLED

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_player2d.hpp>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/resource_loader.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// AudioMaster::PlayerData
void AudioMaster::PlayerData::rename_parent(const godot::String& new_name)
{
   m_parent->set_name(new_name);
}


void AudioMaster::PlayerData::set_process_mode(ProcessMode mode)
{
   // Instead of changing the mode of each stream player node (which should already be set to Inherit), change
   // the parent node. Leaving commented code that changes all nodes just as reference
   //
   //const int pcount = m_player.size();
   //for (int i = 0; i < pcount; i++)
   //{
   //   m_player[i]->set_process_mode(mode);
   //}

   m_parent->set_process_mode(mode);

}

void AudioMaster::PlayerData::set_max_players(int val)
{
   if (val < 0)
   {
      val = 0;
   }

   m_max_players = val;

   // 0 means no restrictions in the amount of players. So only attempt to remove exceeding numbers if
   // maximum amount is above 0
   if (m_max_players > 0)
   {
      int rem_index = m_player.size() - 1;
      while (m_player.size() > m_max_players)
      {
         m_player[rem_index]->queue_free();
         m_player.remove_at(rem_index--);
      }
   }
}

void AudioMaster::PlayerData::create(const godot::Callable& on_finished)
{
   ERR_FAIL_COND_MSG(m_parent == nullptr, "Can't create new audio stream player without a valid parent");
   ERR_FAIL_COND_MSG(!can_create(), "Imposed player limit has been reached, can't create a new one");

   const int index = m_player.size();
   const godot::String bus_name = m_parent->get_name();

   godot::Node* player = nullptr;
   switch (m_type)
   {
      case PlayerType::PlayerNormal:
      {
         godot::AudioStreamPlayer* p = memnew(godot::AudioStreamPlayer);
         p->set_bus(bus_name);
         p->connect("finished", on_finished.bind(bus_name, index));

         player = p;
      } break;

      case PlayerType::Player2D:
      {
         godot::AudioStreamPlayer2D* p = memnew(godot::AudioStreamPlayer2D);
         p->set_bus(bus_name);
         p->connect("finished", on_finished.bind(bus_name, index));

         player = p;
      } break;

      case PlayerType::Player3D:
      {
         godot::AudioStreamPlayer3D* p = memnew(godot::AudioStreamPlayer3D);
         p->set_bus(bus_name);
         p->connect("finished", on_finished.bind(bus_name, index));

         player = p;
      } break;
   }

   player->set_name(godot::vformat("Player_%d", index));
   // While Inherit should be the default one, ensure this is the one set as requesting to change to a specific
   // mode depends on the stream players inheriting from their parent node.
   player->set_process_mode(ProcessMode::PROCESS_MODE_INHERIT);
   m_player.append(player);

   m_available.insert(index);

   m_parent->add_child(player);
}

void AudioMaster::PlayerData::clear_players()
{
   const int pcount = m_player.size();
   for (int i = 0; i < pcount; i++)
   {
      m_player[i]->queue_free();
   }

   m_player.clear();
   m_available.clear();
}

int AudioMaster::PlayerData::get_free_player_index() const
{
   if (m_available.is_empty()) { return -1; }

   return *m_available.begin();
}

void AudioMaster::PlayerData::release_player(int index)
{
   if (!m_available.has(index))
   {
      m_available.insert(index);
   }

   switch (m_type)
   {
      case PlayerType::PlayerNormal:
      {
         get_stream_player<godot::AudioStreamPlayer>(index, false)->set_stream(nullptr);
      } break;

      case PlayerType::Player2D:
      {
         get_stream_player<godot::AudioStreamPlayer2D>(index, false)->set_stream(nullptr);
      } break;

      case PlayerType::Player3D:
      {
         get_stream_player<godot::AudioStreamPlayer3D>(index, false)->set_stream(nullptr);
      } break;
   }
}

bool AudioMaster::PlayerData::is_playing(int index) const
{
   ERR_FAIL_INDEX_V_MSG(index, m_player.size(), false, "Querying if player is playing, but requested index is out of bounds");
   switch (m_type)
   {
      case PlayerType::PlayerNormal:
      {
         return get_stream_player<godot::AudioStreamPlayer>(index)->is_playing();
      } break;

      case PlayerType::Player2D:
      {
         return get_stream_player<godot::AudioStreamPlayer2D>(index)->is_playing();
      } break;

      case PlayerType::Player3D:
      {
         return get_stream_player<godot::AudioStreamPlayer3D>(index)->is_playing();
      }
   }

   // Should never reach this, but returning something to shut up the compiler
   return false;
}

void AudioMaster::PlayerData::fill_currently_playing(godot::Vector<int>& out_playing)
{
   const int pcount = m_player.size();
   for (int i = 0; i < pcount; i++)
   {
      if (is_playing(i))
      {
         out_playing.append(i);
      }
   }
}

double AudioMaster::PlayerData::get_playback_position(int index) const
{
   ERR_FAIL_INDEX_V_MSG(index, m_player.size(), 0.0, "Attempting to retrieve playback position but specified player index is out of bounds");

   switch (m_type)
   {
      case PlayerType::PlayerNormal:
      {
         return get_stream_player<godot::AudioStreamPlayer>(index)->get_playback_position();
      } break;

      case PlayerType::Player2D:
      {
         return get_stream_player<godot::AudioStreamPlayer2D>(index)->get_playback_position();
      } break;

      case PlayerType::Player3D:
      {
         return get_stream_player<godot::AudioStreamPlayer3D>(index)->get_playback_position();
      } break;
   }

   // It should never get here, but returning something to shut up the compiler
   return 0.0;
}


void AudioMaster::PlayerData::set_playback_position(int index, double position)
{
   ERR_FAIL_INDEX_MSG(index, m_player.size(), "Attempting to set playback position but specified player index is out of bounds");

   switch (m_type)
   {
      case PlayerType::PlayerNormal:
      {
         get_stream_player<godot::AudioStreamPlayer>(index, false)->seek(position);
      } break;

      case PlayerType::Player2D:
      {
         get_stream_player<godot::AudioStreamPlayer2D>(index, false)->seek(position);
      } break;

      case PlayerType::Player3D:
      {
         get_stream_player<godot::AudioStreamPlayer3D>(index, false)->seek(position);
      } break;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// AudioMaster::Fader
void AudioMaster::Fader::start(double time, double from, double to)
{
   m_duration = time;
   m_elapsed = 0.0;
   m_from_vol = from;
   m_to_vol = to;

   m_fade_in = m_player != nullptr && m_from_vol < m_to_vol;
}


bool AudioMaster::Fader::tick(double dt)
{
   m_elapsed += dt;
   double alpha = godot::Math::clamp<double>(m_elapsed / m_duration, 0.0, 1.0);

   const double nvol = godot::Math::lerp(m_from_vol, m_to_vol, alpha);
   const double db = godot::UtilityFunctions::linear_to_db(nvol);

   if (m_player)
   {
      switch (m_type)
      {
         case PlayerType::PlayerNormal:
         {
            godot::AudioStreamPlayer* player = godot::Object::cast_to<godot::AudioStreamPlayer>(m_player);
            player->set_volume_db(db);
         } break;

         case PlayerType::Player2D:
         {
            godot::AudioStreamPlayer2D* player = godot::Object::cast_to<godot::AudioStreamPlayer2D>(m_player);
            player->set_volume_db(db);
         } break;

         case PlayerType::Player3D:
         {
            godot::AudioStreamPlayer3D* player = godot::Object::cast_to<godot::AudioStreamPlayer3D>(m_player);
            player->set_volume_db(db);
         } break;
      }
   }
   else
   {
      // Dealing with the Bus.
      godot::AudioServer* aserver = godot::AudioServer::get_singleton();
      aserver->set_bus_volume_db(m_bus_index, db);
   }


   return alpha >= 1.0;
}

void AudioMaster::Fader::setup_player_data(int bus_index, int player_index, godot::Node* player, PlayerType type)
{
   godot::AudioServer* aserver = godot::AudioServer::get_singleton();
   m_bus = aserver->get_bus_name(bus_index);
   m_bus_index = bus_index;
   m_player_index = player_index;
   m_player = player;
   m_type = type;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// AudioMaster
void AudioMaster::cleanup()
{
   while (get_child_count() > 0)
   {
      godot::Node* n = get_child(0);
      remove_child(n);
      memdelete(n);
   }

   m_player_map.clear();
}


godot::String AudioMaster::type_to_name(PlayerType type) const
{
   switch (type)
   {
      case PlayerType::PlayerNormal: return "Normal";
      case PlayerType::Player2D: return "2D";
      case PlayerType::Player3D: return "3D";
   }

   // Should never get here, but this shuts up the compiler
   return "Unknown Type";
}


void AudioMaster::create_fader(int bus_index, int player_index, godot::Node* player, PlayerType type, double time, double from, double to)
{
   const double db = godot::UtilityFunctions::linear_to_db(from);

   // Ensure the stream player begins with the correct "from"
   switch (type)
   {
      case PlayerType::PlayerNormal:
      {
         godot::Object::cast_to<godot::AudioStreamPlayer>(player)->set_volume_db(from);
      } break;

      case PlayerType::Player2D:
      {
         godot::Object::cast_to<godot::AudioStreamPlayer2D>(player)->set_volume_db(from);
      } break;

      case PlayerType::Player3D:
      {
         godot::Object::cast_to<godot::AudioStreamPlayer3D>(player)->set_volume_db(from);
      } break;
   }

   Fader fader;
   fader.setup_player_data(bus_index, player_index, player, type);
   fader.start(time, from, to);

   m_to_process.append(fader);
   set_process_internal(true);
}


void AudioMaster::handle_fader_list()
{
   int index = 0;
   while (index < m_to_process.size())
   {
      Fader& fader = (Fader&)m_to_process[index];

      if (fader.tick(get_process_delta_time()))
      {
         PlayerData& pd = m_player_map[fader.get_bus_name()];

         if (fader.should_stop_on_end())
         {
            pd.release_player(fader.get_player_index());
            emit_signal("playback_finished", fader.get_bus_name(), fader.get_player_index());
         }

         m_to_process.remove_at(index);
      }
      else
      {
         index++;
      }
   }

   if (m_to_process.size() == 0)
   {
      set_process_internal(false);
   }
}


void AudioMaster::setup_player(godot::AudioStreamPlayer* player, const godot::Dictionary& extra)
{
   const int32_t mix_target = extra.get("mix_target", 0);

   //player->set_max_polyphony((int)extra.get("max_polyphony", 1));       // this is irrelevant here
   player->set_mix_target((godot::AudioStreamPlayer::MixTarget)mix_target);
   player->set_pitch_scale((double)extra.get("pitch_scale", 1.0));
}


void AudioMaster::setup_player(godot::AudioStreamPlayer2D* player, const godot::Dictionary& extra)
{
   player->set_global_position((godot::Vector2)extra.get("position", godot::Vector2()));

   player->set_area_mask((int)extra.get("area_mask", 1));
   player->set_attenuation((double)extra.get("attenuation", 1.0));
   player->set_max_distance((double)extra.get("max_distance", 2000.0));
   //player->set_max_polyphony((int)extra.get("max_polyphony", 1));
   player->set_panning_strength((double)extra.get("panning_strength", 1.0));
   player->set_pitch_scale((double)extra.get("pitch_scale", 1.0));
}


void AudioMaster::setup_player(godot::AudioStreamPlayer3D* player, const godot::Dictionary& extra)
{
   player->set_global_position((godot::Vector3)extra.get("position", godot::Vector3()));
   
   const int attenuation_model = extra.get("attenuation_model", 0);
   const int doppler_tracking = extra.get("doppler_tracking", 0);

   player->set_area_mask((int)extra.get("area_mask", 1));
   player->set_attenuation_filter_cutoff_hz((double)extra.get("attenuation_filter_cutoff_hz", 5000.0));
   player->set_attenuation_filter_db((double)extra.get("attenuation_filter_db", -24.0));
   player->set_attenuation_model((godot::AudioStreamPlayer3D::AttenuationModel)attenuation_model);
   player->set_doppler_tracking((godot::AudioStreamPlayer3D::DopplerTracking)doppler_tracking);
   player->set_emission_angle((double)extra.get("emission_angle_degrees", 45.0));
   player->set_emission_angle_enabled((bool)extra.get("emission_angle_enabled", false));
   player->set_emission_angle_filter_attenuation_db((double)extra.get("emission_angle_filter_attenuation_db", -12.0));
   player->set_max_db((double)extra.get("max_db", 3.0));
   player->set_max_distance((double)extra.get("max_distance", 0.0));
   //player->set_max_polyphony((int)extra.get("max_polyphony", 1));
   player->set_panning_strength((double)extra.get("panning_strength", 1.0));
   player->set_pitch_scale((double)extra.get("pitch_scale", 1.0));
   player->set_unit_size((double)extra.get("unit_size", 10.0));
}


void AudioMaster::apply_preconfig()
{
   for (godot::KeyValue<godot::String, PlayerData>& pdata : m_player_map)
   {
      godot::HashMap<godot::String, BusPreConfig>::ConstIterator conf_iter = m_preconfig.find(pdata.key);
      if (conf_iter == m_preconfig.end())
      {
         // Should not be here but... perhaps print a warning message about this 'bug'?
         continue;
      }

      set_player_type(pdata.key, conf_iter->value.type);
      set_maximum_players(pdata.key, conf_iter->value.max_players);
      if (conf_iter->value.pre_allocate > 0)
      {
         allocate_players(pdata.key, conf_iter->value.pre_allocate);
      }
      set_bus_volume_percent(pdata.key, conf_iter->value.volume);

      set_player_process_mode(pdata.key, conf_iter->value.process_mode);
   }
}


void AudioMaster::on_playback_finished(const godot::String& bus, int index)
{
   m_player_map[bus].release_player(index);

   emit_signal("playback_finished", bus, index);
}


void AudioMaster::on_bus_layout_changed()
{
   // If this becomes true then property list must be updated
   bool changed = false;

   // Use this set to help remove internal data from any Bus that might have been removed
   godot::HashSet<godot::String> bus_list;

   // Iterate through existing internal data to fill bus_name
   for(godot::KeyValue<godot::String, PlayerData> pdata : m_player_map)
   {
      bus_list.insert(pdata.key);
   }
   

   godot::AudioServer* aserver = godot::AudioServer::get_singleton();
   const int32_t bcount = aserver->get_bus_count();
   for (int32_t i = 0; i < bcount; i++)
   {
      const godot::String bus_name = aserver->get_bus_name(i);

      // Remove this bus name from bus_list. After this list any remaining entry in bus_list means bus names that doesn't exist
      bus_list.erase(bus_name);

      godot::HashMap<godot::String, PlayerData>::Iterator pdata_iter = m_player_map.find(bus_name);
      if (pdata_iter == m_player_map.end())
      {
         // PlayerData for this Bus does not exist. Create it
         // The Node is not entirely necessary, but this helps organize the tree hierarchy. All players for this Bus will be
         // attached to this node
         godot::Node* pnode = memnew(godot::Node);
         pnode->set_name(bus_name);

         PlayerData pd = PlayerData(pnode, i);
         add_child(pnode);

         m_player_map[bus_name] = pd;

         // If here almost sure the preconfig does not exist. Create it
         m_preconfig[bus_name] = BusPreConfig();

         changed = true;
      }
      else
      {
         // Player data for this Bus already exists. However make sure the internal Index matches that found in the AudioServer
         pdata_iter->value.set_bus_index(i);
      }
   }

   // Now remove any internal data that does not exist in the AudioServer
   for (const godot::String& bname : bus_list)
   {
      godot::Node* parent = get_node<godot::Node>(bname);
      ERR_FAIL_COND_MSG(!parent, godot::vformat("There is PlayerData for non existing audio bus ", bname));
      parent->queue_free();

      m_player_map.erase(bname);
      m_preconfig.erase(bname);
   }

   if (changed)
   {
      notify_property_list_changed();
   }
}


void AudioMaster::on_bus_renamed(int32_t index, const godot::StringName& from, const godot::StringName& to)
{
   m_player_map[to] = m_player_map[from];
   m_player_map.erase(from);
   m_player_map[to].rename_parent(to);

   m_preconfig[to] = m_preconfig[from];
   m_preconfig.erase(from);

   notify_property_list_changed();
}


void AudioMaster::_get_property_list(godot::List<godot::PropertyInfo>* list) const
{
   using namespace godot;

   // Push an empty group name so property grouping is reset.
   list->push_back(PropertyInfo(Variant::NIL, "", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_GROUP));

   // The format here creates a "main group" named pre_config
   // Inside of it there will be a subgroup for each audio bus (the first '%s')
   // Then for each audio bus subgroup there will be the fields of the BusPreConfig struct
   const godot::String formatter("pre_config/%s/%s");

   for (const godot::KeyValue<godot::String, BusPreConfig>& config : m_preconfig)
   {
      list->push_back(PropertyInfo(Variant::INT, vformat(formatter, config.key, "type"), PROPERTY_HINT_ENUM, "Normal,2D,3D"));
      list->push_back(PropertyInfo(Variant::INT, vformat(formatter, config.key, "max_players")));
      list->push_back(PropertyInfo(Variant::INT, vformat(formatter, config.key, "pre_allocate")));
      list->push_back(PropertyInfo(Variant::FLOAT, vformat(formatter, config.key, "volume")));
      list->push_back(PropertyInfo(Variant::INT, vformat(formatter, config.key, "process_mode"), PROPERTY_HINT_ENUM, "Inherit,Pausable,WhenPaused,Always,Disabled"));
   }
}


bool AudioMaster::_set(const godot::StringName& prop_name, const godot::Variant& value)
{
   const godot::PackedStringArray prop_section = prop_name.split("/", true, 2);
   if (prop_section[0] == "pre_config")
   {
      // prop_section[1] holds bus name
      godot::HashMap<godot::String, BusPreConfig>::Iterator iter = m_preconfig.find(prop_section[1]);
      if (iter == m_preconfig.end())
      {
         m_preconfig[prop_section[1]] = BusPreConfig();
         iter = m_preconfig.find(prop_section[1]);
      }

      if (prop_section[2] == "type")
      {
         const int32_t itype = value;
         iter->value.type = (PlayerType)itype;
      }
      else if (prop_section[2] == "max_players")
      {
         iter->value.max_players = godot::Math::max<int32_t>(0, value);
      }
      else if (prop_section[2] == "pre_allocate")
      {
         iter->value.pre_allocate = godot::Math::max<int32_t>(0, value);
      }
      else if (prop_section[2] == "volume")
      {
         iter->value.volume = godot::Math::clamp<double>(value, 0.0, 1.0);
      }
      else if (prop_section[2] == "process_mode")
      {
         const int32_t imode = value;
         iter->value.process_mode = (ProcessMode)imode;
      }

      return true;
   }

   return false;
}


bool AudioMaster::_get(const godot::StringName& prop_name, godot::Variant& out_value) const
{
   const godot::PackedStringArray prop_section = prop_name.split("/", true, 2);
   if (prop_section[0] == "pre_config")
   {
      godot::HashMap<godot::String, BusPreConfig>::ConstIterator iter = m_preconfig.find(prop_section[1]);
      if (iter == m_preconfig.end())
      {
         return false;
      }

      if (prop_section[2] == "type")
      {
         const int32_t ival = iter->value.type;
         out_value = ival;
      }
      else if (prop_section[2] == "max_players")
      {
         out_value = iter->value.max_players;
      }
      else if (prop_section[2] == "pre_allocate")
      {
         out_value = iter->value.pre_allocate;
      }
      else if (prop_section[2] == "volume")
      {
         out_value = iter->value.volume;
      }
      else if (prop_section[2] == "process_mode")
      {
         const int32_t imode = iter->value.process_mode;
         out_value = imode;
      }

      return true;
   }

   return false;
}


void AudioMaster::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         //scan_audio_bus();
         godot::AudioServer::get_singleton()->connect("bus_layout_changed", callable_mp(this, &AudioMaster::on_bus_layout_changed));
         godot::AudioServer::get_singleton()->connect("bus_renamed", callable_mp(this, &AudioMaster::on_bus_renamed));

         on_bus_layout_changed();
      } break;

      case NOTIFICATION_READY:
      {
         set_process_internal(m_to_process.size() > 0);
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         handle_fader_list();
      } break;

      case NOTIFICATION_ENTER_TREE:
      {
         apply_preconfig();
      } break;

      case NOTIFICATION_EXIT_TREE:
      {
         cleanup();
      } break;
   }
}


void AudioMaster::_bind_methods()
{
   using namespace godot;

   /// Functions
   //ClassDB::bind_method(D_METHOD("scan_audio_bus"), &AudioMaster::scan_audio_bus);
   ClassDB::bind_method(D_METHOD("get_bus_volume_percent", "bus_name"), &AudioMaster::get_bus_volume_percent);
   ClassDB::bind_method(D_METHOD("set_bus_volume_percent", "bus_name", "new_volume", "during"), &AudioMaster::set_bus_volume_percent, DEFVAL(0.0));
   ClassDB::bind_method(D_METHOD("set_player_type", "bus_name", "type"), &AudioMaster::set_player_type);
   ClassDB::bind_method(D_METHOD("set_maximum_players", "bus_name", "amount"), &AudioMaster::set_maximum_players);
   ClassDB::bind_method(D_METHOD("allocate_player", "bus_name", "num"), &AudioMaster::allocate_players);
   ClassDB::bind_method(D_METHOD("set_player_process_mode", "bus_name", "mode"), &AudioMaster::set_player_process_mode);
   ClassDB::bind_method(D_METHOD("play_audio", "bus_name", "stream", "index", "fade_time", "extra"), &AudioMaster::play_audio, DEFVAL(-1), DEFVAL(0.0), DEFVAL(Dictionary()));
   ClassDB::bind_method(D_METHOD("load_and_play", "bus_name", "path", "index", "fade_time", "extra"), &AudioMaster::load_and_play, DEFVAL(-1), DEFVAL(0.0), DEFVAL(Dictionary()));
   ClassDB::bind_method(D_METHOD("is_used", "bus_name", "index"), &AudioMaster::is_used);
   ClassDB::bind_method(D_METHOD("is_playing", "bus_name", "index"), &AudioMaster::is_playing);
   ClassDB::bind_method(D_METHOD("stop", "bus_name", "index", "fade_time"), &AudioMaster::stop, DEFVAL(0.0));
   ClassDB::bind_method(D_METHOD("stop_all_in_bus", "bus_name", "fade_time"), &AudioMaster::stop_all_in_bus, DEFVAL(0.0));
   ClassDB::bind_method(D_METHOD("stop_all", "fade_time"), &AudioMaster::stop_all, DEFVAL(0.0));
   ClassDB::bind_method(D_METHOD("get_playback_position", "bus_name", "player_index"), &AudioMaster::get_playback_position);
   ClassDB::bind_method(D_METHOD("set_playback_position", "bus_name", "player_index", "position"), &AudioMaster::set_playback_position);
   ClassDB::bind_method(D_METHOD("get_available_player_index", "bus_name"), &AudioMaster::get_available_player_index);
   ClassDB::bind_method(D_METHOD("get_details_info"), &AudioMaster::get_details_info);

   /// Enum Constants
   BIND_ENUM_CONSTANT(PlayerNormal);
   BIND_ENUM_CONSTANT(Player2D);
   BIND_ENUM_CONSTANT(Player3D);

   /// Signals
   ADD_SIGNAL(MethodInfo("playback_finished", PropertyInfo(Variant::STRING, "bus_name"), PropertyInfo(Variant::INT, "player_index")));
}


/*void AudioMaster::scan_audio_bus()
{
   godot::AudioServer* aserver = godot::AudioServer::get_singleton();
   if (!aserver)
   {
      return;
   }

   // Maybe not the best performance-wise, but definitely the easiest. That is, first remove everything that
   // has already been built (internally) and rebuild.
   cleanup();
   
   const int32_t bcount = aserver->get_bus_count();
   for (int32_t i = 0; i < bcount; i++)
   {
      const godot::String bus_name = aserver->get_bus_name(i);

      // While not entirely necessary, create this node to hold all stream players associated with this Bus
      godot::Node* pnode = memnew(godot::Node);
      pnode->set_name(bus_name);
      
      PlayerData pd = PlayerData(pnode, i);
      add_child(pnode);

      m_player_map[bus_name] = pd;
   }
}*/


double AudioMaster::get_bus_volume_percent(const godot::String& bus) const
{
   pdata_map_t::ConstIterator iter = m_player_map.find(bus);
   
   ERR_FAIL_COND_V_MSG(iter == m_player_map.end(), 0.0, "Attempting to get bus volume, but specified name wasn't found. If you changed bus configuration during runtime call scan_audio_bus().");

   godot::AudioServer* aserver = godot::AudioServer::get_singleton();

   const int index = iter->value.get_bus_index();
   const double db = aserver->get_bus_volume_db(index);
   return godot::UtilityFunctions::db_to_linear(db);
}


void AudioMaster::set_bus_volume_percent(const godot::String& bus, double new_vol, double during)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to set bus volume, but specified name wasn't found. If you changed bus configuration during runtime call scan_audio_bus().");

   godot::AudioServer* aserver = godot::AudioServer::get_singleton();
   const int32_t index = iter->value.get_bus_index();

   if (during > 0.0)
   {
      const double cdb = aserver->get_bus_volume_db(index);
      const double clin = godot::UtilityFunctions::db_to_linear(cdb);
      create_fader(index, -1, nullptr, iter->value.get_player_type(), during, clin, new_vol);
   }
   else
   {
      const double db = godot::UtilityFunctions::linear_to_db(new_vol);
      aserver->set_bus_volume_db(index, db);
   }
}


void AudioMaster::set_player_type(const godot::String& bus, PlayerType type)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to set player type, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   if (iter->value.get_player_type() != type)
   {
      // Store current amount of existing stream player nodes
      const int32_t pcount = iter->value.get_player_count();

      // Remove all the existing players
      iter->value.clear_players();

      // Assign the new type
      iter->value.set_player_type(type);

      // Restore previous amount of stream players, but now of the new type
      for (int i = 0; i < pcount; i++)
      {
         iter->value.create(callable_mp(this, &AudioMaster::on_playback_finished));
      }
   }
}


void AudioMaster::set_maximum_players(const godot::String& bus, int val)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to set maximum players, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   iter->value.set_max_players(val);
}


void AudioMaster::allocate_players(const godot::String& bus, int num)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to allocate players, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   const int32_t count = iter->value.has_player_node_limit() ? godot::Math::min<int32_t>(num, iter->value.get_max_new()) : num;

   const godot::Callable on_finished = callable_mp(this, &AudioMaster::on_playback_finished);

   for (int i = 0; i < count; i++)
   {
      iter->value.create(on_finished);
   }
}


void AudioMaster::set_player_process_mode(const godot::String& bus, ProcessMode mode)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to change player process mode, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   iter->value.set_process_mode(mode);
}


void AudioMaster::play_audio(const godot::String& bus, const godot::Ref<godot::AudioStream>& stream, int index, double fade_time, const godot::Dictionary& extra)
{
   if (!stream.is_valid()) { return; }

   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to play audio, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   if (!iter->value.has_available_player() && index < 0)
   {
      // All created stream player nodes are in use and it was requested to use any free one. Check if it's possible
      // to create a new one
      if (!iter->value.can_create())
      {
         /// TODO: Use a project setting that if enable output an error message from here - this setting should be off by default
         return;
      }

      // If here then a new stream player can be created. Do it!
      iter->value.create(callable_mp(this, &AudioMaster::on_playback_finished));
   }

   const int32_t pindex = index < 0 ? iter->value.get_free_player_index() : index;
   const int32_t bus_index = iter->value.get_bus_index();

   switch (iter->value.get_player_type())
   {
      case PlayerType::PlayerNormal:
      {
         godot::AudioStreamPlayer* player = iter->value.get_stream_player<godot::AudioStreamPlayer>(pindex, true);
         ERR_FAIL_COND_MSG(player == nullptr, "While there is a normal stream player available to be used, failed to retrieve it.");

         player->set_stream(stream);

         //const int mti = (int)extra.get("mix_target", 0);        // 0 is equal to Stereo
         //const godot::AudioStreamPlayer::MixTarget mt = (godot::AudioStreamPlayer::MixTarget)mti;
         //player->set_mix_target(mt);
         setup_player(player, extra);

         if (fade_time > 0.0)
         {
            create_fader(bus_index, pindex, player, PlayerType::PlayerNormal, fade_time, 0.0, 1.0);
         }
         else
         {
            // Ensure the correct volume is set
            player->set_volume_db(0.0);
         }

         const double start_from = (double)extra.get("start_from", 0.0);
         player->play(start_from);
      } break;

      case PlayerType::Player2D:
      {
         godot::AudioStreamPlayer2D* player = iter->value.get_stream_player<godot::AudioStreamPlayer2D>(pindex, true);
         ERR_FAIL_COND_MSG(player == nullptr, "While there is a 2D stream player available to be used, failed to retrieve it.");

         player->set_stream(stream);
         setup_player(player, extra);

         if (fade_time > 0.0)
         {
            create_fader(bus_index, pindex, player, PlayerType::Player2D, fade_time, 0.0, 1.0);
         }
         else
         {
            player->set_volume_db(0.0);
         }

         const double start_from = (double)extra.get("start_from", 0.0);
         player->play(start_from);
      } break;

      case PlayerType::Player3D:
      {
         godot::AudioStreamPlayer3D* player = iter->value.get_stream_player<godot::AudioStreamPlayer3D>(pindex, true);
         ERR_FAIL_COND_MSG(player == nullptr, "While there is a 3D stream player available to be used, failed to retrieve it.");

         player->set_stream(stream);
         setup_player(player, extra);

         if (fade_time > 0.0)
         {
            create_fader(bus_index, pindex, player, PlayerType::Player3D, fade_time, 0.0, 1.0);
         }
         else
         {
            player->set_volume_db(0.0);
         }

         const double start_from = (double)extra.get("start_from", 0.0);
         player->play(start_from);
      } break;
   }
}

void AudioMaster::load_and_play(const godot::String& bus, const godot::String& path, int index, double fade_time, const godot::Dictionary& extra)
{
   if (!godot::ResourceLoader::get_singleton()->exists(path))
   {
      ERR_PRINT(godot::vformat("Requesting to load and play '%s', however that file does not exist.", path));
      return;
   }

   godot::Ref<godot::AudioStream> stream = godot::ResourceLoader::get_singleton()->load(path);
   if (stream.is_valid())
   {
      play_audio(bus, stream, index, fade_time, extra);
   }
}


// Returns true if the audio stream player at index within the specified bus name is currently "marked" as in use
bool AudioMaster::is_used(const godot::String& bus, int index) const
{
   pdata_map_t::ConstIterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_V_MSG(iter == m_player_map.end(), false, "Attempting to query if player is in used, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   return iter->value.is_used(index);
}


// Returns true if the audio stream player at index within the specified bus name is currently playing something
bool AudioMaster::is_playing(const godot::String& bus, int index) const
{
   pdata_map_t::ConstIterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_V_MSG(iter == m_player_map.end(), false, "Attempting to query if player is playing, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   return iter->value.is_playing(index);
}


void AudioMaster::stop(const godot::String& bus, int index, double fade_time)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to stop playback but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   if (index >= iter->value.get_player_count()) { return; }

   if (fade_time > 0.0)
   {
      godot::Node* player = iter->value.get_stream_player<godot::Node>(index, false);
      create_fader(iter->value.get_bus_index(), index, player, iter->value.get_player_type(), fade_time, 1.0, 0.0);
   }
   else
   {
      if (iter->value.is_playing(index))
      {
         iter->value.release_player(index);
      }
   }
}

void AudioMaster::stop_all_in_bus(const godot::String& bus, double fade_time)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to stop playback in bus, but specified name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");


   godot::Vector<int> playing;
   iter->value.fill_currently_playing(playing);
   const int pcount = playing.size();

   if (fade_time > 0.0)
   {
      for (int i = 0; i < pcount; i++)
      {
         godot::Node* player = iter->value.get_stream_player<godot::Node>(playing[i], false);
         create_fader(iter->value.get_bus_index(), playing[i], player, iter->value.get_player_type(), fade_time, 1.0, 0.0);
      }
   }
   else
   {
      for (int i = 0; i < pcount; i++)
      {
         iter->value.release_player(playing[i]);
      }
   }
}


void AudioMaster::stop_all(double fade_time)
{
   godot::AudioServer* aserver = godot::AudioServer::get_singleton();
   const int bcount = aserver->get_bus_count();
   for (int i = 0; i < bcount; i++)
   {
      const godot::String bname = aserver->get_bus_name(i);
      stop_all_in_bus(bname, fade_time);
   }
}


double AudioMaster::get_playback_position(const godot::String& bus, int player_index) const
{
   pdata_map_t::ConstIterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_V_MSG(iter == m_player_map.end(), 0.0, "Attempting to retrieve playback position, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   return iter->value.get_playback_position(player_index);
}


void AudioMaster::set_playback_position(const godot::String& bus, int index, double position)
{
   pdata_map_t::Iterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_MSG(iter == m_player_map.end(), "Attempting to change playback position, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   iter->value.set_playback_position(index, position);
}


int AudioMaster::get_available_player_index(const godot::String& bus) const
{
   pdata_map_t::ConstIterator iter = m_player_map.find(bus);
   ERR_FAIL_COND_V_MSG(iter == m_player_map.end(), -1, "Attempting to retrieve available player index, but specified bus name wasn't found. If you changed bus configuration during runtime call scan_audio_bus()");

   return iter->value.get_free_player_index();
}


godot::Array AudioMaster::get_details_info() const
{
   godot::AudioServer* aserver = godot::AudioServer::get_singleton();
   godot::Array ret;

   const int bcount = aserver->get_bus_count();
   for (int i = 0; i < bcount; i++)
   {
      const godot::String bname = aserver->get_bus_name(i);
      const PlayerData& pd = m_player_map[bname];

      godot::Dictionary info;
      info["bus"] = bname;
      info["player_count"] = pd.get_player_count();
      info["max_count"] = pd.get_max_players();
      info["playing"] = pd.get_currently_playing_count();
      info["available"] = pd.get_available_player_count();
      info["type"] = type_to_name(pd.get_player_type());

      ret.append(info);
   }


   return ret;
}


AudioMaster::AudioMaster()
{

}



#endif
