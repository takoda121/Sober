#pragma once

extern int sdl_scancode_to_linux_keycode_table[];
extern int sdl_scancode_to_linux_keycode_table_length;

int sdl_scancode_to_android_keycode(int sdl_scancode);
