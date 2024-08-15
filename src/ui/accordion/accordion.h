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

#ifndef _kehui_accordion_h_included
#define _kehui_accordion_h_included 1

#ifndef ACCORDION_DISABLED

#include <godot_cpp/classes/container.hpp>
#include <godot_cpp/classes/input_event.hpp>       // for some reason Accordion fails to compile without this!
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>

// Forward declares
namespace godot
{
   class Button;
   class Curve;
   class Font;
   class HScrollBar;
   class InputEventMouseButton;
   class InputEventMouseMotion;
   class InputEventPanGesture;
   class StyleBox;
   class Texture2D;
   class VScrollBar;
}


/// In order for the accordion to work correctly it needs proper information regarding the minimum size of
/// the direct children. The *Container* Control nodes are the more reliable in this regard. However when
/// animation is desired, Containers wont work simply because those nodes can't become smaller than the reported
/// minimum ones. Yet "ordinary" Control nodes wont work either because they don't fail to correctly report
/// the necessary size when they are visible.
/// If Godot provided a Slot system for nodes then this problem could be more easily solved. Since there isn't,
/// the approach here is to create a node specifically to represent Accordion Items, which is the class bellow.
/// To make things a lot easier to deal with, Accordion will require its direct children to be AccordionItem's
class AccordionItem : public godot::Container
{
   GDCLASS(AccordionItem, godot::Container);
private:
   /// Internal variables
   // While the button pointer will be held here, it will actually be added as an internal child of the Accordion
   godot::Button* m_button;

   // Cache minimum size required to show the Control - this will be used mostly for the animation and will be calculated
   // within the "sort_children" notification. Yet the "get_minimum_size()" function will return a value based on the
   // animation state.
   godot::Size2i m_min_size;

   // Cache minimum size when contents are hidden
   godot::Size2i m_min_bt_size;

   // Instead of keeping the spacing as a theme item specific to the AccordionItem, "inherit" said value from the
   // parent Accordion. While it requires slightly more upkeep, final usability becomes way more practical
   // Just to make clear, this is the vertical spacing between each Control
   int m_separation;

   // This will be provided by the parent Accordion
   godot::Ref<godot::StyleBox> m_background;
   
   // Also provided by the parent Accordion. If this is true then prevent toggling state when animation is in progress
   bool m_anim_block;

   // The animation related values wont be directly exposed. Instead the parent Accordion will set those. By doing
   // this usage becomes easier as changing from a single place (the Accordion) will automatically assign the values
   // into all items. In a way, easier to reach consistency
   float m_hide_anim_time;
   godot::Ref<godot::Curve> m_hide_curve;
   float m_show_anim_time;
   godot::Ref<godot::Curve> m_show_curve;
   bool m_animate_on_physics;

   struct AnimState
   {
      float time;           // Current elapsed time
      float target_time;    // How much time current animation should last

      float start_height;      // Height at the starting of the animation
      float end_height;        // Height at the end of the animation
      float current_height;    // Height at current state

      bool is_hiding() const { return start_height > end_height; }

      bool update(float dt)
      {
         time += dt;
         return time >= target_time;
      }

      float calculate(const godot::Ref<godot::Curve>& curve);

      AnimState() :
         time(0.0),
         target_time(0.0),
         start_height(0.0),
         end_height(0.0),
         current_height(0.0)
      {}
   } m_animation;

   // This will cache the margin settings obtained from the background style
   struct ContentMargin
   {
      int left;
      int top;
      int right;
      int bottom;

      ContentMargin() :
         left(6),
         top(0),
         right(6),
         bottom(0)
      {}
   } m_content_margin;

   /// Exposed variables
   godot::String m_title;     // Button text. If blank then use item's node name


   /// Internal functions
   void sort_children();

   // Tells if animation is already in progress or not
   bool is_animating() const { return is_physics_processing_internal() || is_processing_internal(); }

   // Setup data so animation occurs
   void start_animation(bool active);

   // Update the animation state - if any
   void handle_animation(float dt);

   /// Event handlers
   void on_button_toggled(bool on);

protected:
   //void _get_property_list(godot::List<godot::PropertyInfo>* out_list) const;
   //bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   //bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   godot::Vector2 _get_minimum_size() const;     // "override"

   // This is only for Nodes and can be used to add warning messages within the hierarchy if there is a problem.
   virtual godot::PackedStringArray _get_configuration_warnings() const override;

   /// Exposed functions

   /// Setters/Getters
   void set_title(const godot::String& val);
   godot::String get_title() const { return m_title; }

