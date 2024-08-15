# Profile used for debug builds (those that are needed for the editor).
# This profile is included for easier customization of the output library

# If set to 1 then all editor stuff (that is, editor plugins) will be removed from the debug build
force_no_editor = 0


# This is a debug helper that provides means to draw lines in 3D.
line3d_enabled = 1
# If the next line is 1 and the above one is also 1 then Line3D will not be removed but will be empty, effectively
# doing nothing. This is useful to hide debug stuff in release builds without requiring to delete the code using it
line3d_empty = 0

# Another debug helper, which provides easy ways to display labels on screen, including timed ones
overlayinfo_enabled = 1
# If the next line is 1 and the above one is also 1 then OverlayInfo will not be removed but will be empty, effectively
# doing nothing. This is useful to hide debug stuff in release builds without requiring to delete the code using it
overlayinfo_empty = 0

# Audio playback through stream player pools.
audiomaster_enabled = 1

# Nodes used to help achieve interpolation
autointerpolate_enabled = 1
smooth2d_enabled = 1
smooth3d_enabled = 1

# Functions that allow lossy compression of floating point numbers and rotation quaternions
quantize_enabled = 1

# Nodes that apply an impulse into all physical bodies that are within a configurable radius, from the
# location the radial impulse is placed at.
radial_impulse2d_enabled = 1
radial_impulse3d_enabled = 1

# A UI Control that borrows from Web UI. Buttons are added for each "category", allowing to show/hide
# corresponding contents bellow them.
accordion_enabled = 1

# A panel that attaches itself into a side of its owning container. It expands or shrinks to display
# multiple pages of contents
expandable_panel_enabled = 1

# A simple control that shows a file or directory selection with a button that brings the file/directory
# dialog to assign that into the line edit.
filedir_picker_enabled = 1

# A Control that works very similarly to the SpinBox, however if a value range is well defined the spin
# buttons are hidden and a slider is shown bellow the rendered value.
spin_slider_enabled = 1

# Allows editing values through a tabular display.
tabular_box_enabled = 1

# Provides a (relational) database system implemented using Godot resource system. Uses TabularBox to
# implement an editor plugin help visualize and edit databases.
database_enabled = 1
