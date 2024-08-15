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


#include "autointerpolate.h"

#ifndef INTERPOLATION_DISABLED


#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>

void Interpolator2D::_cycle()
{
   m_from = m_to;
   m_to = m_target->get_global_transform();
}

godot::Transform2D Interpolator2D::calculate2d()
{
   const double alpha = godot::Engine::get_singleton()->get_physics_interpolation_fraction();
   return m_from.interpolate_with(m_to, alpha);
}

void Interpolator2D::snap_to_target()
{
   if (m_target)
   {
      const godot::Transform2D gt = m_target->get_global_transform();
      set_from_to(gt, gt);
   }
}

void Interpolator2D::set_from_to(const godot::Transform2D& from, const godot::Transform2D& to)
{
   m_from = from;
   m_to = to;
}

void Interpolator2D::change_target(godot::Node2D* target)
{
   m_target = target;
   if (target)
   {
      const godot::Transform2D gt = target->get_global_transform();
      set_from_to(gt, gt);
   }
}

void Interpolator2D::apply(godot::Node2D* into, const godot::Transform2D& t, InterpMode translation, InterpMode orientation, InterpMode scale) const
{
   godot::Transform2D result = into->get_global_transform();

   if (translation == Full)
   {
      result.set_origin(t.get_origin());
   }
   else if (translation == Snap)
   {
      result.set_origin(m_to.get_origin());
   }

   const godot::Size2 s = scale == Full ? t.get_scale() : (scale == Snap ? m_to.get_scale() : result.get_scale());
   const double rot = orientation == Full ? t.get_rotation() : (orientation == Snap ? m_to.get_rotation() : result.get_rotation());
   const double cr = godot::Math::cos(rot);
   const double sr = godot::Math::sin(rot);
   
   result[0] = godot::Vector2(cr, sr) * s.x;
   result[1] = godot::Vector2(-sr, cr) * s.y;

   into->set_global_transform(result);
}

