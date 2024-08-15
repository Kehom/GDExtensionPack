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

#ifndef _kehui_tabular_box_cell_h_included
#define _kehui_tabular_box_cell_h_included 1

// For easier testing not removing the commented line bellow
//#define FORCE_SPIN_BOX

#ifndef TABULAR_BOX_DISABLED

#include "tbox_theme_cache.h"

#include <godot_cpp/classes/control.hpp>

#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>


class SpinSlider;

namespace godot
{
   class Button;
   class Font;
   class InputEvent;
   class LineEdit;
   class SpinBox;
   class StyleBox;
   class TextEdit;
   class Texture2D;
}

// This is the base class for cells within the TabularBox
class TabularBoxCell : public godot::Control
{
   GDCLASS(TabularBoxCell, godot::Control);
private:
   /// Internal variables
   TBoxThemeCache* m_theme_cache;

   bool m_selected;
   bool m_can_select_next;

   godot::Size2i m_min_size;

   // Originally requesting a file dialog was done through a signal. However it becomes rather limiting if the "file selected handler"
   // requires extra information like calling "bind()" in the callable assigned to the event. To that end this callable is used to
   // request the owning TabularBox for said dialog while also allowing an optional payload to be given and used when calling back the
   // cell whenever a file has been selected.
   godot::Callable m_request_file_dialog;

   /// Exposed variables

   /// Internal functions
   bool is_odd_row() const;

   /// Event handlers

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   virtual godot::Vector2 _get_minimum_size() const override { return m_min_size; }

   virtual void _gui_input(const godot::Ref<godot::InputEvent>& evt) override;

   virtual void _unhandled_input(const godot::Ref<godot::InputEvent>& evt) override;

   /// Exposed virtual functions
   // This will be called when the theme is changed or assigned into the cell. Derived cells are expected to update
   // their internal styling through this function
   GDVIRTUAL0(_check_theme);

   // Extra settings can be applied into specific Cell types. Those settings obviously depend very much on the DataSource
   // implementation. Still, those are given to the cell stuffed into a Dictionary. If the Cell does indeed require
   // additional settings, then override this function
   GDVIRTUAL1(_apply_extra_settings, const godot::Dictionary&);

   // This will be called whenever the value is changed. This should update the UI to display the incoming value
   GDVIRTUAL1(_assign_value, const godot::Variant&);

   // Certain cells might need special requirements when "forcefully committing" changes. Normally that that have internal
   // input editors like LineEdit, TextEdit and so on. Override this function to perform such operation
   GDVIRTUAL0(_commit_changes);


   // If a cell requires any internal action will it's selected, override this
   GDVIRTUAL1(_selected_changed, bool);

   // This function must return true if the Cell implementation might request a FileDialog.
   GDVIRTUAL0RC(bool, _requires_file_dialog);

   // A cell implementation might have internal shared data between multiple instances of the same cell type. When that is the case the
   // next two functions must be implemented. The first function is used to setup the internal "shared data". It will receive a Control
   // that can be used as a "parent" for any possible extra controls needed by the cells. The second function is used to perform the
   // sharing between two cells.
   GDVIRTUAL1(_setup_shared, godot::Control*);
   GDVIRTUAL1(_share_with, TabularBoxCell*);

   

   /// Exposed functions
   bool has_theme() const { return m_theme_cache != nullptr; }

   void set_min_height(int32_t height);

   // Depending on the cell implementation, it might need to retrieve row index. This function makes this retrieval easier
   int64_t get_row_index() const;

   double get_internal_margin_left() const;
   double get_internal_margin_top() const;
   double get_internal_margin_right() const;
   double get_internal_margin_bottom() const;

   // Apply default styling and setup to the provided LineEdit
   void apply_line_edit_style(godot::LineEdit* edit);

   // Given a button, apply default styling into it. This should help keep consistency through the various
   // cells that might require internal button
   void apply_button_style(godot::Button* into_button);

   // Calculate the height of a button styled by the apply_button_style() function
   float get_button_height() const;

   // Calculate the height of a button with an icon (that must be provided). If "include_font" is enabled then the
   // calculation will take font into account, considering the button has both text and icon.
   float get_button_min_height(bool include_font, const godot::Ref<godot::Texture2D>& icon, bool make_square) const;

   godot::Ref<godot::StyleBox> get_style_empty() const;
   godot::Ref<godot::StyleBox> get_style_background() const;
   godot::Ref<godot::StyleBox> get_style_button_normal() const;
   godot::Ref<godot::StyleBox> get_style_button_hover() const;
   godot::Ref<godot::StyleBox> get_style_button_pressed() const;
   godot::Ref<godot::StyleBox> get_style_focus() const;

