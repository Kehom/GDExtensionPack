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


#include "radialimpulse.h"

#include <godot_cpp/classes/area2d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/collision_shape2d.hpp>
#include <godot_cpp/classes/collision_shape3d.hpp>
#include <godot_cpp/classes/rigid_body2d.hpp>
#include <godot_cpp/classes/rigid_body3d.hpp>


#ifndef RADIAL_IMPULSE2D_DISABLED
void RadialImpulse2D::iterate_cache()
{
   const int bcount = m_body.size();
   for (int i = 0; i < bcount; i++)
   {
      godot::RigidBody2D* body = m_body[i];

      godot::Vector2 dir = body->get_global_position() - get_global_position();
      const double dist = dir.length();

      if (godot::Math::is_equal_approx(dist, 0.0) || (dist > m_radius && m_falloff == LINEAR))
      {
         // A distance of "0" will result in problems when calculating the normalized direction.
         // Also if the falloff is set to LINEAR then there is no point in calculating anything if distance is bigger than radius.
         // So just skip to the next rigid body
         continue;
      }

      // The expensive operation of vector normalization (calculate its length) has already been done. To prevent doing it again
      // by calling "dir.normalized()", manually normalize it by simply dividing the vector by its length
      dir = dir / dist;

      switch (m_falloff)
      {
         case CONSTANT:
         {
            body->apply_impulse(dir * m_force);
         } break;

         case LINEAR:
         {
            const double f = ((m_radius - dist) / m_radius) * m_force;
            body->apply_impulse(dir * f);
         } break;
      }
   }

   emit_signal("impulse_applied");

   set_physics_process(false);
}


void RadialImpulse2D::on_body_entered(godot::Node2D* body)
{
   godot::RigidBody2D* rigid = godot::Object::cast_to<godot::RigidBody2D>(body);
   if (!rigid) { return; }

   m_body.append(rigid);
}

void RadialImpulse2D::on_body_exited(godot::Node2D* body)
{
   godot::RigidBody2D* rigid = godot::Object::cast_to<godot::RigidBody2D>(body);
   if (!rigid) { return; }

   m_body.erase(rigid);
}

void RadialImpulse2D::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_READY:
      {
         set_physics_process(false);
         set_process(false);
      } break;

      case NOTIFICATION_PHYSICS_PROCESS:
      {
         iterate_cache();
      } break;

      case NOTIFICATION_POSTINITIALIZE:
      {
         m_area->connect("body_entered", callable_mp(this, &RadialImpulse2D::on_body_entered));
         m_area->connect("body_exited", callable_mp(this, &RadialImpulse2D::on_body_exited));
      } break;
   }
}

void RadialImpulse2D::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("apply_impulse"), &RadialImpulse2D::apply_impulse);
   ClassDB::bind_method(D_METHOD("get_radius"), &RadialImpulse2D::get_radius);
   ClassDB::bind_method(D_METHOD("set_radius", "radius"), &RadialImpulse2D::set_radius);
   ClassDB::bind_method(D_METHOD("get_force"), &RadialImpulse2D::get_force);
   ClassDB::bind_method(D_METHOD("set_force", "force"), &RadialImpulse2D::set_force);
   ClassDB::bind_method(D_METHOD("get_falloff"), &RadialImpulse2D::get_falloff);
   ClassDB::bind_method(D_METHOD("set_falloff", "falloff"), &RadialImpulse2D::set_falloff);
   
//   ClassDB::bind_method(D_METHOD("get_collision_layer"), &RadialImpulse2D::get_collision_layer);
//   ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &RadialImpulse2D::set_collision_layer);
   ClassDB::bind_method(D_METHOD("get_collision_mask"), &RadialImpulse2D::get_collision_mask);
   ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &RadialImpulse2D::set_collision_mask);
//   ClassDB::bind_method(D_METHOD("get_collision_priority"), &RadialImpulse2D::get_collision_priority);
//   ClassDB::bind_method(D_METHOD("set_collision_priority", "priority"), &RadialImpulse2D::set_collision_priority);


   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "force"), "set_force", "get_force");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "falloff", PROPERTY_HINT_ENUM, "CONSTANT,LINEAR"), "set_falloff", "get_falloff");

   ADD_GROUP("Collision", "collision_");
	//ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_mask", "get_collision_mask");
	//ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_priority"), "set_collision_priority", "get_collision_priority");

   BIND_ENUM_CONSTANT(CONSTANT);
   BIND_ENUM_CONSTANT(LINEAR);

   ADD_SIGNAL(MethodInfo("impulse_applied"));
}

