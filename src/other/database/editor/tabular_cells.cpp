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

#ifndef DATABASE_DISABLED
#ifndef EDITOR_DISABLED

#include "tabular_cells.h"
#include "../../../internal.h"
#include "../../../extpackutils.h"
#include "../../../ui/spin_slider/spin_slider.h"

#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/color_picker_button.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/h_slider.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/popup.hpp>
#include <godot_cpp/classes/scroll_container.hpp>
#include <godot_cpp/classes/spin_box.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/v_box_container.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


static const char* ICON_PLAY = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GjRiNolhYTAyrGQ1qYmMxk1+Fxcwog83Mm19qfrzem0mTrbJVlNj4teAvYKuslSJSslPWxIbpOW9GjWTu7d7zud97zuncc8ESSisZvcEDmWxeC0z6HAvhRYf1GatMO530RhRdnQ1OhKg5Pu6oM+2N28xV2+/f0RKL6wrUNQmPKaqWF54SnlnNqyZvC3coqUhM+FTYpUmBwremHq3wi8nJCn+ZrIUCfrC0CTuSvzj6i5WUlhGWl+PMpAvKTz3mS2zx7HxQbI+sbnQCTOLDwTTj+PEyyKjsXtwMMSAnasR7yvFz5CRWkV2liMYKSVLkcYlakOxxsQnR4zLTFM3+/+2rnhgeqmS3+aDxyTDe+sC6BaVNw/g8NIzSEdQ/wkW2Gp87gJF30TermnMf7OtwdlnVojtwvgFdD2pEi5SlelmWRAJeT6A1DO3X0LxU6dnPPcf3EFqTr7qC3T3oF3/78jdRE2fcalyuKAAAAAlwSFlzAAALEwAACxMBAJqcGAAAAL9JREFUOI2t0z1OAkEYBuBvBBobD4T3sKSyMTGh5A6GxHAFKzsrEiMVrQUJDRAbr8BPYkPy2Lhxk13WAXyraebJvN/MRPxX0MLVOcANNlhgjFu0jgEeVTPDAJc5wEsNUGSFe6Qm4LUBKPKG7iFgmgHAGg+V0+A9EyjyhPZF9pQPpAx8Ze7ZRsQwInoppX25Qs4QJ7iuZTVf4wf6qFRul9afNe48Ip4jYpRS2jUW8/uUlz917tD5ayBl4LzPdGq+AVtPZRe+ZraeAAAAAElFTkSuQmCC";
static const char* ICON_STOP = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GjRiNolhYTAyrGQ1qYmMxk1+Fxcwog83Mm19qfrzem0mTrbJVlNj4teAvYKuslSJSslPWxIbpOW9GjWTu7d7zud97zuncc8ESSisZvcEDmWxeC0z6HAvhRYf1GatMO530RhRdnQ1OhKg5Pu6oM+2N28xV2+/f0RKL6wrUNQmPKaqWF54SnlnNqyZvC3coqUhM+FTYpUmBwremHq3wi8nJCn+ZrIUCfrC0CTuSvzj6i5WUlhGWl+PMpAvKTz3mS2zx7HxQbI+sbnQCTOLDwTTj+PEyyKjsXtwMMSAnasR7yvFz5CRWkV2liMYKSVLkcYlakOxxsQnR4zLTFM3+/+2rnhgeqmS3+aDxyTDe+sC6BaVNw/g8NIzSEdQ/wkW2Gp87gJF30TermnMf7OtwdlnVojtwvgFdD2pEi5SlelmWRAJeT6A1DO3X0LxU6dnPPcf3EFqTr7qC3T3oF3/78jdRE2fcalyuKAAAAAlwSFlzAAALEwAACxMBAJqcGAAAAI5JREFUOI2107ENwyAQheGf2A2MgZdgDEvxnskYLGGPAVVEmnO4ylIO+UnXcR8UPBiMA2itzcAKJCACAfAyAFWmAAeQgZdz7oMAW/s/T4CH3JAMr08aiAZg0UAwAF4D/uLgbUDQgDknUA27ZRSoGiijwGEAdg1kA5Chl2mil2mhF+n8YIVeqF2W378yjeQLis9Uivhs6/UAAAAASUVORK5CYII=";
static const char* ICON_PAUSE = "iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GjRiNolhYTAyrGQ1qYmMxk1+Fxcwog83Mm19qfrzem0mTrbJVlNj4teAvYKuslSJSslPWxIbpOW9GjWTu7d7zud97zuncc8ESSisZvcEDmWxeC0z6HAvhRYf1GatMO530RhRdnQ1OhKg5Pu6oM+2N28xV2+/f0RKL6wrUNQmPKaqWF54SnlnNqyZvC3coqUhM+FTYpUmBwremHq3wi8nJCn+ZrIUCfrC0CTuSvzj6i5WUlhGWl+PMpAvKTz3mS2zx7HxQbI+sbnQCTOLDwTTj+PEyyKjsXtwMMSAnasR7yvFz5CRWkV2liMYKSVLkcYlakOxxsQnR4zLTFM3+/+2rnhgeqmS3+aDxyTDe+sC6BaVNw/g8NIzSEdQ/wkW2Gp87gJF30TermnMf7OtwdlnVojtwvgFdD2pEi5SlelmWRAJeT6A1DO3X0LxU6dnPPcf3EFqTr7qC3T3oF3/78jdRE2fcalyuKAAAAAlwSFlzAAALEwAACxMBAJqcGAAAAEpJREFUOI1jYKAQMMIY////Z2ZgYMiDcicxMjL+RVZISJ7h////Cv8RQAHdJlzyTJR6YdSAUQMGhwEsSOzHDAwMRUhsdEBInjwAABk3KvDx/synAAAAAElFTkSuQmCC";


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBAudioPreview
godot::Ref<godot::Texture2D> DBAudioPreview::s_icon_play;
godot::Ref<godot::Texture2D> DBAudioPreview::s_icon_pause;
godot::Ref<godot::Texture2D> DBAudioPreview::s_icon_stop;


void DBAudioPreview::on_btplay_clicked()
{
   m_player->play(m_progress->get_value());

   m_playing = true;
   m_btplay->hide();
   m_btpause->show();
   set_process_internal(true);
   m_btpause->call_deferred("grab_focus");
   emit_signal("interacted");
   m_focused = m_btpause;
}


void DBAudioPreview::on_btpause_clicked()
{
   m_player->stop();

   m_playing = false;
   m_btplay->show();
   m_btpause->hide();

   m_btplay->call_deferred("grab_focus");

   set_process_internal(false);

   emit_signal("interacted");
   m_focused = m_btplay;
}


void DBAudioPreview::on_btstop_clicked()
{
   m_player->stop();

   m_playing = false;
   m_btplay->show();
   m_btpause->hide();

   m_progress->set_value(0);
   set_process_internal(false);

   emit_signal("interacted");
   m_focused = m_btstop;
}


void DBAudioPreview::on_seek_audio(float pos)
{
   if (m_seeking)
   {
      m_player->seek(pos);
   }
}


void DBAudioPreview::on_slider_drag_started()
{
   m_seeking = true;
   emit_signal("interacted");

   m_focused = m_progress;
}


void DBAudioPreview::on_slider_drag_finished(bool changed)
{
   m_seeking = false;
}


void DBAudioPreview::on_playback_finished()
{
   const bool has_focus = m_btpause->has_focus();

   m_playing = false;
   m_progress->set_value(0);
   m_btplay->show();
   m_btpause->hide();
   set_process_internal(false);

   if (has_focus)
   {
      m_btplay->call_deferred("grab_focus");
      m_focused = m_btplay;
   }
}


void DBAudioPreview::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_btplay->connect("pressed", callable_mp(this, &DBAudioPreview::on_btplay_clicked));
         m_btpause->connect("pressed", callable_mp(this, &DBAudioPreview::on_btpause_clicked));
         m_btstop->connect("pressed", callable_mp(this, &DBAudioPreview::on_btstop_clicked));
         m_progress->connect("value_changed", callable_mp(this, &DBAudioPreview::on_seek_audio));

         m_progress->connect("drag_ended", callable_mp(this, &DBAudioPreview::on_slider_drag_finished));
         m_progress->connect("drag_started", callable_mp(this, &DBAudioPreview::on_slider_drag_started));

         m_player->connect("finished", callable_mp(this, &DBAudioPreview::on_playback_finished));
      } break;

      case NOTIFICATION_PREDELETE:
      {
         s_icon_play.unref();
         s_icon_pause.unref();
         s_icon_stop.unref();
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         if (m_playing)
         {
            m_progress->set_value(m_player->get_playback_position());
         }
         else
         {
            set_process_internal(false);
         }
      } break;
   }
}


void DBAudioPreview::_bind_methods()
{
   using namespace godot;

   ADD_SIGNAL(MethodInfo("interacted"));
}


void DBAudioPreview::setup(const godot::Dictionary& styles)
{
   godot::Ref<godot::StyleBox> normal = styles.get("normal", nullptr);
   godot::Ref<godot::StyleBox> hover = styles.get("hover", nullptr);
   godot::Ref<godot::StyleBox> pressed = styles.get("pressed", nullptr);
   godot::Ref<godot::StyleBox> disabled = styles.get("disabled", nullptr);

   const godot::Size2i slider_minsz = m_progress->get_combined_minimum_size();
   const int32_t bt_side = styles.get("button_side", 16);
   const int32_t separation = styles.get("separation", 2);


   m_btplay->begin_bulk_theme_override();
   m_btplay->add_theme_stylebox_override("normal", normal);
   m_btplay->add_theme_stylebox_override("hover", hover);
   m_btplay->add_theme_stylebox_override("pressed", pressed);
   m_btplay->add_theme_stylebox_override("hover_pressed", pressed);
   m_btplay->add_theme_stylebox_override("disabled", disabled);
   m_btplay->end_bulk_theme_override();

   m_btpause->begin_bulk_theme_override();
   m_btpause->add_theme_stylebox_override("normal", normal);
   m_btpause->add_theme_stylebox_override("hover", hover);
   m_btpause->add_theme_stylebox_override("pressed", pressed);
   m_btpause->add_theme_stylebox_override("hover_pressed", pressed);
   m_btpause->add_theme_stylebox_override("disabled", disabled);
   m_btpause->end_bulk_theme_override();

   m_btstop->begin_bulk_theme_override();
   m_btstop->add_theme_stylebox_override("normal", normal);
   m_btstop->add_theme_stylebox_override("hover", hover);
   m_btstop->add_theme_stylebox_override("pressed", pressed);
   m_btstop->add_theme_stylebox_override("hover_pressed", pressed);
   m_btstop->add_theme_stylebox_override("disabled", disabled);
   m_btstop->end_bulk_theme_override();

   const int32_t bt_top = slider_minsz.height + separation;
   const int32_t bt_bottom = bt_top + bt_side;

   ExtPackUtils::set_control_offsets(m_progress, 0, 0, 0, slider_minsz.height);
   ExtPackUtils::set_control_offsets(m_btplay, 0, bt_top, bt_side, bt_bottom);
   ExtPackUtils::set_control_offsets(m_btpause, 0, bt_top, bt_side, bt_bottom);
   ExtPackUtils::set_control_offsets(m_btstop, bt_side + separation, bt_top, bt_side + separation + bt_side, bt_bottom);

   m_min_size.width = bt_side + separation + bt_side;
   m_min_size.height = slider_minsz.height + separation + bt_side;
   update_minimum_size();
}


