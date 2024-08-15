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

#include "accordion.h"

#ifndef ACCORDION_DISABLED

#include "../custom_theme.h"
#include "../../internal.h"

#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/h_scroll_bar.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/input_event_mouse_button.hpp>
#include <godot_cpp/classes/input_event_mouse_motion.hpp>
#include <godot_cpp/classes/input_event_pan_gesture.hpp>
#include <godot_cpp/classes/style_box.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/v_scroll_bar.hpp>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// AccordionItem
float AccordionItem::AnimState::calculate(const godot::Ref<godot::Curve>& curve)
{
   float alpha = target_time > 0.0 ? (time / target_time) : 1.0;

   if (curve.is_valid())
   {
      alpha = curve->sample(alpha);
   }

   current_height = godot::Math::lerp(start_height, end_height, alpha);

   return current_height;
}



void AccordionItem::sort_children()
{
   const godot::Size2i ctrl_size = get_size();
   const int ccount = get_child_count();
   m_min_bt_size = m_button->get_combined_minimum_size();
   const int bth = m_min_bt_size.height;

   fit_child_in_rect(m_button, godot::Rect2(godot::Vector2(), godot::Vector2(ctrl_size.x, bth)));

   const int side_margins = m_content_margin.left + m_content_margin.right;
   int min_w = side_margins;
   const int cwidth = ctrl_size.width - side_margins;
   int cy = bth + m_content_margin.top;
   for (int i = 0; i < ccount; i++)
   {
      godot::Control* ctrl = godot::Object::cast_to<godot::Control>(get_child(i));
      if (!ctrl || !ctrl->is_visible() || ctrl->is_set_as_top_level())
      {
         continue;
      }

      const godot::Vector2 minsz = ctrl->get_combined_minimum_size();
      fit_child_in_rect(ctrl, godot::Rect2(godot::Vector2(m_content_margin.left, cy), godot::Vector2(cwidth, minsz.y)));
      cy += minsz.y + m_separation;

      min_w = godot::Math::max<float>(min_w, minsz.x + side_margins);
   }

   m_min_size = godot::Size2i(min_w, cy + m_content_margin.bottom);

   update_minimum_size();
}


void AccordionItem::start_animation(bool active)
{
   // Assume no animation should actually start
   bool start = false;

   if (active && m_show_anim_time > 0)
   {
      // Show animation is enabled and item is now activated.
      m_animation.target_time = m_show_anim_time;
      m_animation.start_height = m_min_bt_size.height;
      m_animation.end_height = m_min_size.height;

      if (is_animating())
      {
         // However hiding animation was already in progress. Artificially set elapsed time
         // based on progress, ignoring curve as it makes things rather unpredictable
         const float p = (1.0 - m_animation.time / m_hide_anim_time);
         m_animation.time = p * m_show_anim_time;

         // NOTE: Should hide_animation_ended signal be given here? It was interrupted.
      }
      else
      {
         // Since no animation was in progress, can start from 0
         m_animation.time = 0;
         m_animation.current_height = m_animation.start_height;
      }

      emit_signal("show_animation_started");

      start = true;
   }
   else if (!active && m_hide_anim_time > 0)
   {
      // Hide animation is enabled and item is now deactivated
      m_animation.target_time = m_hide_anim_time;
      m_animation.start_height = m_min_size.height;
      m_animation.end_height = m_min_bt_size.height;

      if (is_animating())
      {
         // However hide animation was already in progress. Artificially set elapsed time
         // based on progress, ignoring curve as it makes things rather unpredictable
         const float p = (1.0 - m_animation.time / m_show_anim_time);
         m_animation.time = p * m_hide_anim_time;
      }
      else
      {
         // Since no animation was in progress, can start from 0
         m_animation.time = 0;
         m_animation.current_height = m_animation.start_height;

         // NOTE: Should show_animation_ended signal be given here? It as interrupted.
      }

      emit_signal("hide_animation_started");

      start = true;
   }

   if (start)
   {
      if (m_animate_on_physics)
      {
         set_physics_process_internal(true);
      }
      else
      {
         set_process_internal(true);
      }
   }
}


void AccordionItem::handle_animation(float dt)
{
   if (m_animation.update(dt))
   {
      // Animation finished
      set_physics_process_internal(false);
      set_process_internal(false);

      if (m_animation.is_hiding())
      {
         emit_signal("hide_animation_ended");
      }
      else
      {
         emit_signal("show_animation_ended");
      }
   }
   else
   {
      godot::Ref<godot::Curve> curve = m_animation.is_hiding() ? m_hide_curve : m_show_curve;
      // This updates the "current" height
      m_animation.calculate(curve);
   }

   // With this, parent Accordion will take care of properly resizing this Item   
   update_minimum_size();
}


void AccordionItem::on_button_toggled(bool on)
{
   if (m_anim_block && is_animating())
   {
      // Animation is in progress yet it's desired to block state change. So revert button's pressed state and bail
      m_button->set_pressed_no_signal(!on);
      return;
   }

   set_active(on);
   emit_signal("active_changed", on);
}



void AccordionItem::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         m_button->connect("toggled", callable_mp(this, &AccordionItem::on_button_toggled));
      } break;

      case NOTIFICATION_PATH_RENAMED:
      case NOTIFICATION_ENTER_TREE:
      {
         if (m_title.is_empty())
         {
            m_button->set_text(get_name());
         }
         else
         {
            m_button->set_text(m_title);
         }
      } break;

      case NOTIFICATION_READY:
      {
         set_process(false);
         set_process_internal(false);
         set_physics_process(false);
         set_physics_process_internal(false);
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         handle_animation(get_process_delta_time());
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         handle_animation(get_physics_process_delta_time());
      } break;

      case NOTIFICATION_SORT_CHILDREN:
      {
         sort_children();
      } break;

      case NOTIFICATION_DRAW:
      {
         if (m_background.is_valid())
         {
            draw_style_box(m_background, godot::Rect2(godot::Vector2(), godot::Vector2(get_size().x, m_min_size.y)));
         }
      } break;
   }
}

