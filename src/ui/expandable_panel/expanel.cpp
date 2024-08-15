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

#include "expanel.h"

#ifndef EXPANDABLE_PANEL_DISABLED

#include "../custom_theme.h"
#include "../../internal.h"
#include "../../extpackutils.h"

#include <godot_cpp/classes/box_container.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/classes/style_box_flat.hpp>
#include <godot_cpp/classes/texture2d.hpp>

#include <godot_cpp/variant/utility_functions.hpp>


// Converted from PNG to base64 strings using Godot itself!
static const char* ICON_ARROW_LEFT = "iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAYAAAAfSC3RAAAFUmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgeG1sbnM6ZXhpZj0iaHR0cDovL25zLmFkb2JlLmNvbS9leGlmLzEuMC8iCiAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyIKICAgIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgZXhpZjpQaXhlbFhEaW1lbnNpb249IjE0IgogICBleGlmOlBpeGVsWURpbWVuc2lvbj0iMTQiCiAgIGV4aWY6Q29sb3JTcGFjZT0iMSIKICAgdGlmZjpJbWFnZVdpZHRoPSIxNCIKICAgdGlmZjpJbWFnZUxlbmd0aD0iMTQiCiAgIHRpZmY6UmVzb2x1dGlvblVuaXQ9IjIiCiAgIHRpZmY6WFJlc29sdXRpb249IjcyLzEiCiAgIHRpZmY6WVJlc29sdXRpb249IjcyLzEiCiAgIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiCiAgIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSIKICAgeG1wOk1vZGlmeURhdGU9IjIwMjEtMTEtMDlUMTA6MzY6NTYtMDM6MDAiCiAgIHhtcDpNZXRhZGF0YURhdGU9IjIwMjEtMTEtMDlUMTA6MzY6NTYtMDM6MDAiPgogICA8ZGM6dGl0bGU+CiAgICA8cmRmOkFsdD4KICAgICA8cmRmOmxpIHhtbDpsYW5nPSJ4LWRlZmF1bHQiPmFycm93c18xNHgxNDwvcmRmOmxpPgogICAgPC9yZGY6QWx0PgogICA8L2RjOnRpdGxlPgogICA8eG1wTU06SGlzdG9yeT4KICAgIDxyZGY6U2VxPgogICAgIDxyZGY6bGkKICAgICAgc3RFdnQ6YWN0aW9uPSJwcm9kdWNlZCIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iQWZmaW5pdHkgUGhvdG8gMS4xMC40IgogICAgICBzdEV2dDp3aGVuPSIyMDIxLTExLTA5VDEwOjM2OjU2LTAzOjAwIi8+CiAgICA8L3JkZjpTZXE+CiAgIDwveG1wTU06SGlzdG9yeT4KICA8L3JkZjpEZXNjcmlwdGlvbj4KIDwvcmRmOlJERj4KPC94OnhtcG1ldGE+Cjw/eHBhY2tldCBlbmQ9InIiPz5KvIvkAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GjYxpFAsLZZqGldEYJTYWI4bCYowy2Lx55oeaN17vjSRbZasosfFrwV/AVlkrRaRkp6yJDdNznpmaSebczj2f+733nO49FxyxrKqZdUHQcnkjGgl7Z+NzXucLLhpw46NDUU19cno0RlX7vKfGjrcBu1b1c/+aazFpqlDTIDyk6kZeeEx4YjWv27wj3KpmlEXhM+FuQy4ofGfriSK/2pwu8rfNRiw6DI5mYW+6ghMVrGYMTVhejl/Lrqil+9gvaUrmZqYl+sTbMYkSIYyXcUYYpp9eBmXuJ0CIHllRJT/4mz/FsuSqMuusYbBEmgx5ukVdkepJiSnRkzKyrNn9/9tXM9UXKlZvCkP9s2W9d4JzGwpblvV1ZFmFY6h9gstcOX/5EAY+RN8qa/4D8GzA+VVZS+zCxSa0PeqKofxKteKOVAreTsEdh5YbaJwv9qy0z8kDxNblq65hbx+65Lxn4QdnMWfm3535lwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAGRJREFUKJFjYCACyMmF/peTC/2PLMZEjCYjI34McbwacWnCqxGfJpwaCWnCqpEYTRgaidWE1UZiAYrGR49WM54795E8G4nVjNWpxGjG6UdCmvEGDj7NLHjdA9XMwICawBkYGBgAmVcsBBVUf5gAAAAASUVORK5CYII=";
static const char* ICON_ARROW_RIGHT = "iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAYAAAAfSC3RAAAFUmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgeG1sbnM6ZXhpZj0iaHR0cDovL25zLmFkb2JlLmNvbS9leGlmLzEuMC8iCiAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyIKICAgIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgZXhpZjpQaXhlbFhEaW1lbnNpb249IjE0IgogICBleGlmOlBpeGVsWURpbWVuc2lvbj0iMTQiCiAgIGV4aWY6Q29sb3JTcGFjZT0iMSIKICAgdGlmZjpJbWFnZVdpZHRoPSIxNCIKICAgdGlmZjpJbWFnZUxlbmd0aD0iMTQiCiAgIHRpZmY6UmVzb2x1dGlvblVuaXQ9IjIiCiAgIHRpZmY6WFJlc29sdXRpb249IjcyLzEiCiAgIHRpZmY6WVJlc29sdXRpb249IjcyLzEiCiAgIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiCiAgIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSIKICAgeG1wOk1vZGlmeURhdGU9IjIwMjEtMTEtMDlUMTA6MzY6MzEtMDM6MDAiCiAgIHhtcDpNZXRhZGF0YURhdGU9IjIwMjEtMTEtMDlUMTA6MzY6MzEtMDM6MDAiPgogICA8ZGM6dGl0bGU+CiAgICA8cmRmOkFsdD4KICAgICA8cmRmOmxpIHhtbDpsYW5nPSJ4LWRlZmF1bHQiPmFycm93c18xNHgxNDwvcmRmOmxpPgogICAgPC9yZGY6QWx0PgogICA8L2RjOnRpdGxlPgogICA8eG1wTU06SGlzdG9yeT4KICAgIDxyZGY6U2VxPgogICAgIDxyZGY6bGkKICAgICAgc3RFdnQ6YWN0aW9uPSJwcm9kdWNlZCIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iQWZmaW5pdHkgUGhvdG8gMS4xMC40IgogICAgICBzdEV2dDp3aGVuPSIyMDIxLTExLTA5VDEwOjM2OjMxLTAzOjAwIi8+CiAgICA8L3JkZjpTZXE+CiAgIDwveG1wTU06SGlzdG9yeT4KICA8L3JkZjpEZXNjcmlwdGlvbj4KIDwvcmRmOlJERj4KPC94OnhtcG1ldGE+Cjw/eHBhY2tldCBlbmQ9InIiPz4PZT8WAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GjYxpFAsLZZqGldEYJTYWI4bCYowy2Lx55oeaN17vjSRbZasosfFrwV/AVlkrRaRkp6yJDdNznpmaSebczj2f+733nO49FxyxrKqZdUHQcnkjGgl7Z+NzXucLLhpw46NDUU19cno0RlX7vKfGjrcBu1b1c/+aazFpqlDTIDyk6kZeeEx4YjWv27wj3KpmlEXhM+FuQy4ofGfriSK/2pwu8rfNRiw6DI5mYW+6ghMVrGYMTVhejl/Lrqil+9gvaUrmZqYl+sTbMYkSIYyXcUYYpp9eBmXuJ0CIHllRJT/4mz/FsuSqMuusYbBEmgx5ukVdkepJiSnRkzKyrNn9/9tXM9UXKlZvCkP9s2W9d4JzGwpblvV1ZFmFY6h9gstcOX/5EAY+RN8qa/4D8GzA+VVZS+zCxSa0PeqKofxKteKOVAreTsEdh5YbaJwv9qy0z8kDxNblq65hbx+65Lxn4QdnMWfm3535lwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAFlJREFUKJGVy0kRgDAQBdEGAzgYEfESlXiJiDhAQbhSMMunj1314JFZX2Z9IbS/R2sHCv5AFbtQwSGscAozXMIIS9BLgmNczHluv6CHShihFGYohBVyoYIAbj1QLTtFizJoAAAAAElFTkSuQmCC";
static const char* ICON_ARROW_UP = "iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAYAAAAfSC3RAAAFUmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgeG1sbnM6ZXhpZj0iaHR0cDovL25zLmFkb2JlLmNvbS9leGlmLzEuMC8iCiAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyIKICAgIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgZXhpZjpQaXhlbFhEaW1lbnNpb249IjE0IgogICBleGlmOlBpeGVsWURpbWVuc2lvbj0iMTQiCiAgIGV4aWY6Q29sb3JTcGFjZT0iMSIKICAgdGlmZjpJbWFnZVdpZHRoPSIxNCIKICAgdGlmZjpJbWFnZUxlbmd0aD0iMTQiCiAgIHRpZmY6UmVzb2x1dGlvblVuaXQ9IjIiCiAgIHRpZmY6WFJlc29sdXRpb249IjcyLzEiCiAgIHRpZmY6WVJlc29sdXRpb249IjcyLzEiCiAgIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiCiAgIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSIKICAgeG1wOk1vZGlmeURhdGU9IjIwMjEtMTEtMDlUMTA6Mzg6MjEtMDM6MDAiCiAgIHhtcDpNZXRhZGF0YURhdGU9IjIwMjEtMTEtMDlUMTA6Mzg6MjEtMDM6MDAiPgogICA8ZGM6dGl0bGU+CiAgICA8cmRmOkFsdD4KICAgICA8cmRmOmxpIHhtbDpsYW5nPSJ4LWRlZmF1bHQiPmFycm93c18xNHgxNDwvcmRmOmxpPgogICAgPC9yZGY6QWx0PgogICA8L2RjOnRpdGxlPgogICA8eG1wTU06SGlzdG9yeT4KICAgIDxyZGY6U2VxPgogICAgIDxyZGY6bGkKICAgICAgc3RFdnQ6YWN0aW9uPSJwcm9kdWNlZCIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iQWZmaW5pdHkgUGhvdG8gMS4xMC40IgogICAgICBzdEV2dDp3aGVuPSIyMDIxLTExLTA5VDEwOjM4OjIxLTAzOjAwIi8+CiAgICA8L3JkZjpTZXE+CiAgIDwveG1wTU06SGlzdG9yeT4KICA8L3JkZjpEZXNjcmlwdGlvbj4KIDwvcmRmOlJERj4KPC94OnhtcG1ldGE+Cjw/eHBhY2tldCBlbmQ9InIiPz6xNAdtAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GjYxpFAsLZZqGldEYJTYWI4bCYowy2Lx55oeaN17vjSRbZasosfFrwV/AVlkrRaRkp6yJDdNznpmaSebczj2f+733nO49FxyxrKqZdUHQcnkjGgl7Z+NzXucLLhpw46NDUU19cno0RlX7vKfGjrcBu1b1c/+aazFpqlDTIDyk6kZeeEx4YjWv27wj3KpmlEXhM+FuQy4ofGfriSK/2pwu8rfNRiw6DI5mYW+6ghMVrGYMTVhejl/Lrqil+9gvaUrmZqYl+sTbMYkSIYyXcUYYpp9eBmXuJ0CIHllRJT/4mz/FsuSqMuusYbBEmgx5ukVdkepJiSnRkzKyrNn9/9tXM9UXKlZvCkP9s2W9d4JzGwpblvV1ZFmFY6h9gstcOX/5EAY+RN8qa/4D8GzA+VVZS+zCxSa0PeqKofxKteKOVAreTsEdh5YbaJwv9qy0z8kDxNblq65hbx+65Lxn4QdnMWfm3535lwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAE5JREFUKJFjYBj6QE4u9L+cXOh/XPIsuDQZGfFDeaH/Hz1azYiuBkMAVRMEnDv3kQFdMyMhTbg0MxKjCZtmRmI1oWtmJEUTsmYWGGMYAwD94SwS4U2/MAAAAABJRU5ErkJggg==";
static const char* ICON_ARROW_DOWN = "iVBORw0KGgoAAAANSUhEUgAAAA4AAAAOCAYAAAAfSC3RAAAFUmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6ZGM9Imh0dHA6Ly9wdXJsLm9yZy9kYy9lbGVtZW50cy8xLjEvIgogICAgeG1sbnM6ZXhpZj0iaHR0cDovL25zLmFkb2JlLmNvbS9leGlmLzEuMC8iCiAgICB4bWxuczp0aWZmPSJodHRwOi8vbnMuYWRvYmUuY29tL3RpZmYvMS4wLyIKICAgIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgZXhpZjpQaXhlbFhEaW1lbnNpb249IjE0IgogICBleGlmOlBpeGVsWURpbWVuc2lvbj0iMTQiCiAgIGV4aWY6Q29sb3JTcGFjZT0iMSIKICAgdGlmZjpJbWFnZVdpZHRoPSIxNCIKICAgdGlmZjpJbWFnZUxlbmd0aD0iMTQiCiAgIHRpZmY6UmVzb2x1dGlvblVuaXQ9IjIiCiAgIHRpZmY6WFJlc29sdXRpb249IjcyLzEiCiAgIHRpZmY6WVJlc29sdXRpb249IjcyLzEiCiAgIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiCiAgIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSIKICAgeG1wOk1vZGlmeURhdGU9IjIwMjEtMTEtMDlUMTA6Mzg6MzgtMDM6MDAiCiAgIHhtcDpNZXRhZGF0YURhdGU9IjIwMjEtMTEtMDlUMTA6Mzg6MzgtMDM6MDAiPgogICA8ZGM6dGl0bGU+CiAgICA8cmRmOkFsdD4KICAgICA8cmRmOmxpIHhtbDpsYW5nPSJ4LWRlZmF1bHQiPmFycm93c18xNHgxNDwvcmRmOmxpPgogICAgPC9yZGY6QWx0PgogICA8L2RjOnRpdGxlPgogICA8eG1wTU06SGlzdG9yeT4KICAgIDxyZGY6U2VxPgogICAgIDxyZGY6bGkKICAgICAgc3RFdnQ6YWN0aW9uPSJwcm9kdWNlZCIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iQWZmaW5pdHkgUGhvdG8gMS4xMC40IgogICAgICBzdEV2dDp3aGVuPSIyMDIxLTExLTA5VDEwOjM4OjM4LTAzOjAwIi8+CiAgICA8L3JkZjpTZXE+CiAgIDwveG1wTU06SGlzdG9yeT4KICA8L3JkZjpEZXNjcmlwdGlvbj4KIDwvcmRmOlJERj4KPC94OnhtcG1ldGE+Cjw/eHBhY2tldCBlbmQ9InIiPz6jDFmmAAABgWlDQ1BzUkdCIElFQzYxOTY2LTIuMQAAKJF1kc8rRFEUxz8GjYxpFAsLZZqGldEYJTYWI4bCYowy2Lx55oeaN17vjSRbZasosfFrwV/AVlkrRaRkp6yJDdNznpmaSebczj2f+733nO49FxyxrKqZdUHQcnkjGgl7Z+NzXucLLhpw46NDUU19cno0RlX7vKfGjrcBu1b1c/+aazFpqlDTIDyk6kZeeEx4YjWv27wj3KpmlEXhM+FuQy4ofGfriSK/2pwu8rfNRiw6DI5mYW+6ghMVrGYMTVhejl/Lrqil+9gvaUrmZqYl+sTbMYkSIYyXcUYYpp9eBmXuJ0CIHllRJT/4mz/FsuSqMuusYbBEmgx5ukVdkepJiSnRkzKyrNn9/9tXM9UXKlZvCkP9s2W9d4JzGwpblvV1ZFmFY6h9gstcOX/5EAY+RN8qa/4D8GzA+VVZS+zCxSa0PeqKofxKteKOVAreTsEdh5YbaJwv9qy0z8kDxNblq65hbx+65Lxn4QdnMWfm3535lwAAAAlwSFlzAAALEwAACxMBAJqcGAAAAF1JREFUKJHV0bENwCAMBdGzlD4beAh2Ycrs4iHYgAmcCgkBQYQqufJbrzL8JlGNvgMPgBDOV8gsIwCq0VexWSalS6QMK7ggAKkPM1yjDj7hFg1hi0dommr03Vd9rBsbTyq7zrOhHQAAAABJRU5ErkJggg==";


