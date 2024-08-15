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

#include "overlayinfo.h"
#include "../internal.h"

#ifndef OVERLAY_INFO_DISABLED

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/h_box_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/scene_tree_timer.hpp>
//#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#include <godot_cpp/classes/window.hpp>

#include <godot_cpp/variant/utility_functions.hpp>



OverlayInfo* OverlayInfo::s_singleton = nullptr;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// _OverlayControl
#ifndef EMPTY_OVERLAY_INFO

void _OverlayControl::enable()
{
   if (!is_inside_tree())
   {
      // set_process() and set_physics_process() don't work before the ready notification is given
      // If this is not inside the tree then certainly it didn't get such notification, so bail
      return;
   }

   const bool use_physics = Internals::global_get("keh_extension_pack/overlay_info/use_physics_process");
   const bool needs_processing = m_timed.size() > 0;

   set_process_internal(!use_physics && needs_processing);
   set_physics_process_internal(use_physics && needs_processing);
}


void _OverlayControl::disable()
{
   set_process_internal(false);
   set_physics_process_internal(false);
}


void _OverlayControl::check_auto_show()
{
   if (is_visible()) { return; }
   if (!has_any_labels()) {return; }

   const bool auto_show = Internals::global_get("keh_extension_pack/overlay_info/auto_show");
   if (auto_show)
   {
      set_visible(true);
   }
}


void _OverlayControl::check_auto_hide()
{
   if (!is_visible()) { return; }
   if (has_any_labels()) { return; }

   const bool auto_hide = Internals::global_get("keh_extension_pack/overlay_info/auto_hide");
   if (auto_hide)
   {
      set_visible(false);
   }
}


void _OverlayControl::handle_timed(float dt)
{
   // godot::Vector<> does not provided a non const access through the [] operator. So, using this to properly
   // edit things.
   LabelInfo* vecptr = m_timed.ptrw();

   int32_t index = 0;
   bool done = index >= m_timed.size();
   while (!done)
   {
      vecptr[index].time -= dt;
      if (vecptr[index].time <= 0)
      {
         // Time to remove the label. Because the internal container will have its current element removed there is
         // no need to advance the index
         vecptr[index].label->queue_free();
         m_timed.remove_at(index);
         vecptr = m_timed.ptrw();       // Just to ensure the pointer remains valid
      }
      else
      {
         // No further changes to the internal container. Advance the index to read the next element - if there is one
         index++;
      }

      done = index >= m_timed.size();
   }

   if (m_timed.size() == 0)
   {
      disable();

      // If here no timed labels are in the panel. Check if it should auto-hide
      check_auto_hide();
   }
}


void _OverlayControl::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_READY:
      {
         // The "enable()" function will not do anything if there is nothing to be processed
         enable();

         if (m_label_box->get_child_count() > 0)
         {
            check_auto_show();
         }
         else
         {
            check_auto_hide();
         }
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         handle_timed(get_process_delta_time());
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         handle_timed(get_physics_process_delta_time());
      } break;
   }
}


void _OverlayControl::set_label(const godot::StringName& label_id, const godot::String& text)
{
   godot::Label* lbl = nullptr;
   godot::HashMap<godot::StringName, godot::Label*>::Iterator iter = m_label_node.find(label_id);
   if (iter == m_label_node.end())
   {
      lbl = memnew(godot::Label);
      m_label_node[label_id] = lbl;

      m_label_box->add_child(lbl);
   }
   else
   {
      lbl = iter->value;
   }

   if (lbl)
   {
      lbl->set_text(text);
      check_auto_show();
   }
}


void _OverlayControl::remove_label(const godot::StringName& label_id)
{
   godot::HashMap<godot::StringName, godot::Label*>::Iterator iter = m_label_node.find(label_id);
   if (iter != m_label_node.end())
   {
      iter->value->queue_free();
      m_label_node.erase(label_id);
      check_auto_hide();
   }
}


void _OverlayControl::clear_labels()
{
   for (godot::HashMap<godot::StringName, godot::Label*>::Iterator iter = m_label_node.begin(); iter != m_label_node.end(); ++iter)
   {
      iter->value->queue_free();
   }
   m_label_node.clear();

   check_auto_hide();
}