void AccordionItem::_bind_methods()
{
   using namespace godot;

   /// Exposed functions
   ClassDB::bind_method(D_METHOD("set_title", "value"), &AccordionItem::set_title);
   ClassDB::bind_method(D_METHOD("get_title"), &AccordionItem::get_title);
   ClassDB::bind_method(D_METHOD("set_icon", "icon"), &AccordionItem::set_icon);
   ClassDB::bind_method(D_METHOD("get_icon"), &AccordionItem::get_icon);
   ClassDB::bind_method(D_METHOD("set_active", "value"), &AccordionItem::set_active);
   ClassDB::bind_method(D_METHOD("get_active"), &AccordionItem::get_active);
   ClassDB::bind_method(D_METHOD("set_button_tooltip", "value"), &AccordionItem::set_button_tooltip);
   ClassDB::bind_method(D_METHOD("get_button_tooltip"), &AccordionItem::get_button_tooltip);
   ClassDB::bind_method(D_METHOD("set_enabled", "val"), &AccordionItem::set_enabled);
   ClassDB::bind_method(D_METHOD("get_enabled"), &AccordionItem::get_enabled);

   /// Exposed variables
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");
   ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_icon", "get_icon");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "active"), "set_active", "get_active");
   ADD_PROPERTY(PropertyInfo(Variant::STRING, "button_tooltip"), "set_button_tooltip", "get_button_tooltip");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "get_enabled");

   /// Signals
   ADD_SIGNAL(MethodInfo("active_changed", PropertyInfo(Variant::BOOL, "active")));
   ADD_SIGNAL(MethodInfo("hide_animation_started"));
   ADD_SIGNAL(MethodInfo("hide_animation_ended"));
   ADD_SIGNAL(MethodInfo("show_animation_started"));
   ADD_SIGNAL(MethodInfo("show_animation_ended"));
}


godot::Vector2 AccordionItem::_get_minimum_size() const
{
   if (is_physics_processing_internal() || is_processing_internal())
   {
      // If here the animation is in progress
      return godot::Vector2(m_min_size.width, m_animation.current_height);
   }

   // If here no animation is in progress
   return m_button->is_pressed() ? m_min_size : m_min_bt_size;
}


godot::PackedStringArray AccordionItem::_get_configuration_warnings() const
{
   godot::PackedStringArray ret;

   Accordion* acc = godot::Object::cast_to<Accordion>(get_parent());
   if (!acc)
   {
      ret.append("AccordionItem is designed to be a direct child of Accordion.");
   }

   return ret;
}


void AccordionItem::set_title(const godot::String& val)
{
   m_title = val;

   if (m_title.is_empty())
   {
      m_button->set_text(get_name());
   }
   else
   {
      m_button->set_text(val);
   }
}

void AccordionItem::set_icon(const godot::Ref<godot::Texture2D>& icon)
{
   m_button->set_button_icon(icon);
}

godot::Ref<godot::Texture2D> AccordionItem::get_icon() const
{
   return m_button->get_button_icon();
}


bool AccordionItem::get_active() const
{
   return m_button->is_pressed();
}


void AccordionItem::set_button_tooltip(const godot::String& text)
{
   m_button->set_tooltip_text(text);
}

godot::String AccordionItem::get_button_tooltip() const
{
   return m_button->get_tooltip_text();
}


void AccordionItem::set_enabled(bool val)
{
   m_button->set_disabled(!val);
}

bool AccordionItem::get_enabled() const
{
   return !m_button->is_disabled();
}


godot::Button* AccordionItem::get_button() const
{
   return m_button;
}


void AccordionItem::change_active_state(bool active, bool notify)
{
   if (m_anim_block && is_animating())
   {
      return;
   }

   start_animation(active);
   m_button->set_pressed(active);
   queue_sort();

   if (notify)
   {
      // Please check explanation of the "on_item_toggled()" function in the Accordion class, header file.
      Accordion* parent = godot::Object::cast_to<Accordion>(get_parent());
      if (parent)
      {
         parent->on_item_toggled(this);
      }
   }
}


void AccordionItem::assign_separation(int val)
{
   m_separation = val;
   queue_sort();
}


void AccordionItem::setup_animation(float hide_time, const godot::Ref<godot::Curve> hide_curve, float show_time, const godot::Ref<godot::Curve>& show_curve, bool anim_on_physics)
{
   m_hide_anim_time = hide_time;
   m_hide_curve = hide_curve;
   m_show_anim_time = show_time;
   m_show_curve = show_curve;
   m_animate_on_physics = anim_on_physics;
}

void AccordionItem::setup_alignment(godot::HorizontalAlignment text, godot::HorizontalAlignment icon)
{
   m_button->set_text_alignment(text);
   m_button->set_icon_alignment(icon);
}

void AccordionItem::setup_background(const godot::Ref<godot::StyleBox>& background)
{
   m_background = background;
   if (m_background.is_valid())
   {
      m_content_margin.left = m_background->get_margin(godot::Side::SIDE_LEFT);
      m_content_margin.top = m_background->get_margin(godot::Side::SIDE_TOP);
      m_content_margin.right = m_background->get_margin(godot::Side::SIDE_RIGHT);
      m_content_margin.bottom = m_background->get_margin(godot::Side::SIDE_BOTTOM);
   }
   else
   {
      m_content_margin.left = 6;
      m_content_margin.top = 0;
      m_content_margin.right = 6;
      m_content_margin.bottom = 0;
   }

   queue_sort();
   queue_redraw();
}