ExpandablePanel::AnimationState::AnimationState()
{
   time = 0.0;
   target_time = 0.0;
   starting = -1.0;
   ending = -1.0;
   target_page = -1;
}


float ExpandablePanel::AnimationState::calculate() const
{
  float alpha = target_time > 0.0 ? (time / target_time) : 1.0;
  
  if (curve.is_valid())
  {
      alpha = curve->sample(alpha);
  }

  return godot::Math::lerp(starting, ending, alpha);
}


void ExpandablePanel::calculate_sizes()
{
   // Find the widest icon dimension as well as the tallest icon dimension.
   {
      godot::Size2i size(0, 0);

      if (m_page_array.size() == 0)
      {
         // If there are no pages use the default icons to calculate a minimum bar size. Since all four default icons are of the exact
         // same dimensions there is no difference in which one is used.
         // Perform this initialization only if there are no pages just so smaller custom icons can be used without forcing everything
         // to the default icons' sizes
         size = TEX_ARROW_LEFT->get_size();
      }

      for (const PanelPage& page : m_page_array)
      {
         godot::Ref<godot::Texture2D> icon1 = page.control->get_meta("_expanded_icon", is_vertical() ? TEX_ARROW_LEFT : TEX_ARROW_UP);
         godot::Ref<godot::Texture2D> icon2 = page.control->get_meta("_shrunk_icon", is_vertical() ? TEX_ARROW_RIGHT : TEX_ARROW_DOWN);

         if (icon1.is_valid())
         {
            size.width = godot::Math::max<int32_t>(size.width, icon1->get_width());
            size.height = godot::Math::max<int32_t>(size.height, icon1->get_height());
         }
         if (icon2.is_valid())
         {
            size.width = godot::Math::max<int32_t>(size.width, icon2->get_width());
            size.height = godot::Math::max<int32_t>(size.height, icon2->get_height());
         }
      }

      m_size_cache.icon = size;
   }

   // Using the calculated icon size, calculate the button size, taking into consideration the style boxes of all possible
   // states the buttons can be in
   {
      godot::Size2i size = m_size_cache.icon;

      const godot::Size2i size1 = m_theme_cache.button_normal->get_minimum_size();
      const godot::Size2i size2 = m_theme_cache.button_hover->get_minimum_size();
      const godot::Size2i size3 = m_theme_cache.button_pressed->get_minimum_size();
      const godot::Size2i size4 = m_theme_cache.button_disabled->get_minimum_size();
      const godot::Size2i size5 = m_theme_cache.button_focus->get_minimum_size();

      const int32_t width = Internals::vmax<int32_t>(size1.width, size2.width, size3.width, size4.width, size5.width);
      const int32_t height = Internals::vmax<int32_t>(size1.height, size2.height, size3.height, size4.height, size5.height);

      size.width += width;
      size.height += height;

      size.width = godot::Math::max<int32_t>(size.width, m_theme_cache.button_min_width);
      size.height = godot::Math::max<int32_t>(size.height, m_theme_cache.button_min_height);

      m_size_cache.button = size;
   }

   // Finally calculate the bar size, which should use button and the style box used to draw the bar
   {
      const godot::Size2i size = m_theme_cache.bar->get_minimum_size();
      m_size_cache.bar = m_size_cache.button + size;
   }

   for (PanelPage& page : m_page_array)
   {
      page.button->set_custom_minimum_size(m_size_cache.button);
   }
}