bool _OverlayControl::has_any_labels()
{
   // m_label_box->get_child_count() is not fully reliable. The thing is, when labels are removed through "queue_free",
   // those will take some time until fully removed. An alternative would be to first remove the node from the tree then
   // manually delete it. Since there are two internal containers used to track labels, those can be used too, so...
   return ((m_label_node.size() + m_timed.size()) > 0);
}


void _OverlayControl::add_timed_label(const godot::String& text, real_t timeout)
{
   const real_t wait = godot::Math::max<real_t>(0.5, timeout);
   LabelInfo li(wait, memnew(godot::Label));
   li.label->set_text(text);

   m_label_box->add_child(li.label);

   m_timed.push_back(li);

   enable();

   check_auto_show();
}


void _OverlayControl::set_halign(AliMode align)
{
   m_haligner->set_alignment(align);
}


void _OverlayControl::set_valign(AliMode align)
{
   m_valigner->set_alignment(align);
}


void _OverlayControl::assign_theme(const godot::Ref<godot::Theme>& theme)
{
   m_background->set_theme(theme);
}


_OverlayControl::_OverlayControl()
{
   m_background = memnew(godot::PanelContainer);
   m_haligner = memnew(godot::HBoxContainer);
   m_valigner = memnew(godot::VBoxContainer);
   m_label_box = memnew(godot::VBoxContainer);

   m_background->set_name("_background_");
   m_haligner->set_name("_horizontal_aligner_");
   m_valigner->set_name("_vertical_aligner_");
   m_label_box->set_name("_label_box_");

   m_background->set_mouse_filter(godot::Control::MouseFilter::MOUSE_FILTER_IGNORE);
   m_haligner->set_mouse_filter(godot::Control::MouseFilter::MOUSE_FILTER_IGNORE);
   m_valigner->set_mouse_filter(godot::Control::MouseFilter::MOUSE_FILTER_IGNORE);
   m_label_box->set_mouse_filter(godot::Control::MouseFilter::MOUSE_FILTER_IGNORE);

   //m_background->set_self_modulate(godot::Color(0.0f, 0.0f, 0.0f, 0.65f));
   m_background->set_v_size_flags(0);


   // Make the outer "main box" fill the entire screen
   m_haligner->set_anchor(godot::Side::SIDE_RIGHT, 1.0);
   m_haligner->set_anchor(godot::Side::SIDE_BOTTOM, 1.0);

   m_valigner->set_anchor(godot::Side::SIDE_RIGHT, 1.0);
   m_valigner->set_anchor(godot::Side::SIDE_BOTTOM, 1.0);

   add_child(m_haligner, false, godot::Node::InternalMode::INTERNAL_MODE_FRONT);
   m_haligner->add_child(m_valigner, false, godot::Node::InternalMode::INTERNAL_MODE_FRONT);
   m_valigner->add_child(m_background, false, godot::Node::InternalMode::INTERNAL_MODE_FRONT);
   m_background->add_child(m_label_box, false, godot::Node::InternalMode::INTERNAL_MODE_FRONT);
}

#endif   // EMPTY_OVERLAY_INFO

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// OverlayInfo
void OverlayInfo::setup()
{
#ifndef EMPTY_OVERLAY_INFO
   if (m_overlay) { return; }
   godot::Window* root = Internals::get_tree_root();
   if (!root)
   {
      return;
   }

   m_overlay = memnew(_OverlayControl);
   
   // Unfortunately there is no easy way to verify if the Root has finished setting up children, so deferring the call regardless
   root->call_deferred("add_child", m_overlay);

   m_overlay->set_layer(1000);

   // By default make the entire thing hidden
   m_overlay->set_visible(false);
#endif
}