AccordionItem::AccordionItem()
{
   set_clip_contents(true);

   m_separation = 4;
   m_anim_block = false;

   m_button = memnew(godot::Button);
   add_child(m_button, false, INTERNAL_MODE_BACK);
   m_button->set_name("_toggle_item_");
   m_button->set_toggle_mode(true);
   m_button->set_pressed(false);

   set_mouse_filter(MOUSE_FILTER_IGNORE);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Accordion
void Accordion::apply_style_to_button(godot::Button* bt)
{
   bt->begin_bulk_theme_override();
   bt->add_theme_stylebox_override("normal", m_theme_cache.button_normal);
   bt->add_theme_stylebox_override("hover", m_theme_cache.button_hovered);
   bt->add_theme_stylebox_override("pressed", m_theme_cache.button_pressed);
   bt->add_theme_stylebox_override("disabled", m_theme_cache.button_disabled);
   bt->add_theme_stylebox_override("focus", m_theme_cache.button_focus);
   bt->add_theme_font_override("font", m_theme_cache.font);
   bt->add_theme_color_override("font_color", m_theme_cache.font_normal);
   bt->add_theme_color_override("font_hover_color", m_theme_cache.font_hovered);
   bt->add_theme_color_override("font_focus_color", m_theme_cache.font_focus);
   bt->add_theme_color_override("font_pressed_color", m_theme_cache.font_pressed);
   bt->add_theme_color_override("font_hover_pressed_color", m_theme_cache.font_hover_pressed);
   bt->add_theme_color_override("font_disabled_color", m_theme_cache.font_disabled);
   bt->add_theme_color_override("font_outline_color", m_theme_cache.font_outline);
   bt->add_theme_color_override("icon_normal_color", m_theme_cache.icon_normal);
   bt->add_theme_color_override("icon_pressed_color", m_theme_cache.icon_pressed);
   bt->add_theme_color_override("icon_hover_color", m_theme_cache.icon_hover);
   bt->add_theme_color_override("icon_hover_pressed_color", m_theme_cache.icon_hover_pressed);
   bt->add_theme_color_override("icon_focus_color", m_theme_cache.icon_focus);
   bt->add_theme_color_override("icon_disabled_color", m_theme_cache.icon_disabled);
   bt->add_theme_font_size_override("font_size", m_theme_cache.font_size);
   bt->add_theme_constant_override("outline_size", m_theme_cache.font_outline_size);
   bt->end_bulk_theme_override();

   //bt->set_text_direction(TextDirection::TEXT_DIRECTION_AUTO)
}

void Accordion::apply_internal_styles()
{
   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      apply_style_to_button(m_item[i]->get_button());

      m_item[i]->assign_separation(m_theme_cache.inter_space);
      m_item[i]->setup_background(m_theme_cache.content_background);
   }
}

void Accordion::apply_alignment()
{
   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      m_item[i]->setup_alignment(m_text_align, m_icon_align);
   }
}

void Accordion::apply_anim_setup()
{
   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      m_item[i]->setup_animation(m_hide_anim_time, m_hide_anim_curve, m_show_anim_time, m_show_anim_curve, m_animate_on_physics);
   }
}



void Accordion::refresh_contents()
{
   // The idea of this function is basically to simply cache the valid children. In other words, cache
   // which children are AccordionItem instances. Item data will be saved within that node
   // It's way easier to simply rebuild the entire thing instead of attempting to verify data while
   // adding/removing entries
   m_item.clear();

   const int ccount = get_child_count();
   for (int i = 0; i < ccount; i++)
   {
      AccordionItem* item = godot::Object::cast_to<AccordionItem>(get_child(i));
      if (item && !item->is_set_as_top_level())
      {
         m_item.append(item);

         // Just ensure the styling is applied, even though it might already be
         apply_style_to_button(item->get_button());

         item->assign_separation(m_theme_cache.inter_space);
         item->setup_animation(m_hide_anim_time, m_hide_anim_curve, m_show_anim_time, m_show_anim_curve, m_animate_on_physics);
         item->setup_alignment(m_text_align, m_icon_align);
         item->setup_background(m_theme_cache.content_background);
         item->set_anim_block(m_anim_block);

         item->get_button()->set_text_direction(m_text_direction);
         item->get_button()->set_language(m_language);
      }
   }

   notify_property_list_changed();
}



void Accordion::calculate_req_size()
{
   const godot::Vector2 bgsize = m_theme_cache.background->get_minimum_size();
   const godot::Vector2 size = get_size();
   m_required_size = godot::Vector2(0, 0);

   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      if (i > 0)
      {
         m_required_size.y += m_theme_cache.button_space;
      }

      const godot::Size2i iminsz = m_item[i]->get_minimum_size();

      //m_required_size.y += m_item[i]->get_minimum_size().y;
      m_required_size.y += iminsz.height;

      //m_required_size.x = godot::Math::max<float>(m_required_size.x, m_item[i]->get_combined_minimum_size().x);
      m_required_size.x = godot::Math::max<float>(m_required_size.x, iminsz.width);
   }

   m_hscroll->set_visible(m_always_show_hscroll || m_required_size.x > size.x);
   m_vscroll->set_visible(m_always_show_vscroll || m_required_size.y > size.y);

   m_hscroll->set_max(m_required_size.x + bgsize.x);
   m_vscroll->set_max(m_required_size.y + bgsize.y);
}