void ExpandablePanel::apply_button_style(godot::Button* bt)
{
   bt->set_clip_text(true);
   bt->set_toggle_mode(true);

   bt->begin_bulk_theme_override();
   bt->add_theme_stylebox_override("normal", m_theme_cache.button_normal);
   bt->add_theme_stylebox_override("hover_pressed", m_theme_cache.button_pressed);
   bt->add_theme_stylebox_override("pressed", m_theme_cache.button_pressed);
   bt->add_theme_stylebox_override("hover", m_theme_cache.button_hover);
   bt->add_theme_stylebox_override("disabled", m_theme_cache.button_disabled);
   bt->add_theme_stylebox_override("focus", m_theme_cache.button_focus);
   bt->end_bulk_theme_override();

   bt->set_icon_alignment(godot::HORIZONTAL_ALIGNMENT_CENTER);
   bt->set_expand_icon(m_expand_button_icon);

   if (is_vertical())
   {
      bt->set_h_size_flags(SIZE_SHRINK_CENTER);
      bt->set_v_size_flags(SIZE_FILL);
   }
   else
   {
      bt->set_h_size_flags(SIZE_FILL);
      bt->set_v_size_flags(SIZE_SHRINK_CENTER);
   }
}


godot::Ref<godot::Texture2D> ExpandablePanel::get_page_icon(const PanelPage& page) const
{
   const bool expanded = (get_view_page() == page.index);

   // First check if there is any custom icon assigned to the page
   if (expanded)
   {
      godot::Ref<godot::Texture2D> exp_icon = page.control->get_meta("_expanded_icon", godot::Ref<godot::Texture2D>());
      if (exp_icon.is_valid())
      {
         return exp_icon;
      }
   }
   else
   {
      godot::Ref<godot::Texture2D> shk_icon = page.control->get_meta("_shrunk_icon", godot::Ref<godot::Texture2D>());
      if (shk_icon.is_valid())
      {
         return shk_icon;
      }
   }

   // There is no custom icon. Use the default one
   godot::Ref<godot::Texture2D> ret;

   switch (m_attachto)
   {
      case Left:
      {
         ret = expanded ? TEX_ARROW_LEFT : TEX_ARROW_RIGHT;
      } break;

      case Top:
      {
         ret = expanded ? TEX_ARROW_UP : TEX_ARROW_DOWN;
      } break;

      case Right:
      {
         ret = expanded ? TEX_ARROW_RIGHT : TEX_ARROW_LEFT;
      } break;

      case Bottom:
      {
         ret = expanded ? TEX_ARROW_DOWN : TEX_ARROW_UP;
      } break;
   }

   // If here there is an error in the code. Yet, this return must be here otherwise the compiler will complain
   return ret;
}


void ExpandablePanel::assign_button_tooltip(const PanelPage& page)
{
   if (m_use_tooltip)
   {
      // For some weird reason directly inserting "get_page_tooltip()" within the "set_tooltip_text()" isn't working!
      const godot::String tt = get_page_tooltip(page.index);
      page.button->set_tooltip_text(tt);
   }
   else
   {
      page.button->set_tooltip_text("");
   }
}


int ExpandablePanel::get_control_width() const
{
   // In here take into account animation as well as expanded/shrunk states
   if (godot::Engine::get_singleton()->is_editor_hint())
   {
      // But animation is not meant to occur in editor
      return get_view_page() != -1 ? m_expanded_width : m_size_cache.bar.width;
   }

   int ret = m_size_cache.bar.width;

   if (m_animation_queue.size() > 0 && is_vertical())
   {
      ret = (int)m_animation_queue[0].calculate();
   }
   else if (get_view_page() != -1)
   {
      ret = godot::Math::max<int>(m_expanded_width, MIN_CONTENT_WIDTH);
   }

   return ret;
}