void DBAudioPreview::set_stream(const godot::Ref<godot::AudioStream>& stream)
{
   m_player->set_stream(stream);

   const bool bt_visible = stream.is_valid();
   m_progress->set_visible(bt_visible);
   m_btplay->set_visible(bt_visible);
   m_btpause->hide();
   m_btstop->set_visible(bt_visible);

   if (stream.is_valid())
   {
      m_progress->set_max(stream->get_length());
      m_progress->set_value(0);
   }
}


void DBAudioPreview::remove_focus()
{
   if (m_focused && m_focused->has_focus())
   {
      m_focused->release_focus();
   }
   m_focused = nullptr;
}


DBAudioPreview::DBAudioPreview()
{
   set_clip_contents(true);
   set_mouse_filter(MOUSE_FILTER_PASS);
   m_focused = nullptr;
   m_seeking = false;
   m_playing = false;

   if (!s_icon_play.is_valid())
   {
      s_icon_play = Internals::texture_from_base64(ICON_PLAY);
   }
   if (!s_icon_pause.is_valid())
   {
      s_icon_pause = Internals::texture_from_base64(ICON_PAUSE);
   }
   if (!s_icon_stop.is_valid())
   {
      s_icon_stop = Internals::texture_from_base64(ICON_STOP);
   }

   m_progress = memnew(godot::HSlider);
   ExtPackUtils::set_control_anchors(m_progress, 0, 0, 1, 0);
   m_progress->set_mouse_filter(MOUSE_FILTER_PASS);
   m_progress->set_step(0.01);
   m_progress->set_visible(false);
   add_child(m_progress, false, INTERNAL_MODE_BACK);

   m_btplay = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btplay, 0, 0, 0, 0);
   m_btplay->set_visible(false);
   m_btplay->set_button_icon(s_icon_play);
   m_btplay->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
   add_child(m_btplay, false, INTERNAL_MODE_BACK);

   /// FIXME: Remove the pause button - and update the play button to perform the task of pausing if already playing
   m_btpause = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btpause, 0, 0, 0, 0);
   m_btpause->set_visible(false);
   m_btpause->set_button_icon(s_icon_pause);
   m_btpause->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
   add_child(m_btpause, false, INTERNAL_MODE_BACK);

   m_btstop = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btstop, 0, 0, 0, 0);
   m_btstop->set_visible(false);
   m_btstop->set_button_icon(s_icon_stop);
   m_btstop->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
   add_child(m_btstop, false, INTERNAL_MODE_BACK);


   m_player = memnew(godot::AudioStreamPlayer);
   add_child(m_player, false, INTERNAL_MODE_BACK);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBRowPreview
void DBRowPreview::clear_row()
{
   for (ColumnData& column : m_row)
   {
      for (godot::Control* aux : column.auxiliary)
      {
         aux->queue_free();
      }
   }

   m_row.clear();
}


void DBRowPreview::calculate_min_width()
{
   if (!m_theme_cache.initialized) { return; }

   const godot::HorizontalAlignment align = godot::HORIZONTAL_ALIGNMENT_LEFT;

   const float fheight = m_theme_cache.font->get_height(PREVIEW_FONT_SIZE);
   const float obj_side = fheight * 2.0f;
   const godot::Size2i obj_sz(obj_side, obj_side);

   float min_width = 0.0f;

   if (m_draw_background)
   {
      /// FIXME: Take the widest between 'normal', 'hover' and 'pressed' instead of just 'normal'
      const float hmargin = m_theme_cache.normal->get_margin(godot::SIDE_LEFT) + m_theme_cache.normal->get_margin(godot::SIDE_RIGHT);
      min_width += min_width;
   }

   bool need_vline = false;
   for (ColumnData& data : m_row)
   {
      if (data.title == "ID" && !m_render_id)
      {
         continue;
      }

      if (need_vline)
      {
         min_width += LINE_WIDTH + SEPARATION + SEPARATION;
      }

      switch (data.type)
      {
         case DBTable::VT_UniqueString:
         case DBTable::VT_UniqueInteger:
         case DBTable::VT_ExternalString:
         case DBTable::VT_ExternalInteger:
         case DBTable::VT_RandomWeight:
         case DBTable::VT_String:
         case DBTable::VT_Integer:
         case DBTable::VT_Float:
         case DBTable::VT_GenericRes:
         case DBTable::VT_MultiLineString:
         case DBTable::VT_StringArray:
         case DBTable::VT_IntegerArray:
         case DBTable::VT_FloatArray:
         case DBTable::VT_GenericResArray:
         {
            const godot::String strval = data.value;
            const int twidth = m_theme_cache.font->get_string_size(data.title, align, -1, PREVIEW_FONT_SIZE).x;
            const int vwidth = m_theme_cache.font->get_string_size(strval, align, -1, PREVIEW_FONT_SIZE).x;

            data.size.width = godot::Math::max<int>(twidth, vwidth);
            min_width += data.size.width;
         } break;

         case DBTable::VT_Bool:
         {
            godot::Ref<godot::Texture2D> tex;
            if (data.value.get_type() == godot::Variant::BOOL)
            {
               const bool bval = data.value;
               data.value = bval ? m_theme_cache.checked : m_theme_cache.unchecked;
            }

            tex = data.value;

            if (tex->get_height() > obj_side)
            {
               data.size = ExtPackUtils::get_draw_rect_size(tex, obj_sz);
            }
            else
            {
               data.size.width += tex->get_width();
            }
         } break;

         case DBTable::VT_Texture:
         {
            godot::Ref<godot::Texture2D> tex = data.value;
            if (!tex.is_valid())
            {
               // When "set_row_id()" is called there is a chance the theme cache is still not initialized. Because of that
               // it's possible the data.value is invalid. So, fix that here
               tex = m_theme_cache.no_image;
               data.value = tex;
            }

            data.size = ExtPackUtils::get_draw_rect_size(tex, obj_sz);
            min_width += data.size.width;
         } break;

         case DBTable::VT_Color:
         {
            //data.size.width = obj_sz.width;
            data.size = godot::Size2i(fheight, obj_side);
            min_width += data.size.width;
         } break;

         case DBTable::VT_ColorArray:
         {
            const int64_t count = ((godot::PackedColorArray)data.value).size();
            data.size.width = (fheight * count) + SEPARATION;
            min_width += data.size.width;
         } break;

         case DBTable::VT_Audio:
         case DBTable::VT_AudioArray:
         {
            godot::Size2i total;
            godot::Ref<godot::Texture2D> iplay;
            godot::Dictionary styles;
            styles["normal"] = m_theme_cache.empty;
            styles["pressed"] = m_theme_cache.empty;
            styles["hover"] = m_theme_cache.empty;
            styles["disabled"] = m_theme_cache.empty;
            styles["separation"] = SEPARATION;

            for (godot::Control* ctrl : data.auxiliary)
            {
               DBAudioPreview* aux = godot::Object::cast_to<DBAudioPreview>(ctrl);

               if (!iplay.is_valid())
               {
                  iplay = aux->get_icon_play();
                  styles["button_side"] = Internals::vmax<int32_t>(iplay->get_width(), iplay->get_height());
               }

               aux->setup(styles);

               const godot::Size2i s = aux->get_minimum_size();
               total.width += s.width;
               total.height = s.height;
            }

            data.size = total;
            min_width += total.width;
         } break;

         case DBTable::VT_TextureArray:
         {
            godot::Size2i total(0, obj_sz.height);

            for (godot::Control* ctrl : data.auxiliary)
            {
               godot::TextureRect* trect = godot::Object::cast_to<godot::TextureRect>(ctrl);
               const godot::Size2i dsize = ExtPackUtils::get_draw_rect_size(trect->get_texture(), obj_sz);
               trect->set_custom_minimum_size(dsize);

               total.width += dsize.width;
            }


            data.size = total;
            min_width += total.width;
         } break;

         //case DBTable::VT_ExtStringArray:
         //case DBTable::VT_ExtIntArray:
         //{
         //} break;
      }


      need_vline = true;
   }
   
   m_min_size.width = min_width;
}


void DBRowPreview::calculate_min_height()
{
   if (!m_theme_cache.initialized) { return; }

   float vmargin = 0;

   if (m_draw_background)
   {
      /// FIXME: use the "tallest" of "normal", "hover" and "pressed"
      vmargin = m_theme_cache.normal->get_margin(godot::SIDE_TOP) + m_theme_cache.normal->get_margin(godot::SIDE_BOTTOM);
   }

   const float fheight = m_theme_cache.font->get_height(PREVIEW_FONT_SIZE) * 2.0;

   // Assume there isn't any auxiliary control
   float aux_height = 0.0;
   for (const ColumnData& col : m_row)
   {
      if (col.auxiliary.size() > 0)
      {
         aux_height = godot::Math::max<float>(aux_height, col.auxiliary[0]->get_minimum_size().y);
      }
   }

   m_min_size.height = vmargin + godot::Math::max<float>(fheight, aux_height) + SEPARATION;
}


void DBRowPreview::draw_values()
{
   if (!m_theme_cache.initialized) { return; }
   
   const godot::HorizontalAlignment align = godot::HORIZONTAL_ALIGNMENT_LEFT;
   const float fheight = m_theme_cache.font->get_height(PREVIEW_FONT_SIZE);
   const float render_height = fheight * 2.0;

   float mleft = 0.0f;
   float mtop = 0.0f;
   float mright = 0.0f;
   float mbottom = 0.0f;

   if (m_draw_background)
   {
      godot::Ref<godot::StyleBox> stl;
      if (m_selected)
      {
         stl = m_theme_cache.pressed;
      }
      else
      {
         stl = m_mouse_over ? m_theme_cache.hover : m_theme_cache.normal;
      }
      draw_style_box(stl, godot::Rect2(godot::Vector2(), get_size()));

      mleft = stl->get_margin(godot::SIDE_LEFT);
      mtop = stl->get_margin(godot::SIDE_TOP);
      mright = stl->get_margin(godot::SIDE_RIGHT);
      mbottom = stl->get_margin(godot::SIDE_BOTTOM);
   }

   const float y0 = m_theme_cache.font->get_ascent(PREVIEW_FONT_SIZE) + mtop;
   const float y1 = y0 + fheight;
   float x = mleft;
   bool need_line = false;

   for (const ColumnData& data : m_row)
   {
      if (data.title == "ID" && !m_render_id)
      {
         continue;
      }

      if (need_line)
      {
         x += SEPARATION;
         draw_line(godot::Vector2(x, mtop), godot::Vector2(x, get_size().y - mbottom), m_theme_cache.text_color, LINE_WIDTH);
         x += LINE_WIDTH + SEPARATION;
      }

      switch (data.type)
      {
         case DBTable::VT_UniqueString:
         case DBTable::VT_UniqueInteger:
         case DBTable::VT_ExternalString:
         case DBTable::VT_ExternalInteger:
         case DBTable::VT_RandomWeight:
         case DBTable::VT_String:
         case DBTable::VT_Integer:
         case DBTable::VT_Float:
         case DBTable::VT_GenericRes:
         case DBTable::VT_MultiLineString:
         case DBTable::VT_StringArray:
         case DBTable::VT_IntegerArray:
         case DBTable::VT_FloatArray:
         case DBTable::VT_GenericResArray:
         {
            const godot::String strval = data.value;

            draw_string(m_theme_cache.font, godot::Vector2(x, y0), data.title, align, -1, PREVIEW_FONT_SIZE, m_theme_cache.text_color);
            draw_string(m_theme_cache.font, godot::Vector2(x, y1), strval, align, -1, PREVIEW_FONT_SIZE, m_theme_cache.text_color);
         } break;

         case DBTable::VT_Bool:
         case DBTable::VT_Texture:
         {
            godot::Ref<godot::Texture2D> tex = data.value;
            godot::Vector2 pos(x, mtop);
            if (data.size.height == 0)
            {
               // In here the texture is meant to be drawn in the middle of the available vertical space
               const float offset = (render_height - tex->get_height()) * 0.5;
               pos.y += offset;

               draw_texture(tex, pos);
            }
            else
            {
               draw_texture_rect(tex, godot::Rect2(pos, data.size), false);
            }
         } break;

         case DBTable::VT_Color:
         {
            const godot::Color cval = data.value;
            draw_rect(godot::Rect2(godot::Vector2(x, mtop), data.size), cval);
         } break;

         case DBTable::VT_ColorArray:
         {
            const godot::PackedColorArray arr = data.value;
            int32_t off = 0;
            const godot::Size2i sz(fheight, fheight * 2.0);

            for (const godot::Color& color : arr)
            {
               draw_rect(godot::Rect2(godot::Vector2(x + off, mtop), sz), color);

               off += sz.width;
            }
         } break;

         case DBTable::VT_Audio:
         case DBTable::VT_AudioArray:
         {
            int32_t off = 0;
            for (godot::Control* ctrl : data.auxiliary)
            {
               DBAudioPreview* aux = godot::Object::cast_to<DBAudioPreview>(ctrl);
               aux->set_position(godot::Vector2(x + off, mtop));
               off += aux->get_size().x;
            }
         } break;

         /// Perhaps move the VT_Texture to this case too
         case DBTable::VT_TextureArray:
         {
            int32_t off = 0;
            for (godot::Control* ctrl : data.auxiliary)
            {
               godot::TextureRect* trect = godot::Object::cast_to<godot::TextureRect>(ctrl);
               trect->set_position(godot::Vector2(x + off, mtop));

               off += trect->get_size().x;
            }
         } break;
      }

      need_line = true;
      x += data.size.width;
   }
}


