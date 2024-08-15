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


#include "register_types.h"
#include "internal.h"

#include <gdextension_interface.h>

#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/resource_saver.hpp>


#include "extpackutils.h"

/// Includes from "debug helpers"
#include "dbghelper/line3d.h"
#include "dbghelper/overlayinfo.h"

/// Includes from "general"
#include "general/quantize.h"

/// Includes from "nodes"
#include "nodes/audiomaster.h"
#include "nodes/autointerpolate.h"
#include "nodes/radialimpulse.h"

/// Includes from 'other"
#include "other/database/database.h"
#include "other/database/editor/dbeplugin.h"      // The contents will not be included if Editor is disabled
#include "other/database/editor/tabular_cells.h"


/// Includes from "ui"
#include "ui/custom_theme.h"
#include "ui/editor_theme_builder.h"
#include "ui/accordion/accordion.h"
#include "ui/expandable_panel/expanel.h"
#include "ui/filedir_picker/filedir_picker.h"
#include "ui/spin_slider/spin_slider.h"
#include "ui/tabular_box/data_source.h"
#include "ui/tabular_box/data_source_simple.h"
#include "ui/tabular_box/tabular_header.h"
#include "ui/tabular_box/tabular_box.h"
#include "ui/tabular_box/tabular_box_cell.h"

#include <godot_cpp/variant/utility_functions.hpp>

#ifndef LINE3D_DISABLED
   static Line3D* s_line3d = nullptr;
#endif

#ifndef OVERLAY_INFO_DISABLED
   static OverlayInfo* s_overlayinfo = nullptr;
#endif

#ifndef ALL_UI_DISABLED
   static CustomControlThemeDB* s_custom_theme_db = nullptr;
#endif


#ifndef DATABASE_DISABLED
   ///static godot::Ref<ResourceFormatLoaderGDDatabase> res_loader_database;
   ///static godot::Ref<ResourceFormatSaverGDDatabase> res_saver_database;
#endif


void register_pack_types(godot::ModuleInitializationLevel p_level)
{
   switch (p_level)
   {
      case godot::MODULE_INITIALIZATION_LEVEL_CORE:
      {
          GDREGISTER_CLASS(ExtPackUtils);
      } break;

      case godot::MODULE_INITIALIZATION_LEVEL_SERVERS:
      {

      } break;

      case godot::MODULE_INITIALIZATION_LEVEL_SCENE:
      {
         // Even if all addons that require these singletons are disabled, just obtain them for simplicity
         godot::Engine* engine = godot::Engine::get_singleton();

         /// Register classes in the "debug helpers" category
         #ifndef LINE3D_DISABLED
            #ifndef EMPTY_LINE3D
            GDREGISTER_INTERNAL_CLASS(_LineNode);
            #endif
            GDREGISTER_CLASS(Line3D);

            s_line3d = memnew(Line3D);
            engine->register_singleton("Line3D", Line3D::get_singleton());
            s_line3d->register_project_settings();
         #endif

         #ifndef OVERLAY_INFO_DISABLED
            #ifndef EMPTY_OVERLAY_INFO
            GDREGISTER_INTERNAL_CLASS(_OverlayControl);
            #endif
            GDREGISTER_CLASS(OverlayInfo);

            s_overlayinfo = memnew(OverlayInfo);
            engine->register_singleton("OverlayInfo", OverlayInfo::get_singleton());
            s_overlayinfo->register_project_settings();
         #endif


         /// Register classes in the "general" category
         #ifndef QUANTIZE_DISABLED
            GDREGISTER_CLASS(Quantize);
         #endif


         /// Register classes in the "nodes" category
         #ifndef AUDIO_MASTER_DISABLED
            GDREGISTER_CLASS(AudioMaster);
         #endif

         #ifndef INTERPOLATION_DISABLED
            // If here at least one of the interpolation nodes is enabled. Must register the internal
            // Interpolator class
            GDREGISTER_INTERNAL_CLASS(Interpolator);
         #endif
         #ifndef AUTO_INTERPOLATE_DISABLED
            GDREGISTER_CLASS(AutoInterpolate);
         #endif
         #ifndef SMOOTH2D_DISABLED
            GDREGISTER_CLASS(Smooth2D);
         #endif
         #ifndef SMOOTH3D_DISABLED
            GDREGISTER_CLASS(Smooth3D);
         #endif

         #ifndef RADIAL_IMPULSE2D_DISABLED
            GDREGISTER_CLASS(RadialImpulse2D);
         #endif
         #ifndef RADIAL_IMPULSE3D_DISABLED
            GDREGISTER_CLASS(RadialImpulse3D);
         #endif


         /// Register classes in the "UI" category
         #ifndef ALL_UI_DISABLED
            GDREGISTER_INTERNAL_CLASS(CustomControlThemeDB);
            s_custom_theme_db = memnew(CustomControlThemeDB);
            engine->register_singleton("CustomControlThemeDB", CustomControlThemeDB::get_singleton());
         #endif


         #ifndef ACCORDION_DISABLED
            GDREGISTER_CLASS(AccordionItem);
            GDREGISTER_CLASS(Accordion);
         #endif

         #ifndef EXPANDABLE_PANEL_DISABLED
            GDREGISTER_CLASS(ExpandablePanel);
         #endif

         #ifndef FILEDIRPICKER_DISABLED
            GDREGISTER_CLASS(FileDirPicker);
         #endif

         #ifndef SPIN_SLIDER_DISABLED
            GDREGISTER_CLASS(SpinSlider);
         #endif

         #ifndef TABULAR_BOX_DISABLED
            GDREGISTER_VIRTUAL_CLASS(TabularDataSource);
            GDREGISTER_CLASS(TabularDataSourceSimple);
            GDREGISTER_VIRTUAL_CLASS(TabularBoxCell);
            GDREGISTER_CLASS(TabularBoxCellBool);
            GDREGISTER_CLASS(TabularBoxCellMultilineString);

            GDREGISTER_CLASS(TabularBoxCellNumeric);
            GDREGISTER_CLASS(TabularBoxCellInteger);
            GDREGISTER_CLASS(TabularBoxCellFloat);

            GDREGISTER_CLASS(TabularBoxCellString);
            GDREGISTER_CLASS(TabularBoxCellTexture);
            
            GDREGISTER_INTERNAL_CLASS(TabularBoxCellRowNumber);
            GDREGISTER_INTERNAL_CLASS(TabularBoxHeader);
            GDREGISTER_CLASS(TabularBox);
         #endif

         /// Register classes in the "other" category - while I would like to register the categories in alphabetical order,
         // this can't be done. Most things in the "other" category have internal dependencies. As an example, the Database
         // requires the TabularBox, which is in the UI category.
         #ifndef DATABASE_DISABLED
         #ifndef EDITOR_DISABLED
            // The Editor Plugin will use this class, but it's not needed elsewhere. So register as internal and only if
            // in editor build.
            GDREGISTER_INTERNAL_CLASS(DBAudioPreview);
            GDREGISTER_INTERNAL_CLASS(DBRowPreview);

            GDREGISTER_INTERNAL_CLASS(DBTabularCellAudio);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellColor);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellExternalID);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellGenericResource);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellLockedRowID);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellRandomWeight);

            GDREGISTER_INTERNAL_CLASS(DBTabularCellArray);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellArrayAudio);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellArrayColor);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellArrayGenericRes);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellArrayInteger);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellArrayFloat);
            GDREGISTER_INTERNAL_CLASS(DBTabularCellArrayTexture);

            GDREGISTER_INTERNAL_CLASS(DBEditorDataSource);
            GDREGISTER_INTERNAL_CLASS(DBETableEntry);
            GDREGISTER_INTERNAL_CLASS(GDDatabaseEditor);
         #endif
            // If the format loader and format saver are not registered they simply don't work! That is, ResourceLoader and
            // ResourceSaver completely ignore that those classes exist!
            ///godot::ClassDB::register_internal_class<ResourceFormatLoaderGDDatabase>();
            ///godot::ClassDB::register_internal_class<ResourceFormatSaverGDDatabase>();

            ///res_saver_database.instantiate();
            ///res_loader_database.instantiate();

            ///godot::ResourceLoader::get_singleton()->add_resource_format_loader(res_loader_database);
            ///godot::ResourceSaver::get_singleton()->add_resource_format_saver(res_saver_database);
            
            GDREGISTER_CLASS(DBTable);
            //GDREGISTER_CLASS(DBTablePatch);
            GDREGISTER_CLASS(GDDatabase);

            
         #endif
      } break;

      case godot::MODULE_INITIALIZATION_LEVEL_EDITOR:
      {
         #ifndef EDITOR_DISABLED

         #ifndef DATABASE_DISABLED
            GDREGISTER_INTERNAL_CLASS(DBEditorPlugin);

            godot::EditorPlugins::add_by_type<DBEditorPlugin>();
         #endif

         #ifndef ALL_UI_DISABLED
            GDREGISTER_INTERNAL_CLASS(UIThemeBuilder);

            godot::EditorPlugins::add_by_type<UIThemeBuilder>();
         #endif

         #endif      //EDITOR_DISABLED
      } break;
   }

}