   void set_icon(const godot::Ref<godot::Texture2D>& icon);
   godot::Ref<godot::Texture2D> get_icon() const;

   // The next two are meant to change pressed state of the toggle button. 
   void set_active(bool val) { change_active_state(val, true); }
   bool get_active() const;

   // Those are for the button tooltip
   void set_button_tooltip(const godot::String& text);
   godot::String get_button_tooltip() const;

   // Allow enabling/disabling the button
   void set_enabled(bool val);
   bool get_enabled() const;

   /// Public non exposed functions
   godot::Button* get_button() const;

   // It's possible the parent Accordion will request the active state to be changed. This function is used to perform
   // the tasks to toggle this state while notifying back only if the incoming flag requests it.
   void change_active_state(bool active, bool notify);
   
   // Allow parent Accordion to set the separation
   void assign_separation(int val);

   void setup_animation(float hide_time, const godot::Ref<godot::Curve> hide_curve, float show_time, const godot::Ref<godot::Curve>& show_curve, bool anim_on_physics);
   void setup_alignment(godot::HorizontalAlignment text, godot::HorizontalAlignment icon);
   void setup_background(const godot::Ref<godot::StyleBox>& background);
   void set_anim_block(bool val) { m_anim_block = val; }


   AccordionItem();
};



/// Each child of this container generates a button that when activated (pressed) will reveal the
/// contents of that child. Releasing the button (toggle) will then hide the corresponding contents.
class Accordion : public godot::Container
{
   GDCLASS(Accordion, godot::Container);

private:
   struct ThemeCache
   {
      godot::Ref<godot::StyleBox> background;
      godot::Ref<godot::StyleBox> content_background;

      godot::Ref<godot::StyleBox> button_normal;
      godot::Ref<godot::StyleBox> button_hovered;
      godot::Ref<godot::StyleBox> button_pressed;
      godot::Ref<godot::StyleBox> button_disabled;
      godot::Ref<godot::StyleBox> button_focus;

      godot::Ref<godot::StyleBox> hscroll;
      godot::Ref<godot::StyleBox> vscroll;
      godot::Ref<godot::StyleBox> scroll_focus;
      godot::Ref<godot::StyleBox> scroll_grabber;
      godot::Ref<godot::StyleBox> scroll_grabber_hl;
      godot::Ref<godot::StyleBox> scroll_grabber_pressed;

      godot::Ref<godot::Font> font;

      godot::Color font_normal;
      godot::Color font_hovered;
      godot::Color font_focus;
      godot::Color font_pressed;
      godot::Color font_hover_pressed;
      godot::Color font_disabled;
      godot::Color font_outline;

      godot::Color icon_normal;
      godot::Color icon_pressed;
      godot::Color icon_hover;
      godot::Color icon_hover_pressed;
      godot::Color icon_focus;
      godot::Color icon_disabled;

      int font_size;
      int font_outline_size;
      int button_space;         // spacing between toggle buttons
      int inter_space;          // space between a child Control and the next toggle button
   } m_theme_cache;

   // Some data related to touch screen/dragging - the entire code related to this aspect has been taken from
   // the 'scene/gui/scroll_container' (both .h and .cpp) found in the Godot source code
   struct DragTouchData
   {
      godot::Vector2 drag_speed;
      godot::Vector2 drag_accum;
      godot::Vector2 drag_from;
      godot::Vector2 last_drag_accum;

      float time_since_motion = 0.0f;
      bool drag_touching = false;
      bool drag_touching_deaccel = false;
      bool beyond_deadzone = false;
   } m_drag_touch;


   /// Internal variables
   // This is not meant to be stored, only cache "valid pages"
   godot::Vector<AccordionItem*> m_item;


   // Allow scrolling
   godot::HScrollBar* m_hscroll;
   godot::VScrollBar* m_vscroll;

   // This will cache the required size to fully display everything that is should be shown
   // That is, buttons plus enabled contents.
   godot::Vector2 m_required_size;

   /// Exposed variables
   // If this is true then a single "page" can be shown at a time
   bool m_exclusive;

   // If this is true then children will block the toggle button when animation is in progress
   bool m_anim_block;

   // This will be applied to all items
   godot::HorizontalAlignment m_text_align;
   godot::HorizontalAlignment m_icon_align;

   // Determine the amount of seconds required to hide a content item. If 0 or negative then animation is disabled
   float m_hide_anim_time;