   godot::Ref<godot::Font> get_font() const;
   int32_t get_font_size() const;

   godot::Color get_font_color() const;
   godot::Color get_font_selected_color() const;
   godot::Color get_selection_color() const;
   godot::Color get_caret_color() const;

   godot::Ref<godot::Texture2D> get_icon_checked() const;
   godot::Ref<godot::Texture2D> get_icon_unchecked() const;
   godot::Ref<godot::Texture2D> get_icon_trash() const;
   godot::Ref<godot::Texture2D> get_icon_no_texture() const;

   int32_t get_theme_separation() const;


   void notify_value_changed(const godot::Variant& new_value);
   
   void notify_selected();

   // After committing a value, call this function to request the TabularBox to select the next cell
   void request_select_next();

   // Call this function to request the TabularBox to unselect this cell
   void request_unselect();


   // Call this to request the TabularBox to display a FileDialog. The provided Callable will be used as the event handler
   // for the "file_selected" signal
   void request_file_dialog(const godot::String& title, const godot::PackedStringArray& filters, const godot::Callable& selected_handler);


   /// Setters/Getters

   /// Public non exposed functions
   virtual void check_theme();// { GDVIRTUAL_REQUIRED_CALL(_check_theme); }
   virtual void apply_extra_settings(const godot::Dictionary& extra_settings);// { GDVIRTUAL_CALL(_apply_extra_settings, extra_settings); }
   virtual void assign_value(const godot::Variant& value);// { GDVIRTUAL_REQUIRED_CALL(_assign_value, value); }

   virtual void commit_changes();// { GDVIRTUAL_CALL(_commit_changes); }

   virtual void selected_changed(bool selected);// { GDVIRTUAL_CALL(_selected_changed, selected); }

   virtual bool requires_file_dialog() const;

   virtual void setup_shared(godot::Control* parent);// { GDVIRTUAL_CALL(_setup_shared, parent); }
   virtual void share_with(TabularBoxCell* other_cell);// { GDVIRTUAL_CALL(_share_with, other_cell); }

   void setup_theme(TBoxThemeCache* theme_cache);

   void set_dialog_requester(const godot::Callable& func);

   void set_selected(bool selected);
   bool get_selected() const { return m_selected; }

   TabularBoxCell();
};


// Cells used for single line strings
class TabularBoxCellString : public TabularBoxCell
{
   GDCLASS(TabularBoxCellString, TabularBoxCell);
private:
   /// Internal variables
   godot::LineEdit* m_txt_cell;

   // This will help revert changes when "cancelling" (using the ui_cancel action - Escape key for example)
   godot::String m_assigned;

   //godot::Size2i m_min_size;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_txt_entered(const godot::String& new_value);
   void on_txt_unfocus();
   void on_txt_input(const godot::Ref<godot::InputEvent>& evt);

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   //virtual godot::Vector2 _get_minimum_size() const override { return m_min_size; }

   virtual void check_theme() override;
   virtual void assign_value(const godot::Variant& value) override;

   virtual void selected_changed(bool selected) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   TabularBoxCellString();
};


// Cells used to show/edit boolean values
class TabularBoxCellBool : public TabularBoxCell
{
   GDCLASS(TabularBoxCellBool, TabularBoxCell);
private:
   /// Internal variables
   bool m_value;

   //godot::Size2i m_min_size;

   /// Exposed variables

   /// Internal functions
   void toggle_value();

   /// Event handlers

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   //virtual godot::Vector2 _get_minimum_size() const override { return m_min_size; }

   virtual void check_theme() override;
   virtual void assign_value(const godot::Variant& value) override;

   virtual void _gui_input(const godot::Ref<godot::InputEvent>& evt) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   TabularBoxCellBool();
};


// Cells used to show/edit numbers. A variable (property) is used to determine if integers or floats
class TabularBoxCellNumeric : public TabularBoxCell
{
   GDCLASS(TabularBoxCellNumeric, TabularBoxCell);
protected:
#if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
   typedef SpinSlider spin_t;
#else
   typedef godot::SpinBox spin_t;
#endif

private:
   /// Internal variables
   spin_t* m_spin;

   float m_assigned;

   /// Exposed variables
   bool m_floating_point;

   /// Internal functions

