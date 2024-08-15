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

#ifndef _kehgeneral_autointerpolate_h_included
#define _kehgeneral_autointerpolate_h_included 1

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/node_path.hpp>
#include <godot_cpp/templates/vector.hpp>

// What is different here when compared to GDScript implementation and original work by Lawnjelly:
// - Now each part of the transform (translation, orientation and scale) can be individually set to either fully interpolate,
//   don't interpolate or completely ignore.
// - There is one Node that can be used both for 2D and 3D, depending on the target node, which can be chosen. This node will
//   optionally interpolate its children and also an optional list that can be assigned within the editor.
// - All 3 nodes provided here will use global transform which ensures correct behavior regardless of the hierarchy.


// This will be defined if all three interpolation nodes (AutoInterpolate, Smooth2D and Smooth3D) are disabled
#ifndef INTERPOLATION_DISABLED


// This class will not be exposed - nor the two derived ones
// Separating the core of the interpolation just so AutoInterpolate, Smooth2D and Smooth3D can share *some* code
// Ideally this should be abstract, however since it's also meant to be internal class, can't exactly declare
// the pure virtual functions. Comments are added into the functions that would be ideally pure.
class Interpolator : public godot::RefCounted
{
   GDCLASS(Interpolator, godot::RefCounted);
private:
   bool m_had_physics;

protected:
   virtual void _cycle() {}          // should be pure virtual
   static void _bind_methods() {}
public:
   enum InterpMode { Snap, Full, Ignore };


   void cycle(bool is_in_physics)
   {
      if (m_had_physics) { _cycle(); }
      m_had_physics = is_in_physics;
   }

   virtual godot::Transform2D calculate2d() { return godot::Transform2D(); }
   virtual godot::Transform3D calculate3d() { return godot::Transform3D(); }

   /// Abstract classes can be registered but not if they need to be internal. Unfortunately there are a few functions that could easily be
   /// pure but can't for that reason (this class is meant to be internal)
   virtual godot::Node* get_target() const { return nullptr; }       // This should be pure
   virtual bool is_2d() const { return true; }              // This should be pure
   virtual void snap_to_target() {}           // This should be pure
   virtual void set_from_to(const godot::Transform2D& from, const godot::Transform2D& to) {}
   virtual void set_from_to(const godot::Transform3D& from, const godot::Transform3D& to) {}
   virtual void change_target(godot::Node2D* target) {}
   virtual void change_target(godot::Node3D* target) {}

   virtual void apply(godot::Node2D* into, const godot::Transform2D& t, InterpMode translation, InterpMode orientation, InterpMode scale) const {}
   virtual void apply(godot::Node3D* into, const godot::Transform3D& t, InterpMode translation, InterpMode orientation, InterpMode scale) const {}
   

   Interpolator() : m_had_physics(false) {}
};


class Interpolator2D : public Interpolator
{
private:
   godot::Transform2D m_from;
   godot::Transform2D m_to;
   godot::Node2D* m_target;

protected:
   virtual void _cycle() override;
public:
   virtual godot::Transform2D calculate2d() override;
   virtual godot::Node* get_target() const override { return (godot::Node*)m_target; }
   virtual bool is_2d() const override { return true; }
   virtual void snap_to_target() override;
   virtual void set_from_to(const godot::Transform2D& from, const godot::Transform2D& to) override;
   virtual void change_target(godot::Node2D* target) override;
   virtual void apply(godot::Node2D* into, const godot::Transform2D& t, InterpMode translation, InterpMode orientation, InterpMode scale) const override;
   Interpolator2D(godot::Node2D* t = nullptr);
};