void DBRowPreview::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_DRAW:
      {
         draw_values();
      } break;

      case NOTIFICATION_MOUSE_ENTER:
      {
         m_mouse_over = true;
         if (!m_selected && m_draw_background)
         {
            queue_redraw();
         }
      } break;

      case NOTIFICATION_MOUSE_EXIT:
      {
         m_mouse_over = false;
         if (!m_selected && m_draw_background)
         {
            queue_redraw();
         }
      } break;
   }
}


void DBRowPreview::_bind_methods()
{
   using namespace godot;

   ADD_SIGNAL(MethodInfo("clicked", PropertyInfo(Variant::NIL, "row_id")));
}


void DBRowPreview::_gui_input(const godot::Ref<godot::InputEvent>& event)
{
   godot::Ref<godot::InputEventMouseButton> mb = event;
   if (mb.is_valid() && mb->is_pressed())
   {
      if (mb->get_button_index() == godot::MOUSE_BUTTON_LEFT)
      {
         // Yes, OK, not exactly a click... whatever!
         emit_signal("clicked", m_id);
      }
   }
}


void DBRowPreview::setup(const godot::Dictionary& styles)
{
   m_theme_cache.font = styles.get("font", nullptr);
   m_theme_cache.empty = styles.get("empty", nullptr);
   m_theme_cache.normal = styles.get("normal", nullptr);
   m_theme_cache.hover = styles.get("hover", nullptr);
   m_theme_cache.pressed = styles.get("pressed", nullptr);
   m_theme_cache.no_image = styles.get("no_image", nullptr);
   m_theme_cache.unchecked = styles.get("unchecked", nullptr);
   m_theme_cache.checked = styles.get("checked", nullptr);
   m_theme_cache.text_color = styles.get("text_color", godot::Color(1, 1, 1, 1));

   m_theme_cache.initialized = true;


   calculate_min_width();
   calculate_min_height();

   update_minimum_size();
}


void DBRowPreview::set_table(const godot::Ref<DBTable>& table)
{
   m_table = table;
}


void DBRowPreview::set_row_id(const godot::Variant& id)
{
   m_id = id;
   clear_row();

   ERR_FAIL_COND_MSG(!m_table.is_valid(), "Setting RowID but DBTable is not valid!");

   godot::Dictionary row;
   if (m_id.get_type() == godot::Variant::INT || m_id.get_type() == godot::Variant::STRING)
   {
      row = m_table->get_row(m_id);
   }

   if (!row.is_empty())
   {
      // Even if ID is not meant to be rendered, add an entry for it as things becomes way easier later
      {
         ColumnData data;
         data.title = "ID";
         data.value = m_id;
         data.type = DBTable::VT_String;         // Regardless if Int or String, the rendering will be string

         m_row.append(data);
      }


      godot::PackedStringArray corder;
      m_table->get_column_order(corder);

      for (const godot::String& col_title : corder)
      {
         ColumnData data;
         data.title = col_title;
         data.type = m_table->get_column_value_type(col_title);

         switch (data.type)
         {
            case DBTable::VT_UniqueString:
            case DBTable::VT_UniqueInteger:
            case DBTable::VT_ExternalString:
            case DBTable::VT_ExternalInteger:
            case DBTable::VT_RandomWeight:
            case DBTable::VT_String:
            case DBTable::VT_Integer:
            case DBTable::VT_Float:
            case DBTable::VT_GenericRes:
            {
               const godot::String strval = row.get(col_title, "");
               data.value = strval;
            } break;

            case DBTable::VT_Bool:
            {
               const bool bval = row.get(col_title, false);
               if (m_theme_cache.initialized)
               {
                  godot::Ref<godot::Texture2D> tex = bval ? m_theme_cache.checked : m_theme_cache.unchecked;
                  data.value = tex;
               }
               else
               {
                  data.value = bval;
               }
            } break;

            case DBTable::VT_Texture:
            {
               godot::Ref<godot::Texture2D> tex;
               const godot::String path = row.get(col_title, "");
               if (path.is_empty() || !godot::ResourceLoader::get_singleton()->exists(path))
               {
                  tex = m_theme_cache.no_image;
               }
               else
               {
                  tex = godot::ResourceLoader::get_singleton()->load(path);
                  if (!tex.is_valid())
                  {
                     tex = m_theme_cache.no_image;
                  }
               }

               data.value = tex;
            } break;

            case DBTable::VT_Color:
            {
               const godot::Color cval = row.get(col_title, godot::Color(0, 0, 0, 0));
               data.value = cval;
            } break;

            case DBTable::VT_Audio:
            {
               DBAudioPreview* aux = memnew(DBAudioPreview);

               godot::Ref<godot::AudioStream> stream;
               const godot::String path = row.get(col_title, "");
               if (!path.is_empty() && godot::ResourceLoader::get_singleton()->exists(path))
               {
                  stream = godot::ResourceLoader::get_singleton()->load(path);
               }

               data.auxiliary.append(aux);
               add_child(aux);

               data.value = stream;
               aux->set_stream(stream);
               
            } break;

            case DBTable::VT_MultiLineString:
            {
               const godot::String fullstr = row.get(col_title, "");

               if (fullstr.is_empty())
               {
                  data.value = godot::String("");
               }
               else
               {
                  godot::PackedStringArray split = fullstr.split("\n");
                  data.value = split[0] + "...";
               }
            } break;

            //case DBTable::VT_ExtStringArray:
            //case DBTable::VT_ExtIntArray:
            //{
            //} break;

            case DBTable::VT_StringArray:
            case DBTable::VT_GenericResArray:
            {
               const godot::PackedStringArray arr = row.get(col_title, godot::PackedStringArray());
               // Limit the amount of string values to be displayed to 3. After that append a "..."
               const int32_t sz = godot::Math::min<int32_t>(arr.size(), 3);

               godot::String val;

               for (int32_t i = 0; i < sz; i++)
               {
                  if (i > 0)
                  {
                     val += ", ";
                  }

                  val += data.type == DBTable::VT_StringArray ? arr[i] : arr[i].get_file();
               }

               if (arr.size() > 3)
               {
                  val += ", ...";
               }

               data.value = godot::vformat("[%s]", val);
            } break;

            case DBTable::VT_IntegerArray:
            {
               const godot::PackedInt64Array arr = row.get(col_title, godot::PackedInt64Array());
               // Limit the amount of int value to be displayed to 3. After that append a "..."
               const int32_t sz = godot::Math::min<int32_t>(arr.size(), 3);

               godot::String val;

               for (int32_t i = 0; i < sz; i++)
               {
                  if (i > 0)
                  {
                     val += ", ";
                  }

                  val += godot::String::num_int64(arr[i]);
               }

               if (arr.size() > 3)
               {
                  val += ", ...";
               }

               data.value = godot::vformat("[%s]", val);
            } break;

            case DBTable::VT_FloatArray:
            {
               const godot::PackedFloat64Array arr = row.get(col_title, godot::PackedFloat64Array());
               const int32_t sz = godot::Math::min<int32_t>(arr.size(), 3);

               godot::String val;

               for (int32_t i = 0; i < sz; i++)
               {
                  if (i > 0)
                  {
                     val += ", ";
                  }

                  val += godot::String::num_real(arr[i]);
               }

               if (arr.size() > 3)
               {
                  val += ", ...";
               }

               data.value = godot::vformat("[%s]", val);
            } break;

            case DBTable::VT_TextureArray:
            {
               // This is a packed string array, of paths to the textures. Relay the texture drawing to instances of godot::TextureRect
               // In this case the data.value is irrelevant
               const godot::PackedStringArray arr = row.get(col_title, godot::PackedStringArray());

               for (const godot::String& path : arr)
               {
                  if (path.is_empty() || !godot::ResourceLoader::get_singleton()->exists(path))
                  {
                     continue;
                  }

                  godot::Ref<godot::Texture2D> tex = godot::ResourceLoader::get_singleton()->load(path);
                  if (!tex.is_valid())
                  {
                     continue;
                  }

                  godot::TextureRect* trect = memnew(godot::TextureRect);
                  trect->set_expand_mode(godot::TextureRect::EXPAND_IGNORE_SIZE);
                  trect->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_ASPECT);
                  trect->set_texture(tex);
                  add_child(trect);

                  data.auxiliary.append(trect);
               }
            } break;

            case DBTable::VT_AudioArray:
            {
               const godot::PackedStringArray arr = row.get(col_title, godot::PackedStringArray());

               for (const godot::String& path : arr)
               {
                  if (path.is_empty() || !godot::ResourceLoader::get_singleton()->exists(path))
                  {
                     continue;
                  }

                  godot::Ref<godot::AudioStream> stream = godot::ResourceLoader::get_singleton()->load(path);
                  if (!stream.is_valid())
                  {
                     continue;
                  }

                  DBAudioPreview* aux = memnew(DBAudioPreview);
                  data.auxiliary.append(aux);
                  add_child(aux);

                  aux->set_stream(stream);
               }
            } break;

            case DBTable::VT_ColorArray:
            {
               const godot::PackedColorArray arr = row.get(col_title, godot::PackedColorArray());
               data.value = arr;
            } break;
         }

         m_row.append(data);
      }
   }

   
   calculate_min_width();
   calculate_min_height();
   if (is_inside_tree())
   {
      update_minimum_size();
   }
}


