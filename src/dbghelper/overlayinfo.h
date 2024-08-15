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

#ifndef _kehgeneral_overlay_info_h_included
#define _kehgeneral_overlay_info_h_included 1


#include <godot_cpp/classes/box_container.hpp>
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/theme.hpp>

#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>


// Forward declares
namespace godot
{
   class HBoxContainer;
   class Label;
   class PanelContainer;
   class VBoxContainer;
}

#ifndef OVERLAY_INFO_DISABLED


#ifndef EMPTY_OVERLAY_INFO

/// This will be internally created and perform most of the tasks
class _OverlayControl : public godot::CanvasLayer
{
   GDCLASS(_OverlayControl, godot::CanvasLayer);
public:
   using AliMode = godot::BoxContainer::AlignmentMode;
private:
   // The original idea here was to use scene tree timers to "automatically" handle removal of the
   // timed labels. However things became rather finicky from the GDExtension part. There is a big
   // chance that the tree has not finished initialization when the OverlayInfo is used. Because of
   // that, requesting to add a timed label will create an instance of this struct and added into an
   // internal container. If this control is already inside the tree then processing will enabled.
   // When the ready notification is given, it will test if the container is empty or not. If it's
   // not then it will ensure the processing will be enabled.
   struct LabelInfo
   {
      float time;
      godot::Label* label;

      // Unfortunately must provide a default constructor otherwise godot::Vector can't be used
      LabelInfo(float t = 0.0, godot::Label* lbl = nullptr) :
         time(t),
         label(lbl)
      {}
   };


   /// Internal variables
   // Use a background panel to help with readability of the text
   godot::PanelContainer* m_background;

   // Keep a "main box" to allow setting horizontal alignment
   godot::HBoxContainer* m_haligner;

   // And this inner "main box" helps with vertical alignment
   godot::VBoxContainer* m_valigner;

   // The box that will hold the labels
   godot::VBoxContainer* m_label_box;

   // This map should help perform manipulations easier on non timed labels
   godot::HashMap<godot::StringName, godot::Label*> m_label_node;

   // And this is an internal container to deal with timed labels. Or to help delay adding non timed labels
   // when requesting one before this node is inside the tree
   godot::Vector<LabelInfo> m_timed;

   /// Exposed variables

   /// Internal functions
   void enable();
   void disable();

   void handle_timed(float dt);

   void check_auto_show();
   void check_auto_hide();

   /// Event handlers

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void set_label(const godot::StringName& label_id, const godot::String& text);
   void remove_label(const godot::StringName& label_id);
   void clear_labels();
   bool has_any_labels();
   void add_timed_label(const godot::String& text, real_t timeout);
   void set_halign(AliMode align);
   void set_valign(AliMode align);

   void assign_theme(const godot::Ref<godot::Theme>& theme);


   _OverlayControl();
};

#endif   // EMPTY_OVERLAY_INFO



/// Provides a "box" with a few functions to add labels of text into it. There are two ways to add
/// labels into that box:
/// - With specified ID, meaning that those labels will persist on screen until explicitly removed
/// - Timed, meaning that those pieces of text will be automatically removed from screen after the
///   specified amount of time has elapsed.
/// The position of this box can be changed, left/center/right and top/center/bottom
/// When a timed label is added time counting is performed within the idle process by default. It
/// can be changed within the project settings under "Keh Extension Pack/Overlay Info/Use Physics Process"
class OverlayInfo : public godot::Object
{
   GDCLASS(OverlayInfo, godot::Object);
   using AliMode = godot::BoxContainer::AlignmentMode;
private:
   /// Internal variables
   static OverlayInfo* s_singleton;

#ifndef EMPTY_OVERLAY_INFO
   _OverlayControl* m_overlay;
#else
   // If "empty" must keep track of "visibility" so code depending on this logic doesn't fail
   bool m_visible;
#endif

   /// Exposed variables
   // Regardless if this is "empty" or not, keep the theme as it's exposed as a property
   godot::Ref<godot::Theme> m_theme;

   /// Internal functions
   void setup();

   /// Event handlers

protected:
   static void _bind_methods();
public:
   /// Overrides

   /// Exposed functions
   // Create a non temporary label and assign to it the incoming text.
   void set_label(const godot::StringName& label_id, const godot::String& text);

   // Removes the specified non temporary label from the UI
   void remove_label(const godot::StringName& label_id);

   // Remove all non temporary labels from the UI
   void clear_labels();

   // Create a temporary text that will stay for the incoming amount of seconds. Timed labels cannot be changed after they are created
   void add_timed_label(const godot::String& text, real_t timeout);

   // Horizontal alignment to the left
   void set_horizontal_align_left();

   // Horizontal alignment to the center
   void set_horizontal_align_center();

   // Horizontal alignment to the right
   void set_horizontal_align_right();

   // Vertical align to the top
   void set_vertical_align_top();

   // Vertical align to the center
   void set_vertical_align_center();

   // Vertical align to the bottom
   void set_vertical_align_bottom();

   // Returns true if there is at least one label in the internal panel
   bool has_any_label();

   // Returns true if the main panel is currently visible - indeed not setting this as a const while ideally it should be.
   // However if this function is the very first one called by the user then the m_overlay will not be initialized yet.
   // While I could add a warning that this can't be the first function to be called, very few users actually read manuals.
   // So... yeah, const is not exactly a possibility here.
   bool is_visible();

   // Change the visibility of the main panel
   void set_visible(bool visible);

   // Hmmm...
   void toggle_visiblity();

   // Show the main panel. Same thing as calling set_visible(true)
   void show() { set_visible(true); }

   // Hide the main panel. Same thing as calling set_visible(false)
   void hide() { set_visible(false); }


   /// Setters/Getters
   void set_theme(const godot::Ref<godot::Theme>& theme);
   godot::Ref<godot::Theme> get_theme() const;

   /// Public non exposed functions
   void register_project_settings();

   static OverlayInfo* get_singleton() { return s_singleton; }
   OverlayInfo();
};


#endif  //OVERLAY_INFO_DISABLED

#endif  //_kehgeneral_overlay_info_h_included