Interpolator2D::Interpolator2D(godot::Node2D* t)
{
   m_target = t;
   snap_to_target();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
void Interpolator3D::_cycle()
{
   m_from = m_to;
   m_to = m_target->get_global_transform();
}


godot::Transform3D Interpolator3D::calculate3d()
{
   const double alpha = godot::Engine::get_singleton()->get_physics_interpolation_fraction();
   return m_from.interpolate_with(m_to, alpha);
}

void Interpolator3D::snap_to_target()
{
   if (m_target)
   {
      const godot::Transform3D gt = m_target->get_global_transform();
      set_from_to(gt, gt);
   }
}

void Interpolator3D::set_from_to(const godot::Transform3D& from, const godot::Transform3D& to)
{
   m_from = from;
   m_to = to;
}

void Interpolator3D::change_target(godot::Node3D* target)
{
   m_target = target;
   if (target)
   {
      const godot::Transform3D gt = target->get_global_transform();
      set_from_to(gt, gt);
   }
}

void Interpolator3D::apply(godot::Node3D* into, const godot::Transform3D& t, InterpMode translation, InterpMode orientation, InterpMode scale) const
{  
   godot::Transform3D result = into->get_global_transform();

   if (translation == Full)
   {
      result.set_origin(t.get_origin());
   }
   else if (translation == Snap)
   {
      result.set_origin(m_to.get_origin());
   }

   const godot::Vector3 s = scale == Full ? t.get_basis().get_scale() : (scale == Snap ? m_to.get_basis().get_scale() : result.get_basis().get_scale());
   const godot::Vector3 e = orientation == Full ? t.get_basis().get_euler() : (orientation == Snap ? m_to.get_basis().get_euler() : result.get_basis().get_euler());

   result.set_basis(godot::Basis(e).scaled(s));

   into->set_global_transform(result);
}

Interpolator3D::Interpolator3D(godot::Node3D* t)
{
   m_target = t;
   snap_to_target();
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
#ifndef AUTO_INTERPOLATE_DISABLED
void AutoInterpolate::check_interp_list()
{
   m_interp2d.clear();
   m_interp3d.clear();

   bool changed = false;
   int index = 0;
   bool done = index >= m_interp_node_path.size();
   while (!done)
   {
      const godot::NodePath np = m_interp_node_path[index];

      godot::Node* n = get_node_or_null(np);
      if (n)
      {
         godot::Node2D* n2 = godot::Object::cast_to<godot::Node2D>(n);
         godot::Node3D* n3 = godot::Object::cast_to<godot::Node3D>(n);

         if (n2)
         {
            m_interp2d.append(n2);
         }
         if (n3)
         {
            m_interp3d.append(n3);
         }
         index++;
      }
      else
      {
         changed = true;
         m_interp_node_path.remove_at(index);
      }

      done = index >= m_interp_node_path.size();
   }

   if (changed)
   {
      notify_property_list_changed();
   }

   update_configuration_warnings();
}


void AutoInterpolate::check_target()
{
   godot::Node* tnode = nullptr;
   if (!m_target_path.is_empty())
   {
      tnode = get_node_or_null(m_target_path);
      if (!tnode && m_interpolator.is_valid())
      {
         m_target_path = get_path_to(m_interpolator->get_target());
         tnode = get_node_or_null(m_target_path);
      }
   }

   if (!tnode)
   {
      m_target_path = "";
      tnode = get_parent();
   }

   if (tnode)
   {
      godot::Node2D* t2d = godot::Object::cast_to<godot::Node2D>(tnode);
      godot::Node3D* t3d = godot::Object::cast_to<godot::Node3D>(tnode);

      if (t2d)
      {
         if (m_interpolator.is_valid() && m_interpolator->is_2d())
         {
            // OK, interpolator already exists and it's 2D. Just ensure the target is the correct pointer
            m_interpolator->change_target(t2d);
         }
         else
         {
            // Either the interpolator does not exist or it's in 3D mode.
            m_interpolator = godot::Ref<Interpolator2D>(memnew(Interpolator2D(t2d)));
         }
      }
      else if (t3d)
      {
         if (m_interpolator.is_valid() && !m_interpolator->is_2d())
         {
            // OK, interpolator already exists and it's 3D. Just ensure the target is the correct pointer
            m_interpolator->change_target(t3d);
         }
         else
         {
            // Either the interpolator does not exist or it's in 2D mode.
            m_interpolator = godot::Ref<Interpolator3D>(memnew(Interpolator3D(t3d)));
         }
      }
      else
      {
         m_interpolator = godot::Ref<Interpolator>(nullptr);
      }
   }
   else
   {
      // No valid target - clear things
      m_target_path = "";

      if (m_interpolator.is_valid())
      {
         m_interpolator = godot::Ref<Interpolator>(nullptr);
      }
   }

   if (m_interp_node_path.size() > 0)
   {
      check_interp_list();
   }
   else
   {
      update_configuration_warnings();
   }
}


void AutoInterpolate::interpolate()
{
   if (!m_interpolator.is_valid()) { return; }

   m_interpolator->cycle(false);

   if (m_interpolator->is_2d())
   {
      const godot::Transform2D gt = m_interpolator->calculate2d();

      // Iterate through list of nodes within internal array
      const int icount = m_interp2d.size();
      for (int i = 0; i < icount; i++)
      {
         godot::Node2D* child = m_interp2d[i];
         m_interpolator->apply(child, gt, (MainInterp)m_interp_translation, (MainInterp)m_interp_orientation, (MainInterp)m_interp_scale);
      }

      // Then through children - if enabled
      if (get_interpolate_children())
      {
         const int ccount = get_child_count();
         for (int i = 0; i < ccount; i++)
         {
            godot::Node2D* child = godot::Object::cast_to<godot::Node2D>(get_child(i));
            if (!child) { continue; }

            m_interpolator->apply(child, gt, (MainInterp)m_interp_translation, (MainInterp)m_interp_orientation, (MainInterp)m_interp_scale);
         }
      }
   }
   else
   {
      const godot::Transform3D gt = m_interpolator->calculate3d();

      // Iterate through list of nodes within internal array
      const int icount = m_interp3d.size();
      for (int i = 0; i < icount; i++)
      {
         godot::Node3D* child = m_interp3d[i];
         m_interpolator->apply(child, gt, (MainInterp)m_interp_translation, (MainInterp)m_interp_orientation, (MainInterp)m_interp_scale);
      }

      // Then through children - if enabled
      if (get_interpolate_children())
      {
         const int ccount = get_child_count();
         for (int i = 0; i < ccount; i++)
         {
            godot::Node3D* child = godot::Object::cast_to<godot::Node3D>(get_child(i));
            if (!child) { continue; }

            m_interpolator->apply(child, gt, (MainInterp)m_interp_translation, (MainInterp)m_interp_orientation, (MainInterp)m_interp_scale);
         }
      }
   }
}

void AutoInterpolate::_get_property_list(godot::List<godot::PropertyInfo>* list) const
{
   const int lcount = m_interp_node_path.size();
   for (int i = 0; i < lcount; i++)
   {
      const godot::Variant& val = m_interp_node_path[i];
      if (val.get_type() == godot::Variant::NODE_PATH)
      {
         godot::NodePath np = val;
         list->push_back(godot::PropertyInfo(godot::Variant::NODE_PATH, godot::vformat("interpolate %s", i + 1)));
      }
   }

   list->push_back(godot::PropertyInfo(godot::Variant::NODE_PATH, godot::vformat("interpolate %s", lcount + 1)));
}

bool AutoInterpolate::_set(const godot::StringName& prop_name, const godot::Variant& value)
{
   const godot::PackedStringArray parr = prop_name.split(" ");
   bool retval = false;

   if (parr.size() != 2) { return retval; }

   if (parr[0] == "interpolate" && value.get_type() == godot::Variant::NODE_PATH)
   {
      const int lcount = m_interp_node_path.size();
      const godot::NodePath np = value;

      const int index = parr[1].to_int() - 1;
      if (index < lcount)
      {
         m_interp_node_path[index] = np;
         retval = true;
      }
      else if (index == lcount && !np.is_empty())
      {
         m_interp_node_path.push_back(np);
         retval = true;
      }
   }

   if (retval)
   {
      if (is_inside_tree())
      {
         check_interp_list();
      }
      else
      {
         callable_mp(this, &AutoInterpolate::check_interp_list).call_deferred();
      }
      notify_property_list_changed();
   }

   return retval;
}

bool AutoInterpolate::_get(const godot::StringName& prop_name, godot::Variant& out_value) const
{
   const godot::PackedStringArray parr = prop_name.split(" ");
   bool retval = false;

   if (parr.size() == 2)
   {
      if (parr[0] == "interpolate")
      {
         const int lcount = m_interp_node_path.size();
         const int index = parr[1].to_int() - 1;
         if (index < lcount)
         {
            out_value = m_interp_node_path[index];
            retval = true;
         }
         else if (index == lcount)
         {
            out_value = godot::NodePath();
            retval = true;
         }
      }
   }

   return retval;
}


void AutoInterpolate::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_ENTER_TREE:
      {
         // Re-parenting also triggers this notification
         check_target();
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         interpolate();
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         if (m_interpolator.is_valid())
         {
            m_interpolator->cycle(true);
         }
         
      } break;


      case NOTIFICATION_READY:
      {
         set_process_internal(true);
         set_physics_process_internal(true);
      } break;
   }
}