class Interpolator3D : public Interpolator
{
private:
   godot::Transform3D m_from;
   godot::Transform3D m_to;
   godot::Node3D* m_target;

protected:
   virtual void _cycle() override;
public:
   virtual godot::Transform3D calculate3d() override;
   virtual godot::Node* get_target() const override { return (godot::Node*)m_target; }
   virtual bool is_2d() const override { return false; }
   virtual void snap_to_target() override;
   virtual void set_from_to(const godot::Transform3D& from, const godot::Transform3D& to) override;
   virtual void change_target(godot::Node3D* target) override;
   virtual void apply(godot::Node3D* into, const godot::Transform3D& t, InterpMode translation, InterpMode orientation, InterpMode scale) const override;
   Interpolator3D(godot::Node3D* t = nullptr);
};

#ifndef AUTO_INTERPOLATE_DISABLED


class AutoInterpolate : public godot::Node
{
   GDCLASS(AutoInterpolate, godot::Node);
   using MainInterp = Interpolator::InterpMode;       // To make casting easier
public:
   // Unfortunately I couldn't find a way to use the same Enum from the Interpolator without causing problems in Godot.
   // The thing is, Interpolator is registered as an internal class. So the major problem here is that some "casting" will
   // be done to suppress compiler warnings
   enum InterpMode { Snap, Full, Ignore };
private:
   /// Exposed variables (properties if you really prefer that. Whatever...)
   // Allows user to specify a target node that is not the parent of this AutoInterpolate
   godot::NodePath m_target_path;


   /// Internal variables
   // This must be saved (serialized) but not exposed. It's meant to hold the list of node paths for
   // nodes that should be interpolated
   godot::Array m_interp_node_path;

   // This will be built by iterating through m_interp_node_path and will cache the valid nodes that
   // can be interpolated by AutoInterpolate in its current mode
   godot::Vector<godot::Node2D*> m_interp2d;
   godot::Vector<godot::Node3D*> m_interp3d;
   

   // What will be interpolated, if at all
   InterpMode m_interp_translation;
   InterpMode m_interp_orientation;
   InterpMode m_interp_scale;

   bool m_interpolate_children;


   // This is what will calculate the interpolation
   godot::Ref<Interpolator> m_interpolator;


   /// Internal functions
   void check_interp_list();

   // Check the target path and node, caching the results
   void check_target();

   void interpolate();


protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* list) const;
   bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   void _notification(int what);

   static void _bind_methods();
public:
   /// Overrides
   virtual godot::PackedStringArray _get_configuration_warnings() const override;

   /// Non exposed
   // While the two functions are technically set/get, those are not meant to be exposed. However they must exist in
   // order to register m_interp_node_path as something to be serialized
   void set_interp_node_path(const godot::Array& arr);
   godot::Array get_interp_node_path() const { return m_interp_node_path; }

   /// Exposed functions
   void teleport_to2d(const godot::Transform2D& t);
   void teleport_to3d(const godot::Transform3D& t);
   void snap_to_target();


   /// Setters/Getters
   void set_target_path(const godot::NodePath& path);
   const godot::NodePath& get_target_path() const { return m_target_path; }

   void set_interpolate_translation(InterpMode mode) { m_interp_translation = mode; }
   InterpMode get_interpolate_translation() const { return m_interp_translation; }

   void set_interpolate_orientation(InterpMode mode) { m_interp_orientation = mode; }
   InterpMode get_interpolate_orientation() const { return m_interp_orientation; }

   void set_interpolate_scale(InterpMode mode) { m_interp_scale = mode; }
   InterpMode get_interpolate_scale() const { return m_interp_scale; }

   void set_interpolate_children(bool enable) { m_interpolate_children = enable; }
   bool get_interpolate_children() const { return m_interpolate_children; }

   AutoInterpolate();
};

VARIANT_ENUM_CAST(AutoInterpolate::InterpMode);

#endif   // AUTO_INTERPOLATE_DISABLED


#ifndef SMOOTH2D_DISABLED

