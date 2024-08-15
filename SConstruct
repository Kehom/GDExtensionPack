#!/usr/bin/env python
import sys
import os
import json


# This is meant to verify if an addon contains requirements and ensure those are enabled.
def check_requires(addon):
   requires = addon["requires"]

   for req in requires:
      if env[req] == False:
         print("ATTENTION: {addkey} requires {other} to be True, which is not the case. Setting it to True.".format(addkey = addon["key"], other=req))
         env[req] = True


# This function is called only if the incoming addon is effectively enabled. No need to check that again
# Nevertheless this is meant to verify if an addon contains extra options and apply defines accordingly
def check_extra_options(addon):
   if not "extra_options" in addon:
      return
   
   for extra in addon["extra_options"]:
      if env[extra["key"]]:
         #print("- Adding new define {edefine} as extra option {ext} is enabled".format(edefine=extra["def"], ext=extra["key"]))
         env.Append(CPPDEFINES=extra["def"])



def check_addon_implements(addon):
   if "shared_key" in addon:
      skey = addon["shared_key"]
      if not env[skey]:
         print("- Shared key {key} is disabled. So implementation {name} should be disabled.".format(key = skey, name = addon["name"]))
         return False

   retval = False   
   if not "key" in addon or env[addon["key"]]:
      check_extra_options(addon)
      retval = True
   
   elif "disabled_def" in addon:
      print("- Disabling {name} as '{key} = 0'".format(name = addon["name"], key = addon["key"]))
      env.Append(CPPDEFINES=addon["disabled_def"])
   
   return retval



# Load the JSON file containing all the required information regarding the plugins. The information in there includes which
# source files are to be built if the corresponding plugin variable is set to true. Also which define must be added in case
# the plugin is disabled.
addon_f = open('addon_data.json')
addon_data = json.load(addon_f)

# Attempt to load arguments from a file that *might* be specified through command line arguments
customs = ["custom.py"]
profile = ARGUMENTS.get("profile", "")
if profile:
    if not profile.endswith('.py'):
        profile += '.py'
    
    path = str(Entry('#' + profile))

    if (os.path.isfile(path)):
        customs.append(path)


### Declare the extra options - those that can be set through SCons argument variables (or a custom .py file)
custom_opts = []

# Indeed the data will be iterated through a few times, but it's absolutely unlikely that it will become a problem
# since the number of addons will hardly reach a problematic amount
# Nevertheless, first add the additional options
for entry in addon_data:
   implements = entry["implements"]
   
   custom_opts.append(BoolVariable(key="force_no_editor", help="Force removal of editor stuff on template_debug builds", default=0))
   custom_opts.append(BoolVariable(key="copy_bin_to_demo", help="If this is enabled then resulting binaries are copied into the demo project", default=0))

   for addon in implements:
      if "key" in addon:
         custom_opts.append(BoolVariable(key=addon["key"], help=addon["help"], default=1))

         # This implementation might offer extra options. Check them. Extra options always default to false
         if "extra_options" in addon:
            for extra in addon["extra_options"]:
               custom_opts.append(BoolVariable(key=extra["key"], help=extra["help"], default=extra["default"]))


# Create the godot_cpp in an "isolated" environment. Doing so allows setting Extension flags without interfering with godot-cpp. In other
# words, it prevents full godot-cpp rebuilds when the target is not changed but only extensions are added/removed through options/defines
env_gdcpp = SConscript("godot-cpp/SConstruct")


# And create the "local" environment specifically for this Extension
env = env_gdcpp.Clone()
env.Append(CPPPATH=["src/"])

# Get the options
opts = Variables(customs, ARGUMENTS)


# Then append own options
for opt in custom_opts:
   opts.Add(opt)


# Finally update the environment with the options
opts.Update(env)
Help(opts.GenerateHelpText(env))


### Check dependencies. That is, addons that are required by other enabled addons cannot be disabled. In this case
### automatically enable dependencies while also outputting a warning message
for entry in addon_data:
   implements = entry["implements"]

   for addon in implements:
      if not "key" in addon:
         continue
      if not env[addon["key"]]:
         continue
      
      check_requires(addon)


# Core source files - anything at the "root" will be added into the final build. Subdirectories will be handled based on the variables
sources = Glob("src/*.cpp")

# Assume there will be at least one icon added into the final .gdextension file - this will hold the string that will be added into the file
icons = "\n[icons]\n"

# Add paths to the icons here just so the relevant ones can be copied into the bin directory
icon_path = []