void AutoInterpolate::_bind_methods()
{
   using namespace godot;
   
   // The functions
   ClassDB::bind_method(D_METHOD("teleport_to2d", "transform"), &AutoInterpolate::teleport_to2d);
   ClassDB::bind_method(D_METHOD("teleport_to3d", "transform"), &AutoInterpolate::teleport_to3d);
   ClassDB::bind_method(D_METHOD("snap_to_target"), &AutoInterpolate::snap_to_target);
   ClassDB::bind_method(D_METHOD("_set_interp_node_path", "arr"), &AutoInterpolate::set_interp_node_path);
   ClassDB::bind_method(D_METHOD("_get_interp_node_path"), &AutoInterpolate::get_interp_node_path);
   ClassDB::bind_method(D_METHOD("set_target", "node_path"), &AutoInterpolate::set_target_path);
   ClassDB::bind_method(D_METHOD("get_target"), &AutoInterpolate::get_target_path);
   ClassDB::bind_method(D_METHOD("set_interpolate_translation", "mode"), &AutoInterpolate::set_interpolate_translation);
   ClassDB::bind_method(D_METHOD("get_interpolate_translation"), &AutoInterpolate::get_interpolate_translation);
   ClassDB::bind_method(D_METHOD("set_interpolate_orientation", "mode"), &AutoInterpolate::set_interpolate_orientation);
   ClassDB::bind_method(D_METHOD("get_interpolate_orientation"), &AutoInterpolate::get_interpolate_orientation);
   ClassDB::bind_method(D_METHOD("set_interpolate_scale", "mode"), &AutoInterpolate::set_interpolate_scale);
   ClassDB::bind_method(D_METHOD("get_interpolate_scale"), &AutoInterpolate::get_interpolate_scale);
   ClassDB::bind_method(D_METHOD("set_interpolate_children", "enable"), &AutoInterpolate::set_interpolate_children);
   ClassDB::bind_method(D_METHOD("get_interpolate_children"), &AutoInterpolate::get_interpolate_children);

   // The variables
   ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "_interp_node_path_", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE | PROPERTY_USAGE_INTERNAL), "_set_interp_node_path", "_get_interp_node_path");
   ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target"), "set_target", "get_target");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_translation", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_translation", "get_interpolate_translation");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_orientation", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_orientation", "get_interpolate_orientation");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_scale", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_scale", "get_interpolate_scale");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "interpolate_children"), "set_interpolate_children", "get_interpolate_children");

   BIND_ENUM_CONSTANT(Snap);
   BIND_ENUM_CONSTANT(Full);
   BIND_ENUM_CONSTANT(Ignore);
}