int ExpandablePanel::get_control_height() const
{
   // In here take into account animation as well as expanded/shrunk states
   if (godot::Engine::get_singleton()->is_editor_hint())
   {
      // But animation is not meant to occur in editor
      return get_view_page() != -1 ? m_expanded_height : m_size_cache.bar.height;
   }

   int ret = m_size_cache.bar.height;

   if (m_animation_queue.size() && !is_vertical())
   {
      ret = (int)m_animation_queue[0].calculate();
   }
   else if (get_view_page() != -1)
   {
      ret = godot::Math::max<int>(m_expanded_height, MIN_CONTENT_HEIGHT);
   }

   return ret;
}


int32_t ExpandablePanel::count_pages() const
{
   int32_t ret = 0;

   const int32_t ccount = get_child_count();
   for (int32_t i = 0; i < ccount; i++)
   {
      const godot::Control* ctrl = godot::Object::cast_to<godot::Control>(get_child(i));
      if (ctrl && !ctrl->is_set_as_top_level() && ctrl != m_togglebox)
      {
         ret++;
      }
   }

   return ret;
}


void ExpandablePanel::refresh_pages(int32_t diff_toggle)
{
   // The children order might even be different. In other words, it's way easier to simply rebuild the array
   m_page_array.clear();

   const godot::Callable button_clicked = callable_mp(this, &ExpandablePanel::on_page_button_clicked);
   
   if (diff_toggle > 0)
   {
      // Function caller requested to create toggle buttons. The exact amount is given by diff_toggle.
      for (int32_t i = 0; i < diff_toggle; i++)
      {
         godot::Button* bt = memnew(godot::Button);
         bt->set_text("");
         m_togglebox->add_child(bt);

         if (!bt->is_connected("pressed", button_clicked))
         {
            bt->connect("pressed", button_clicked.bind(bt));
         }
      }
   }
   else if (diff_toggle < 0)
   {
      // Function caller requested to remove toggle buttons. The exact amount is given by diff_toggle. Well, the
      // positive of diff_toggle!
      const int to_rem = -diff_toggle;
      for (int32_t i = 0; i < to_rem; i++)
      {
         const int32_t to_rem = m_togglebox->get_child_count() - 1;
         m_togglebox->get_child(to_rem)->queue_free();
      }
   }

   const int32_t view_page = get_view_page();
   const int32_t ccount = get_child_count();
   const godot::Callable on_node_renamed = callable_mp(this, &ExpandablePanel::on_node_renamed);

   for (int32_t i = 0; i < ccount; i++)
   {
      godot::Control* ctrl = godot::Object::cast_to<godot::Control>(get_child(i));
      if (!ctrl || ctrl->is_set_as_top_level() || ctrl == m_togglebox)
      {
         continue;
      }

      const int32_t index = m_page_array.size();
      godot::Button* bt = godot::Object::cast_to<godot::Button>(m_togglebox->get_child(index));

      PanelPage pageinfo(m_page_array.size(), ctrl, bt);
      m_page_array.push_back(pageinfo);

      pageinfo.button->set_meta("_page_index", pageinfo.index);
      const bool disabled = pageinfo.control->get_meta("_disabled", false);
      pageinfo.button->set_disabled(disabled);
      
      assign_button_tooltip(pageinfo);
      pageinfo.button->set_button_icon(get_page_icon(pageinfo));
      apply_button_style(pageinfo.button);
      pageinfo.button->set_custom_minimum_size(m_size_cache.button);

      if (!pageinfo.control->is_connected("renamed", on_node_renamed))
      {
         pageinfo.control->connect("renamed", on_node_renamed.bind(pageinfo.control));
      }

      ctrl->set_visible(view_page == pageinfo.index);
   }

   notify_property_list_changed();
}


int ExpandablePanel::get_view_page() const
{
#ifdef DEBUG_ENABLED
   if (godot::Engine::get_singleton()->is_editor_hint())// && m_page.size() > 0)
   {
      //godot::UtilityFunctions::print("Requesting view page. Returning preview page, which is ", m_preview_page);
      return m_preview_page;
   }
#endif

   //godot::UtilityFunctions::print("Requesting view page. Returning current page, which is ", m_current_page);
   return m_current_page;
}


int32_t ExpandablePanel::get_page_index_by_name(const godot::String& pname) const
{
   const int32_t pcount = m_page_array.size();
   for (int32_t i = 0; i < pcount; i++)
   {
      if (m_page_array[i].control->get_name() == pname)
      {
         return i;
      }
   }

   return -1;
}


void ExpandablePanel::check_panel_anchors()
{
   switch (m_attachto)
   {
      case Left:
      {
         ExtPackUtils::set_control_anchors(this, 0.0, 0.0, 0.0, 1.0);
         ExtPackUtils::set_control_offsets(this, 0.0, 0.0, get_control_width(), 0.0);
      } break;

      case Top:
      {
         ExtPackUtils::set_control_anchors(this, 0.0, 0.0, 1.0, 0.0);
         ExtPackUtils::set_control_offsets(this, 0.0, 0.0, 0.0, get_control_height());
      } break;

      case Right:
      {
         ExtPackUtils::set_control_anchors(this, 1.0, 0.0, 1.0, 1.0);
         ExtPackUtils::set_control_offsets(this, -get_control_width(), 0.0, 0.0, 0.0);
      } break;

      case Bottom:
      {
         ExtPackUtils::set_control_anchors(this, 0.0, 1.0, 1.0, 1.0);
         ExtPackUtils::set_control_offsets(this, 0.0, -get_control_height(), 0.0, 0.0);
      } break;
   }
}


void ExpandablePanel::check_layout()
{
   const godot::Vector2 current_size = get_size();

   // get_control_width() and get_control_height() take current animation state into account
   const int current_width = get_control_width();
   const int current_height = get_control_height();

   // Just two "shortcuts"
   const int bar_width = m_size_cache.bar.width;
   const int bar_height = m_size_cache.bar.height;

   // Background margins
   const int bg_ml = m_theme_cache.background->get_margin(godot::Side::SIDE_LEFT);
   const int bg_mt = m_theme_cache.background->get_margin(godot::Side::SIDE_TOP);
   const int bg_mr = m_theme_cache.background->get_margin(godot::Side::SIDE_RIGHT);
   const int bg_mb = m_theme_cache.background->get_margin(godot::Side::SIDE_BOTTOM);

   // Horizontal and vertical margin sizes
   const int bg_hor = bg_ml + bg_mr;
   const int bg_ver = bg_mt + bg_mb;

   godot::Rect2 bar_rect;
   godot::Rect2 page_rect;

   switch (m_attachto)
   {
      case Left:
      {
         bar_rect.position = godot::Vector2(current_width - bar_width, 0);
         bar_rect.size = godot::Vector2(bar_width, current_size.y);

         page_rect.position = godot::Vector2(bg_ml, bg_mt);
         page_rect.size = godot::Vector2(current_width - bar_width - bg_hor, current_size.y - bg_ver);
      } break;

      case Top:
      {
         bar_rect.position = godot::Vector2(0, current_height - bar_height);
         bar_rect.size = godot::Vector2(current_size.x, bar_height);

         page_rect.position = godot::Vector2(bg_ml, bg_mt);
         page_rect.size = godot::Vector2(current_size.x - bg_hor, current_height - bar_height - bg_ver);
      } break;

      case Right:
      {
         bar_rect.position = godot::Vector2();
         bar_rect.size = godot::Vector2(bar_width, current_size.y);

         page_rect.position = godot::Vector2(bar_width + bg_ml, bg_mt);
         page_rect.size = godot::Vector2(current_width - bar_width - bg_hor, current_size.y - bg_ver);
      } break;

      case Bottom:
      {
         bar_rect.position = godot::Vector2();
         bar_rect.size = godot::Vector2(current_size.x, bar_height);

         page_rect.position = godot::Vector2(bg_ml, bar_height + bg_mt);
         page_rect.size = godot::Vector2(current_size.x - bg_hor, current_height - bg_ver);
      } break;
   }

   m_togglebox->set_custom_minimum_size(bar_rect.size);
   fit_child_in_rect(m_togglebox, bar_rect);

   const int cpage = get_view_page();
   if (cpage != -1 && m_page_array.size() > 0)
   {
      fit_child_in_rect(m_page_array[cpage].control, page_rect);
   }
}

