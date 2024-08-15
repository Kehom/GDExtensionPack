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

#ifndef _kehgeneral_radial_impulse_h_included
#define _kehgeneral_radial_impulse_h_included 1

#include <godot_cpp/classes/circle_shape2d.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/sphere_shape3d.hpp>
#include <godot_cpp/templates/vector.hpp>

// Forward declares
namespace godot
{
   class Area2D;
   class Area3D;
   class CollisionShape2D;
   class CollisionShape3D;
   class RigidBody2D;
   class RigidBody3D;
}


#ifndef RADIAL_IMPULSE2D_DISABLED
class RadialImpulse2D : public godot::Node2D
{
   GDCLASS(RadialImpulse2D, godot::Node2D);
public:
   enum ImpulseFalloff { CONSTANT, LINEAR };
private:
   /// Internal variables
   // Attached into this node, this will automate the rigid body detection
   godot::Area2D* m_area;

   // Area2D requires a shape node
   godot::CollisionShape2D* m_shape;

   // Determines the shape assigned into the internal CollisionShape2D
   godot::Ref<godot::CircleShape2D> m_circle;


   /// Exposed variables
   // Determines the radius of the circular area that will be used to detect which rigid bodies should
   // receive an impulse when requested
   double m_radius;

   // Determines the intensity of the impulse
   double m_force;

   // And if there is any falloff
   ImpulseFalloff m_falloff;

   // Cache rigid bodies that are within the circular shape
   godot::Vector<godot::RigidBody2D*> m_body;

   /// Internal functions
   void iterate_cache();

   /// Event handlers
   void on_body_entered(godot::Node2D* body);

   void on_body_exited(godot::Node2D* body);

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides

   /// Exposed functions
   void apply_impulse();


   /// Setters/Getters
   double get_radius() const { return m_radius; }
   void set_radius(double val);

   double get_force() const { return m_force; }
   void set_force(double val) { m_force = val; }

   ImpulseFalloff get_falloff() const { return m_falloff; }
   void set_falloff(ImpulseFalloff val) { m_falloff = val; }

//   void set_collision_layer(uint32_t layer);
//   uint32_t get_collision_layer() const;

   void set_collision_mask(uint32_t mask);
   uint32_t get_collision_mask() const;

//   void set_collision_priority(real_t priority);
//   real_t get_collision_priority() const;

   /// Public non exposed functions
   RadialImpulse2D();
};

VARIANT_ENUM_CAST(RadialImpulse2D::ImpulseFalloff);

#endif //RADIAL_IMPULSE2D_DISABLED



#ifndef RADIAL_IMPULSE3D_DISABLED
class RadialImpulse3D : public godot::Node3D
{
   GDCLASS(RadialImpulse3D, godot::Node3D);
public:
   enum ImpulseFalloff { CONSTANT, LINEAR };
private:
   /// Internal variables
   // Attached into this node, this will automate the rigid body detection
   godot::Area3D* m_area;

   // Area2D requires a shape node
   godot::CollisionShape3D* m_shape;

   // Determines the shape assigned into the internal CollisionShape2D
   godot::Ref<godot::SphereShape3D> m_sphere;


   /// Exposed variables
   // Determines the radius of the circular area that will be used to detect which rigid bodies should
   // receive an impulse when requested
   double m_radius;

   // Determines the intensity of the impulse
   double m_force;

   // And if there is any falloff
   ImpulseFalloff m_falloff;

   // Cache rigid bodies that are within the circular shape
   godot::Vector<godot::RigidBody3D*> m_body;

   /// Internal functions
   void iterate_cache();

   /// Event handlers
   void on_body_entered(godot::Node3D* body);

   void on_body_exited(godot::Node3D* body);

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides

   /// Exposed functions
   void apply_impulse();

   /// Setters/Getters
   double get_radius() const { return m_radius; }
   void set_radius(double val);

   double get_force() const { return m_force; }
   void set_force(double val) { m_force = val; }

   ImpulseFalloff get_falloff() const { return m_falloff; }
   void set_falloff(ImpulseFalloff val) { m_falloff = val; }

//   void set_collision_layer(uint32_t layer);
//   uint32_t get_collision_layer() const;

	void set_collision_mask(uint32_t mask);
   uint32_t get_collision_mask() const;

//   void set_collision_priority(real_t priority);
//   real_t get_collision_priority() const;

   /// Public non exposed functions
   RadialImpulse3D();
};

VARIANT_ENUM_CAST(RadialImpulse3D::ImpulseFalloff);

#endif  //RADIAL_IMPULSE3D_DISABLED


#endif //_kehgeneral_radial_impulse_h_included
