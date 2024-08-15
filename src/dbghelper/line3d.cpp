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


#include "line3d.h"
#include "../internal.h"

#ifndef LINE3D_DISABLED

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/window.hpp>

#include <godot_cpp/variant/utility_functions.hpp>

Line3D* Line3D::s_singleton = nullptr;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// _LineNode
#ifndef EMPTY_LINE3D

void _LineNode::handle_queue(float dt)
{
   const int64_t ofcount = m_one_frame.size();
   const int64_t tcount = m_timed.size();

   // Clear lines from previous frame
   m_mesh->clear_surfaces();


   if (ofcount > 0 || tcount > 0)
   {
      m_mesh->surface_begin(godot::Mesh::PrimitiveType::PRIMITIVE_LINES);

      // Iterate through "one-frame" lines
      for (int64_t l = 0; l < ofcount; l++)
      {
         m_mesh->surface_set_color(m_one_frame[l].color);
         m_mesh->surface_add_vertex(m_one_frame[l].p0);
         m_mesh->surface_add_vertex(m_one_frame[l].p1);
      }

      // godot::Vector<> does not provide non const access through the [] operator. Using this because internal
      // data will be updated
      TimedLine* timed = m_timed.ptrw();
      int64_t index = 0;
      while (index < m_timed.size())
      {
         m_mesh->surface_set_color(timed[index].line.color);
         m_mesh->surface_add_vertex(timed[index].line.p0);
         m_mesh->surface_add_vertex(timed[index].line.p1);

         timed[index].time -= dt;
         if (timed[index].time <= 0)
         {
            m_timed.remove_at(index);
            timed = m_timed.ptrw();
         }
         else
         {
            index++;
         }
      }

      // Finalize the "procedural mesh"
      m_mesh->surface_end();
   }

   // Clear data from the "one-frame"
   m_one_frame.clear();
}


void _LineNode::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_READY:
      {
         enable();
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         handle_queue(get_process_delta_time());
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         handle_queue(get_physics_process_delta_time());
      } break;
   }
}


void _LineNode::enable()
{
   const bool use_physics = Internals::global_get("keh_extension_pack/debug_line_3d/use_physics_process");
   set_process_internal(!use_physics);
   set_physics_process_internal(use_physics);
}


void _LineNode::disable()
{
   set_process_internal(false);
   set_physics_process_internal(false);
}


void _LineNode::set_enabled(bool e)
{
   if (e)
   {
      enable();
   }
   else
   {
      disable();
   }
}


void _LineNode::add_line(const godot::Vector3& p0, const godot::Vector3& p1, const godot::Color& color)
{
   if (!is_enabled()) { return; }
   m_one_frame.append(LineData(p0, p1, color));
}


void _LineNode::add_timed_line(const godot::Vector3& p0, const godot::Vector3& p1, float time, const godot::Color& color)
{
   if (!is_enabled()) { return; }
   if (time > 0.0)
   {
      m_timed.append(TimedLine(p0, p1, time, color));
   }
}


void _LineNode::clear_lines()
{
   // Probably no point in removing one-frame lines, but no harm in doing that either so...
   m_one_frame.clear();
   m_timed.clear();

   if (!is_enabled())
   {
      m_mesh->clear_surfaces();
   }
}


