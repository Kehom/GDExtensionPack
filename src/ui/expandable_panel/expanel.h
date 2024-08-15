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

#ifndef _kehui_expandable_panel_h_included
#define _kehui_expandable_panel_h_included 1

#ifndef EXPANDABLE_PANEL_DISABLED

#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/container.hpp>
#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>


// Forward declares
namespace godot
{
   class BoxContainer;
   class Button;
   class Curve;
   class StyleBox;
   class Texture2D;
}


class ExpandablePanel : public godot::Container
{
   GDCLASS(ExpandablePanel, godot::Container);
public:
   enum AttachTo { Left, Top, Right, Bottom };
private:
   struct// ThemeCache
   {
      godot::Ref<godot::StyleBox> background;
      godot::Ref<godot::StyleBox> bar;

      godot::Ref<godot::StyleBox> button_normal;
      godot::Ref<godot::StyleBox> button_hover;
      godot::Ref<godot::StyleBox> button_pressed;
      godot::Ref<godot::StyleBox> button_disabled;
      godot::Ref<godot::StyleBox> button_focus;

      int toggle_button_separation;

      int button_min_width;
      int button_min_height;
   } m_theme_cache;


   // Depending on how the animation system is configured, multiple animation sequences will need to be queued. Namely,
   // if it's set to shrink first before expanding into a different page. To make things easier, this inner struct is used
   // to store "animation targets" as well as their states.
   struct AnimationState
   {
      float time;                // current elapsed time
      float target_time;         // how much time this animation state should last

      // The animation will only change either the width or the height of the main control. This holds the starting value
      // The specific dimension will be dealt with within the animation processing itself
      float starting;

      // And this holds the ending size of the width/height
      float ending;

      // This is the page index affected by the animation described by this
      int32_t target_page;

      // A curve to further customize the animation
      godot::Ref<godot::Curve> curve;

      AnimationState();

      bool is_expanding() const { return starting < ending; }

      bool update(float dt)
      {
         time += dt;
         return time >= target_time;
      }

      float calculate() const;
   };

   // Used to cache "valid" pages. This will help with the association of "valid page" and its corresponding toggle button
   struct PanelPage
   {
      // Index of the page - matching the one within an internal array
      int index;

      // The Control node that generated the page
      godot::Control* control;

      // Button used to toggle this page visibility
      godot::Button* button;

      PanelPage(int i = -1, godot::Control* ctrl = nullptr, godot::Button* bt = nullptr) : index(i), control(ctrl), button(bt) {}
   };

   /// Internal variables

   // Absolutely wish the texture references could be declared as static. However doing so results in Godot failing to load
   // the Dynamic library. Also note that those can't be declared as Consts. Doing so will result in memory leak because the
   // references must change internal data.
   godot::Ref<godot::Texture2D> TEX_ARROW_LEFT;
   godot::Ref<godot::Texture2D> TEX_ARROW_RIGHT;
   godot::Ref<godot::Texture2D> TEX_ARROW_UP;
   godot::Ref<godot::Texture2D> TEX_ARROW_DOWN;

   static const int MIN_CONTENT_HEIGHT = 70;
   static const int MIN_CONTENT_WIDTH = 80;

   // Cache minimum sizes in here. Those must be updated only when theme is changed or when custom icons are assigned into the
   // toggle buttons
   struct // SizeCache
   {
      // Combined maximum sizes of icons
      godot::Size2i icon;

      // Takes into consideration the icon and all possible states the button can be in (the style boxes)
      godot::Size2i button;

      // The box holding toggle buttons. Takes in consideration the style box used to represent it plus the button sizing
      godot::Size2i bar;
   } m_size_cache;

   // This will not be stored. Each direct child godot::Control node will generate an entry in this array
   godot::Vector<PanelPage> m_page_array;

   // The default toggle button icons have dimensions of 14x14. Custom icons might be of different size. The bigger icon will determine
   // the allocated button container size. This property is meant to cache the bigger icon size.
   //godot::Vector2i m_bigger_icon_size;

   // Store the desired expanded size of the panel. If those are with non default values they will be serialized, however not exposed to
   // the inspector. This is done through the _get_property_list(). Unfortunately I was unable to register a property to be serialized
   // without also exposing it to the inspector and/or in editor documentation.
   int32_t m_expanded_width;
   int32_t m_expanded_height;

   // If not empty then the panel is either shrinking or expanding through an animation
   godot::Vector<AnimationState> m_animation_queue;


   /// Internal godot::Control's
   // Will be set as "vertical" or "horizontal" based on the main panel settings. Each "page" within the main panel will
   // generate a "toggle button" as a child of this container.
   godot::BoxContainer* m_togglebox;


   /// Exposed variables
   // Attach this widget to the left, top, right or bottom section of the UI
   AttachTo m_attachto;

   // Tells which page should be displayed. -1 means the panel is should not be expanded
   int32_t m_current_page;

   // IF this is true then the page title will be assigned as tooltip text in the toggle button associated with pages
   bool m_use_tooltip;

   // If this flag is set to true then focus will be kept within clicked button
   bool m_keep_button_focus;

   // If this is true then the assigned button icon will expand to the button size. Aspect ratio is maintained
   bool m_expand_button_icon;

   // If this is true then animation will be updated within the physics processing
   bool m_animate_on_physics;

   // If this is true, then selecting a page when another is expanded will first shrink the panel before expanding into the new one
   // Otherwise the new page will be immediately shown.
   bool m_shrink_on_change;

   // Determine how lon, in seconds, the expand animation should last. If zero no animation
   float m_expand_time;