void unregister_pack_types(godot::ModuleInitializationLevel p_level)
{
   godot::Engine* engine = godot::Engine::get_singleton();
   if (p_level == godot::MODULE_INITIALIZATION_LEVEL_SCENE)
   {
      godot::Engine* engine = godot::Engine::get_singleton();

      /// Cleanup Debug Helpers stuff
      #ifndef LINE3D_DISABLED
         engine->unregister_singleton("Line3D");
         memdelete(s_line3d);
         s_line3d = nullptr;        // Not exactly necessary, but...
      #endif
      #ifndef OVERLAY_INFO_DISABLED
         engine->unregister_singleton("OverlayInfo");
         memdelete(s_overlayinfo);
         s_overlayinfo = nullptr;        // Not exactly necessary, but...
      #endif

      /// Cleanup UI related stuff
      #ifndef ALL_UI_DISABLED
         engine->unregister_singleton("CustomControlThemeDB");
         memdelete(s_custom_theme_db);
         s_custom_theme_db = nullptr;        // Not exactly necessary, but...
      #endif

      /// Cleanup "Other" related stuff
      #ifndef DATABASE_DISABLED
         ///godot::ResourceLoader::get_singleton()->remove_resource_format_loader(res_loader_database);
         ///godot::ResourceSaver::get_singleton()->remove_resource_format_saver(res_saver_database);

         ///res_loader_database.unref();
         ///res_saver_database.unref();
      #endif
   }

   if (p_level == godot::MODULE_INITIALIZATION_LEVEL_EDITOR)
   {
      #ifndef EDITOR_DISABLED

      #ifndef DATABASE_DISABLED
         godot::EditorPlugins::remove_by_type<DBEditorPlugin>();
      #endif

      #ifndef ALL_UI_DISABLED
         godot::EditorPlugins::remove_by_type<UIThemeBuilder>();
      #endif


      #endif   //EDITOR_DISABLED
   }
}


extern "C"
{


GDExtensionBool GDE_EXPORT keh_extension_pack_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization)
{
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

   init_obj.register_initializer(register_pack_types);
   init_obj.register_terminator(unregister_pack_types);

   init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_CORE);

	return init_obj.init();
}


}