void ExpandablePanel::refresh_layout()
{
   check_panel_anchors();
   check_layout();
}


void ExpandablePanel::handle_animation(float dt)
{
   if (m_animation_queue.size() == 0)
   {
      set_process_internal(false);
      set_physics_process_internal(false);
      return;
   }

   AnimationState* state = m_animation_queue.ptrw();

   const bool expanding = state[0].is_expanding();
   const PanelPage& pageinfo = m_page_array[state[0].target_page];

   if (state[0].time == 0.0)
   {
      if (expanding)
      {
         pageinfo.control->set_visible(true);
         m_current_page = state[0].target_page;

         emit_signal("expand_started", pageinfo.index);
      }
      else
      {
         m_current_page = -1;

         emit_signal("shrink_started", pageinfo.index);
      }

      pageinfo.button->set_button_icon(get_page_icon(pageinfo));
   }

   if (state[0].update(dt))
   {
      if (expanding)
      {
         emit_signal("expand_finished", pageinfo.index);
      }
      else
      {
         pageinfo.control->set_visible(false);
         emit_signal("shrink_finished", pageinfo.index);
      }

      m_animation_queue.remove_at(0);
   }

   refresh_layout();
}


void ExpandablePanel::on_draw_button_box()
{
   m_togglebox->draw_style_box(m_theme_cache.bar, godot::Rect2(godot::Vector2(), m_togglebox->get_size()));
}


void ExpandablePanel::on_draw_main()
{
   const godot::Vector2 ctrl_size = get_size();

   godot::Rect2 rect;

   switch (m_attachto)
   {
      case Left:
      {
         rect.size = godot::Vector2(ctrl_size.x - m_togglebox->get_size().x, ctrl_size.y);
      } break;

      case Top:
      {
         rect.size = godot::Vector2(ctrl_size.x, ctrl_size.y - m_togglebox->get_size().y);
      } break;

      case Right:
      {
         const real_t tglw = m_togglebox->get_size().x;
         rect.size = godot::Vector2(ctrl_size.x - tglw, ctrl_size.y);
         rect.position.x = tglw;
      } break;

      case Bottom:
      {
         const real_t tglh = m_togglebox->get_size().y;
         rect.size = godot::Vector2(ctrl_size.x, ctrl_size.y - tglh);
         rect.position.y = tglh;
      } break;
   }

#ifdef DEBUG_ENABLED
   if (godot::Engine::get_singleton()->is_editor_hint() && m_preview_page == -1)
   {
      rect.size = godot::Vector2();
   }
#endif

   draw_style_box(m_theme_cache.background, rect);
}


void ExpandablePanel::on_child_order_changed()
{
   // This event is triggered whenever a child node is added, removed or moved. Note that it may happen even
   // during loading. Bailing if not inside the tree has the advantage of helping differentiate a node removal
   // that occurs from direct removal vs when this container is also being removed from the tree.
   // Being able to identify those is required as when closing the editor this function will be triggered
   if (!is_inside_tree())
   {
      // Either loading or closing the scene. Bail to prevent:
      // - Unnecessary upkeep
      // - Cleanup that would cause data loss
      return;
   }

   const int32_t page_count = count_pages();
   refresh_pages(page_count - m_page_array.size());
}

void ExpandablePanel::on_node_renamed(godot::Node* node)
{
   // It's absolutely unlikely that this panel will ever hold enough controls to render this linear search bad
   const int32_t pcount = m_page_array.size();
   for (int32_t i = 0; i < pcount; i++)
   {
      const PanelPage& pageinfo = m_page_array[i];

      if (pageinfo.control == node)
      {
         assign_button_tooltip(pageinfo);
         break;
      }
   }

   notify_property_list_changed();
}


void ExpandablePanel::on_page_button_clicked(godot::Button* button)
{
   const int pgindex = button->get_meta("_page_index", -2);

   if (pgindex == -2)
   {
      godot::UtilityFunctions::push_error("Requesting to change page, but toggle button does not have expected metadata.");
      return;
   }

   set_current_page(pgindex == m_current_page ? -1 : pgindex);

   if (!m_keep_button_focus)
   {
      button->release_focus();
   }

   emit_signal("page_button_clicked", pgindex);
}




