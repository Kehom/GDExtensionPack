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

#ifndef _kehdbghelper_line3d_h_included
#define _kehdbghelper_line3d_h_included 1


#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/templates/vector.hpp>


#ifndef LINE3D_DISABLED

// This is the internal class and will be initialized by the exposed singleton. All the tasks related
// to drawing the lines will be performed by this node
#ifndef EMPTY_LINE3D
class _LineNode : public godot::Node3D
{
   GDCLASS(_LineNode, godot::Node3D);
private:
   struct LineData
   {
      godot::Vector3 p0;
      godot::Vector3 p1;
      godot::Color color;

      LineData() {}  // Default constructor to make godot::Vector<> happy
      LineData(const godot::Vector3& point0, const godot::Vector3& point1, const godot::Color& c)
         : p0(point0), p1(point1), color(c) {}
   };

   struct TimedLine
   {
      LineData line;
      float time;

      TimedLine() {}    // Default constructor to make godot::Vector<> happy
      TimedLine(const godot::Vector3& p0, const godot::Vector3& p1, float t, const godot::Color& c) : line(p0, p1, c), time(t) {}
   };

   /// Internal variables
   // This will be used to generate the "procedural meshes". Well, just lines. This resource will be attached
   // into a MeshInstance3D node, which is then attached into this one.
   godot::Ref<godot::ImmediateMesh> m_mesh;

   // Hold lines that are meant to be drawn in a single frame then that's it
   godot::Vector<LineData> m_one_frame;

   // Hold timed lines. Those will be redrawn through frames until time expires
   godot::Vector<TimedLine> m_timed;

   /// Exposed variables

   /// Internal functions
   void handle_queue(float dt);

   /// Event handlers

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides

   /// Exposed functions
   bool is_enabled() const { return is_processing_internal() || is_physics_processing_internal(); }
   void enable();
   void disable();
   void set_enabled(bool e);

   void add_line(const godot::Vector3& p0, const godot::Vector3& p1, const godot::Color& color);
   void add_timed_line(const godot::Vector3& p0, const godot::Vector3& p1, float time, const godot::Color& color);

   // Clear all lines
   void clear_lines();

   /// Setters/Getters

   /// Public non exposed functions
   _LineNode();
};

#endif   // EMPTY_LINE3D


// This is the exposed singleton.
class Line3D : public godot::Object
{
   GDCLASS(Line3D, godot::Object);
private:
   /// Internal variables
   static Line3D* s_singleton;

#ifndef EMPTY_LINE3D
   _LineNode* m_line_node;
#else
   // When "empty", keep track of enabled state so if any code that depends on this logic doesn't fail
   bool m_enabled;
#endif

   /// Exposed variables

   /// Internal functions
   void setup();

   /// Event handlers

protected:
   static void _bind_methods();
public:
   /// Overrides

   /// Exposed functions
   // Indeed not setting this as a const function while ideally it should be. However if this function is the very first
   // Line3D one called by the user then the internal m_line_node will not be initialized yet. While I could add a warning
   // that this can't be the first function to be called, very few users actually read manuals. So... yeah, const is not
   // exactly a possibility here.
   bool is_enabled();

   // Enable processing of the lines
   void enable();

   // Disable processing of Lines. If there are queue lines those wont be deleted, however no drawing will occur.
   void disable();

   // Change the enabled state of the line processing
   void set_enabled(bool e);


   // Add a one frame line
   void add_line(const godot::Vector3& p0, const godot::Vector3& p1, const godot::Color& color = godot::Color(1.0, 1.0, 1.0, 1.0));

   // Based on the vector direction and its length, as well as a location, calculate both line segment end points and add it
   void add_from_vector(const godot::Vector3& point, const godot::Vector3 vector, const godot::Color& color = godot::Color(1.0, 1.0, 1.0, 1.0));

   // Line added by this will remain by 'time' seconds then will disappear
   void add_timed_line(const godot::Vector3& p0, const godot::Vector3& p1, float time, const godot::Color& color = godot::Color(1.0, 1.0, 1.0, 1.0));

   // Based on the vector direction and its length, as well as a location, calculate both line segment end points and add a timed line
   void add_timed_from_vector(const godot::Vector3& point, const godot::Vector3& vector, float time, const godot::Color& color = godot::Color(1.0, 1.0, 1.0, 1.0));

   // Clear all lines
   void clear_lines();

   /// Setters/Getters

   /// Public non exposed functions
   void register_project_settings();

   static Line3D* get_singleton() { return s_singleton; }
   Line3D();
};


#endif //LINE3D_DISABLED

#endif //_kehdbghelper_line3d_h_included