void Accordion::check_layout()
{
   const int mleft = m_theme_cache.background->get_margin(godot::Side::SIDE_LEFT);
   const int mright = m_theme_cache.background->get_margin(godot::Side::SIDE_RIGHT);
   const int mtop = m_theme_cache.background->get_margin(godot::Side::SIDE_TOP);
   const int mbottom = m_theme_cache.background->get_margin(godot::Side::SIDE_BOTTOM);

   const int vwidth = m_vscroll->get_combined_minimum_size().x;
   const int hheight = m_hscroll->get_combined_minimum_size().y;

   const godot::Size2i size = get_size();
   const int width = godot::Math::max<int>((m_vscroll->is_visible() ? size.width - vwidth : size.width) - (mleft + mright), m_required_size.x);

   const int cx = mleft - m_hscroll->get_value();

   int cy = mtop - m_vscroll->get_value();
   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      const godot::Vector2 rsize = m_item[i]->get_minimum_size();

      fit_child_in_rect(m_item[i], godot::Rect2(godot::Vector2(cx, cy), godot::Vector2(width, rsize.y)));

      cy += rsize.y + m_theme_cache.button_space;
   }

   
   m_hscroll->set_page(m_vscroll->is_visible() ? size.width - vwidth : size.width);
   m_hscroll->set_offset(godot::Side::SIDE_RIGHT, m_vscroll->is_visible() ? -vwidth : 0);
   
   m_vscroll->set_page(m_hscroll->is_visible() ? size.height - hheight : size.height);
   m_vscroll->set_offset(godot::Side::SIDE_BOTTOM, m_hscroll->is_visible() ? -hheight : 0);
}


void Accordion::cancel_drag()
{
   set_physics_process_internal(false);
   m_drag_touch.drag_touching_deaccel = false;
   m_drag_touch.drag_touching = false;
   m_drag_touch.drag_speed = godot::Vector2();
   m_drag_touch.drag_accum = godot::Vector2();
   m_drag_touch.last_drag_accum = godot::Vector2();
   m_drag_touch.drag_from = godot::Vector2();

   if (m_drag_touch.beyond_deadzone)
   {
      emit_signal("scroll_ended");
      m_drag_touch.beyond_deadzone = false;
   }
}


void Accordion::handle_mouse_button(const godot::Ref<godot::InputEventMouseButton>& mb)
{
   const float vval = m_vscroll->get_value();
   const float hval = m_hscroll->get_value();

   if (mb->is_pressed())
   {
      const int btindex = mb->get_button_index();
      const bool wheel_u = btindex == godot::MouseButton::MOUSE_BUTTON_WHEEL_UP;
      const bool wheel_d = btindex == godot::MouseButton::MOUSE_BUTTON_WHEEL_DOWN;
      const bool wheel_l = btindex == godot::MouseButton::MOUSE_BUTTON_WHEEL_LEFT;
      const bool wheel_r = btindex == godot::MouseButton::MOUSE_BUTTON_WHEEL_RIGHT;

      // An attempt to scroll vertically requires: wheel up/down without shift pressed or wheel left/right with shift pressed,
      // while also having vscroll visible. To make the "if" easier to read, build part of the test within this
      const bool try_vert = ((wheel_u || wheel_d) && !mb->is_shift_pressed()) || ((wheel_l || wheel_r) && mb->is_shift_pressed());
      // An attempt to scroll horizontally requires: wheel left/right without shift pressed or wheel up/down with shift pressed,
      // while also having hscroll visible. To make the "if" easier to read, build part of the test within this
      const bool try_horz = ((wheel_l || wheel_r) && !mb->is_shift_pressed()) || ((wheel_u || wheel_d) && mb->is_shift_pressed());

      // Assume not scrolling
      bool scrolled = false;

      if (try_vert && m_vscroll->is_visible())
      {
         const float page = m_vscroll->get_page();
         const float factor = mb->get_factor();
         const float delta = (wheel_u || wheel_l) ? -page / 8 * factor : page / 8 * factor;

         m_vscroll->set_value(vval + delta);
         scrolled = m_vscroll->get_value() != vval;
      }
      
      if (try_horz && m_hscroll->is_visible())
      {
         const float page = m_hscroll->get_page();
         const float factor = mb->get_factor();
         const float delta = (wheel_u || wheel_l) ? -page / 8 * factor : page / 8 * factor;

         m_hscroll->set_value(hval + delta);
         scrolled = m_hscroll->get_value() != hval;
      }


      if (scrolled)
      {
         accept_event();
         return;
      }
   }

   const bool is_touch_available = godot::DisplayServer::get_singleton()->is_touchscreen_available();
   if (!is_touch_available) { return; }

   if (mb->get_button_index() != godot::MouseButton::MOUSE_BUTTON_LEFT) { return; }

   if (mb->is_pressed())
   {
      if (m_drag_touch.drag_touching)
      {
         cancel_drag();
      }

      m_drag_touch.drag_speed = godot::Vector2();
      m_drag_touch.drag_accum = godot::Vector2();
      m_drag_touch.last_drag_accum = godot::Vector2();
      m_drag_touch.drag_from = godot::Vector2(hval, vval);
      m_drag_touch.drag_touching = true;
      m_drag_touch.drag_touching_deaccel = false;
      m_drag_touch.beyond_deadzone = false;
      m_drag_touch.time_since_motion = 0;
      set_physics_process_internal(true);
   }
   else
   {
      if (m_drag_touch.drag_touching)
      {
         if (m_drag_touch.drag_speed == godot::Vector2())
         {
            cancel_drag();
         }
         else
         {
            m_drag_touch.drag_touching_deaccel = true;
         }
      }
   }
}