void RadialImpulse2D::apply_impulse()
{
   if (godot::Math::is_equal_approx(m_force, 0.0) || !is_inside_tree())
   {
      return;
   }

   // Enable physics processing and apply the impulses only at that moment. This is the easies (and probably the "correct")
   // way of ensuring the internal m_body array is holding the correct rigid bodies that must be affected.
   // Otherwise there is a gig chance it will be "out-date" with old information before the Physics server updates the
   // states of all physics bodies/areas...
   set_physics_process(true);
}


void RadialImpulse2D::set_radius(double val)
{
   m_radius = val;
   m_circle->set_radius(val);
}

/*void RadialImpulse2D::set_collision_layer(uint32_t layer)
{
   if (m_area)
   {
      m_area->set_collision_layer(layer);
   }
}

uint32_t RadialImpulse2D::get_collision_layer() const
{
   return m_area ? m_area->get_collision_layer() : 0;
}*/

void RadialImpulse2D::set_collision_mask(uint32_t mask)
{
   if (m_area)
   {
      m_area->set_collision_mask(mask);
   }
}

uint32_t RadialImpulse2D::get_collision_mask() const
{
   return m_area ? m_area->get_collision_mask() : 0;
}

/*void RadialImpulse2D::set_collision_priority(real_t priority)
{
   if (m_area)
   {
      m_area->set_collision_priority(priority);
   }
}

real_t RadialImpulse2D::get_collision_priority() const
{
   return m_area ? m_area->get_collision_priority() : 0.0;
}*/


RadialImpulse2D::RadialImpulse2D()
{
   m_radius = 10;
   m_force = 0.0;
   m_falloff = ImpulseFalloff::CONSTANT;

   m_area = memnew(godot::Area2D);
   m_shape = memnew(godot::CollisionShape2D);
   m_circle = godot::Ref<godot::CircleShape2D>(memnew(godot::CircleShape2D));

   m_area->set_name("_area_");
   add_child(m_area, false, INTERNAL_MODE_BACK);
   m_area->add_child(m_shape);
   m_shape->set_shape(m_circle);

   // Connecting to events at this point will lead to warning messages. This task is done within the _notification() function
}
#endif


#ifndef RADIAL_IMPULSE3D_DISABLED
void RadialImpulse3D::iterate_cache()
{
   const int bcount = m_body.size();
   for (int i = 0; i < bcount; i++)
   {
      godot::RigidBody3D* body = m_body[i];

      godot::Vector3 dir = body->get_global_position() - get_global_position();
      const double dist = dir.length();

      if (godot::Math::is_equal_approx(dist, 0.0) || (dist > m_radius && m_falloff == LINEAR))
      {
         // A distance of "0" will result in problems when calculating the normalized direction.
         // Also if the falloff is set to LINEAR then there is no point in calculating anything if distance is bigger than radius.
         // So just skip to the next rigid body
         continue;
      }

      // The expensive operation of vector normalization (calculate its length) has already been done. To prevent doing it again
      // by calling "dir.normalized()", manually normalize it by simply dividing the vector by its length
      dir = dir / dist;

      switch (m_falloff)
      {
         case CONSTANT:
         {
            body->apply_impulse(dir * m_force);
         } break;

         case LINEAR:
         {
            const double f = ((m_radius - dist) / m_radius) * m_force;
            body->apply_impulse(dir * f);
         } break;
      }
   }

   emit_signal("impulse_applied");

   set_physics_process(false);
}

void RadialImpulse3D::on_body_entered(godot::Node3D* body)
{
   godot::RigidBody3D* rigid = godot::Object::cast_to<godot::RigidBody3D>(body);
   if (!rigid) { return; }

   m_body.append(rigid);
}

void RadialImpulse3D::on_body_exited(godot::Node3D* body)
{
   godot::RigidBody3D* rigid = godot::Object::cast_to<godot::RigidBody3D>(body);
   if (!rigid) { return; }

   m_body.erase(rigid);
}

void RadialImpulse3D::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_READY:
      {
         set_physics_process(false);
         set_process(false);
      } break;

      case NOTIFICATION_PHYSICS_PROCESS:
      {
         iterate_cache();
      } break;

      case NOTIFICATION_POSTINITIALIZE:
      {
         m_area->connect("body_entered", callable_mp(this, &RadialImpulse3D::on_body_entered));
         m_area->connect("body_exited", callable_mp(this, &RadialImpulse3D::on_body_exited));
      } break;
   }
}