class Smooth2D : public godot::Node2D
{
   GDCLASS(Smooth2D, godot::Node2D);
   using MainInterp = Interpolator::InterpMode;       // To make casting easier
public:
   // Unfortunately I couldn't find a way to use the same Enum from the Interpolator without causing problems in Godot.
   // The thing is, Interpolator is registered as an internal class. So the major problem here is that some "casting" will
   // be done to suppress compiler warnings
   enum InterpMode { Snap, Full, Ignore };
private:
   /// Exposed variables (properties if you really prefer that. Whatever...)
   // Allows user to specify a target node that is not the parent of this AutoInterpolate
   godot::NodePath m_target_path;


   /// Internal variables
   godot::Ref<Interpolator2D> m_interp;

   // What will be interpolated, if at all
   InterpMode m_interp_translation;
   InterpMode m_interp_orientation;
   InterpMode m_interp_scale;

   /// Internal functions
   void check_target();

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   virtual godot::PackedStringArray _get_configuration_warnings() const override;

   /// Exposed functions
   void teleport_to(const godot::Transform2D t);

   void snap_to_target();

   /// Setters/Getters
   void set_target_path(const godot::NodePath& path);
   const godot::NodePath& get_target_path() const { return m_target_path; }

   void set_interpolate_translation(InterpMode mode) { m_interp_translation = mode; }
   InterpMode get_interpolate_translation() const { return m_interp_translation; }

   void set_interpolate_orientation(InterpMode mode) { m_interp_orientation = mode; }
   InterpMode get_interpolate_orientation() const { return m_interp_orientation; }

   void set_interpolate_scale(InterpMode mode) { m_interp_scale = mode; }
   InterpMode get_interpolate_scale() const { return m_interp_scale; }

   Smooth2D();
};

VARIANT_ENUM_CAST(Smooth2D::InterpMode);

#endif   // SMOOTH2D_DISABLED


#ifndef SMOOTH3D_DISABLED

class Smooth3D : public godot::Node3D
{
   GDCLASS(Smooth3D, godot::Node3D);
   using MainInterp = Interpolator::InterpMode;       // To make casting easier
public:
   // Unfortunately I couldn't find a way to use the same Enum from the Interpolator without causing problems in Godot.
   // The thing is, Interpolator is registered as an internal class. So the major problem here is that some "casting" will
   // be done to suppress compiler warnings
   enum InterpMode { Snap, Full, Ignore };
private:
   /// Exposed variables (properties if you really prefer that. Whatever...)
   // Allows user to specify a target node that is not the parent of this AutoInterpolate
   godot::NodePath m_target_path;


   //Interpolator3D m_interpolator;
   godot::Ref<Interpolator3D> m_interp;

   // What will be interpolated, if at all
   InterpMode m_interp_translation;
   InterpMode m_interp_orientation;
   InterpMode m_interp_scale;

   /// Internal functions
   void check_target();

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   virtual godot::PackedStringArray _get_configuration_warnings() const override;

   /// Exposed functions
   void teleport_to(const godot::Transform3D t);

   void snap_to_target();

   /// Setters/Getters
   void set_target_path(const godot::NodePath& path);
   const godot::NodePath& get_target_path() const { return m_target_path; }

   void set_interpolate_translation(InterpMode mode) { m_interp_translation = mode; }
   InterpMode get_interpolate_translation() const { return m_interp_translation; }

   void set_interpolate_orientation(InterpMode mode) { m_interp_orientation = mode; }
   InterpMode get_interpolate_orientation() const { return m_interp_orientation; }

   void set_interpolate_scale(InterpMode mode) { m_interp_scale = mode; }
   InterpMode get_interpolate_scale() const { return m_interp_scale; }

   Smooth3D();
};

VARIANT_ENUM_CAST(Smooth3D::InterpMode);

#endif    // SMOOTH3D_DISABLED

//VARIANT_ENUM_CAST(Interpolator::InterpMode);


#endif    // INTERPOLATION_DISABLED

#endif    // _kehgeneral_autointerpolate_h_included