godot::PackedStringArray AutoInterpolate::_get_configuration_warnings() const
{
   godot::PackedStringArray ret;

   if (!m_interpolator.is_valid())
   {
      // In here there is absolutely no interpolator.
      ret.append("The target node is invalid. Consider attaching this into a node derived from 2D or 3D. Alternatively set the target to a valid node path.");
   }
   else
   {
      const int lcount = m_interp_node_path.size();
      if (lcount > 0)
      {
         if (m_interpolator->is_2d() && m_interp2d.size() != lcount)
         {
            ret.append("Interpolation mode is 2D but at least one of the nodes assigned for interpolation is not 2D.");
         }
         if (!m_interpolator->is_2d() && m_interp3d.size() != lcount)
         {
            ret.append("Interpolation mode is 3D but at least onf of the nodes assigned for interpolation is not 3D.");
         }
      }
   }

   return ret;
}


void AutoInterpolate::set_interp_node_path(const godot::Array& arr)
{
   m_interp_node_path = arr;
}


void AutoInterpolate::teleport_to2d(const godot::Transform2D& t)
{
   if (m_interpolator.is_valid() && m_interpolator->is_2d())
   {
      m_interpolator->set_from_to(t, t);
   }
}

void AutoInterpolate::teleport_to3d(const godot::Transform3D& t)
{
   if (m_interpolator.is_valid() && !m_interpolator->is_2d())
   {
      m_interpolator->set_from_to(t, t);
   }
}

void AutoInterpolate::snap_to_target()
{
   if (m_interpolator.is_valid())
   {
      m_interpolator->snap_to_target();
   }
}


void AutoInterpolate::set_target_path(const godot::NodePath& path)
{
   m_target_path = path;

   if (is_inside_tree())
   {
      check_target();
   }
   else
   {
      callable_mp(this, &AutoInterpolate::check_target).call_deferred();
   }
}


AutoInterpolate::AutoInterpolate()
{
   m_interp_translation = InterpMode::Full;
   m_interp_orientation = InterpMode::Full;
   m_interp_scale = InterpMode::Ignore;
   m_interpolate_children = true;
}

#endif  // AUTO_INTERPOLATE_DISABLED


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
#ifndef SMOOTH2D_DISABLED
void Smooth2D::check_target()
{
   godot::Node2D* tnode = nullptr;
   if (!m_target_path.is_empty())
   {
      tnode = godot::Object::cast_to<godot::Node2D>(get_node_or_null(m_target_path));
      if (!tnode && m_interp->get_target())
      {
         m_target_path = get_path_to(m_interp->get_target());
         tnode = godot::Object::cast_to<godot::Node2D>(get_node_or_null(m_target_path));
      }

      if (tnode == this)
      {
         // Well, attempting to set itself as a target is not good!
         tnode = nullptr;
      }
   }

   if (!tnode)
   {
      m_target_path = "";
      tnode = godot::Object::cast_to<godot::Node2D>(get_parent());
   }

   if (tnode)
   {
      m_interp->change_target(tnode);
   }
   else
   {
      m_target_path = "";
      m_interp->change_target(nullptr);
   }

   update_configuration_warnings();
}