   // Curve that will modify the expand animation
   godot::Ref<godot::Curve> m_expand_curve;

   // Determine how long, in seconds, the shrink animation should last
   float m_shrink_time;

   // Curve that will modify the shrink animation
   godot::Ref<godot::Curve> m_shrink_curve;


   // When in editor this allows to preview a page different from the desired default one, which might even be -1 (non expanded panel)
#ifdef DEBUG_ENABLED
   // Exposed through _get_property_list(). By doing this way it becomes possible to eliminate this property from non editor builds
   int32_t m_preview_page;             
#endif


   /// Internal functions
   // Some functionality is based on vertical (left/right) vs horizontal (top/bottom) orientation (attachment) of the ExpandablePanel
   // This function might provide some "shortcuts" around the code.
   bool is_vertical() const { return (m_attachto == Left || m_attachto == Right); }

   // This is meant to update the sizes cache. Must be called whenever the theme is changed or an icon is assigned into a toggle button
   void calculate_sizes();

   // Apply the correct/expected styling into the incoming button
   void apply_button_style(godot::Button* bt);


   // Retrieve the icon to be assigned into the toggle button that is associated with the incoming page
   godot::Ref<godot::Texture2D> get_page_icon(const PanelPage& page) const;

   // Shortcut function to obtain the page title - this is based on metadata within the node that generated the page
   void assign_button_tooltip(const PanelPage& page);


   int get_control_width() const;
   int get_control_height() const;

   // Counts number of child control nodes that generated a "page" within the container
   int32_t count_pages() const;

   // (Re)build the internal m_page_array. Again, that container is used to make several tasks easier to deal with when
   // interacting with the ExpandablePanel
   void refresh_pages(int32_t diff_toggle = 0);


   // This function is used so when using the editor it becomes possible to preview a page without changing the default starting
   // page, which might be "-1". In which case, -1 means the panel should not be expanded.
   int get_view_page() const;

   int32_t get_page_index_by_name(const godot::String& pname) const;


   void check_panel_anchors();
   void check_layout();
   void refresh_layout();

   void handle_animation(float dt);

   /// Event handlers
   // Draws the box holding toggle buttons (the m_togglebox)
   void on_draw_button_box();

   // Draws the main control
   void on_draw_main();

   // This will also be called if a new child is added
   void on_child_order_changed();

   // Children of this panel that resulted in a page should have their renamed event handled.
   void on_node_renamed(godot::Node* node);

   // A toggle button has been clicked. Currently visible page must change
   void on_page_button_clicked(godot::Button* button);

protected:
   void _get_property_list(godot::List<godot::PropertyInfo>* list) const;
   bool _set(const godot::StringName& prop_name, const godot::Variant& value);
   bool _get(const godot::StringName& prop_name, godot::Variant& out_value) const;

   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   godot::Vector2 _get_minimum_size() const;     // "override"

   /// Exposed functions
   int get_page_count() const { return m_page_array.size(); }

   godot::String get_page_tooltip(int32_t page_index) const;
   void set_page_tooltip(int32_t page_index, const godot::String& new_title);

   godot::Ref<godot::Texture2D> get_page_icon_expanded(int32_t page_index) const;
   void set_page_icon_expanded(int32_t page_index, const godot::Ref<godot::Texture2D>& texture);

   godot::Ref<godot::Texture2D> get_page_icon_shrunk(int32_t page_index) const;
   void set_page_icon_shrunk(int32_t page_index, const godot::Ref<godot::Texture2D>& texture);

   bool is_page_disabled(int32_t page_index) const;
   void set_page_disabled(int32_t page_index, bool disabled);

   void enable_page(int32_t page_index) { set_page_disabled(page_index, false); }
   void disable_page(int32_t page_index) { set_page_disabled(page_index, true); }

   /// Setters/Getters
   AttachTo get_attach_to() const { return m_attachto; }
   void set_attach_to(AttachTo value);

   int get_current_page() const { return m_current_page; }
   void set_current_page(int page_index);

   bool get_use_tooltip() const { return m_use_tooltip; }
   void set_use_tooltip(bool value);

   bool get_keep_toggle_button_focus() const { return m_keep_button_focus; }
   void set_keep_toggle_button_focus(bool value) { m_keep_button_focus = value; }

   bool get_expand_icons_in_toggle_button() const { return m_expand_button_icon; }
   void set_expand_icons_in_toggle_button(bool value);

   bool get_animate_on_physics() const { return m_animate_on_physics; }
   void set_animate_on_physics(bool value) { m_animate_on_physics = value; }

   bool get_shrink_on_change() const { return m_shrink_on_change; }
   void set_shrink_on_change(bool value) { m_shrink_on_change = value; }

   float get_expand_time() const { return m_expand_time; }
   void set_expand_time(float value) { m_expand_time = value; }

   godot::Ref<godot::Curve> get_expand_curve() const { return m_expand_curve; }
   void set_expand_curve(const godot::Ref<godot::Curve>& curve) { m_expand_curve = curve; }

   float get_shrink_time() const { return m_shrink_time; }
   void set_shrink_time(float value) { m_shrink_time = value; }

   godot::Ref<godot::Curve> get_shrink_curve() const { return m_shrink_curve; }
   void set_shrink_curve(const godot::Ref<godot::Curve>& curve) { m_shrink_curve = curve; }

   /// Public non exposed functions
   ExpandablePanel();
};

VARIANT_ENUM_CAST(ExpandablePanel::AttachTo);


#endif     // EXPANDABLE_PANEL_DISABLED


#endif        // safe include