# UI Controls will always require "custom_theme" to be added. So use a flag to identify if there is at least one UI
# extension enabled. Assume all are disabled
has_ui = False

### Add source files based on JSON data and requested Addons (read settings)
# Each entry is obtained from the JSON and corresponds to a source (.cpp) file
for entry in addon_data:
   # Contains information of what this .cpp file implements
   implements = entry["implements"]

   # Assume all addons in this file are disabled
   add_file = False

   # If an addon requires additional files, those will be in an array. Store the data here
   extra_files = []

   for addon in implements:
      add_file = check_addon_implements(addon)

      if add_file:
         if addon["icon"]:
            icons += "{addon_name} = \"icons/{icon}\"\n".format(addon_name = addon["name"], icon = addon["icon"])
            icon_path += ["icons/{icon}".format(icon = addon["icon"])]
         if "extra_files" in addon:
            extra_files += addon["extra_files"]
   

   if add_file:
      sources += [entry["file"]]
      sources += extra_files

      if "src/ui/" in entry["file"]:
         has_ui = True


   elif "disabled_def" in entry:
      print("- Disabling {file} as none of its addons are enabled.".format(file = entry["file"]))
      env.Append(CPPDEFINES=entry["disabled_def"])


if has_ui:
   sources += ["src/ui/custom_theme.cpp", "src/ui/editor_theme_builder.cpp"]
else:
   env.Append(CPPDEFINES=["ALL_UI_DISABLED"])


# Add documentation if in build meant for editor
if (env["target"] in ["editor", "template_debug"]):
   doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
   sources.append(doc_data)


### The typical build for development (with editor) uses target=template_debug instead of target=editor
### This means that env.editor_build will be False but env.debug_features will be True.
### As a result of that, TOOLS_ENABLE is not defined. While a debug build might not need editor stuff,
### include that (basically by defining TOOLS_ENABLE). If a debug build is required without any of the
### editor things, then use the option "force_no_editor=1"
add_editor = (env.editor_build or env.debug_features) and not env["force_no_editor"]

if not env.editor_build and add_editor:
   # If here then editor has not been forced out while building a debug version. Define TOOLS_ENABLED so
   # C++ code can properly include editor code
   env.Append(CPPDEFINES=["TOOLS_ENABLED"])

if not add_editor:
   # Add this define so C++ code can rely on inverted #if logic, which helps with syntax highlighting
   env.Append(CPPDEFINES=["EDITOR_DISABLED"])


### Within 'godot-cpp/tools/godotcpp.py' the output static library name is built with this:
### This is required so when building the Extension the godot-cpp library can be correctly linked
libgodot = "libgodot-cpp" + env["suffix"] + env["LIBSUFFIX"]

### Determine the output name/path of the Extension library, based on platform
if env["platform"] == "macos":
   outpath = "bin/kehgdextpack.{}.{}.framework/kehgdextpack.{}.{}".format(env["platform"], env["target"], env["platform"], env["target"])
else:
   lib_prefix = ""
   if "is_msvc" in env and env["is_msvc"] == True:
      # MSVC does not prefix the file name with a "lib", however MingW-GCC does. To keep things consistent
      # and simplify the .gdextension contents, prefix the file name if compiling with MSVC.
      lib_prefix = "lib"
   outpath = "bin/{}kehgdextpack{}{}".format(lib_prefix, env["suffix"], env["SHLIBSUFFIX"])


### Build the extension while linking godot-cpp
library = env.SharedLibrary(outpath, source=sources, LIBS=[libgodot], LIBPATH='godot-cpp/bin')

# Copy the relevant icon files, both in to the "bin" and the "demo" directories. If none of the enabled addons
# require icons then clear up the string that will be used to correctly build the final .gdextension file
if len(icon_path) == 0:
   icons = ""
else:
   ibin = env.Install("bin/icons", icon_path)
   idemo = env.Install("demo/extensions/icons", icon_path)
   Default(ibin, idemo)


### Create the .gdextension file. 
gdfile = env.Substfile("bin/kehgdextpack.gdextension", "kehgdextpack.gdextension", SUBST_DICT={"{ICONS}": icons})

### Copy the generated library into the "demo" project
lib_copy = env.Install(target="demo/extensions", source=library[0]) if env["copy_bin_to_demo"] else []
gdf_copy = env.Install(target="demo/extensions", source="bin/kehgdextpack.gdextension") if env["copy_bin_to_demo"] else []



### "Commit" the generations
Default(library, lib_copy, gdfile, gdf_copy)

#print(env.Dump())