void Smooth2D::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_READY:
      case NOTIFICATION_VISIBILITY_CHANGED:
      {
         const bool v = is_visible_in_tree();
         set_process_internal(v);
         set_physics_process_internal(v);
      } break;

      case NOTIFICATION_ENTER_TREE:
      {
         check_target();
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         if (m_interp->get_target())
         {
            const godot::Transform2D gt = m_interp->calculate2d();
            m_interp->apply(this, gt, (MainInterp)m_interp_translation, (MainInterp)m_interp_orientation, (MainInterp)m_interp_scale);
         }
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         if (m_interp->get_target())
         {
            m_interp->cycle(true);
         }
      } break;
   }
}

void Smooth2D::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("set_target", "node_path"), &Smooth2D::set_target_path);
   ClassDB::bind_method(D_METHOD("get_target"), &Smooth2D::get_target_path);
   ClassDB::bind_method(D_METHOD("set_interpolate_translation", "mode"), &Smooth2D::set_interpolate_translation);
   ClassDB::bind_method(D_METHOD("get_interpolate_translation"), &Smooth2D::get_interpolate_translation);
   ClassDB::bind_method(D_METHOD("set_interpolate_orientation", "mode"), &Smooth2D::set_interpolate_orientation);
   ClassDB::bind_method(D_METHOD("get_interpolate_orientation"), &Smooth2D::get_interpolate_orientation);
   ClassDB::bind_method(D_METHOD("set_interpolate_scale", "mode"), &Smooth2D::set_interpolate_scale);
   ClassDB::bind_method(D_METHOD("get_interpolate_scale"), &Smooth2D::get_interpolate_scale);

   ClassDB::bind_method(D_METHOD("teleport_to", "target"), &Smooth2D::teleport_to);
   ClassDB::bind_method(D_METHOD("snap_to_target"), &Smooth2D::snap_to_target);


   ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target"), "set_target", "get_target");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_translation", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_translation", "get_interpolate_translation");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_orientation", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_orientation", "get_interpolate_orientation");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_scale", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_scale", "get_interpolate_scale");

   BIND_ENUM_CONSTANT(Snap);
   BIND_ENUM_CONSTANT(Full);
   BIND_ENUM_CONSTANT(Ignore);
}

godot::PackedStringArray Smooth2D::_get_configuration_warnings() const
{
   godot::PackedStringArray ret;

   if (!m_interp->get_target())
   {
      if (m_target_path.is_empty())
      {
         ret.append("This is not attached to a 2D node. If this is intentional consider manually setting the target to a 2D node.");
      }
      else
      {
         ret.append("The target is not set to a 2D node.");
      }
   }


   return ret;
}


void Smooth2D::teleport_to(const godot::Transform2D t)
{
   m_interp->set_from_to(t, t);
}

void Smooth2D::snap_to_target()
{
   m_interp->snap_to_target();
}


void Smooth2D::set_target_path(const godot::NodePath& path)
{
   m_target_path = path;

   if (is_inside_tree())
   {
      check_target();
   }
   else
   {
      callable_mp(this, &Smooth2D::check_target).call_deferred();
   }
}

Smooth2D::Smooth2D()
{
   m_interp.instantiate();

   m_interp_translation = InterpMode::Full;
   m_interp_orientation = InterpMode::Full;
   m_interp_scale = InterpMode::Ignore;
}

