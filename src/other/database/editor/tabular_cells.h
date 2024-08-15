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


#ifndef _kehother_database_editor_tabular_box_cells_h_included
#define _kehother_database_editor_tabular_box_cells_h_included 1

#ifndef DATABASE_DISABLED
#ifndef EDITOR_DISABLED

#include "../dbtable.h"
#include "../../../ui/tabular_box/tabular_box_cell.h"

#include <godot_cpp/templates/hash_set.hpp>

class SpinSlider;

namespace godot
{
   class AudioStream;
   class AudioStreamPlayer;
   class Button;
   class ColorPickerButton;
   class Font;
   class HBoxContainer;
   class HSlider;
   class InputEvent;
   class LineEdit;
   class Popup;
   class SpinBox;
   class StyleBox;
   class Texture2D;
   class VBoxContainer;
}


// Not exactly a cell. This is meant to provide a very rudimentary and simple "audio player". This is how audio
// resources will be "previewed" within the cells
class DBAudioPreview : public godot::Control
{
   GDCLASS(DBAudioPreview, godot::Control);
private:
   /// Internal variables
   static godot::Ref<godot::Texture2D> s_icon_play;
   static godot::Ref<godot::Texture2D> s_icon_pause;
   static godot::Ref<godot::Texture2D> s_icon_stop;

   godot::HSlider* m_progress;

   godot::Button* m_btplay;
   godot::Button* m_btpause;
   godot::Button* m_btstop;

   /// NOTE: If this becomes a problem (performance/resources...), then create a system to share a single instance of the stream player
   godot::AudioStreamPlayer* m_player;
   // m_player->is_playing() sometimes return true after the playback has finished. During the internal process this results in error
   // message from Godot. So use an internal flag to track this
   bool m_playing;


   bool m_seeking;

   godot::Size2i m_min_size;

   // Cache which internal control has focus. This will help with certain cell interactions
   godot::Control* m_focused;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_btplay_clicked();
   void on_btpause_clicked();
   void on_btstop_clicked();

   void on_seek_audio(float pos);
   void on_slider_drag_started();
   void on_slider_drag_finished(bool changed);

   void on_playback_finished();

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   virtual godot::Vector2 _get_minimum_size() const override { return m_min_size; }

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   godot::Ref<godot::Texture2D> get_icon_play() const { return s_icon_play; }
   godot::Ref<godot::Texture2D> get_icon_pause() const { return s_icon_pause; }
   godot::Ref<godot::Texture2D> get_icon_stop() const { return s_icon_stop; }

   void setup(const godot::Dictionary& styles);

   void set_stream(const godot::Ref<godot::AudioStream>& stream);

   bool has_focused_control() const { return m_focused; }
   void remove_focus();

   DBAudioPreview();
};



// This is not exactly a cell, but a Control meant to preview the values of a table row.
class DBRowPreview : public godot::Control
{
   GDCLASS(DBRowPreview, godot::Control);
private:
   static const int PREVIEW_FONT_SIZE = 12;
   static const int LINE_WIDTH = 1;
   static const int SEPARATION = 3;

   /// Internal variables
   // Rationale here: loading a texture within the "notification_draw" results in a white rectangle being
   // drawn instead of the texture. This means that the stuff to be rendered (at least the textures) have
   // to be preloaded. Since this route has to be taken, well, stuffing all rendering data into a vector
   // of instances of this struct
   struct ColumnData
   {
      godot::String title;
      godot::Variant value;
      DBTable::ValueType type;

      // Some displays will use auxiliary Control nodes, such as Audio that uses the DBAudioPreview.
      // Using a Vector<> so VT_*Array types can be displayed too.
      godot::Vector<godot::Control*> auxiliary;
      
      // Some value types might completely ignore the height of this thing. Nevertheless, caching the width
      // helps a lot during the drawing
      godot::Size2i size;
   };


   struct
   {
      bool initialized;

      godot::Ref<godot::Font> font;