   // If hide animation is enabled then this curve can be used to modify the behavior
   godot::Ref<godot::Curve> m_hide_anim_curve;

   // Determine the amount of seconds required to show a content item. If 0 or negative the animation is disabled
   float m_show_anim_time;

   // If show animation is enabled then this curve can be used to modify the behavior
   godot::Ref<godot::Curve> m_show_anim_curve;

   // If this is true then animation will be updated during physics updates instead
   bool m_animate_on_physics;

   // Determine visibility of scroll bars to either automatic or always visible.
   bool m_always_show_vscroll;
   bool m_always_show_hscroll;

   // The scrolling dead zone - used primarily when dealing with touch screen
   int m_deadzone;

   // Related to language settings of the buttons
   TextDirection m_text_direction;
   godot::String m_language;


   /// Internal functions
   // The next two functions are meant to apply styling into internal controls
   void apply_style_to_button(godot::Button* bt);
   void apply_internal_styles();
   void apply_alignment();
   void apply_anim_setup();


   // Ensure internal data matches children
   void refresh_contents();


   // This will be used fo ra "first pass" in order to calculate the required "internal size".
   // The check_layout() function will need the information calculated by this function
   void calculate_req_size();

   // Deal with the distribution of the children
   void check_layout();

   // Part of input handling, specially touch screen devices
   void cancel_drag();

   // Help make code slightly more readable by separating mouse button and mouse motion handling
   void handle_mouse_button(const godot::Ref<godot::InputEventMouseButton>& mb);
   void handle_mouse_motion(const godot::Ref<godot::InputEventMouseMotion>& mm);
   void handle_pan_gesture(const godot::Ref<godot::InputEventPanGesture>& pg);

   // Deal with drag from touch screen
   void handle_drag_touching(float dt);


   /// Event handlers
   // Every time a child node is added, removed or moved a signal is generated. This function handles that
   void on_child_order_changed();

   void on_scroll_changed(float val);


protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* out_list) const;
   //bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   //bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   godot::Vector2 _get_minimum_size() const;     // "override"

   virtual void _gui_input(const godot::Ref<godot::InputEvent>& event) override;
   
   virtual godot::PackedStringArray _get_configuration_warnings() const override;

   /// Exposed functions


   /// Setters/Getters
   void set_exclusive(bool val);
   bool get_exclusive() const { return m_exclusive; }

   void set_anim_block(bool val);
   bool get_anim_block() const { return m_anim_block; }

   void set_title_alignment(godot::HorizontalAlignment align);
   godot::HorizontalAlignment get_title_alignment() const { return m_text_align; }

   void set_icon_alignment(godot::HorizontalAlignment align);
   godot::HorizontalAlignment get_icon_alignment() const { return m_icon_align; }

   void set_hide_anim_time(float val);
   float get_hide_anim_time() const { return m_hide_anim_time; }

   void set_hide_anim_curve(const godot::Ref<godot::Curve>& val);
   godot::Ref<godot::Curve> get_hide_anim_curve() const;

   void set_show_anim_time(float val);
   float get_show_anim_time() const { return m_show_anim_time; }

   void set_show_anim_curve(const godot::Ref<godot::Curve>& val);
   godot::Ref<godot::Curve> get_show_anim_curve() const;

   void set_animate_on_physics(bool val);
   bool get_animate_on_physics() const { return m_animate_on_physics; }

   void set_always_show_vscroll(bool val);
   bool get_always_show_vscroll() const { return m_always_show_vscroll; }
   void set_always_show_hscroll(bool val);
   bool get_always_show_hscroll() const { return m_always_show_hscroll; }

   void set_deadzone(int val) { m_deadzone = val; }
   int get_deadzone() const { return m_deadzone; }

   void set_text_direction(TextDirection direction);
   TextDirection get_text_direction() const { return m_text_direction; }

   void set_language(const godot::String& language);
   godot::String get_language() const { return m_language; }


   /// Public non exposed functions
   // This will be called by the AccordionItem when its active state is changed. A much cleaner way to handle this would be
   // to connect an event handler. However dealing with signals in this way brings another problem. Unfortunately GDExtension
   // has no easy way to detect when a node is added/remove as a direct child. While it's possible, a lot of upkeep is required.
   // So decided to go the slightly uncleaner way since it's way easier in the upkeep aspect.
   // The goal of this function is to ensure that the "m_exclusive" flag is taken into account when showing an item.
   void on_item_toggled(AccordionItem* item);

   Accordion();
};


#endif   // accordion disabled


#endif   // _kehui_accordion_h_included