_LineNode::_LineNode()
{
   m_mesh = godot::Ref<godot::ImmediateMesh>(memnew(godot::ImmediateMesh));

   godot::MeshInstance3D* mi = memnew(godot::MeshInstance3D);
   mi->set_name("_mesh_instance");
   mi->set_mesh(m_mesh);
   mi->set_cast_shadows_setting(godot::GeometryInstance3D::ShadowCastingSetting::SHADOW_CASTING_SETTING_OFF);
   add_child(mi);

   godot::Ref<godot::StandardMaterial3D> mat = godot::Ref<godot::StandardMaterial3D>(memnew(godot::StandardMaterial3D));
   mat->set_shading_mode(godot::BaseMaterial3D::ShadingMode::SHADING_MODE_UNSHADED);
   mat->set_flag(godot::BaseMaterial3D::Flags::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
   mat->set_transparency(godot::BaseMaterial3D::Transparency::TRANSPARENCY_ALPHA);
   mi->set_material_override(mat);
}

#endif    // EMPTY_LINE3D

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Line3D
void Line3D::setup()
{
#ifndef EMPTY_LINE3D
   if (m_line_node) { return; }
   godot::Window* root = Internals::get_tree_root();
   if (!root)
   {
      return;
   }

   m_line_node = memnew(_LineNode);
   
   // Unfortunately there is no easy way to verify if the Root has finished setting up children, so deferring this call regardless
   root->call_deferred("add_child", m_line_node);
#endif
}


void Line3D::_bind_methods()
{
   // Registration of functions and variables must be done even if "empty" is enabled.
   using namespace godot;

   ClassDB::bind_method(D_METHOD("is_enabled"), &Line3D::is_enabled);
   ClassDB::bind_method(D_METHOD("enable"), &Line3D::enable);
   ClassDB::bind_method(D_METHOD("disable"), &Line3D::disable);
   ClassDB::bind_method(D_METHOD("set_enabled", "e"), &Line3D::set_enabled);
   ClassDB::bind_method(D_METHOD("add_line", "point0", "point1", "color"), &Line3D::add_line, DEFVAL(Color(1, 1, 1, 1)));
   ClassDB::bind_method(D_METHOD("add_from_vector", "point", "vector", "color"), &Line3D::add_from_vector, DEFVAL(Color(1, 1, 1, 1)));
   ClassDB::bind_method(D_METHOD("add_timed_line", "point0", "point1", "time", "color"), &Line3D::add_timed_line, DEFVAL(Color(1, 1, 1, 1)));
   ClassDB::bind_method(D_METHOD("add_timed_from_vector", "point", "vector", "time", "color"), &Line3D::add_timed_from_vector, DEFVAL(Color(1, 1, 1, 1)));
   ClassDB::bind_method(D_METHOD("clear_lines"), &Line3D::clear_lines);

   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
}


bool Line3D::is_enabled()
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return false; }
   }
   return m_line_node->is_enabled();
#else
   return m_enabled;
#endif
}


void Line3D::enable()
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   m_line_node->enable();
#else
   m_enabled = true;
#endif
}


void Line3D::disable()
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   m_line_node->disable();
#else
   m_enabled = false;
#endif
}


void Line3D::set_enabled(bool e)
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   m_line_node->set_enabled(e);
#else
   m_enabled = e;
#endif
}


void Line3D::add_line(const godot::Vector3& p0, const godot::Vector3& p1, const godot::Color& color)
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   m_line_node->add_line(p0, p1, color);
#endif
}


void Line3D::add_from_vector(const godot::Vector3& point, const godot::Vector3 vector, const godot::Color& color)
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   const godot::Vector3 p1(point + vector);
   add_line(point, p1, color);
#endif
}


void Line3D::add_timed_line(const godot::Vector3& p0, const godot::Vector3& p1, float time, const godot::Color& color)
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   m_line_node->add_timed_line(p0, p1, time, color);
#endif
}


void Line3D::add_timed_from_vector(const godot::Vector3& point, const godot::Vector3& vector, float time, const godot::Color& color)
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   const godot::Vector3 p1(point + vector);
   add_timed_line(point, p1, time, color);
#endif
}


void Line3D::clear_lines()
{
#ifndef EMPTY_LINE3D
   if (!m_line_node)
   {
      setup();
      if (!m_line_node) { return; }
   }
   m_line_node->clear_lines();
#endif
}


void Line3D::register_project_settings()
{
   Internals::global_set("keh_extension_pack/debug_line_3d/use_physics_process", (bool)false);
}


Line3D::Line3D()
{
   ERR_FAIL_COND(s_singleton != nullptr);
   s_singleton = this;
#ifndef EMPTY_LINE3D
   m_line_node = nullptr;
#else
   m_enabled = false;
#endif
}


#endif
