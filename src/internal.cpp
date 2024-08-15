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


#include "internal.h"

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/font.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/script.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/window.hpp>

godot::HashMap<godot::StringName, GlobalClassCache::ScriptClass> GlobalClassCache::m_global_class;


godot::Window* Internals::get_tree_root()
{
  godot::SceneTree* main_tree = godot::Object::cast_to<godot::SceneTree>(godot::Engine::get_singleton()->get_main_loop());
  return main_tree ? main_tree->get_root() : nullptr;
}


godot::Ref<godot::ImageTexture> Internals::texture_from_base64(const char* data)
{
   godot::String base64 = godot::String(data);

   if (base64.length() == 0)
   {
      return godot::Ref<godot::ImageTexture>();
   }

   const godot::PackedByteArray buffer = godot::Marshalls::get_singleton()->base64_to_raw(base64);
   godot::Ref<godot::Image> img = memnew(godot::Image);
   img->load_png_from_buffer(buffer);
   

   return godot::ImageTexture::create_from_image(img);
}


void Internals::connector(godot::Object* obj, const godot::StringName& event, const godot::Callable& handler)
{
   if (obj->is_connected(event, handler)) { return; }
   obj->connect(event, handler);
}


void Internals::disconnector(godot::Object* obj, const godot::StringName& event, const godot::Callable& handler)
{
   if (!obj->is_connected(event, handler)) { return; }
   obj->disconnect(event, handler);
}


float Internals::get_text_vertical_center(const godot::Ref<godot::Font>& font, int32_t font_size, float box_height)
{
   return ((box_height + (font->get_height(font_size) - font->get_descent() * 2.0)) * 0.5);
}


godot::Variant Internals::global_set(const godot::String& var, const godot::Variant& defval, bool restart, bool basic, bool internal)
{
   godot::Variant ret;

   godot::ProjectSettings* ps = godot::ProjectSettings::get_singleton();
   if (!ps) { return ret; }

   if (!ps->has_setting(var))
   {
      ps->set(var, defval);
   }
   ret = global_get(var);

   ps->set_initial_value(var, defval);
   ps->set_as_basic(var, basic);
   ps->set_restart_if_changed(var, restart);
   ps->set_as_internal(var, internal);

   return ret;
}


godot::Variant Internals::global_get(const godot::String& var)
{
   godot::ProjectSettings* ps = godot::ProjectSettings::get_singleton();
   return ps != nullptr ? ps->get_setting_with_override(var) : godot::Variant();
}


godot::Ref<godot::StyleBoxFlat> Internals::make_flat_stylebox(const godot::Color& color, float margin_left, float margin_top, float margin_right, float margin_bottom, float corner_radius, bool draw_center, int border_width)
{
   godot::Ref<godot::StyleBoxFlat> ret(memnew(godot::StyleBoxFlat));
   ret->set_bg_color(color);
   // Core StyleBox offers a function named set_content_margin_individual, which allows us to set all margins with a single call. Unfortunately
   // it's not exposed to GDExtension! Each margin must be set by a call
   ret->set_content_margin(godot::Side::SIDE_LEFT, margin_left);
   ret->set_content_margin(godot::Side::SIDE_TOP, margin_top);
   ret->set_content_margin(godot::Side::SIDE_RIGHT, margin_right);
   ret->set_content_margin(godot::Side::SIDE_BOTTOM, margin_bottom);
   ret->set_corner_radius_all(corner_radius);
   ret->set_anti_aliased(true);
   ret->set_draw_center(draw_center);
   ret->set_border_width_all(border_width);
   ret->set_expand_margin_all(0.0);

   return ret;
}


int Internals::step_decimals(double step)
{
   static const int maxn = 10;
	static const double sd[maxn] =
   {
		0.9999, // somehow compensate for floating point error
		0.09999,
		0.009999,
		0.0009999,
		0.00009999,
		0.000009999,
		0.0000009999,
		0.00000009999,
		0.000000009999,
		0.0000000009999
	};

	double abs = godot::Math::abs(step);
	double decs = abs - (int)abs; // Strip away integer part
	for (int i = 0; i < maxn; i++)
   {
		if (decs >= sd[i])
      {
			return i;
		}
	}

	return 0;
}


int Internals::range_step_decimals(double step)
{
   if (step < 0.0000000000001)
   {
		return 16; // Max value hardcoded in String::num
	}
	return step_decimals(step);
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GlobalClassCache
void GlobalClassCache::scan_global_classes()
{
   m_global_class.clear();

   const godot::TypedArray<godot::Dictionary> clist = godot::ProjectSettings::get_singleton()->get_global_class_list();
   const int64_t count = clist.size();
   for (int64_t i = 0; i < count; i++)
   {
      const godot::Dictionary info = clist[i];

      const godot::StringName cname = info["class"];

      ScriptClass sc;
      sc.base = info.get("base", "");
      sc.path = info.get("path", "");

      m_global_class[cname] = sc;
   }
}


bool GlobalClassCache::is_global_class(const godot::StringName& class_name)
{
   return m_global_class.has(class_name);
}


godot::String GlobalClassCache::get_global_class_path(const godot::StringName& class_name)
{
   ERR_FAIL_COND_V(!m_global_class.has(class_name), godot::String());
   return m_global_class[class_name].path;
}


godot::StringName GlobalClassCache::get_global_class_base(const godot::StringName& class_name)
{
   ERR_FAIL_COND_V(!m_global_class.has(class_name), godot::StringName());
   return m_global_class[class_name].base;
}


godot::StringName GlobalClassCache::get_global_class_native_base(const godot::StringName& class_name)
{
   ERR_FAIL_COND_V(!m_global_class.has(class_name), godot::StringName());
   godot::StringName base = m_global_class[class_name].base;
   while (m_global_class.has(base))
   {
      base = m_global_class[base].base;
   }
   return base;
}