void Accordion::handle_mouse_motion(const godot::Ref<godot::InputEventMouseMotion>& mm)
{
   const float vval = m_vscroll->get_value();
   const float hval = m_hscroll->get_value();

   if (m_drag_touch.drag_touching && !m_drag_touch.drag_touching_deaccel)
   {
      const godot::Vector2 motion = mm->get_relative();
      m_drag_touch.drag_accum -= motion;

      const float accx = godot::Math::abs(m_drag_touch.drag_accum.x);
      const float accy = godot::Math::abs(m_drag_touch.drag_accum.y);

      if (m_drag_touch.beyond_deadzone || (m_hscroll->is_visible() && accx > m_deadzone) || (m_vscroll->is_visible() && accy > m_deadzone))
      {
         if (!m_drag_touch.beyond_deadzone)
         {
            emit_signal("scroll_started");
            m_drag_touch.beyond_deadzone = true;
            m_drag_touch.drag_accum = -motion;
         }

         const godot::Vector2 diff = m_drag_touch.drag_from + m_drag_touch.drag_accum;
         if (m_hscroll->is_visible())
         {
            m_hscroll->set_value(diff.x);
         }
         else
         {
            m_drag_touch.drag_accum.x = 0;
         }

         if (m_vscroll->is_visible())
         {
            m_vscroll->set_value(diff.y);
         }
         else
         {
            m_drag_touch.drag_accum.y = 0;
         }
         m_drag_touch.time_since_motion = 0;
      }
   }

   if (m_vscroll->get_value() != vval || m_hscroll->get_value() != hval)
   {
      accept_event();
   }
}

void Accordion::handle_pan_gesture(const godot::Ref<godot::InputEventPanGesture>& pg)
{
   const float vval = m_vscroll->get_value();
   const float hval = m_hscroll->get_value();

   if (m_hscroll->is_visible())
   {
      m_hscroll->set_value(hval + m_hscroll->get_page() * pg->get_delta().x / 8);
   }
   if (m_vscroll->is_visible())
   {
      m_vscroll->set_value(vval + m_vscroll->get_page() * pg->get_delta().y / 8);
   }

   if (m_hscroll->get_value() != hval || m_vscroll->get_value() != vval)
   {
      accept_event();
   }
}




void Accordion::handle_drag_touching(float dt)
{
   if (m_drag_touch.drag_touching_deaccel)
   {
      godot::Vector2 pos = godot::Vector2(m_hscroll->get_value(), m_vscroll->get_value());
      pos += m_drag_touch.drag_speed * dt;

      bool turnoff_h = false;
      bool turnoff_v = false;

      if (pos.x < 0)
      {
         pos.x = 0;
         turnoff_h = true;
      }
      if (pos.x > (m_hscroll->get_max() - m_hscroll->get_page()))
      {
         pos.x = m_hscroll->get_max() - m_hscroll->get_page();
         turnoff_h = true;
      }

      if (pos.y < 0)
      {
         pos.y = 0;
         turnoff_v = true;
      }
      if (pos.y > (m_vscroll->get_max() - m_vscroll->get_page()))
      {
         pos.y = m_vscroll->get_max() - m_vscroll->get_page();
         turnoff_v = true;
      }

      m_hscroll->set_value(pos.x);
      m_vscroll->set_value(pos.y);

      const float sgn_x = m_drag_touch.drag_speed.x < 0 ? -1 : 1;
      const float val_x = godot::Math::abs(m_drag_touch.drag_speed.x) - (1000 * dt);

      if (val_x < 0)
      {
         turnoff_h = true;
      }

      const float sgn_y = m_drag_touch.drag_speed.y < 0 ? -1 : 1;
      const float val_y = godot::Math::abs(m_drag_touch.drag_speed.y) - (1000 * dt);

      if (val_y < 0)
      {
         turnoff_v = true;
      }

      m_drag_touch.drag_speed = godot::Vector2(sgn_x * val_x, sgn_y * val_y);

      if (turnoff_h && turnoff_v)
      {
         cancel_drag();
      }
   }
   else
   {
      if (m_drag_touch.time_since_motion == 0 || m_drag_touch.time_since_motion > 0.1)
      {
         const godot::Vector2 diff = m_drag_touch.drag_accum - m_drag_touch.last_drag_accum;
         m_drag_touch.last_drag_accum = m_drag_touch.drag_accum;
         m_drag_touch.drag_speed = diff / dt;
      }

      m_drag_touch.time_since_motion += dt;
   }
}


void Accordion::on_child_order_changed()
{
   // This event is triggered whenever a child node is added, removed or moved. If this is not
   // inside the tree then it's possible the contents aren't fully built, so bail
   if (!is_inside_tree())
   {
      // Either loading or closing the scene. Bail...
      return;
   }

   refresh_contents();
   update_configuration_warnings();
}


void Accordion::on_scroll_changed(float val)
{
   queue_sort();
}


void Accordion::_get_property_list(godot::List<godot::PropertyInfo>* out_list) const
{
   /// Fill custom theme entries within the property list
   CustomControlThemeDB::get_singleton()->fill_property_list("Accordion", out_list, this);
}