void DBRowPreview::set_draw_background(bool enable)
{
   m_draw_background = enable;
   
   calculate_min_height();
   calculate_min_width();
   if (is_inside_tree())
   {
      update_minimum_size();
   }
}


void DBRowPreview::set_render_id(bool enable)
{
   m_render_id = enable;
   
   calculate_min_width();
   if (is_inside_tree())
   {
      update_minimum_size();
   }
}


void DBRowPreview::set_selected(bool selected)
{
   m_selected = selected;

   if (is_inside_tree())
   {
      queue_redraw();
   }
}


void DBRowPreview::apply_filter(const godot::String& str)
{
   if (str.is_empty())
   {
      if (!is_visible())
      {
         show();
      }  
   } 
   else
   {
      for (const ColumnData& data : m_row)
      {
         const godot::String strval = godot::String(data.value).to_lower();
         if (strval.contains(str))
         {
            show();
            return;
         }
      }

      // The loop above is meant to interrupt and bail as soon as there is a matching value
      // This that if code reaches here then no column has any value with the filter text.
      hide();
   }
}


DBRowPreview::DBRowPreview()
{
   set_mouse_filter(MOUSE_FILTER_PASS);
   m_theme_cache.initialized = false;

   m_render_id = false;
   m_draw_background = false;
   m_mouse_over = false;
   m_selected = false;

   set_clip_contents(true);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTabularCellAudio
void DBTabularCellAudio::on_btval_clicked()
{
   godot::PackedStringArray filter;
   filter.append("*.ogg; OGG");
   filter.append("*.oggstr; OGGSTR");
   filter.append("*.res; RES");
   filter.append("*.sample; SAMPLE");
   filter.append("*.tres; TRES");
   filter.append("*.wav; WAV");

   request_file_dialog("Load audio", filter, callable_mp(this, &DBTabularCellAudio::file_selected));
   notify_selected();
}


void DBTabularCellAudio::on_btclear_clicked()
{
   notify_value_changed("");
   notify_selected();
}


void DBTabularCellAudio::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_btvalue->connect("pressed", callable_mp(this, &DBTabularCellAudio::on_btval_clicked));
         m_btclear->connect("pressed", callable_mp(this, &DBTabularCellAudio::on_btclear_clicked));

         TabularBoxCell* cell = this;
         m_audio_player->connect("interacted", godot::create_custom_callable_function_pointer(cell, &TabularBoxCell::notify_selected));
      } break;
   }
}


void DBTabularCellAudio::check_theme()
{
   const int32_t side_play = get_button_min_height(false, m_audio_player->get_icon_play(), true);
   const int32_t side_pause = get_button_min_height(false, m_audio_player->get_icon_pause(), true);
   const int32_t side_stop = get_button_min_height(false, m_audio_player->get_icon_stop(), true);

   // This takes Icon in consideration
   const int32_t button_side = Internals::vmax<int32_t>(side_play, side_pause, side_stop);
   // This is for a button without icon (the value)
   const int32_t bt_height = godot::Math::max<int32_t>(get_button_height(), button_side);

   const float mleft = get_internal_margin_left();
   const float mtop = get_internal_margin_top();
   const float mright = get_internal_margin_right();
   const float mbottom = get_internal_margin_bottom();

   const int32_t separation = get_theme_separation();

   godot::Dictionary styles;
   styles["normal"] = get_style_button_normal();
   styles["hover"] = get_style_button_hover();
   styles["pressed"] = get_style_button_pressed();
   styles["disabled"] = get_style_empty();
   styles["separation"] = separation;
   styles["button_side"] = button_side;

   m_audio_player->setup(styles);
   apply_button_style(m_btvalue);

   const int32_t player_height = m_audio_player->get_minimum_size().y;

   ExtPackUtils::set_control_offsets(m_audio_player, mleft, mtop, -mright, mtop + player_height);
   ExtPackUtils::set_control_offsets(m_btvalue, mleft, -(bt_height + mbottom), -(bt_height + separation + mright), -mbottom);

   m_btclear->begin_bulk_theme_override();
   m_btclear->add_theme_stylebox_override("normal", get_style_empty());
   m_btclear->add_theme_stylebox_override("hover", get_style_empty());
   m_btclear->add_theme_stylebox_override("pressed", get_style_empty());
   m_btclear->add_theme_stylebox_override("focus", get_style_empty());
   m_btclear->add_theme_font_size_override("font_size", get_font_size());
   m_btclear->end_bulk_theme_override();
   m_btclear->set_button_icon(get_icon_trash());
   ExtPackUtils::set_control_offsets(m_btclear, -(bt_height + mright), -(bt_height + mbottom), -mright, -mbottom);

   set_min_height(mtop + player_height + separation + bt_height + separation + mbottom);
}


void DBTabularCellAudio::assign_value(const godot::Variant& value)
{
   m_assigned = value;

   godot::Ref<godot::AudioStream> stream;
   godot::String txtval = "...";
   godot::String tooltip = "Load audio resource.";

   godot::ResourceLoader* loader = godot::ResourceLoader::get_singleton();

   if (!m_assigned.is_empty())
   {
      if (!loader->exists(m_assigned))
      {
         txtval = godot::vformat("!%s", m_assigned.get_file());
         tooltip = godot::vformat("'%s' does not exist.\nClick to load audio resource.", m_assigned);
      }
      else
      {
         stream = loader->load(m_assigned);
         if (stream.is_valid())
         {
            txtval = m_assigned.get_file();
            tooltip = m_assigned;
         }
         else
         {
            txtval = godot::vformat("!%s", m_assigned.get_file());
            tooltip = godot::vformat("'%s' is not an Audio.\nClick to load audio resource.", m_assigned);
         }
      }
   }

   m_btvalue->set_text(txtval);
   m_btvalue->set_tooltip_text(tooltip);
   m_audio_player->set_stream(stream);
   m_btclear->set_visible(stream.is_valid());
}


void DBTabularCellAudio::selected_changed(bool selected)
{
   if (selected)
   {
      if (!m_audio_player->has_focused_control() && !m_btvalue->has_focus())
      {
         m_btvalue->grab_focus();
      }
   }
   else
   {
      if (m_audio_player->has_focused_control())
      {
         m_audio_player->remove_focus();
      }
      if (m_btvalue->has_focus())
      {
         m_btvalue->release_focus();
      }
      if (m_btclear->has_focus())
      {
         m_btclear->release_focus();
      }
   }
}


bool DBTabularCellAudio::_can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const
{
   if (data.get_type() != godot::Variant::DICTIONARY) { return false; }

   const godot::Dictionary ddata = data;

   const godot::String type = ddata.get("type", "");
   if (type != "files") { return false; }

   const godot::Array files = ddata.get("files", godot::Array());
   if (files.size() != 1)
   {
      return false;
   }

   const godot::String path = files[0];

   /// TODO: Check if there is a better way to verify if the dragged file is of a proper resource type that don't rely
   // on loading it first
   godot::Ref<godot::AudioStream> stream = godot::ResourceLoader::get_singleton()->load(path);

   return stream.is_valid();
}


void DBTabularCellAudio::_drop_data(const godot::Vector2& pos, const godot::Variant& data)
{
   const godot::Dictionary ddata = data;
   const godot::Array files = ddata.get("files", godot::Array());
   const godot::String path = files[0];
   notify_value_changed(path);
}


void DBTabularCellAudio::file_selected(const godot::String& path)
{
   notify_value_changed(path);
}


DBTabularCellAudio::DBTabularCellAudio()
{
   m_audio_player = memnew(DBAudioPreview);
   ExtPackUtils::set_control_anchors(m_audio_player, 0, 0, 1, 0);
   add_child(m_audio_player);

   m_btvalue = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btvalue, 0, 1, 1, 1);
   m_btvalue->set_text("...");
   m_btvalue->set_clip_text(true);
   m_btvalue->set_text_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
   add_child(m_btvalue, false, INTERNAL_MODE_BACK);


   m_btclear = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btclear, 1, 1, 1, 1);
   m_btclear->set_visible(false);
   m_btclear->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
   add_child(m_btclear, false, INTERNAL_MODE_BACK);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTabularCellGenericResource
void DBTabularCellGenericResource::on_btvalue_clicked()
{
   godot::PackedStringArray filter;
   
   request_file_dialog("Load resource", filter, callable_mp(this, &DBTabularCellGenericResource::file_selected));
   notify_selected();
}


void DBTabularCellGenericResource::on_btclear_clicked()
{
   notify_value_changed("");
}


void DBTabularCellGenericResource::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_btvalue->connect("pressed", callable_mp(this, &DBTabularCellGenericResource::on_btvalue_clicked));
         m_btclear->connect("pressed", callable_mp(this, &DBTabularCellGenericResource::on_btclear_clicked));
      } break;
   }
}


void DBTabularCellGenericResource::check_theme()
{
   apply_button_style(m_btvalue);
   //apply_button_style(m_btclear);

   const float btheight = get_button_height();
   const float half_btheight = btheight * 0.5;
   const float mtop = get_internal_margin_top();
   const float mright = get_internal_margin_right();
   const float mbottom = get_internal_margin_bottom();
   const int32_t separation = get_theme_separation();

   m_btclear->begin_bulk_theme_override();
   m_btclear->add_theme_stylebox_override("normal", get_style_empty());
   m_btclear->add_theme_stylebox_override("pressed", get_style_empty());
   m_btclear->add_theme_stylebox_override("hover", get_style_empty());
   m_btclear->end_bulk_theme_override();

   m_btclear->set_button_icon(get_icon_trash());
   ExtPackUtils::set_control_offsets(m_btvalue, get_internal_margin_left(), -half_btheight, -(btheight + separation + mright), half_btheight);
   ExtPackUtils::set_control_offsets(m_btclear, -(btheight + mright), -half_btheight, -mright, half_btheight);


   set_min_height(mtop + btheight + mbottom);
}


void DBTabularCellGenericResource::assign_value(const godot::Variant& value)
{
   m_assigned = value;

   if (m_assigned.is_empty())
   {
      m_btvalue->set_text("...");
      m_btclear->hide();
   }
   else
   {
      if (godot::ResourceLoader::get_singleton()->exists(m_assigned))
      {
         m_btvalue->set_text(m_assigned.get_file());
         m_btvalue->set_tooltip_text(m_assigned);
      }
      else
      {
         m_btvalue->set_text(godot::vformat("!%s", m_assigned.get_file()));
         m_btvalue->set_tooltip_text(godot::vformat("%s does not exist.", m_assigned));
      }
      
      m_btclear->show();
   }
}


void DBTabularCellGenericResource::selected_changed(bool selected)
{

}


bool DBTabularCellGenericResource::_can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const
{
   if (data.get_type() != godot::Variant::DICTIONARY) { return false; }

   const godot::Dictionary ddata = data;

   const godot::String type = ddata.get("type", "");
   if (type != "files") { return false; }

   const godot::Array files = ddata.get("files", godot::Array());
   if (files.size() != 1)
   {
      return false;
   }

   const godot::String path = files[0];

   /// TODO: Check if there is a better way to verify if the dragged file is a resource without loading it
   godot::Ref<godot::Resource> res = godot::ResourceLoader::get_singleton()->load(path);

   return res.is_valid();
}


void DBTabularCellGenericResource::_drop_data(const godot::Vector2& pos, const godot::Variant& data)
{
   const godot::Dictionary ddata = data;
   const godot::Array files = ddata.get("files", godot::Array());
   const godot::String path = files[0];
   notify_value_changed(path);
}