void ExpandablePanel::_get_property_list(godot::List<godot::PropertyInfo>* list) const
{
   using namespace godot;

   // An empty group name will reset property grouping.
   list->push_back(PropertyInfo(Variant::NIL, "", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_GROUP));

   list->push_back(PropertyInfo(Variant::INT, "__expanded_width", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   list->push_back(PropertyInfo(Variant::INT, "__expanded_height", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE));
   list->push_back(PropertyInfo(Variant::INT, "preview_page", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));

   const godot::String formatter("pages/%s/%s");
   const int32_t pcount = m_page_array.size();
   for (int32_t i = 0; i < pcount; i++)
   {
      // None of the following properties should be directly serialized. The relevant data is actually saved/loaded to/from
      // metadata set within the page nodes themselves. This exposing here is just to allow easy editing of the customization
      // data through the inspector
      const godot::String page_name = m_page_array[i].control->get_name();

      list->push_back(PropertyInfo(Variant::STRING, vformat(formatter, page_name, "tooltip"), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));
      list->push_back(PropertyInfo(Variant::OBJECT, vformat(formatter, page_name, "icon_expanded"), PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", PROPERTY_USAGE_EDITOR));
      list->push_back(PropertyInfo(Variant::OBJECT, vformat(formatter, page_name, "icon_shrunk"), PROPERTY_HINT_RESOURCE_TYPE, "Texture2D", PROPERTY_USAGE_EDITOR));
      list->push_back(PropertyInfo(Variant::BOOL, vformat(formatter, page_name, "disabled"), PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR));
   }

   // Append the custom theme entries
   CustomControlThemeDB::get_singleton()->fill_property_list("ExpandablePanel", list, this);
}

bool ExpandablePanel::_set(const godot::StringName& prop_name, const godot::Variant& value)
{
   if (prop_name == godot::StringName("__expanded_width"))
   {
      m_expanded_width = value;
      callable_mp(this, &ExpandablePanel::refresh_layout).call_deferred();
      return true;
   }
   else if (prop_name == godot::StringName("__expanded_height"))
   {
      m_expanded_height = value;
      callable_mp(this, &ExpandablePanel::refresh_layout).call_deferred();
      return true;
   }
   
#ifdef DEBUG_ENABLED
   else if (prop_name == godot::StringName("preview_page"))
   {
      const int32_t pcount = m_page_array.size();

      m_preview_page = value;
      if (m_preview_page < -1)
      {
         m_preview_page = -1;
      }
      else if (m_preview_page >= pcount && pcount > 0)
      {
         m_preview_page = pcount - 1;
      }
      else if (pcount == 0)
      {
         m_preview_page = godot::Math::min<int>(0, m_preview_page);
      }

      if (godot::Engine::get_singleton()->is_editor_hint())
      {
         for (int32_t i = 0; i < pcount; i++)
         {
            m_page_array[i].control->set_visible(i == m_preview_page);
         }
      }

      // The loop above changes visibility of pages. This will trigger a "NOTIFICATION_SORT_CHILDREN". At that moment
      // the correct tasks will be taken to ensure the layout will be respected

      queue_redraw();

      return true;
   }
#endif

   const godot::PackedStringArray prop_section = prop_name.split("/", true, 2);

   if (prop_section[0] == "pages")
   {
      const int32_t pindex = get_page_index_by_name(prop_section[1]);

      if (pindex == -1)
      {
         return false;
      }

      if (prop_section[2] == "tooltip")
      {
         set_page_tooltip(pindex, value);
      }
      else if (prop_section[2] == "icon_expanded")
      {
         set_page_icon_expanded(pindex, value);
      }
      else if (prop_section[2] == "icon_shrunk")
      {
         set_page_icon_shrunk(pindex, value);
      }
      else if (prop_section[2] == "disabled")
      {
         set_page_disabled(pindex, value);
      }


      return true;
   }

   return false;
}

bool ExpandablePanel::_get(const godot::StringName& prop_name, godot::Variant& out_value) const
{
   if (prop_name == godot::StringName("__expanded_width"))
   {
      out_value = m_expanded_width;
      return true;
   }
   else if (prop_name == godot::StringName("__expanded_height"))
   {
      out_value = m_expanded_height;
      return true;
   }
#ifdef DEBUG_ENABLED
   else if (prop_name == godot::StringName("preview_page"))
   {
      out_value = m_preview_page;
      return true;
   }
#endif

   const godot::PackedStringArray prop_section = prop_name.split("/", true, 2);

   if (prop_section[0] == "pages")
   {
      const godot::Control* ctrl = get_node<godot::Control>(prop_section[1]);

      if (!ctrl)
      {
         return false;
      }

      if (prop_section[2] == "tooltip")
      {
         out_value = ctrl->get_meta("_page_name", "");
      }
      else if (prop_section[2] == "icon_expanded")
      {
         out_value = ctrl->get_meta("_expanded_icon", godot::Ref<godot::Texture2D>());
      }
      else if (prop_section[2] == "icon_shrunk")
      {
         out_value = ctrl->get_meta("_shrunk_icon", godot::Ref<godot::Texture2D>());
      }
      else if (prop_section[2] == "disabled")
      {
         out_value = ctrl->get_meta("_disabled", false);
      }


      return true;
   }


   return false;
}



void ExpandablePanel::_notification(int what)
{
   switch (what)
   {
      case NOTIFICATION_POSTINITIALIZE:
      {
         CustomControlThemeDB::get_singleton()->update_control(this);
         calculate_sizes();

         // Connecting events within Constructor results in error message. To be more specific:
         // "_instance_bindings != nullptr && _instance_bindings[0].binding != nullptr" is true."
         m_togglebox->connect("draw", callable_mp(this, &ExpandablePanel::on_draw_button_box));
         connect("child_order_changed", callable_mp(this, &ExpandablePanel::on_child_order_changed));
         m_togglebox->queue_redraw();
      } break;

      case NOTIFICATION_READY:
      {
         // Probably not necessary to disable all four, but do so just to ensure none is enabled
         set_process(false);
         set_process_internal(false);
         set_physics_process(false);
         set_physics_process_internal(false);
      } break;

      case NOTIFICATION_ENTER_TREE:
      {
         // Deal with the pages...
         const int32_t pcount = count_pages();
         if (pcount > 0)
         {
            // Changing scenes in editor will trigger this notification, meaning that m_page_array might not be empty
            refresh_pages(pcount - m_page_array.size());
         }
      } break;


      case NOTIFICATION_PREDELETE:
      {
         while (m_togglebox->get_child_count() > 0)
         {
            godot::Node* c = m_togglebox->get_child(0);
            m_togglebox->remove_child(c);
            memdelete(c);
         }
      } break;

      case NOTIFICATION_THEME_CHANGED:
      {
         // Ensure the cache is updated
         CustomControlThemeDB::get_singleton()->update_control(this);
         calculate_sizes();

         // Apply the separation into the internal toggle button box
         m_togglebox->add_theme_constant_override("separation", m_theme_cache.toggle_button_separation);

         //const int32_t btw = get_button_width();
         //const int32_t bth = get_button_height();
         for (PanelPage& page : m_page_array)
         {
            //apply_style_to_button(page.button, btw, bth);
            apply_button_style(page.button);
         }

         // If this call is not deferred the content size will not be correctly used when checking the layout. The end result is
         // that upon project loading the panel will become really small and the desired expanded width/height will be lost.
         // Another thing is that a newly added ExpandablePanel should be automatically attached to the left. If this call is not
         // deferred that attachment will not occur.
         callable_mp(this, &ExpandablePanel::refresh_layout).call_deferred();

         // If the minimum size did actually change, notify the engine
         update_minimum_size();

         // Request a redraw of the toggle button box
         m_togglebox->queue_redraw();
      } break;

      case NOTIFICATION_DRAW:
      {
         on_draw_main();
      } break;

      case NOTIFICATION_INTERNAL_PROCESS:
      {
         handle_animation(get_process_delta_time());
      } break;

      case NOTIFICATION_INTERNAL_PHYSICS_PROCESS:
      {
         handle_animation(get_physics_process_delta_time());
      } break;

      case NOTIFICATION_PRE_SORT_CHILDREN:
      {
         m_togglebox->set_vertical(is_vertical());
      } break;

      case NOTIFICATION_SORT_CHILDREN:
      {
         check_layout();
      } break;

      case NOTIFICATION_RESIZED:
      {
         if (godot::Engine::get_singleton()->is_editor_hint())// && get_view_page() >= 0)
         {
            if (is_vertical())
            {
               m_expanded_width = get_size().x;
            }
            else
            {
               m_expanded_height = get_size().y;
            }
         }
      } break;
   }
}



void ExpandablePanel::_bind_methods()
{
   using namespace godot;

   /// Functions
   ClassDB::bind_method(D_METHOD("get_attach_to"), &ExpandablePanel::get_attach_to);
   ClassDB::bind_method(D_METHOD("set_attach_to", "new_attach_to"), &ExpandablePanel::set_attach_to);

   ClassDB::bind_method(D_METHOD("get_current_page"), &ExpandablePanel::get_current_page);
   ClassDB::bind_method(D_METHOD("set_current_page", "page_index"), &ExpandablePanel::set_current_page);

   ClassDB::bind_method(D_METHOD("get_use_tooltip"), &ExpandablePanel::get_use_tooltip);
   ClassDB::bind_method(D_METHOD("set_use_tooltip", "value"), &ExpandablePanel::set_use_tooltip);

   ClassDB::bind_method(D_METHOD("get_keep_toggle_button_focus"), &ExpandablePanel::get_keep_toggle_button_focus);
   ClassDB::bind_method(D_METHOD("set_keep_toggle_button_focus", "value"), &ExpandablePanel::set_keep_toggle_button_focus);

   ClassDB::bind_method(D_METHOD("get_expand_icons_in_toggle_button"), &ExpandablePanel::get_expand_icons_in_toggle_button);
   ClassDB::bind_method(D_METHOD("set_expand_icons_in_toggle_button", "value"), &ExpandablePanel::set_expand_icons_in_toggle_button);

   ClassDB::bind_method(D_METHOD("get_animate_on_physics"), &ExpandablePanel::get_animate_on_physics);
   ClassDB::bind_method(D_METHOD("set_animate_on_physics", "value"), &ExpandablePanel::set_animate_on_physics);

   ClassDB::bind_method(D_METHOD("get_shrink_on_page_change"), &ExpandablePanel::get_shrink_on_change);
   ClassDB::bind_method(D_METHOD("set_shrink_on_page_change", "value"), &ExpandablePanel::set_shrink_on_change);

   ClassDB::bind_method(D_METHOD("get_expand_time"), &ExpandablePanel::get_expand_time);
   ClassDB::bind_method(D_METHOD("set_expand_time", "value"), &ExpandablePanel::set_expand_time);
   ClassDB::bind_method(D_METHOD("get_expand_curve"), &ExpandablePanel::get_expand_curve);
   ClassDB::bind_method(D_METHOD("set_expand_curve", "curve"), &ExpandablePanel::set_expand_curve);

   ClassDB::bind_method(D_METHOD("get_shrink_time"), &ExpandablePanel::get_shrink_time);
   ClassDB::bind_method(D_METHOD("set_shrink_time", "value"), &ExpandablePanel::set_shrink_time);
   ClassDB::bind_method(D_METHOD("get_shrink_curve"), &ExpandablePanel::get_shrink_curve);
   ClassDB::bind_method(D_METHOD("set_shrink_curve", "curve"), &ExpandablePanel::set_shrink_curve);

   ClassDB::bind_method(D_METHOD("get_page_count"), &ExpandablePanel::get_page_count);
   ClassDB::bind_method(D_METHOD("set_page_tooltip", "page_index", "new_title"), &ExpandablePanel::set_page_tooltip);
   ClassDB::bind_method(D_METHOD("set_page_icon_expanded", "page_index", "texture"), &ExpandablePanel::set_page_icon_expanded);
   ClassDB::bind_method(D_METHOD("set_page_icon_shrunk", "page_index", "texture"), &ExpandablePanel::set_page_icon_shrunk);
   ClassDB::bind_method(D_METHOD("set_page_disabled", "page_index", "disabled"), &ExpandablePanel::set_page_disabled);
   ClassDB::bind_method(D_METHOD("is_page_disabled", "page_index"), &ExpandablePanel::is_page_disabled);

   ClassDB::bind_method(D_METHOD("enable_page", "page_index"), &ExpandablePanel::enable_page);
   ClassDB::bind_method(D_METHOD("disable_page", "page_index"), &ExpandablePanel::disable_page);

   /// Constants
   BIND_ENUM_CONSTANT(Left);
   BIND_ENUM_CONSTANT(Top);
   BIND_ENUM_CONSTANT(Right);
   BIND_ENUM_CONSTANT(Bottom);

   /// Variables (properties)
   const godot::String attach_options = "Left:0,Top:1,Right:2,Bottom:3";
   ADD_PROPERTY(PropertyInfo(Variant::INT, "attach_to", PROPERTY_HINT_ENUM, attach_options, PROPERTY_USAGE_DEFAULT), "set_attach_to", "get_attach_to");
   ADD_PROPERTY(PropertyInfo(Variant::INT, "current_page", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_current_page", "get_current_page");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_tooltip", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_use_tooltip", "get_use_tooltip");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "keep_toggle_button_focus", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_keep_toggle_button_focus", "get_keep_toggle_button_focus");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "expand_icons_in_toggle_buttons", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_expand_icons_in_toggle_button", "get_expand_icons_in_toggle_button");


   ADD_GROUP("Animation", "");
   
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "animate_on_physics", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_animate_on_physics", "get_animate_on_physics");
   ADD_PROPERTY(PropertyInfo(Variant::BOOL, "shrink_on_page_change", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_shrink_on_page_change", "get_shrink_on_page_change");

   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "expand_time", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_expand_time", "get_expand_time");
   ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "expand_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve", PROPERTY_USAGE_DEFAULT), "set_expand_curve", "get_expand_curve");

   ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "shrink_time", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT), "set_shrink_time", "get_shrink_time");
   ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "shrink_curve", PROPERTY_HINT_RESOURCE_TYPE, "Curve", PROPERTY_USAGE_DEFAULT), "set_shrink_curve", "get_shrink_curve");


   /// Signals
   ADD_SIGNAL(godot::MethodInfo("page_button_clicked", godot::PropertyInfo(godot::Variant::INT, "page_index")));
   ADD_SIGNAL(godot::MethodInfo("expand_started", godot::PropertyInfo(godot::Variant::INT, "page_index")));
   ADD_SIGNAL(godot::MethodInfo("expand_finished", godot::PropertyInfo(godot::Variant::INT, "page_index")));
   ADD_SIGNAL(godot::MethodInfo("shrink_started", godot::PropertyInfo(godot::Variant::INT, "page_index")));
   ADD_SIGNAL(godot::MethodInfo("shrink_finished", godot::PropertyInfo(godot::Variant::INT, "page_index")));


   /// Use the CustomControlThemeDB system to work around the limitation that currently exists when attempting to add custom theme
   /// items in the default theme obtained from the ThemeDB. Also note that currently ThemeDB can't be obtained from _bind_methods()
   /// in a GDExtension (it's null - IF hot reloading it becomes valid after a recompilation with the Editor open)
   /// The first thing is to create the default theme items as those must be captured by the Lambda used within the Macros
   Ref<StyleBoxFlat> stl_background = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.85), 4, 4, 4, 4, 0, true, 2);
   stl_background->set_border_color(Color(0.2, 0.2, 0.2, 1.0));

   Ref<StyleBoxFlat> stl_bar = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 0.85), 2, 2, 2, 2, 0);

   Ref<StyleBoxFlat> stl_bt_normal = Internals::make_flat_stylebox(Color(0.15, 0.14, 0.14, 1.0), 1, 1, 1, 1, 2, true, 1);
   stl_bt_normal->set_border_color(Color(0.5, 0.5, 0.5, 1.0));

   Ref<StyleBoxFlat> stl_bt_hover = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 1.0), 1, 1, 1, 1, 2, true, 1);
   stl_bt_hover->set_border_color(Color(0.4, 0.4, 0.4, 1.0));

   Ref<StyleBoxFlat> stl_bt_pressed = Internals::make_flat_stylebox(Color(0.1, 0.1, 0.1, 1.0), 1, 1, 1, 1, 2, true, 1);
   stl_bt_pressed->set_border_color(Color(0.35, 0.35, 0.35, 1.0));

   Ref<StyleBoxFlat> stl_bt_disabled = Internals::make_flat_stylebox(Color(0.3, 0.3, 0.3, 1.0), 1, 1, 1, 1, 2, true, 1);
   stl_bt_disabled->set_border_color(Color(0.25, 0.25, 0.25, 1.0));

   Ref<StyleBoxFlat> stl_bt_focus = Internals::make_flat_stylebox(Color(0.2, 0.2, 0.2, 1.0), 1, 1, 1, 1, 2, false, 1);
   stl_bt_focus->set_border_color(Color(0.8, 0.1, 0.1, 1.0));

   const int bt_separation = 10;
   const int bt_min_width = 0;
   const int bt_min_height = 0;



   REGISTER_CUSTOM_STYLE(background, ExpandablePanel, stl_background, m_theme_cache);
   REGISTER_CUSTOM_STYLE(bar, ExpandablePanel, stl_bar, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_normal, ExpandablePanel, stl_bt_normal, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_hover, ExpandablePanel, stl_bt_hover, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_pressed, ExpandablePanel, stl_bt_pressed, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_disabled, ExpandablePanel, stl_bt_disabled, m_theme_cache);
   REGISTER_CUSTOM_STYLE(button_focus, ExpandablePanel, stl_bt_focus, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(toggle_button_separation, ExpandablePanel, bt_separation, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(button_min_width, ExpandablePanel, bt_min_width, m_theme_cache);
   REGISTER_CUSTOM_CONSTANT(button_min_height, ExpandablePanel, bt_min_height, m_theme_cache);
}


godot::Vector2 ExpandablePanel::_get_minimum_size() const
{
   return is_vertical() ? godot::Vector2(m_size_cache.bar.width, 0) : godot::Vector2(0, m_size_cache.bar.height);
}


godot::String ExpandablePanel::get_page_tooltip(int32_t page_index) const
{
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      return "";
   }

   const PanelPage& pageinfo = m_page_array[page_index];
   return pageinfo.control->get_meta("_page_name", pageinfo.control->get_name());
}