void Accordion::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         CustomControlThemeDB::get_singleton()->update_control(this);

         // Whenever a child is added, removed or moved this signal will be given. So use it to properly
         // build the internal data
         connect("child_order_changed", callable_mp(this, &Accordion::on_child_order_changed));

         // React to scroll bars changing
         m_vscroll->connect("value_changed", callable_mp(this, &Accordion::on_scroll_changed));
         m_hscroll->connect("value_changed", callable_mp(this, &Accordion::on_scroll_changed));

         m_deadzone = Internals::global_get("gui/common/default_scroll_deadzone");
      } break;

      case NOTIFICATION_READY:
      {
         set_process(false);
         set_physics_process(false);
         set_process_internal(false);
         set_physics_process_internal(false);
      } break;

      case NOTIFICATION_ENTER_TREE:
      {
         // Ensure the internal data is correct, based on children controls (AccordionItem instances)
         refresh_contents();
      } break;


      case NOTIFICATION_THEME_CHANGED:
      {
         CustomControlThemeDB::get_singleton()->update_control(this);
         apply_internal_styles();

         m_vscroll->begin_bulk_theme_override();
         m_vscroll->add_theme_stylebox_override("scroll", m_theme_cache.vscroll);
         m_vscroll->add_theme_stylebox_override("scroll_focus", m_theme_cache.scroll_focus);
         m_vscroll->add_theme_stylebox_override("grabber", m_theme_cache.scroll_grabber);
         m_vscroll->add_theme_stylebox_override("grabber_highlight", m_theme_cache.scroll_grabber_hl);
         m_vscroll->add_theme_stylebox_override("grabber_pressed", m_theme_cache.scroll_grabber_pressed);
         m_vscroll->end_bulk_theme_override();

         m_hscroll->begin_bulk_theme_override();
         m_hscroll->add_theme_stylebox_override("scroll", m_theme_cache.hscroll);
         m_hscroll->add_theme_stylebox_override("scroll_focus", m_theme_cache.scroll_focus);
         m_hscroll->add_theme_stylebox_override("grabber", m_theme_cache.scroll_grabber);
         m_hscroll->add_theme_stylebox_override("grabber_highlight", m_theme_cache.scroll_grabber_hl);
         m_hscroll->add_theme_stylebox_override("grabber_pressed", m_theme_cache.scroll_grabber_pressed);
         m_hscroll->end_bulk_theme_override();
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         if (m_drag_touch.drag_touching)
         {
            handle_drag_touching(get_physics_process_delta_time());
         }
      } break;

      case NOTIFICATION_PRE_SORT_CHILDREN:
      {
         // Perform a first pass through contents to calculate the required size to hold everything
         // This will provide the necessary information to properly adjust scroll bars and actual contents.
         calculate_req_size();
      } break;

      case NOTIFICATION_SORT_CHILDREN:
      {
         check_layout();
      } break;

      case NOTIFICATION_DRAW:
      {
         if (m_theme_cache.background.is_valid())
         {
            draw_style_box(m_theme_cache.background, godot::Rect2(godot::Vector2(), get_size()));
         }
      } break;
   }
}