void DBTabularCellGenericResource::file_selected(const godot::String& path)
{
   if (path != m_assigned)
   {
      notify_value_changed(path);
   }
}


DBTabularCellGenericResource::DBTabularCellGenericResource()
{
   m_btvalue = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btvalue, 0, 0.5, 1, 0.5);
   m_btvalue->set_text_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
   m_btvalue->set_mouse_filter(MOUSE_FILTER_PASS);
   add_child(m_btvalue, false, INTERNAL_MODE_BACK);

   m_btclear = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btclear, 1, 0.5, 1, 0.5);
   m_btclear->set_mouse_filter(MOUSE_FILTER_PASS);
   m_btclear->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
   add_child(m_btclear, false, INTERNAL_MODE_BACK);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTabularCellColor
void DBTabularCellColor::on_bt_clicked()
{
   notify_selected();
}


void DBTabularCellColor::on_bt_value_changed(const godot::Color& val)
{
   notify_value_changed(val);
}


void DBTabularCellColor::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_button->connect("pressed", callable_mp(this, &DBTabularCellColor::on_bt_clicked));
         m_button->connect("color_changed", callable_mp(this, &DBTabularCellColor::on_bt_value_changed));
      } break;
   }
}


void DBTabularCellColor::check_theme()
{
   //apply_button_style(m_button);
   m_button->begin_bulk_theme_override();
   m_button->add_theme_stylebox_override("normal", get_style_empty());
   m_button->add_theme_stylebox_override("pressed", get_style_empty());
   m_button->add_theme_stylebox_override("hover", get_style_empty());
   m_button->add_theme_stylebox_override("focus", get_style_empty());
   m_button->add_theme_font_size_override("font_size", get_font_size());
   m_button->end_bulk_theme_override();

   const float mtop = get_internal_margin_top();
   const float mbottom = get_internal_margin_bottom();

   const int32_t mheight = godot::Math::max<int32_t>(8, m_button->get_combined_minimum_size().y);

   m_button->set_anchor_and_offset(godot::SIDE_LEFT, 0, get_internal_margin_left());
   m_button->set_anchor_and_offset(godot::SIDE_TOP, 0, mtop);
   m_button->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -get_internal_margin_right());
   m_button->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -mbottom);

   set_min_height(mtop + mheight + mbottom);
}


void DBTabularCellColor::assign_value(const godot::Variant& value)
{
   const godot::Color cval = value;
   const godot::Color in_pick = m_button->get_pick_color();
   if (cval != in_pick)
   {
      m_button->set_pick_color(cval);
   }
   m_assigned = cval;

   const godot::String tt = godot::vformat("%s\n#%s", cval, cval.to_html());
   m_button->set_tooltip_text(tt);
}


void DBTabularCellColor::selected_changed(bool selected)
{
   if (selected && !m_button->has_focus())
   {
      m_button->grab_focus();
   }
   else if (!selected && m_button->has_focus())
   {
      m_button->release_focus();
   }
}


DBTabularCellColor::DBTabularCellColor()
{
   m_button = memnew(godot::ColorPickerButton);
   m_button->set_mouse_filter(MOUSE_FILTER_PASS);
   add_child(m_button, false, INTERNAL_MODE_BACK);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTabularCellRandomWeight
void DBTabularCellRandomWeight::_notification(int what)
{
   if (!m_table.is_valid()) { return; }

   if (what == NOTIFICATION_MOUSE_ENTER)
   {
      const int64_t rindex = get_row_index();

      const double acc_weight = m_table->get_row_accumulated_weight(rindex);
      const double total_sum = m_table->get_total_weight_sum();

      const double probability = total_sum > 0 ? get_spin()->get_value() / total_sum : 0.0;
      get_spin()->set_tooltip_text(godot::vformat("Probability: %s%%\nAccumulated weight: %s", probability * 100.0, acc_weight));
   }
}


void DBTabularCellRandomWeight::apply_extra_settings(const godot::Dictionary& extra_settings)
{
   TabularBoxCellFloat::apply_extra_settings(extra_settings);

   m_table = extra_settings.get("table", nullptr);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTabularCellExternalID
void DBTabularCellExternalID::ref_shared(SharedData* shared)
{
   if (m_shared && shared == m_shared)
   {
      return;
   }

   unref_shared();
   m_shared = shared;
   m_shared->owner_list.insert(this);
}


void DBTabularCellExternalID::unref_shared()
{
   if (m_shared)
   {
      m_shared->owner_list.erase(this);
      if (m_shared->owner_list.size() == 0)
      {
         m_shared->pop->queue_free();
         godot::memdelete(m_shared);
         m_shared = nullptr;
      }
   }
}


godot::Dictionary DBTabularCellExternalID::build_styles() const
{
   godot::Dictionary styles;
   if (has_theme())
   {
      styles["font"] = get_font();
      styles["empty"] = get_style_empty();
      styles["normal"] = get_style_button_normal();
      styles["hover"] = get_style_button_hover();
      styles["pressed"] = get_style_button_pressed();
      styles["no_image"] = get_icon_no_texture();
      styles["unchecked"] = get_icon_unchecked();
      styles["checked"] = get_icon_checked();
      styles["text_color"] = get_font_color();
   }

   return styles;
}


void DBTabularCellExternalID::on_pop_hide()
{
   // Several of the event handlers are connected as "one shot". However some are not. Disconnect those here
   if (m_shared->clicked_row_id.get_type() != godot::Variant::NIL && m_shared->clicked_row_id != m_assigned)
   {
      notify_value_changed(m_shared->clicked_row_id);
   }

   // Just to make sure, although when popping the menu it will be reset again
   m_shared->clicked_row_id = godot::Variant();
}


void DBTabularCellExternalID::on_btval_clicked()
{
   if (!m_shared || !m_shared->pop)
   {
      return;
   }

   m_shared->clicked_row_id = godot::Variant();

   godot::Vector2 pop_pos = m_btvalue->get_screen_position();
   pop_pos.y += m_btvalue->get_size().y;

   // Adding 4 to the height here to take the margins (offsets) set in the vbox
   godot::Size2i pop_size(320, m_shared->filter->get_minimum_size().y + 4);

   m_shared->filter->set_text("");

   for (DBRowPreview* preview : m_shared->pop_entry)
   {
      preview->set_selected(preview->get_row_id() == m_assigned);
   }

   if (m_shared->pop_entry.size() == 0)
   {
      m_shared->filter->set_editable(false);
      m_shared->filter->set_placeholder("Referenced table is empty");
   }
   else
   {
      m_shared->filter->set_editable(true);
      m_shared->filter->set_placeholder("Filter...");

      const float entry_height = m_shared->pop_entry[0]->get_minimum_size().y;
      pop_size.height += (entry_height + m_shared->separation) * m_shared->pop_entry.size();

      pop_size.height = godot::Math::min<int>(pop_size.height, 460);
   }

   m_shared->pop->set_position(pop_pos);
   m_shared->pop->set_size(pop_size);
   m_shared->pop->popup();

   m_shared->pop->connect("popup_hide", callable_mp(this, &DBTabularCellExternalID::on_pop_hide), CONNECT_ONE_SHOT);
}


void DBTabularCellExternalID::on_btclear_clicked()
{
   const int id_type = m_shared->other_table->get_id_type();
   if (id_type == godot::Variant::INT)
   {
      notify_value_changed(-1);
   }
   else if (id_type == godot::Variant::STRING)
   {
      notify_value_changed("");
   }
}


void DBTabularCellExternalID::on_filtering(const godot::String& str)
{
   // Making everything lower case so filtering is always case insensitive - this could be a setting but well...
   const godot::String lowerf = str.to_lower();
   for (DBRowPreview* preview : m_shared->pop_entry)
   {
      preview->apply_filter(lowerf);
   }
}


void DBTabularCellExternalID::on_entry_clicked(const godot::Variant& row_id)
{
   m_shared->clicked_row_id = row_id;
   m_shared->pop->hide();
}


void DBTabularCellExternalID::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_btvalue->connect("pressed", callable_mp(this, &DBTabularCellExternalID::on_btval_clicked));
         m_btclear->connect("pressed", callable_mp(this, &DBTabularCellExternalID::on_btclear_clicked));
      } break;
   }
}


void DBTabularCellExternalID::check_theme()
{
   apply_button_style(m_btvalue);

   godot::Dictionary styles = build_styles();

   m_preview->setup(styles);

   const float mleft = get_internal_margin_left();
   const float mtop = get_internal_margin_top();
   const float mright = get_internal_margin_right();
   const float mbottom = get_internal_margin_bottom();
   const int32_t separation = get_theme_separation();
   const float preview_min_height = m_preview->get_minimum_size().y;

   const int32_t btside = get_button_height();

   ExtPackUtils::set_control_offsets(m_preview, mleft, mtop, -mright, mtop + preview_min_height);
   ExtPackUtils::set_control_offsets(m_btvalue, mleft, -(btside + mbottom), -(btside + mright), -mbottom);

   m_btclear->begin_bulk_theme_override();
   m_btclear->add_theme_stylebox_override("normal", get_style_empty());
   m_btclear->add_theme_stylebox_override("hover", get_style_empty());
   m_btclear->add_theme_stylebox_override("pressed", get_style_empty());
   m_btclear->add_theme_stylebox_override("focus", get_style_empty());
   m_btclear->add_theme_font_size_override("font_size", get_font_size());
   m_btclear->end_bulk_theme_override();
   m_btclear->set_button_icon(get_icon_trash());
   ExtPackUtils::set_control_offsets(m_btclear, -(btside + mright), -(btside + mbottom), -mright, -mbottom);

   if (get_row_index() == 0)
   {
      for (DBRowPreview* preview : m_shared->pop_entry)
      {
         preview->setup(styles);
      }
   }

   set_min_height(mtop + preview_min_height + separation + btside + mbottom);
}


void DBTabularCellExternalID::assign_value(const godot::Variant& value)
{
   ERR_FAIL_COND_MSG(!m_shared, "Attempting to assign value into cell that requires shared internal data, however that data hasn't been initialized yet.");

   m_assigned = value;
   m_preview->set_table(m_shared->other_table);
   m_preview->set_row_id(m_assigned);
   m_preview->queue_redraw();

   godot::String strid = value.get_type() != godot::Variant::NIL ? value : "...";
   if (strid.is_empty())
   {
      strid = "...";
   }
   m_btvalue->set_text(strid);
   m_btclear->set_visible(strid != "...");
}


void DBTabularCellExternalID::selected_changed(bool selected)
{

}