void ExpandablePanel::set_page_tooltip(int32_t page_index, const godot::String& new_title)
{
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      return;
   }

   const PanelPage& pageinfo = m_page_array[page_index];

   if (new_title.is_empty())
   {
      pageinfo.control->remove_meta("_page_name");
   }
   else
   {
      pageinfo.control->set_meta("_page_name", new_title);
   }

   assign_button_tooltip(pageinfo);
}


godot::Ref<godot::Texture2D> ExpandablePanel::get_page_icon_expanded(int32_t page_index) const
{
   godot::Ref<godot::Texture2D> ret;
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      return ret;
   }
   const PanelPage& pageinfo = m_page_array[page_index];
   if (pageinfo.control->has_meta("_expanded_icon"))
   {
      ret = pageinfo.control->get_meta("_expanded_icon");
   }

   return ret;
}

void ExpandablePanel::set_page_icon_expanded(int32_t page_index, const godot::Ref<godot::Texture2D>& texture)
{
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      // NOTE: Maybe output an error/warning message?
      return;
   }

   m_page_array[page_index].control->set_meta("_expanded_icon", texture);

   // If some size actually changes the sort children notifcation will be called, which will take care of refreshing the layout
   calculate_sizes();
}

godot::Ref<godot::Texture2D> ExpandablePanel::get_page_icon_shrunk(int32_t page_index) const
{
   godot::Ref<godot::Texture2D> ret;
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      return ret;
   }

   const PanelPage& pageinfo = m_page_array[page_index];
   if (pageinfo.control->has_meta("_shrunk_icon"))
   {
      ret = pageinfo.control->get_meta("_shrunk_icon");
   }

   return ret;
}