      godot::Ref<godot::StyleBox> empty;

      godot::Ref<godot::StyleBox> normal;
      godot::Ref<godot::StyleBox> hover;
      godot::Ref<godot::StyleBox> pressed;

      godot::Ref<godot::Texture2D> no_image;
      godot::Ref<godot::Texture2D> unchecked;
      godot::Ref<godot::Texture2D> checked;

      godot::Color text_color;
   } m_theme_cache;

   godot::Size2i m_min_size;

   // This should be filled in correct column order
   godot::Vector<ColumnData> m_row;

   godot::Ref<DBTable> m_table;
   godot::Variant m_id;

   // If this is true then the ID of the row will be drawn, otherwise skipped
   bool m_render_id;

   bool m_draw_background;

   bool m_mouse_over;

   bool m_selected;


   /// Exposed variables

   /// Internal functions
   void clear_row();

   void calculate_min_width();
   void calculate_min_height();

   void draw_values();

   /// Event handlers

protected:
   void _notification(int what);
   static void _bind_methods();
public:
   /// Overrides
   virtual godot::Vector2 _get_minimum_size() const override { return m_min_size; }
   virtual void _gui_input(const godot::Ref<godot::InputEvent>& event) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void setup(const godot::Dictionary& styles);

   void set_table(const godot::Ref<DBTable>& table);

   void set_row_id(const godot::Variant& id);
   godot::Variant get_row_id() const { return m_id; }


   void set_draw_background(bool enable);

   void set_render_id(bool enable);

   // This is primarily meant to be used within the drop down menu in order to highlight value that is
   // already assigned within the cell that triggered the menu.
   void set_selected(bool selected);

   void apply_filter(const godot::String& str);


   DBRowPreview();
};



class DBTabularCellAudio : public TabularBoxCell
{
   GDCLASS(DBTabularCellAudio, TabularBoxCell);
private:
   /// Internal variables
   DBAudioPreview* m_audio_player;

   godot::Button* m_btvalue;
   godot::Button* m_btclear;

   godot::String m_assigned;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_btval_clicked();
   void on_btclear_clicked();

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual void check_theme() override;
   virtual void assign_value(const godot::Variant& value) override;
   virtual void selected_changed(bool selected) override;

   virtual bool requires_file_dialog() const override { return true; }

   virtual bool _can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const override;
   virtual void _drop_data(const godot::Vector2& pos, const godot::Variant& data) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void file_selected(const godot::String& path);

   DBTabularCellAudio();
};



class DBTabularCellGenericResource : public TabularBoxCell
{
   GDCLASS(DBTabularCellGenericResource, TabularBoxCell);
private:
   /// Internal variables
   godot::String m_assigned;        // generic resources are stored as paths to the files

   godot::Button* m_btvalue;
   godot::Button* m_btclear;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_btvalue_clicked();
   void on_btclear_clicked();

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual void check_theme() override;
   virtual void assign_value(const godot::Variant& value) override;
   virtual void selected_changed(bool selected) override;

   virtual bool requires_file_dialog() const override { return true; }

   virtual bool _can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const override;
   virtual void _drop_data(const godot::Vector2& pos, const godot::Variant& data) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void file_selected(const godot::String& path);

   DBTabularCellGenericResource();
};




class DBTabularCellColor : public TabularBoxCell
{
   GDCLASS(DBTabularCellColor, TabularBoxCell);
private:
   /// Internal variables
   godot::ColorPickerButton* m_button;
   godot::Color m_assigned;

   /// Exposed variables

   /// Internal functions

   /// Event handlers
   void on_bt_clicked();
   void on_bt_value_changed(const godot::Color& val);

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
   DBTabularCellColor();
};


class DBTabularCellRandomWeight : public TabularBoxCellFloat
{
   GDCLASS(DBTabularCellRandomWeight, TabularBoxCellFloat);
private:
   /// Internal variables
   godot::Ref<DBTable> m_table;