void DBTabularCellExternalID::apply_extra_settings(const godot::Dictionary& extra_settings)
{
   // This will always be called *after* either setup_shared() or share_with(). This means that at this
   // point m_shared is valid. The setup_shared() is called only for the cell in the first row, while
   // share_with() is called for every other cell in the column. However the extra settings is called
   // for every single cell. In other words, there is a chance the internal setup has already been done
   if (m_shared->other_table.is_valid())
   {
      m_preview->set_table(m_shared->other_table);
      return;
   }

   m_shared->other_table = extra_settings.get("table", nullptr);
   ERR_FAIL_COND_MSG(!m_shared->other_table.is_valid(), "Setting up table reference, but incoming referenced one is not valid!");

   godot::VBoxContainer* vbox = memnew(godot::VBoxContainer);
   vbox->set_anchor_and_offset(godot::SIDE_LEFT, 0, 2);
   vbox->set_anchor_and_offset(godot::SIDE_TOP, 0, 2);
   vbox->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -2);
   vbox->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -2);
   m_shared->pop->add_child(vbox);
   m_shared->separation = vbox->get_theme_constant("separation", "VBoxContainer");
   

   godot::LineEdit* filter = memnew(godot::LineEdit);
   filter->set_clear_button_enabled(true);
   filter->set_h_size_flags(SIZE_EXPAND_FILL);
   vbox->add_child(filter);
   m_shared->filter = filter;
   m_shared->filter->connect("text_changed", callable_mp(this, &DBTabularCellExternalID::on_filtering));

   godot::ScrollContainer* scroller = memnew(godot::ScrollContainer);
   scroller->set_h_size_flags(SIZE_EXPAND_FILL);
   scroller->set_v_size_flags(SIZE_EXPAND_FILL);
   vbox->add_child(scroller);

   godot::VBoxContainer* entry_box = memnew(godot::VBoxContainer);
   entry_box->set_h_size_flags(SIZE_EXPAND_FILL);
   entry_box->set_v_size_flags(SIZE_EXPAND_FILL);
   scroller->add_child(entry_box);

   godot::Dictionary styles = build_styles();

   const int64_t rcount = m_shared->other_table->get_row_count();
   for (int64_t i = 0; i < rcount; i++)
   {
      DBRowPreview* preview = memnew(DBRowPreview);
      preview->set_table(m_shared->other_table);
      preview->set_row_id(m_shared->other_table->get_row_id(i));
      preview->set_draw_background(true);
      preview->set_render_id(true);
      if (!styles.is_empty())
      {
         preview->setup(styles);
      }
      entry_box->add_child(preview);

      m_shared->pop_entry.append(preview);

      // It doesn't matter which Cell will handle the click itself. The handler will store the clicked
      // row ID into the m_shared object and request the popup to be hidden, which will trigger yet another
      // event. At that point the relevant cell will properly handle the selection
      preview->connect("clicked", callable_mp(this, &DBTabularCellExternalID::on_entry_clicked));
   }
}


void DBTabularCellExternalID::setup_shared(godot::Control* parent)
{
   m_shared = memnew(SharedData);
   m_shared->owner_list.insert(this);
   m_shared->pop_parent = parent;

   m_shared->pop = memnew(godot::Popup);
   m_shared->pop->hide();
   m_shared->pop_parent->add_child(m_shared->pop);
}


void DBTabularCellExternalID::share_with(TabularBoxCell* other_cell)
{
   DBTabularCellExternalID* other = godot::Object::cast_to<DBTabularCellExternalID>(other_cell);
   if (!other) { return; }

   other->ref_shared(m_shared);
}


DBTabularCellExternalID::DBTabularCellExternalID()
{
   m_shared = nullptr;
   
   m_preview = memnew(DBRowPreview);
   ExtPackUtils::set_control_anchors(m_preview, 0, 0, 1, 0);
   m_preview->set_render_id(false);
   add_child(m_preview, false, INTERNAL_MODE_BACK);


   m_btvalue = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btvalue, 0, 1, 1, 1);
   m_btvalue->set_text("...");
   m_btvalue->set_clip_text(true);
   m_btvalue->set_text_alignment(godot::HORIZONTAL_ALIGNMENT_LEFT);
   add_child(m_btvalue, false, INTERNAL_MODE_BACK);

   m_btclear = memnew(godot::Button);
   ExtPackUtils::set_control_anchors(m_btclear, 1, 1, 1, 1);
   m_btclear->set_visible(false);
   m_btclear->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
   add_child(m_btclear, false, INTERNAL_MODE_BACK);
}