void RadialImpulse3D::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("apply_impulse"), &RadialImpulse3D::apply_impulse);
   ClassDB::bind_method(D_METHOD("get_radius"), &RadialImpulse3D::get_radius);
   ClassDB::bind_method(D_METHOD("set_radius", "radius"), &RadialImpulse3D::set_radius);
   ClassDB::bind_method(D_METHOD("get_force"), &RadialImpulse3D::get_force);
   ClassDB::bind_method(D_METHOD("set_force", "force"), &RadialImpulse3D::set_force);
   ClassDB::bind_method(D_METHOD("get_falloff"), &RadialImpulse3D::get_falloff);
   ClassDB::bind_method(D_METHOD("set_falloff", "falloff"), &RadialImpulse3D::set_falloff);
   
//   ClassDB::bind_method(D_METHOD("get_collision_layer"), &RadialImpulse3D::get_collision_layer);
//   ClassDB::bind_method(D_METHOD("set_collision_layer", "layer"), &RadialImpulse3D::set_collision_layer);
   ClassDB::bind_method(D_METHOD("get_collision_mask"), &RadialImpulse3D::get_collision_mask);
   ClassDB::bind_method(D_METHOD("set_collision_mask", "mask"), &RadialImpulse3D::set_collision_mask);
//   ClassDB::bind_method(D_METHOD("get_collision_priority"), &RadialImpulse3D::get_collision_priority);
//   ClassDB::bind_method(D_METHOD("set_collision_priority", "priority"), &RadialImpulse3D::set_collision_priority);


   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "radius"), "set_radius", "get_radius");
   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "force"), "set_force", "get_force");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "falloff", PROPERTY_HINT_ENUM, "CONSTANT,LINEAR"), "set_falloff", "get_falloff");

   ADD_GROUP("Collision", "collision_");
	//ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_layer", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_layer", "get_collision_layer");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "collision_mask", PROPERTY_HINT_LAYERS_2D_PHYSICS), "set_collision_mask", "get_collision_mask");
	//ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "collision_priority"), "set_collision_priority", "get_collision_priority");

   BIND_ENUM_CONSTANT(CONSTANT);
   BIND_ENUM_CONSTANT(LINEAR);

   ADD_SIGNAL(MethodInfo("impulse_applied"));
}

void RadialImpulse3D::apply_impulse()
{
   if (godot::Math::is_equal_approx(m_force, 0.0) || !is_inside_tree())
   {
      return;
   }

   // Enable physics processing and apply the impulses only at that moment. This is the easies (and probably the "correct")
   // way of ensuring the internal m_body array is holding the correct rigid bodies that must be affected.
   // Otherwise there is a gig chance it will be "out-date" with old information before the Physics server updates the
   // states of all physics bodies/areas...
   set_physics_process(true);
}

void RadialImpulse3D::set_radius(double val)
{
   m_radius = val;
   m_sphere->set_radius(val);
}

/*void RadialImpulse3D::set_collision_layer(uint32_t layer)
{
   if (m_area)
   {
      m_area->set_collision_layer(layer);
   }
}

uint32_t RadialImpulse3D::get_collision_layer() const
{
   return m_area ? m_area->get_collision_layer() : 0;
}*/

void RadialImpulse3D::set_collision_mask(uint32_t mask)
{
   if (m_area)
   {
      m_area->set_collision_mask(mask);
   }
}

uint32_t RadialImpulse3D::get_collision_mask() const
{
   return m_area ? m_area->get_collision_mask() : 0;
}

/*void RadialImpulse3D::set_collision_priority(real_t priority)
{
   if (m_area)
   {
      m_area->set_collision_priority(priority);
   }
}

real_t RadialImpulse3D::get_collision_priority() const
{
   return m_area ? m_area->get_collision_priority() : 0.0;
}*/

RadialImpulse3D::RadialImpulse3D()
{
   m_radius = 0.5;
   m_force = 0.0;
   m_falloff = ImpulseFalloff::CONSTANT;

   m_area = memnew(godot::Area3D);
   m_shape = memnew(godot::CollisionShape3D);
   m_sphere = godot::Ref<godot::SphereShape3D>(memnew(godot::SphereShape3D));

   m_area->set_name("_area_");
   add_child(m_area, false, INTERNAL_MODE_BACK);
   m_area->add_child(m_shape);
   m_shape->set_shape(m_sphere);
}
#endif