void Accordion::_bind_methods()
{
   using namespace godot;

   /// Functions
   ClassDB::bind_method(D_METHOD("set_exclusive", "value"), &Accordion::set_exclusive);
   ClassDB::bind_method(D_METHOD("get_exclusive"), &Accordion::get_exclusive);
   ClassDB::bind_method(D_METHOD("set_anim_block", "val"), &Accordion::set_anim_block);
   ClassDB::bind_method(D_METHOD("get_anim_block"), &Accordion::get_anim_block);
   ClassDB::bind_method(D_METHOD("set_title_alignment", "align"), &Accordion::set_title_alignment);
   ClassDB::bind_method(D_METHOD("get_title_alignment"), &Accordion::get_title_alignment);
   ClassDB::bind_method(D_METHOD("set_icon_alignment", "align"), &Accordion::set_icon_alignment);
   ClassDB::bind_method(D_METHOD("get_icon_alignment"), &Accordion::get_icon_alignment);
   ClassDB::bind_method(D_METHOD("set_hide_animation_time", "value"), &Accordion::set_hide_anim_time);
   ClassDB::bind_method(D_METHOD("get_hide_animation_time"), &Accordion::get_hide_anim_time);
   ClassDB::bind_method(D_METHOD("set_hide_animation_curve", "value"), &Accordion::set_hide_anim_curve);
   ClassDB::bind_method(D_METHOD("get_hide_animation_curve"), &Accordion::get_hide_anim_curve);
   ClassDB::bind_method(D_METHOD("set_show_animation_time", "value"), &Accordion::set_show_anim_time);
   ClassDB::bind_method(D_METHOD("get_show_animation_time"), &Accordion::get_show_anim_time);
   ClassDB::bind_method(D_METHOD("set_show_animation_curve", "value"), &Accordion::set_show_anim_curve);
   ClassDB::bind_method(D_METHOD("get_show_animation_curve"), &Accordion::get_show_anim_curve);
   ClassDB::bind_method(D_METHOD("set_animate_on_physics", "value"), &Accordion::set_animate_on_physics);
   ClassDB::bind_method(D_METHOD("get_animate_on_physics"), &Accordion::get_animate_on_physics);
   ClassDB::bind_method(D_METHOD("set_always_show_vscroll", "value"), &Accordion::set_always_show_vscroll);
   ClassDB::bind_method(D_METHOD("get_always_show_vscroll"), &Accordion::get_always_show_vscroll);
   ClassDB::bind_method(D_METHOD("set_always_show_hscroll", "value"), &Accordion::set_always_show_hscroll);
   ClassDB::bind_method(D_METHOD("get_always_show_hscroll"), &Accordion::get_always_show_hscroll);
   ClassDB::bind_method(D_METHOD("set_deadzone", "value"), &Accordion::set_deadzone);
   ClassDB::bind_method(D_METHOD("get_deadzone"), &Accordion::get_deadzone);
   ClassDB::bind_method(D_METHOD("set_text_direction", "direction"), &Accordion::set_text_direction);
   ClassDB::bind_method(D_METHOD("get_text_direction"), &Accordion::get_text_direction);
   ClassDB::bind_method(D_METHOD("set_language", "language"), &Accordion::set_language);
   ClassDB::bind_method(D_METHOD("get_language"), &Accordion::get_language);

   /// Variables
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "exclusive"), "set_exclusive", "get_exclusive");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "block_on_animation"), "set_anim_block", "get_anim_block");

   ADD_GROUP("AccordionLayout", "");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "title_alignment", PROPERTY_HINT_ENUM, "Left,Center,Right"), "set_title_alignment", "get_title_alignment");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "icon_alignment", PROPERTY_HINT_ENUM, "Left,Center,Right"), "set_icon_alignment", "get_icon_alignment");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "always_show_vscroll"), "set_always_show_vscroll", "get_always_show_vscroll");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "always_show_hscroll"), "set_always_show_hscroll", "get_always_show_hscroll");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "scroll_deadzone"), "set_deadzone", "get_deadzone");

   ADD_GROUP("Animation", "");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "animate_on_physics"), "set_animate_on_physics", "get_animate_on_physics");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "hide_animation_time"), "set_hide_animation_time", "get_hide_animation_time");
   ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "hide_animation_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_hide_animation_curve", "get_hide_animation_curve");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "show_animation_time"), "set_show_animation_time", "get_show_animation_time");
   ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "show_animation_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve"), "set_show_animation_curve", "get_show_animation_curve");

   ADD_GROUP("BiDi", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "text_direction", PROPERTY_HINT_ENUM, "Auto,Left-to-Right,Right-to-Left,Inherited"), "set_text_direction", "get_text_direction");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "language", PROPERTY_HINT_LOCALE_ID, ""), "set_language", "get_language");

   /// Signals
   ADD_SIGNAL(MethodInfo("scroll_started"));
   ADD_SIGNAL(MethodInfo("scroll_ended"));
   

   /// Custom Theme
   Ref<StyleBoxFlat> background = Internals::make_flat_stylebox(Color(0.4, 0.4, 0.4, 0.6));
   Ref<StyleBoxFlat> ctbackground = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.5), 6, 0, 6, 0, 3);
   Ref<StyleBoxFlat> btnormal = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6));
   Ref<StyleBoxFlat> bthover = Internals::make_flat_stylebox(Color(0.225, 0.225, 0.225, 0.6));
   Ref<StyleBoxFlat> btpressed = Internals::make_flat_stylebox(Color(0.0, 0.0, 0.0, 0.6));
   Ref<StyleBoxFlat> btdisabled = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.3));
   Ref<StyleBoxFlat> btfocus = Internals::make_flat_stylebox(Color(1.0, 1.0, 1.0, 0.75), 4, 4, 4, 4, 3, false, 2);

   Ref<StyleBoxFlat> hscroll = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), 0, 4, 0, 4, 10);
   Ref<StyleBoxFlat> vscroll = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.6), 4, 0, 4, 0, 10);
   Ref<StyleBoxFlat> scroll_grabber = Internals::make_flat_stylebox(Color(1.0, 1.0, 1.0, 0.4), 4, 4, 4, 4, 10);
   Ref<StyleBoxFlat> scroll_grabber_hl = Internals::make_flat_stylebox(Color(1.0, 1.0, 1.0, 0.75), 4, 4, 4, 4, 10);
   Ref<StyleBoxFlat> scroll_grabber_pressed = Internals::make_flat_stylebox(Color(0.75, 0.75, 0.75, 0.75), 4, 4, 4, 4, 10);
   
   Ref<Font> font;

   const Color font_normal(0.875, 0.875, 0.875, 1.0);
   const Color font_hovered(0.95, 0.95, 0.95, 1.0);
   const Color font_pressed(1.0, 1.0, 1.0, 1.0);
   const Color font_disabled(0.875, 0.875, 0.875, 0.5);
   const Color icon_disabled(1.0, 1.0, 1.0, 0.4);

   const int font_size = -1;
   const int font_outline_size = 0;
   const int bt_space = 4;
   const int inter_space = 4;


   REGISTER_CUSTOM_STYLE(background, Accordion, background, m_theme_cache);
   REGISTER_CUSTOM_STYLE(content_background, Accordion, ctbackground, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_normal, Accordion, btnormal, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_hovered, Accordion, bthover, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_pressed, Accordion, btpressed, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_disabled, Accordion, btdisabled, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_focus, Accordion, btfocus, m_theme_cache);
   REGISTER_CUSTOM_STYLE(hscroll, Accordion, hscroll, m_theme_cache);
   REGISTER_CUSTOM_STYLE(vscroll, Accordion, vscroll, m_theme_cache);
   REGISTER_CUSTOM_STYLE(scroll_focus, Accordion, btfocus, m_theme_cache);
   REGISTER_CUSTOM_STYLE(scroll_grabber, Accordion, scroll_grabber, m_theme_cache);
   REGISTER_CUSTOM_STYLE(scroll_grabber_hl, Accordion, scroll_grabber_hl, m_theme_cache);
   REGISTER_CUSTOM_STYLE(scroll_grabber_pressed, Accordion, scroll_grabber_pressed, m_theme_cache);
   REGISTER_CUSTOM_FONT(font, Accordion, font, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_normal, Accordion, font_normal, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_hovered, Accordion, font_hovered, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_focus, Accordion, font_hovered, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_pressed, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_hover_pressed, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_disabled, Accordion, font_disabled, m_theme_cache);
   REGISTER_CUSTOM_COLOR(font_outline, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(icon_normal, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(icon_pressed, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(icon_hover, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(icon_hover_pressed, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(icon_focus, Accordion, font_pressed, m_theme_cache);
   REGISTER_CUSTOM_COLOR(icon_disabled, Accordion, icon_disabled, m_theme_cache);
   REGISTER_CUSTOM_FONT_SIZE(font_size, Accordion, font_size, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(font_outline_size, Accordion, font_outline_size, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(button_space, Accordion, bt_space, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(inter_space, Accordion, inter_space, m_theme_cache);
}


godot::Vector2 Accordion::_get_minimum_size() const
{
   return godot::Vector2(40, 40);
}


void Accordion::_gui_input(const godot::Ref<godot::InputEvent>& event)
{
   godot::Ref<godot::InputEventMouseButton> mb = event;
   if (mb.is_valid())
   {
      handle_mouse_button(mb);
   }

   godot::Ref<godot::InputEventMouseMotion> mm = event;
   if (mm.is_valid())
   {
      handle_mouse_motion(mm);
   }
}


godot::PackedStringArray Accordion::_get_configuration_warnings() const
{
   godot::PackedStringArray ret;

   const int ccount = get_child_count();
   for (int i = 0; i < ccount; i++)
   {
      godot::Node* node = get_child(i);
      AccordionItem* item = godot::Object::cast_to<AccordionItem>(node);

      if (!item)
      {
         ret.append(godot::vformat("Node '%s' is not an AccordionItem. Ignoring it."));
      }
   }

   return ret;
}


void Accordion::set_exclusive(bool val)
{
   m_exclusive = val;

   // Scan items and keep active only the first one that is found, deactivating all others
   if (m_exclusive)
   {
      const int ccount = m_item.size();
      bool has_active = false;
      for (int i = 0; i < ccount; i++)
      {
         const bool is_active = m_item[i]->get_active();
         m_item[i]->change_active_state(!has_active && is_active, false);
         has_active = has_active || is_active;
      }
   }
}


void Accordion::set_anim_block(bool val)
{
   m_anim_block = val;
   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      m_item[i]->set_anim_block(m_anim_block);
   }
}


void Accordion::set_title_alignment(godot::HorizontalAlignment align)
{
   m_text_align = align;
   apply_alignment();
}

void Accordion::set_icon_alignment(godot::HorizontalAlignment align)
{
   m_icon_align = align;
   apply_alignment();
}


void Accordion::set_hide_anim_time(float val)
{
   m_hide_anim_time = val;
   apply_anim_setup();
}


void Accordion::set_hide_anim_curve(const godot::Ref<godot::Curve>& val)
{
   m_hide_anim_curve = val;
   apply_anim_setup();
}

godot::Ref<godot::Curve> Accordion::get_hide_anim_curve() const
{
   return m_hide_anim_curve;
}


void Accordion::set_show_anim_time(float val)
{
   m_show_anim_time = val;
   apply_anim_setup();
}

void Accordion::set_show_anim_curve(const godot::Ref<godot::Curve>& val)
{
   m_show_anim_curve = val;
   apply_anim_setup();
}

godot::Ref<godot::Curve> Accordion::get_show_anim_curve() const
{
   return m_show_anim_curve;
}


void Accordion::set_animate_on_physics(bool val)
{
   m_animate_on_physics = val;
   apply_anim_setup();
}


void Accordion::set_always_show_vscroll(bool val)
{
   m_always_show_vscroll = val;
   queue_sort();
}

void Accordion::set_always_show_hscroll(bool val)
{
   m_always_show_hscroll = val;
   queue_sort();
}


void Accordion::set_text_direction(TextDirection direction)
{
   m_text_direction = direction;
   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      m_item[i]->get_button()->set_text_direction(m_text_direction);
   }
}

void Accordion::set_language(const godot::String& language)
{
   m_language = language;
   const int ccount = m_item.size();
   for (int i = 0; i < ccount; i++)
   {
      m_item[i]->get_button()->set_language(m_language);
   }
}


void Accordion::on_item_toggled(AccordionItem* item)
{
   // Again, this function will be directly called by the relevant AccordionItem. Explanation for this "wonky" design
   // decision is given within the declaration of this function within the .h file.

   // There is nothing to do here
   if (!m_exclusive) { return; }

   if (item && item->get_active())
   {
      // Exclusive mode is enabled and incoming item is also enabled. Must disable any other item
      const int ccount = m_item.size();
      for (int i = 0; i < ccount; i++)
      {
         if (m_item[i] != item)
         {
            m_item[i]->change_active_state(false, false);
         }
      }
   }
}


Accordion::Accordion()
{
   set_clip_contents(true);


   m_exclusive = false;
   m_anim_block = false;
   m_text_align = godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_LEFT;
   m_icon_align = godot::HorizontalAlignment::HORIZONTAL_ALIGNMENT_RIGHT;
   m_hide_anim_time = 0.3;
   m_show_anim_time = 0.3;
   m_animate_on_physics = false;
   m_always_show_vscroll = false;
   m_always_show_hscroll = false;
   m_deadzone = 0;
   m_text_direction = TextDirection::TEXT_DIRECTION_AUTO;

   m_hscroll = memnew(godot::HScrollBar);
   add_child(m_hscroll, false, INTERNAL_MODE_BACK);
   m_hscroll->set_name("__h_scroll_bar__");
   m_hscroll->set_anchors_and_offsets_preset(PRESET_BOTTOM_WIDE);

   m_vscroll = memnew(godot::VScrollBar);
   add_child(m_vscroll, false, INTERNAL_MODE_BACK);
   m_vscroll->set_name("__v_scroll_bar__");
   m_vscroll->set_anchors_and_offsets_preset(PRESET_RIGHT_WIDE);
}


#endif