DBTabularCellExternalID::~DBTabularCellExternalID()
{
   unref_shared();
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTabularCellArray
void DBTabularCellArray::set_entry(const godot::Variant& value, int64_t index)
{
   ERR_FAIL_COND_MSG(index > m_entry.size(), "Attempting to entry value but incoming index is invalid!");
   ERR_FAIL_COND_MSG(!has_theme(), "Attempting to set entry value before theme has been initialized!");

   const bool must_create = index == m_entry.size();

   if (must_create)
   {
      Entry entry;
      entry.box = memnew(godot::HBoxContainer);
      entry.box->set_h_size_flags(SIZE_EXPAND_FILL);
      m_vbox->add_child(entry.box);

      entry.box->connect("draw", callable_mp(this, &DBTabularCellArray::on_draw_hbox).bind(entry.box));

      m_entry.append(entry);
   }
   
   Entry& entry = m_entry.ptrw()[index];

   switch (m_array_type)
   {
      case DBTable::VT_StringArray:
      {
         if (!entry.editor)
         {
            godot::LineEdit* editor = memnew(godot::LineEdit);
            apply_line_edit_style(editor);
            editor->set_h_size_flags(SIZE_EXPAND_FILL);
            entry.box->add_child(editor);
            entry.editor = editor;

            editor->connect("text_submitted", callable_mp(this, &DBTabularCellArray::on_txt_commited).bind(entry.box));
            editor->connect("focus_entered", callable_mp(this, &DBTabularCellArray::on_txt_focus).bind(editor));
            editor->connect("focus_exited", callable_mp(this, &DBTabularCellArray::on_txt_unfocus).bind(entry.box));
            editor->connect("gui_input", callable_mp(this, &DBTabularCellArray::on_txt_gui_input).bind(entry.box));
         }

         const godot::String strval = value;
         godot::Object::cast_to<godot::LineEdit>(entry.editor)->set_text(value);

      } break;

      case DBTable::VT_IntegerArray:
      case DBTable::VT_FloatArray:
      {
         const bool is_float = m_array_type == DBTable::VT_FloatArray;

         if (!entry.editor)
         {
            spin_t* editor = memnew(spin_t);

            editor->set_h_size_flags(SIZE_EXPAND_FILL);
            entry.box->add_child(editor);
            entry.editor = editor;

         #if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
            editor->begin_bulk_theme_override();
            editor->add_theme_stylebox_override("normal", get_style_empty());
            editor->add_theme_stylebox_override("focus", get_style_empty());
            editor->add_theme_color_override("font_color", get_font_color());
            editor->add_theme_color_override("font_selected_color", get_font_selected_color());
            editor->add_theme_color_override("selection_color", get_selection_color());
            editor->add_theme_color_override("caret_color", get_caret_color());
            editor->add_theme_font_override("font", get_font());
            editor->add_theme_font_size_override("font_size", get_font_size());
            editor->end_bulk_theme_override();
         #else
            apply_line_edit_style(editor->get_line_edit());
         #endif

            
            editor->get_line_edit()->connect("text_submitted", callable_mp(this, &DBTabularCellArray::on_txt_commited).bind(entry.box));
            editor->get_line_edit()->connect("focus_entered", callable_mp(this, &DBTabularCellArray::on_txt_focus).bind(editor->get_line_edit()));
            editor->get_line_edit()->connect("focus_exited", callable_mp(this, &DBTabularCellArray::on_txt_unfocus).bind(entry.box));
            editor->get_line_edit()->connect("gui_input", callable_mp(this, &DBTabularCellArray::on_txt_gui_input).bind(entry.box));
       
            editor->set_step(m_extra_settings.get("step", is_float ? 0.01f : 1.0f));
            const bool has_min_val = m_extra_settings.has("min_value");
            const bool has_max_val = m_extra_settings.has("max_value");
            const bool rounded = m_extra_settings.get("rounded", false);

         #if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
            editor->set_use_min_value(has_min_val);
            editor->set_use_max_value(has_max_val);
            if (has_min_val)
            {
               editor->set_min_value(m_extra_settings["min_value"]);
            }
            if (has_max_val)
            {
               editor->set_max_value(m_extra_settings["max_value"]);
            }

            editor->set_rounded_values(rounded);
         #else
            editor->set_allow_lesser(!has_min_val);
            editor->set_allow_greater(!has_max_val);
            if (has_min_val)
            {
               editor->set_min(m_extra_settings["min_value"]);
            }
            if (has_max_val)
            {
               editor->set_max(m_extra_settigns["max_value"]);
            }

            editor->set_use_rounded_values(rounded);
         #endif
         }

         const double numval = value;
         godot::Object::cast_to<spin_t>(entry.editor)->set_value(numval);
      } break;


      case DBTable::VT_TextureArray:
      {
         if (!entry.editor)
         {
            // In here things are a bit different. The "editor" will actually be a TextureRect. However its purpose
            // is just to show a texture. Yet, a button is required to request a file dialog
            godot::TextureRect* trect = memnew(godot::TextureRect);
            trect->set_expand_mode(godot::TextureRect::EXPAND_IGNORE_SIZE);
            trect->set_stretch_mode(godot::TextureRect::STRETCH_KEEP_ASPECT);
            trect->set_custom_minimum_size(godot::Vector2(32, 32));
            entry.box->add_child(trect);
            entry.editor = trect;

            // Add a dummy control to push both buttons to the end of the HBox
            godot::Control* spacer = memnew(godot::Control);
            spacer->set_h_size_flags(SIZE_EXPAND_FILL);
            entry.box->add_child(spacer);

            godot::Button* btload = memnew(godot::Button);
            apply_button_style(btload);
            btload->set_text("...");
            btload->set_clip_text(false);
            entry.box->add_child(btload);
            btload->connect("pressed", callable_mp(this, &DBTabularCellArray::on_load_clicked).bind(entry.box));
         }

         const godot::String path = value;
         godot::Ref<godot::Texture> tex;

         if (!path.is_empty() && godot::ResourceLoader::get_singleton()->exists(path))
         {
            tex = godot::ResourceLoader::get_singleton()->load(path);
         }

         if (!tex.is_valid())
         {
            tex = get_icon_no_texture();
         }

         godot::Object::cast_to<godot::TextureRect>(entry.editor)->set_texture(tex);
      } break;

      case DBTable::VT_AudioArray:
      {
         if (!entry.editor)
         {
            DBAudioPreview* editor = memnew(DBAudioPreview);
            godot::Ref<godot::Texture2D> icon_play = editor->get_icon_play();
            godot::Ref<godot::Texture2D> icon_pause = editor->get_icon_pause();
            godot::Ref<godot::Texture2D> icon_stop = editor->get_icon_stop();

            const int32_t size_play = godot::Math::max<int32_t>(icon_play->get_width(), icon_play->get_height());
            const int32_t size_pause = godot::Math::max<int32_t>(icon_pause->get_width(), icon_pause->get_height());
            const int32_t size_stop = godot::Math::max<int32_t>(icon_stop->get_width(), icon_stop->get_height());

            godot::Dictionary styles;
            styles["normal"] = get_style_empty();
            styles["pressed"] = get_style_empty();
            styles["hover"] = get_style_empty();
            styles["disabled"] = get_style_empty();
            styles["button_side"] = Internals::vmax<int32_t>(size_play, size_pause, size_stop);
            styles["separation"] = get_theme_separation();
            
            
            
            editor->setup(styles);
            editor->set_h_size_flags(SIZE_EXPAND_FILL);
            entry.box->add_child(editor);

            godot::Button* btload = memnew(godot::Button);
            apply_button_style(btload);
            btload->set_text("...");
            btload->set_clip_text(false);
            entry.box->add_child(btload);
            btload->connect("pressed", callable_mp(this, &DBTabularCellArray::on_load_clicked).bind(entry.box));

            entry.editor = editor;
         }

         godot::Ref<godot::AudioStream> stream;
         godot::String path = value;
         if (!path.is_empty() && godot::ResourceLoader::get_singleton()->exists(path))
         {
            stream = godot::ResourceLoader::get_singleton()->load(path);
         }
         
         godot::Object::cast_to<DBAudioPreview>(entry.editor)->set_stream(stream);
      } break;

      case DBTable::VT_GenericResArray:
      {
         if (!entry.editor)
         {
            godot::Button* editor = memnew(godot::Button);
            apply_button_style(editor);
            editor->set_h_size_flags(SIZE_EXPAND_FILL);
            entry.box->add_child(editor);
            editor->connect("pressed", callable_mp(this, &DBTabularCellArray::on_load_clicked).bind(entry.box));

            entry.editor = editor;
         }

         const godot::String strval = value;
         godot::Button* bt = godot::Object::cast_to<godot::Button>(entry.editor);
         bt->set_text(strval.get_file());
         bt->set_tooltip_text(strval);
      } break;

      case DBTable::VT_ColorArray:
      {
         if (!entry.editor)
         {
            godot::ColorPickerButton* editor = memnew(godot::ColorPickerButton);
            apply_button_style(editor);
            editor->set_h_size_flags(SIZE_EXPAND_FILL);
            entry.box->add_child(editor);
            entry.editor = editor;

            editor->connect("pressed", godot::create_custom_callable_function_pointer<TabularBoxCell>(this, &TabularBoxCell::notify_selected));
            editor->connect("color_changed", callable_mp(this, &DBTabularCellArray::on_color_changed).bind(entry.box));
         }

         const godot::Color color = value;
         godot::Object::cast_to<godot::ColorPickerButton>(entry.editor)->set_pick_color(color);
      } break;
   }


   if (must_create)
   {
      entry.bt_remove = memnew(godot::Button);
      entry.bt_remove->begin_bulk_theme_override();
      entry.bt_remove->add_theme_stylebox_override("normal", get_style_empty());
      entry.bt_remove->add_theme_stylebox_override("pressed", get_style_empty());
      entry.bt_remove->add_theme_stylebox_override("hover", get_style_empty());
      entry.bt_remove->end_bulk_theme_override();
      entry.bt_remove->set_button_icon(get_icon_trash());
      entry.bt_remove->set_h_size_flags(SIZE_SHRINK_END);
      entry.bt_remove->connect("pressed", callable_mp(this, &DBTabularCellArray::on_remove_clicked).bind(entry.box));
      entry.box->add_child(entry.bt_remove);
   }
}


bool DBTabularCellArray::is_valid_array(godot::Variant::Type type) const
{
   bool ret = false;
   switch (m_array_type)
   {
      case DBTable::VT_StringArray:
      case DBTable::VT_TextureArray:
      case DBTable::VT_AudioArray:
      case DBTable::VT_GenericResArray:
      {
         ret = type == godot::Variant::PACKED_STRING_ARRAY;
      } break;

      case DBTable::VT_IntegerArray:
      {
         ret = type == godot::Variant::PACKED_INT64_ARRAY;
      } break;

      case DBTable::VT_FloatArray:
      {
         ret = type == godot::Variant::PACKED_FLOAT64_ARRAY;
      } break;


      case DBTable::VT_ColorArray:
      {
         ret = type == godot::Variant::PACKED_COLOR_ARRAY;
      } break;
   }

   return ret;
}


void DBTabularCellArray::on_vbox_resized()
{
   const float vmargin = get_internal_margin_top() + get_internal_margin_bottom();
   const float btheight = get_button_height();

   set_min_height(m_vbox->get_size().y + vmargin + btheight + get_theme_separation());
}


void DBTabularCellArray::on_btadd_clicked()
{
   switch (m_array_type)
   {
      case DBTable::VT_StringArray:
      {
         godot::PackedStringArray arr = m_assigned;
         const int64_t nindex = arr.size();
         arr.append("");
         notify_value_changed(arr);
         m_entry[nindex].editor->grab_focus();
      } break;

      case DBTable::VT_IntegerArray:
      {
         godot::PackedInt64Array arr = m_assigned;
         const int64_t nindex = arr.size();
         arr.append(0);
         notify_value_changed(arr);
         godot::Object::cast_to<spin_t>(m_entry[nindex].editor)->get_line_edit()->grab_focus();
      } break;

      case DBTable::VT_FloatArray:
      {
         godot::PackedFloat64Array arr = m_assigned;
         const int64_t nindex = arr.size();
         arr.append(0.0);
         notify_value_changed(arr);
         godot::Object::cast_to<spin_t>(m_entry[nindex].editor)->get_line_edit()->grab_focus();
      } break;

      case DBTable::VT_TextureArray:
      case DBTable::VT_AudioArray:
      case DBTable::VT_GenericResArray:
      {
         // Arrays pointing to files are not meant to allow empty entries. So trigger the FileDialog to be shown
         on_load_clicked(nullptr);
      } break;

      case DBTable::VT_ColorArray:
      {
         godot::PackedColorArray arr = m_assigned;
         const int64_t nindex = arr.size();
         arr.append(godot::Color(0, 0, 0, 1));
         notify_value_changed(arr);
         notify_selected();
      } break;
   }
}


void DBTabularCellArray::on_remove_clicked(godot::HBoxContainer* entry_box)
{
   switch (m_array_type)
   {
      case DBTable::VT_StringArray:
      case DBTable::VT_TextureArray:
      case DBTable::VT_AudioArray:
      case DBTable::VT_GenericResArray:
      {
         godot::PackedStringArray arr = m_assigned;
         arr.remove_at(entry_box->get_index());
         notify_value_changed(arr);
      } break;

      case DBTable::VT_IntegerArray:
      {
         godot::PackedInt64Array arr = m_assigned;
         arr.remove_at(entry_box->get_index());
         notify_value_changed(arr);
      } break;

      case DBTable::VT_FloatArray:
      {
         godot::PackedFloat64Array arr = m_assigned;
         arr.remove_at(entry_box->get_index());
         notify_value_changed(arr);
      } break;


      case DBTable::VT_ColorArray:
      {
         godot::PackedColorArray arr = m_assigned;
         arr.remove_at(entry_box->get_index());
         notify_value_changed(arr);
      } break;
   }

   notify_selected();
}


void DBTabularCellArray::on_draw_hbox(godot::HBoxContainer* hbox)
{
   hbox->draw_style_box(get_style_background(), godot::Rect2(godot::Vector2(), hbox->get_size()));
}


void DBTabularCellArray::on_txt_focus(godot::LineEdit* editor)
{
   m_focused = editor;

   if (!get_selected())
   {
      notify_selected();
   }
}


void DBTabularCellArray::on_txt_unfocus(godot::HBoxContainer* entry_box)
{
   const int64_t eindex = entry_box->get_index();
   switch (m_array_type)
   {
      case DBTable::VT_StringArray:
      {
         godot::PackedStringArray arr = m_assigned;
         godot::LineEdit* editor = godot::Object::cast_to<godot::LineEdit>(m_entry[eindex].editor);

         if (editor->get_text() != arr[eindex])
         {
            arr[eindex] = editor->get_text();
            notify_value_changed(arr);
         }

         editor->select(0, 0);
      } break;

      case DBTable::VT_IntegerArray:
      {
         godot::PackedInt64Array arr = m_assigned;
         spin_t* editor = godot::Object::cast_to<spin_t>(m_entry[eindex].editor);

         const int64_t ivalue = editor->get_line_edit()->get_text().to_int();
         if (arr[eindex] != ivalue)
         {
            arr[eindex] = ivalue;
            notify_value_changed(arr);
         }
      } break;

      case DBTable::VT_FloatArray:
      {
         godot::PackedFloat64Array arr = m_assigned;
         spin_t* editor = godot::Object::cast_to<spin_t>(m_entry[eindex].editor);

         const double dvalue = editor->get_line_edit()->get_text().to_float();
         if (arr[eindex] != dvalue)
         {
            arr[eindex] = dvalue;
            notify_value_changed(arr);
         }
      } break;
   }

   m_focused = nullptr;
}


void DBTabularCellArray::on_txt_commited(const godot::String& new_text, godot::HBoxContainer* entry_box)
{
   const int64_t eindex = entry_box->get_index();
   switch (m_array_type)
   {
      case DBTable::VT_StringArray:
      {
         godot::PackedStringArray arr = m_assigned;

         if (new_text != arr[eindex])
         {
            arr[eindex] = new_text;
            notify_value_changed(arr);
         }

         if (eindex + 1 == m_entry.size())
         {
            request_select_next();
         }
         else
         {
            m_entry[eindex + 1].editor->grab_focus();
         }
      } break;

      case DBTable::VT_IntegerArray:
      {
         godot::PackedInt64Array arr = m_assigned;
         spin_t* editor = godot::Object::cast_to<spin_t>(m_entry[eindex].editor);
         const int64_t val = new_text.to_int();

         if (val != arr[eindex])
         {
            arr[eindex] = val;
            notify_value_changed(arr);
         }

         if (eindex + 1 == m_entry.size())
         {
            request_select_next();
         }
         else
         {
            godot::Object::cast_to<spin_t>(m_entry[eindex + 1].editor)->get_line_edit()->grab_focus();
         }
      } break;

      case DBTable::VT_FloatArray:
      {
         godot::PackedFloat64Array arr = m_assigned;
         spin_t* editor = godot::Object::cast_to<spin_t>(m_entry[eindex].editor);
         const double val = new_text.to_float();

         if (val != arr[eindex])
         {
            arr[eindex] = val;
            notify_value_changed(arr);
         }

         if (eindex + 1 == m_entry.size())
         {
            request_select_next();
         }
         else
         {
            godot::Object::cast_to<spin_t>(m_entry[eindex + 1].editor)->get_line_edit()->grab_focus();
         }
      } break;
   }
}


void DBTabularCellArray::on_txt_gui_input(const godot::Ref<godot::InputEvent>& event, godot::HBoxContainer* entry_box)
{
   godot::Ref<godot::InputEventKey> key = event;
   if (key.is_valid() && key->is_pressed() && key->is_action("ui_cancel"))
   {
      switch (m_array_type)
      {
         case DBTable::VT_StringArray:
         {
            const godot::PackedStringArray arr = m_assigned;
            const int64_t eindex = entry_box->get_index();
            godot::LineEdit* editor = godot::Object::cast_to<godot::LineEdit>(m_entry[eindex].editor);

            editor->set_text(arr[eindex]);
            editor->release_focus();
         } break;

         case DBTable::VT_IntegerArray:
         {
            const godot::PackedInt64Array arr = m_assigned;
            const int64_t eindex = entry_box->get_index();
            godot::LineEdit* editor = godot::Object::cast_to<spin_t>(m_entry[eindex].editor)->get_line_edit();

            editor->set_text(godot::vformat("%s", arr[eindex]));
            editor->release_focus();
         } break;

         case DBTable::VT_FloatArray:
         {
            const godot::PackedFloat64Array arr = m_assigned;
            const int64_t eindex = entry_box->get_index();
            godot::LineEdit* editor = godot::Object::cast_to<spin_t>(m_entry[eindex].editor)->get_line_edit();

            editor->set_text(godot::vformat("%s", arr[eindex]));
            editor->release_focus();
         } break;
      }
   }
}


void DBTabularCellArray::on_load_clicked(godot::HBoxContainer* entry_box)
{
   notify_selected();

   switch (m_array_type)
   {
      case DBTable::VT_TextureArray:
      {
         godot::PackedStringArray filters;
         filters.append("*.atlastex; ATLASTEX");
         filters.append("*.bmp; BMP");
         filters.append("*.curvetex; CURVETEX");
         filters.append("*.dds; DDS");   
         filters.append("*.exr; EXR");
         filters.append("*.hdr; HDR");
         filters.append("*.jpeg; JPEG");
         filters.append("*.jpg; JPG");
         filters.append("*.largetex; LARGETEX");
         filters.append("*.meshtex; MESHTEX");
         filters.append("*.pkm; PKM");
         filters.append("*.png; PNG");
         filters.append("*.pvr; PVR");
         filters.append("*.res; RES");
         filters.append("*.svg; SVG");
         filters.append("*.svgz; SVGZ");
         filters.append("*.tex; TEX");
         filters.append("*.tga; TGA");
         filters.append("*.tres; TRES");
         filters.append("*.webp; WEBP");

         request_file_dialog("Open texture", filters, callable_mp(this, &DBTabularCellArray::file_selected).bind(entry_box));
      } break;

      case DBTable::VT_AudioArray:
      {
         godot::PackedStringArray filter;
         filter.append("*.ogg; OGG");
         filter.append("*.oggstr; OGGSTR");
         filter.append("*.res; RES");
         filter.append("*.sample; SAMPLE");
         filter.append("*.tres; TRES");
         filter.append("*.wav; WAV");

         request_file_dialog("Load audio", filter, callable_mp(this, &DBTabularCellArray::file_selected).bind(entry_box));
      } break;

      case DBTable::VT_GenericResArray:
      {
         request_file_dialog("Load resource", godot::PackedStringArray(), callable_mp(this, &DBTabularCellArray::file_selected).bind(entry_box));
      } break;
   }
}


void DBTabularCellArray::on_color_changed(const godot::Color& color, godot::HBoxContainer* entry_box)
{
   const int64_t eindex = entry_box->get_index();
   godot::PackedColorArray arr = m_assigned;
   if (color != arr[eindex])
   {
      arr[eindex] = color;
      notify_value_changed(arr);
   }
}


void DBTabularCellArray::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_btadd->connect("pressed", callable_mp(this, &DBTabularCellArray::on_btadd_clicked));

         m_vbox->connect("resized", callable_mp(this, &DBTabularCellArray::on_vbox_resized));
      } break;
   }
}