   /// Event handlers
#if defined(SPIN_SLIDER_DISABLED) || defined(FORCE_SPIN_BOX)
   // If using godot::SpinBox then must listen to its internal LineEdit 'text_submitted' event to deal with "value entered" as
   // the box itself only emit "value changed" event. In other words, it doesn't offer a "value committed" thingy.
   void on_spin_le_txt_submitted(const godot::String& ntext);
#endif
   void on_value_entered(double val);
   void on_value_changed(double val);
   void on_le_gui_input(const godot::Ref<godot::InputEvent>& evt);

   void on_spin_min_size_changed();

protected:
   // This will be used during property validation. If this is true then don't expose m_floating_point
   bool m_is_fixed;

   // This is mostly for derived classes - somewhat a hack but oh well.
   spin_t* get_spin() const { return m_spin; }

   void _notification(int what);
   void _validate_property(godot::PropertyInfo& property) const;
   static void _bind_methods();
public:
   /// Overrides
   virtual void check_theme() override;
   virtual void apply_extra_settings(const godot::Dictionary& extra_settings) override;
   virtual void assign_value(const godot::Variant& value) override;
   virtual void selected_changed(bool selected) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters
   void set_floating_point(bool enable);
   bool is_floating_point() const { return m_floating_point; }

   /// Public non exposed functions
   TabularBoxCellNumeric(bool is_float = false);
};

class TabularBoxCellInteger : public TabularBoxCellNumeric
{
   GDCLASS(TabularBoxCellInteger, TabularBoxCellNumeric);
protected:
   static void _bind_methods() {}
public:
   TabularBoxCellInteger() : TabularBoxCellNumeric(false) { m_is_fixed = true; }
};

class TabularBoxCellFloat : public TabularBoxCellNumeric
{
   GDCLASS(TabularBoxCellFloat, TabularBoxCellNumeric);
protected:
   static void _bind_methods() {}
public:
   TabularBoxCellFloat() : TabularBoxCellNumeric(true) { m_is_fixed = true; }
};



// Cells used to edit (and show) texture2D resources
class TabularBoxCellTexture : public TabularBoxCell
{
   GDCLASS(TabularBoxCellTexture, TabularBoxCell);
private:
   /// Internal variables
   static const int16_t TEX_DIM = 32;

   // "Texture" value type is meant to be stored as the path to the resource
   godot::String m_assigned;

   // Cache the loaded texture in here for easier drawing
   godot::Ref<godot::Texture2D> m_texture;

   // Button used to bring file dialog and show assigned value
   godot::Button* m_btload;

   // Button used to reset the value
   godot::Button* m_btclear;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_btload_clicked();
   void on_btclear_clicked();

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual void check_theme() override;
   //virtual void apply_extra_settings(const godot::Dictionary& extra_settings) override;
   virtual void assign_value(const godot::Variant& value) override;
   virtual void selected_changed(bool selected) override;

   virtual bool requires_file_dialog() const override { return true; }
   //virtual void file_selected(const godot::String& path) override;


   virtual bool _can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const override;
   virtual void _drop_data(const godot::Vector2& pos, const godot::Variant& data) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void file_selected(const godot::String& path);

   TabularBoxCellTexture();
};


// Cells used to edit (and show) multi line strings
class TabularBoxCellMultilineString : public TabularBoxCell
{
   GDCLASS(TabularBoxCellMultilineString, TabularBoxCell);
private:
   /// Internal variables
   godot::TextEdit* m_txt_edit;

   godot::String m_assigned;
   int32_t m_line_count;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_text_changed();

   void on_txt_unfocus();

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual void check_theme() override;
   virtual void assign_value(const godot::Variant& value) override;
   virtual void selected_changed(bool selected) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   TabularBoxCellMultilineString();
};





// This wont be exposed as it's meant to display the row numbers. In other words, this is a special type of "cell"
class TabularBoxCellRowNumber : public godot::Control
{
   GDCLASS(TabularBoxCellRowNumber, godot::Control);
private:
   /// Internal variables
   TBoxThemeCache* m_theme_cache;

   bool m_draw_chk;
   bool m_draw_num;
   godot::HorizontalAlignment m_num_align;
   bool m_selected;

   /// Exposed variables

   /// Internal functions

   /// Event handlers

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   virtual void _gui_input(const godot::Ref<godot::InputEvent>& evt) override;
   
   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void set_draw_chk(bool enable);
   void set_draw_num(bool enable);
   void set_num_align(godot::HorizontalAlignment align);
   

   void set_selected(bool on);
   void toggle_selected();
   bool get_selected() const { return m_selected; }

   TabularBoxCellRowNumber(TBoxThemeCache* theme = nullptr);
};




#endif  //TABULAR_BOX_DISABLED

#endif   // _kehui_tabular_box_cell_h_included