   /// Exposed variables

   /// Internal functions

   /// Event handlers

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual void apply_extra_settings(const godot::Dictionary& extra_settings) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   DBTabularCellRandomWeight() {}
};


/// This cell is used to display values from another table. When this happens, the value is the row ID from that other
/// referenced table. Yet, a popup menu is provided to allow choosing the value. To prevent having to create said menu
/// several times, one for each instance of this cell, this uses a design very similar to the one found in the godot::Range
/// class, which contains a "Shared" internal pointer.
class DBTabularCellExternalID : public TabularBoxCell
{
   GDCLASS(DBTabularCellExternalID, TabularBoxCell);
private:
   struct SharedData
   {
      godot::Popup* pop;
      godot::LineEdit* filter;
      godot::Control* pop_parent;
      godot::Ref<DBTable> other_table;

      // This makes things way easier to apply proper styling when theme is changed
      godot::Vector<DBRowPreview*> pop_entry;

      godot::HashSet<DBTabularCellExternalID*> owner_list;

      int32_t separation;

      godot::Variant clicked_row_id;

      SharedData() : pop(nullptr), pop_parent(nullptr), separation(0) {}
   };

   /// Internal variables
   godot::Button* m_btvalue;
   godot::Button* m_btclear;

   DBRowPreview* m_preview;

   SharedData* m_shared;

   godot::Variant m_assigned;

   /// Exposed variables

   /// Internal functions
   void ref_shared(SharedData* shared);
   void unref_shared();

   godot::Dictionary build_styles() const;

   /// Event handlers
   void on_pop_hide();

   void on_btval_clicked();
   void on_btclear_clicked();

   void on_filtering(const godot::String& str);

   void on_entry_clicked(const godot::Variant& row_id);

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual void check_theme() override;
   virtual void assign_value(const godot::Variant& value) override;
   virtual void selected_changed(bool selected) override;

   virtual void apply_extra_settings(const godot::Dictionary& extra_settings) override;

   virtual void setup_shared(godot::Control* parent) override;
   virtual void share_with(TabularBoxCell* other_cell) override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   DBTabularCellExternalID();
   ~DBTabularCellExternalID();
};




class DBTabularCellArray : public TabularBoxCell
{
   GDCLASS(DBTabularCellArray, TabularBoxCell);
#if !defined(SPIN_SLIDER_DISABLED) && !defined(FORCE_SPIN_BOX)
   typedef SpinSlider spin_t;
#else
   typedef godot::SpinBox spin_t;
#endif
private:
   struct Entry
   {
      godot::HBoxContainer* box;
      godot::Control* editor;
      godot::Button* bt_remove;

      Entry() : box(nullptr), editor(nullptr), bt_remove(nullptr) {}
   };

   /// Internal variables

   DBTable::ValueType m_array_type;

   godot::VBoxContainer* m_mainbox;
   godot::VBoxContainer* m_vbox;

   godot::Vector<Entry> m_entry;

   godot::Variant m_assigned;

   godot::Button* m_btadd;

   // Certain specific array types might need extra settings. Store them here
   godot::Dictionary m_extra_settings;

   // Keep track of which "editor" has focus (if any). This is mostly to deal with the behavior of cell selection
   godot::Control* m_focused;

   /// Exposed variables

   /// Internal functions
   void set_entry(const godot::Variant& value, int64_t index);

   bool is_valid_array(godot::Variant::Type type) const;

   void focus_next(int64_t index, godot::Control* ctrl);

   /// Event handlers
   void on_vbox_resized();

   void on_btadd_clicked();
   void on_remove_clicked(godot::HBoxContainer* entry_box);

   void on_draw_hbox(godot::HBoxContainer* hbox);