#endif    // SMOOTH2D_DISABLED


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
#ifndef SMOOTH3D_DISABLED
void Smooth3D::check_target()
{
   godot::Node3D* tnode = nullptr;
   if (!m_target_path.is_empty())
   {
      tnode = godot::Object::cast_to<godot::Node3D>(get_node_or_null(m_target_path));
      if (!tnode && m_interp->get_target())
      {
         m_target_path = get_path_to(m_interp->get_target());
         tnode = godot::Object::cast_to<godot::Node3D>(get_node_or_null(m_target_path));
      }

      if (tnode == this)
      {
         // Attempting to set itself as a target is not good
         tnode = nullptr;
      }
   }

   if (!tnode)
   {
      m_target_path = "";
      tnode = godot::Object::cast_to<godot::Node3D>(get_parent());
   }

   if (tnode)
   {
      m_interp->change_target(tnode);
   }
   else
   {
      m_target_path = "";
      m_interp->change_target(nullptr);
   }

   update_configuration_warnings();
}

void Smooth3D::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_READY:
      case NOTIFICATION_VISIBILITY_CHANGED:
      {
         const bool v = is_visible_in_tree();
         set_process_internal(v);
         set_physics_process_internal(v);
      } break;

      case NOTIFICATION_ENTER_TREE:
      {
         check_target();
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         if (m_interp->get_target())
         {
            const godot::Transform3D gt = m_interp->calculate3d();
            m_interp->apply(this, gt, (MainInterp)m_interp_translation, (MainInterp)m_interp_orientation, (MainInterp)m_interp_scale);
         }
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         if (m_interp->get_target());
         {
            m_interp->cycle(true);
         }
      } break;
   }
}

void Smooth3D::_bind_methods()
{
   using namespace godot;

   ClassDB::bind_method(D_METHOD("set_target", "node_path"), &Smooth3D::set_target_path);
   ClassDB::bind_method(D_METHOD("get_target"), &Smooth3D::get_target_path);
   ClassDB::bind_method(D_METHOD("set_interpolate_translation", "mode"), &Smooth3D::set_interpolate_translation);
   ClassDB::bind_method(D_METHOD("get_interpolate_translation"), &Smooth3D::get_interpolate_translation);
   ClassDB::bind_method(D_METHOD("set_interpolate_orientation", "mode"), &Smooth3D::set_interpolate_orientation);
   ClassDB::bind_method(D_METHOD("get_interpolate_orientation"), &Smooth3D::get_interpolate_orientation);
   ClassDB::bind_method(D_METHOD("set_interpolate_scale", "mode"), &Smooth3D::set_interpolate_scale);
   ClassDB::bind_method(D_METHOD("get_interpolate_scale"), &Smooth3D::get_interpolate_scale);

   ClassDB::bind_method(D_METHOD("teleport_to", "target"), &Smooth3D::teleport_to);
   ClassDB::bind_method(D_METHOD("snap_to_target"), &Smooth3D::snap_to_target);


   ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "target"), "set_target", "get_target");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_translation", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_translation", "get_interpolate_translation");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_orientation", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_orientation", "get_interpolate_orientation");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "interpolate_scale", PROPERTY_HINT_ENUM, "Snap,Full,Ignore"), "set_interpolate_scale", "get_interpolate_scale");

   BIND_ENUM_CONSTANT(Snap);
   BIND_ENUM_CONSTANT(Full);
   BIND_ENUM_CONSTANT(Ignore);
}

godot::PackedStringArray Smooth3D::_get_configuration_warnings() const
{
   godot::PackedStringArray ret;

   if (!m_interp->get_target())
   {
      if (m_target_path.is_empty())
      {
         ret.append("This is not attached to a 3D node. If this is intentional consider manually setting the target to a 3D node.");
      }
      else
      {
         ret.append("The target is not set to a 3D node.");
      }
   }

   return ret;
}

void Smooth3D::teleport_to(const godot::Transform3D t)
{
   m_interp->set_from_to(t, t);
}

void Smooth3D::snap_to_target()
{
   m_interp->snap_to_target();
}

void Smooth3D::set_target_path(const godot::NodePath& path)
{
   m_target_path = path;

   if (is_inside_tree())
   {
      check_target();
   }
   else
   {
      callable_mp(this, &Smooth3D::check_target).call_deferred();
   }
}

Smooth3D::Smooth3D()
{
   m_interp.instantiate();

   m_interp_translation = InterpMode::Full;
   m_interp_orientation = InterpMode::Full;
   m_interp_scale = InterpMode::Ignore;
}

#endif   // SMOOTH3D_DISABLED


#endif   // INTERPOLATION_DISABLED