void OverlayInfo::_bind_methods()
{
   // Registration of exposed functions still must happen
   using namespace godot;

   ClassDB::bind_method(D_METHOD("set_label", "label_id", "text"), &OverlayInfo::set_label);
   ClassDB::bind_method(D_METHOD("remove_label", "label_id"), &OverlayInfo::remove_label);
   ClassDB::bind_method(D_METHOD("clear_labels"), &OverlayInfo::clear_labels);
   ClassDB::bind_method(D_METHOD("add_timed_label", "text", "timeout"), &OverlayInfo::add_timed_label);
   ClassDB::bind_method(D_METHOD("set_horizontal_align_left"), &OverlayInfo::set_horizontal_align_left);
   ClassDB::bind_method(D_METHOD("set_horizontal_align_center"), &OverlayInfo::set_horizontal_align_center);
   ClassDB::bind_method(D_METHOD("set_horizontal_align_right"), &OverlayInfo::set_horizontal_align_right);
   ClassDB::bind_method(D_METHOD("set_vertical_align_top"), &OverlayInfo::set_vertical_align_top);
   ClassDB::bind_method(D_METHOD("set_vertical_align_center"), &OverlayInfo::set_vertical_align_center);
   ClassDB::bind_method(D_METHOD("set_vertical_align_bottom"), &OverlayInfo::set_vertical_align_bottom);
   ClassDB::bind_method(D_METHOD("has_any_label"), &OverlayInfo::has_any_label);
   ClassDB::bind_method(D_METHOD("is_visible"), &OverlayInfo::is_visible);
   ClassDB::bind_method(D_METHOD("set_visible", "visible"), &OverlayInfo::set_visible);
   ClassDB::bind_method(D_METHOD("show"), &OverlayInfo::show);
   ClassDB::bind_method(D_METHOD("hide"), &OverlayInfo::hide);
   ClassDB::bind_method(D_METHOD("set_theme", "theme"), &OverlayInfo::set_theme);
   ClassDB::bind_method(D_METHOD("get_theme"), &OverlayInfo::get_theme);

   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "visible"), "set_visible", "is_visible");
   ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "theme", PROPERTY_HINT_RESOURCE_TYPE, "Theme"), "set_theme", "get_theme");
}


void OverlayInfo::set_label(const godot::StringName& label_id, const godot::String& text)
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_label(label_id, text);
#endif
}


void OverlayInfo::remove_label(const godot::StringName& label_id)
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->remove_label(label_id);
#endif
}


void OverlayInfo::clear_labels()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->clear_labels();
#endif
}


void OverlayInfo::add_timed_label(const godot::String& text, real_t timeout)
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->add_timed_label(text, timeout);
#endif
}


void OverlayInfo::set_horizontal_align_left()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_halign(AliMode::ALIGNMENT_BEGIN);
#endif
}


void OverlayInfo::set_horizontal_align_center()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_halign(AliMode::ALIGNMENT_CENTER);
#endif
}


void OverlayInfo::set_horizontal_align_right()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_halign(AliMode::ALIGNMENT_END);
#endif
}


void OverlayInfo::set_vertical_align_top()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_valign(AliMode::ALIGNMENT_BEGIN);
#endif
}


void OverlayInfo::set_vertical_align_center()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_valign(AliMode::ALIGNMENT_CENTER);
#endif
}


void OverlayInfo::set_vertical_align_bottom()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_valign(AliMode::ALIGNMENT_END);
#endif
}


bool OverlayInfo::has_any_label()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return false; }
   }
   return m_overlay->has_any_labels();
#else
   return false;
#endif
}


bool OverlayInfo::is_visible()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return false; }
   }
   return m_overlay->is_visible();
#else
   return m_visible;
#endif
   return false;
}


void OverlayInfo::toggle_visiblity()
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_visible(!m_overlay->is_visible());
#else
   m_visible = !m_visible;
#endif
}


void OverlayInfo::set_visible(bool visible)
{
#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->set_visible(visible);
#else
   m_visible = visible;
#endif
}


void OverlayInfo::set_theme(const godot::Ref<godot::Theme>& theme)
{
   m_theme = theme;

#ifndef EMPTY_OVERLAY_INFO
   if (!m_overlay)
   {
      setup();
      if (!m_overlay) { return; }
   }
   m_overlay->assign_theme(m_theme);
#endif
}


godot::Ref<godot::Theme> OverlayInfo::get_theme() const
{
   return m_theme;
}


void OverlayInfo::register_project_settings()
{
   Internals::global_set("keh_extension_pack/overlay_info/use_physics_process", (bool)false);
   Internals::global_set("keh_extension_pack/overlay_info/auto_hide", (bool)false);
   Internals::global_set("keh_extension_pack/overlay_info/auto_show", (bool)false);
}


OverlayInfo::OverlayInfo()
{
   ERR_FAIL_COND(s_singleton != nullptr);
   s_singleton = this;
#ifndef EMPTY_OVERLAY_INFO
   m_overlay = nullptr;
#else
   m_visible = false;
#endif
}


#endif