/*
      VT_TextureArray,
      VT_AudioArray,
      VT_GenericResArray,
*/
bool DBTabularCellArray::_can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const
{
   // First check the array type for early bailing
   switch (m_array_type)
   {
      case DBTable::VT_StringArray:
      case DBTable::VT_IntegerArray:
      case DBTable::VT_ColorArray:
      {
         return false;
      }
   }

   if (data.get_type() != godot::Variant::DICTIONARY) { return false; }

   const godot::Dictionary ddata = data;
   const godot::String type = ddata.get("type", "");
   if (type != "files") { return false; }

   const godot::Array files = ddata.get("files", godot::Array());
   const int64_t fcount = files.size();

   // Assume there isn't any valid type being dragged
   bool has_valid_file = false;

   for (int64_t i = 0; i < fcount; i++)
   {
      /// TODO: Check if there is a better way to verify the file type without loading it first
      switch (m_array_type)
      {
         case DBTable::VT_TextureArray:
         {
            godot::Ref<godot::Texture2D> tex = godot::ResourceLoader::get_singleton()->load(files[i]);
            has_valid_file = has_valid_file | tex.is_valid();
         } break;

         case DBTable::VT_AudioArray:
         {
            godot::Ref<godot::AudioStream> stream = godot::ResourceLoader::get_singleton()->load(files[i]);
            has_valid_file = has_valid_file | stream.is_valid();
         } break;
         
         case DBTable::VT_GenericResArray:
         {
            has_valid_file = true;
         } break;
      }
   }

   return has_valid_file;
}


void DBTabularCellArray::_drop_data(const godot::Vector2& pos, const godot::Variant& data)
{
   godot::PackedStringArray arr = m_assigned;

   const godot::Dictionary ddata = data;
   const godot::Array files = ddata.get("files", godot::Array());
   const int64_t fcount = files.size();
   for (int64_t i = 0; i < fcount; i++)
   {
      switch (m_array_type)
      {
         case DBTable::VT_TextureArray:
         {
            godot::Ref<godot::Texture2D> tex = godot::ResourceLoader::get_singleton()->load(files[i]);
            if (tex.is_valid())
            {
               arr.append(files[i]);
            }
         } break;

         case DBTable::VT_AudioArray:
         {
            godot::Ref<godot::AudioStream> stream = godot::ResourceLoader::get_singleton()->load(files[i]);
            if (stream.is_valid())
            {
               arr.append(files[i]);
            }
         } break;

         case DBTable::VT_GenericResArray:
         {
            arr.append(files[i]);
         } break;
      }
   }

   notify_value_changed(arr);
}


void DBTabularCellArray::check_theme()
{
   godot::Ref<godot::Font> font = get_font();
   const int32_t fsize = get_font_size();
   const int32_t mwidth = font->get_string_size("+++", godot::HORIZONTAL_ALIGNMENT_CENTER, -1, fsize).x;

   apply_button_style(m_btadd);
   m_btadd->set_clip_text(false);
   m_btadd->set_custom_minimum_size(godot::Vector2(mwidth + 4, 0));

   ExtPackUtils::set_control_offsets(m_mainbox, get_internal_margin_left(), get_internal_margin_top(), -get_internal_margin_right(), -get_internal_margin_bottom());
}


void DBTabularCellArray::assign_value(const godot::Variant& value)
{
   ERR_FAIL_COND_MSG(!is_valid_array(value.get_type()), "Assigning array type, but incoming value isn't the proper array type, or not even an array!");

   const godot::Array varr = value;
   const int64_t vcount = varr.size();

   m_assigned = value;

   while (m_entry.size() > vcount)
   {
      m_entry[0].box->queue_free();
      m_entry.remove_at(0);
   }

   for (int64_t i = 0; i < vcount; i++)
   {
      set_entry(varr[i], i);
   }
}


void DBTabularCellArray::selected_changed(bool selected)
{
   if (selected)
   {
      if (!m_focused && m_entry.size() > 0)
      {
         switch (m_array_type)
         {
            case DBTable::VT_StringArray:
            {
               m_entry[0].editor->grab_focus();
            } break;

            case DBTable::VT_IntegerArray:
            case DBTable::VT_FloatArray:
            {
               spin_t* editor = godot::Object::cast_to<spin_t>(m_entry[0].editor);
               editor->get_line_edit()->grab_focus();
            } break;
         }
      }
   }
   else if (!selected)
   {
      if (m_focused && m_focused->has_focus())
      {
         m_focused->release_focus();
      }
      m_focused = nullptr;
   }
}


void DBTabularCellArray::apply_extra_settings(const godot::Dictionary& extra_settings)
{
   m_extra_settings = extra_settings;
}


bool DBTabularCellArray::requires_file_dialog() const
{
   bool ret = false;

   switch (m_array_type)
   {
      case DBTable::VT_TextureArray:
      case DBTable::VT_AudioArray:
      case DBTable::VT_GenericResArray:
      {
         ret = true;
      } break;
   }

   return ret;
}


void DBTabularCellArray::file_selected(const godot::String& path, godot::HBoxContainer* entry_box)
{
   // All array types that require a file dialog will store values as string arrays.
   godot::PackedStringArray arr = m_assigned;

   if (entry_box)
   {
      const int32_t eindex = entry_box->get_index();
      arr[eindex] = path;
   }
   else
   {
      // If here then file dialog was triggered by the "append button"
      arr.append(path);
   }

   notify_value_changed(arr);
}


DBTabularCellArray::DBTabularCellArray(DBTable::ValueType array_type)
{
   m_array_type = array_type;
   m_focused = nullptr;

   m_mainbox = memnew(godot::VBoxContainer);
   ExtPackUtils::set_control_anchors(m_mainbox, 0, 0, 1, 1);
   add_child(m_mainbox);

   m_vbox = memnew(godot::VBoxContainer);
   m_vbox->set_h_size_flags(SIZE_EXPAND_FILL);
   m_vbox->set_v_size_flags(SIZE_SHRINK_BEGIN);
   m_mainbox->add_child(m_vbox);

   m_btadd = memnew(godot::Button);
   m_btadd->set_text("+");
   m_btadd->set_h_size_flags(SIZE_SHRINK_CENTER);
   m_mainbox->add_child(m_btadd);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// DBTabularCellLockedRowID
void DBTabularCellLockedRowID::_notification(int what)
{
   if (what == NOTIFICATION_POSTINITIALIZE)
   {
      m_txt->connect("focus_entered", godot::create_custom_callable_function_pointer<TabularBoxCell>(this, &TabularBoxCell::notify_selected));
   }
}


void DBTabularCellLockedRowID::check_theme()
{
   godot::Ref<godot::Font> font = get_font();
   const int32_t font_size = get_font_size();
   const double mtop = get_internal_margin_top();
   const double mbottom = get_internal_margin_bottom();

   m_txt->begin_bulk_theme_override();
   m_txt->add_theme_stylebox_override("normal", get_style_empty());
   m_txt->add_theme_stylebox_override("focus", get_style_empty());
   m_txt->add_theme_stylebox_override("read_only", get_style_empty());
   m_txt->add_theme_font_override("font", font);
   m_txt->add_theme_font_size_override("font_size", font_size);
   m_txt->add_theme_color_override("font_color", get_font_color());
   m_txt->add_theme_color_override("font_selected_color", get_font_selected_color());
   m_txt->add_theme_color_override("selection_color", get_selection_color());
   m_txt->add_theme_color_override("caret", get_caret_color());
   m_txt->end_bulk_theme_override();

   m_txt->set_anchor_and_offset(godot::SIDE_LEFT, 0, get_internal_margin_left());
   m_txt->set_anchor_and_offset(godot::SIDE_TOP, 0, mtop);
   m_txt->set_anchor_and_offset(godot::SIDE_RIGHT, 1, -get_internal_margin_right());
   m_txt->set_anchor_and_offset(godot::SIDE_BOTTOM, 1, -mbottom);

   const double fheight = font->get_height(font_size);
   set_min_height(mtop + fheight + mbottom);
}


void DBTabularCellLockedRowID::assign_value(const godot::Variant& value)
{
   m_txt->set_text(value);
}


void DBTabularCellLockedRowID::selected_changed(bool selected)
{
   if (selected && !m_txt->has_focus())
   {
      m_txt->grab_focus();
   }
   else if (!selected && m_txt->has_focus())
   {
      m_txt->select(0, 0);
      m_txt->release_focus();
   }
}


DBTabularCellLockedRowID::DBTabularCellLockedRowID()
{
   m_txt = memnew(godot::LineEdit);
   m_txt->set_editable(false);
   m_txt->set_context_menu_enabled(false);
   m_txt->set_mouse_filter(MOUSE_FILTER_PASS);
   m_txt->add_theme_constant_override("minimum_character_width", 1);
   m_txt->set_caret_blink_enabled(true);
   m_txt->set_clip_contents(true);
   m_txt->set_select_all_on_focus(true);
   add_child(m_txt, false, INTERNAL_MODE_BACK);
}

#endif  //EDITOR_DISABLED
#endif  //DATABASE_DISABLED