   void on_txt_focus(godot::LineEdit* editor);
   void on_txt_unfocus(godot::HBoxContainer* entry_box);
   void on_txt_commited(const godot::String& new_text, godot::HBoxContainer* entry_box);
   void on_txt_gui_input(const godot::Ref<godot::InputEvent>& event, godot::HBoxContainer* entry_box);
   void on_load_clicked(godot::HBoxContainer* entry_box);
   void on_color_changed(const godot::Color& color, godot::HBoxContainer* entry_box);

protected:
   void _notification(int what);
   static void _bind_methods() {}
public:
   /// Overrides
   virtual bool _can_drop_data(const godot::Vector2& pos, const godot::Variant& data) const override;
   virtual void _drop_data(const godot::Vector2& pos, const godot::Variant& data) override;

   virtual void check_theme() override;
   virtual void assign_value(const godot::Variant& value) override;
   virtual void selected_changed(bool selected) override;

   virtual void apply_extra_settings(const godot::Dictionary& extra_settings) override;

   virtual bool requires_file_dialog() const override;

   /// Exposed virtual functions

   /// Exposed functions

   /// Setters/Getters

   /// Public non exposed functions
   void file_selected(const godot::String& path, godot::HBoxContainer* entry_box);

   DBTabularCellArray(DBTable::ValueType array_type = DBTable::VT_StringArray);
};

class DBTabularCellArrayInteger : public DBTabularCellArray
{
   GDCLASS(DBTabularCellArrayInteger, DBTabularCellArray);
protected:
   static void _bind_methods() {}
public:
   DBTabularCellArrayInteger() : DBTabularCellArray(DBTable::VT_IntegerArray) {}
};

class DBTabularCellArrayFloat : public DBTabularCellArray
{
   GDCLASS(DBTabularCellArrayFloat, DBTabularCellArray);
protected:
   static void _bind_methods() {}
public:
   DBTabularCellArrayFloat() : DBTabularCellArray(DBTable::VT_FloatArray) {}
};

class DBTabularCellArrayTexture : public DBTabularCellArray
{
   GDCLASS(DBTabularCellArrayTexture, DBTabularCellArray);
protected:
   static void _bind_methods() {}
public:
   DBTabularCellArrayTexture() : DBTabularCellArray(DBTable::VT_TextureArray) {}
};

class DBTabularCellArrayAudio : public DBTabularCellArray
{
   GDCLASS(DBTabularCellArrayAudio, DBTabularCellArray);
protected:
   static void _bind_methods() {}
public:
   DBTabularCellArrayAudio() : DBTabularCellArray(DBTable::VT_AudioArray) {}
};

class DBTabularCellArrayGenericRes : public DBTabularCellArray
{
   GDCLASS(DBTabularCellArrayGenericRes, DBTabularCellArray);
protected:
   static void _bind_methods() {}
public:
   DBTabularCellArrayGenericRes() : DBTabularCellArray(DBTable::VT_GenericResArray) {}
};

class DBTabularCellArrayColor : public DBTabularCellArray
{
   GDCLASS(DBTabularCellArrayColor, DBTabularCellArray);
protected:
   static void _bind_methods() {}
public:
   DBTabularCellArrayColor() : DBTabularCellArray(DBTable::VT_ColorArray) {}
};




// This is special type of cell that is meant to show read only values. More specifically, row IDs when tables have
// the "locked_id" flag enabled. Since editing is not allowed, both String and Integer IDs will use this same cell type.
// Internally a LineEdit will still be used so user can select the ID and copy it if so desired
class DBTabularCellLockedRowID : public TabularBoxCell
{
   /// TODO: Remove this class and incorporate a "read only" mode into the TabularBoxCellString, to be enabled by using extra settings
   GDCLASS(DBTabularCellLockedRowID, TabularBoxCell);
private:
   /// Internal variables
   godot::LineEdit* m_txt;

   /// Exposed variables

   /// Internal functions

   /// Event handlers

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
   DBTabularCellLockedRowID();
};



#endif  //EDITOR_DISABLED
#endif  //DATABASE_DISABLED

#endif   // _kehother_database_editor_tabular_box_cells_h_included