void ExpandablePanel::set_page_icon_shrunk(int32_t page_index, const godot::Ref<godot::Texture2D>& texture)
{
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      // NOTE: Maybe output an error/warning message?
      return;
   }

   m_page_array[page_index].control->set_meta("_shrunk_icon", texture);

   // If some size actually changes the sort children notifcation will be called, which will take care of refreshing the layout
   calculate_sizes();
}

bool ExpandablePanel::is_page_disabled(int32_t page_index) const
{
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      return true;
   }

   return m_page_array[page_index].control->get_meta("_disabled", false);
}

void ExpandablePanel::set_page_disabled(int32_t page_index, bool disabled)
{
   if (page_index < 0 || page_index >= m_page_array.size())
   {
      // NOTE: Maybe output an error/warning message?
      return;
   }

   // NOTE: If 'disabled' is false perhaps remove the metadata?
   m_page_array[page_index].control->set_meta("_disabled", disabled);
   m_page_array[page_index].button->set_disabled(disabled);

   if (!godot::Engine::get_singleton()->is_editor_hint())
   {
      if (m_current_page == page_index)
      {
         set_current_page(-1);
      }
   }
}




void ExpandablePanel::set_attach_to(ExpandablePanel::AttachTo value)
{
   m_attachto = value;

   update_minimum_size();

   const int32_t pcount = m_page_array.size();
   for (int32_t i = 0; i < pcount; i++)
   {
      m_page_array[i].button->set_button_icon(get_page_icon(m_page_array[i]));
      apply_button_style(m_page_array[i].button);
   }

   m_togglebox->set_vertical(is_vertical());
   refresh_layout();
}


void ExpandablePanel::set_current_page(int page_index)
{
   if (m_current_page == page_index)
      return;
   
   // If the Control is nto inside the tree then m_page is most likely not correct. So only check if the page_index
   // is bigger than amount of pages if this is already in the tree. When entering the tree the index will be double
   // checked anyway.
   if (page_index < -1 || (page_index >= m_page_array.size() && is_inside_tree()))
   {
      page_index = -1;
   }

   // There is no point in playing animation if in editor or if the Control is not inside the tree. So just assign
   // the incoming page number and bail
   if (godot::Engine::get_singleton()->is_editor_hint() || !is_inside_tree())
   {
      m_current_page = page_index;
      return;
   }


   // If there is a page already shown:
   // - Add a shrink animation on two conditions:
   //   1. Shrink animation is enabled (m_shrink_time > 0)
   //   2. Either the target page is -1 or the shrink on page change (m_shrink_on_change) is enabled.
   // - If shrink animation is not to be played, assign the requested page and disable the expand animation.
   //   The thing is, if the panel is not shrunk there is no point in playing the expand animation as the
   //   panel is already expanded.
   bool add_shrink = (m_shrink_time > 0 && (page_index == -1 || m_shrink_on_change));
   bool add_expand = (m_expand_time > 0);
   const bool vertical = is_vertical();

   if (m_current_page != -1)
   {
      // A page is already shown.
      if (add_shrink)
      {
         // Add a shrink animation
         AnimationState astate;
         astate.target_time = m_shrink_time;
         astate.starting = vertical ? m_expanded_width : m_expanded_height;
         astate.ending = vertical ? m_size_cache.bar.width : m_size_cache.bar.height;
         astate.target_page = m_current_page;
         astate.curve = m_shrink_curve;
         m_animation_queue.push_back(astate);
      }
      else
      {
         // But no shrink animation is required. Disable the expand animation too. Assume the requested page is none.
         // If it's not none then the next block of code will deal with that.
         call_deferred("emit_signal", "shrink_finished", m_current_page);

         // Hide the page that is currently shown
         const PanelPage& pageinfo = m_page_array[m_current_page];
         pageinfo.control->set_visible(false);
         pageinfo.button->set_button_icon(get_page_icon(pageinfo));

         m_current_page = -1;
         add_expand = false;
      }
   }

   if (page_index != -1)
   {
      if (add_expand)
      {
         AnimationState astate;
         astate.target_time = m_expand_time;
         astate.starting = vertical ? m_size_cache.bar.width : m_size_cache.bar.height;
         astate.ending = vertical ? m_expanded_width : m_expanded_height;
         astate.target_page = page_index;
         astate.curve = m_expand_curve;
         m_animation_queue.push_back(astate);
      }
      else
      {
         call_deferred("emit_signal", "expand_finished", page_index);
         m_current_page = page_index;

         // No animation is requested. But the Control node corresponding to the page must be set to visible
         const PanelPage& pageinfo = m_page_array[page_index];
         pageinfo.control->set_visible(true);
         pageinfo.button->set_button_icon(get_page_icon(pageinfo));
      }
   }

   if (m_animation_queue.size() > 0)
   {
      if (m_animate_on_physics)
      {
         set_physics_process_internal(true);
      }
      else
      {
         set_process_internal(true);
      }
   }
   // Changing visibility of the Control will trigger a SORT_CHILDREN notification, which in turn will setup the
   // position and size of the child node.
}


void ExpandablePanel::set_use_tooltip(bool value)
{
   m_use_tooltip = value;

   const int pcount = m_page_array.size();
   for (int i = 0; i < pcount; i++)
   {
      assign_button_tooltip(m_page_array[i]);
   }
}

void ExpandablePanel::set_expand_icons_in_toggle_button(bool value)
{
   m_expand_button_icon = value;

   const int pcount = m_page_array.size();
   for (int i = 0; i < pcount; i++)
   {
      m_page_array[i].button->set_expand_icon(m_expand_button_icon);
   }
}


ExpandablePanel::ExpandablePanel()
{
   set_clip_contents(true);
   
   // Initialize default button icons
   TEX_ARROW_LEFT = Internals::texture_from_base64(ICON_ARROW_LEFT);
   TEX_ARROW_RIGHT = Internals::texture_from_base64(ICON_ARROW_RIGHT);
   TEX_ARROW_UP = Internals::texture_from_base64(ICON_ARROW_UP);
   TEX_ARROW_DOWN = Internals::texture_from_base64(ICON_ARROW_DOWN);

   m_expanded_width = MIN_CONTENT_WIDTH;
   m_expanded_height = MIN_CONTENT_HEIGHT;
   
   m_attachto = Left;
   m_current_page = -1;
   m_use_tooltip = true;
   m_keep_button_focus = false;
   m_animate_on_physics = false;
   m_shrink_on_change = true;

   m_expand_time = 0.25;
   m_shrink_time = 0.15;

   m_expand_button_icon = false;


#ifdef DEBUG_ENABLED
   m_preview_page = 0;
#endif

   m_togglebox = memnew(godot::BoxContainer);
   m_togglebox->set_name("__toggle_button_box__");
   m_togglebox->set_alignment(godot::BoxContainer::AlignmentMode::ALIGNMENT_CENTER);
   add_child(m_togglebox, false, godot::Node::INTERNAL_MODE_FRONT);

   // To avoid error messages the event connections are performed through notification
}


#endif

