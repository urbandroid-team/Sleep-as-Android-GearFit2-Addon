/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <watch_app_efl.h>
#include "sleepasandroid-gearfit-ui.h"
#include "view.h"
#include "view_defines.h"

#define MAIN_EDJ "edje/main.edj"

static struct view_info {
	Evas_Object *win;
	Evas_Object *layout;
	Evas_Object *image;
	int w;
	int h;
	int minute;
} s_info = {
	.win = NULL,
	.layout = NULL,
	.image = NULL,
	.w = 0,
	.h = 0,
	.minute = 0,
};

static void _message_outside_object_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg);
static char *_create_resource_path(const char *file_name);
static Evas_Object *_create_layout(void);
static void _part_text_set(int val, const char *part);


/*
 * @brief Creates the application view with received size
 * @param width - Application width
 * @param height - Application height
 */
void view_create_with_size(int width, int height)
{
	s_info.w = width;
	s_info.h = height;
	view_create();
}

/*
 * @brief: Create Essential Object window, conformant and layout
 */
void view_create(void)
{
	/* Create window */
	s_info.win = view_create_win(PACKAGE);
	if (s_info.win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a window.");
		return;
	}

	/* Layout */
	s_info.layout = _create_layout();
	if (!s_info.layout) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create main layout.");
		return;
	}

	/* Show window after main view is set up */
	evas_object_show(s_info.win);
}

/*
 * @brief: Make a basic window named package
 * @param[pkg_name]: Name of the window
 */
Evas_Object *view_create_win(const char *pkg_name)
{
	Evas_Object *win = NULL;
	int ret;

	/*
	 * Window
	 * Create and initialize elm_win.
	 * elm_win is mandatory to manipulate window
	 */
	ret = watch_app_get_elm_win(&win);
	if (ret != APP_ERROR_NONE)
		return NULL;

	elm_win_title_set(win, pkg_name);
	elm_win_borderless_set(win, EINA_TRUE);
	elm_win_alpha_set(win, EINA_FALSE);
	elm_win_indicator_mode_set(win, ELM_WIN_INDICATOR_HIDE);
	elm_win_indicator_opacity_set(win, ELM_WIN_INDICATOR_BG_TRANSPARENT);
	elm_win_prop_focus_skip_set(win, EINA_TRUE);
	elm_win_role_set(win, "no-effect");

	evas_object_resize(win, s_info.w, s_info.h);
	return win;
}

/*
 * @brief: Make and set a layout to the part
 * @param[parent]: Object to which you want to set this layout
 * @param[file_path]: File path of EDJ will be used
 * @param[group_name]: Group name in EDJ that you want to set to layout
 * @param[part_name]: Part name to which you want to set this layout
 */
Evas_Object *view_create_layout_for_part(Evas_Object *parent, char *file_path, char *group_name, char *part_name)
{
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, file_path, group_name);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(layout);
	elm_object_part_content_set(parent, part_name, layout);
	edje_object_message_handler_set(elm_layout_edje_get(layout), _message_outside_object_cb, NULL);

	return layout;
}

/*
 * @brief: Destroy window and free important data to finish this application
 */
void view_destroy(void)
{
	if (s_info.win == NULL)
		return;

	evas_object_del(s_info.win);
}

/*
 * @brief Stores the hour value
 * @param hour - New hour value
 */
void view_set_hour(int hour)
{
	_part_text_set(hour, PART_HOUR_TEXT);
}

/*
 * @brief Stores the minute value
 * @param minute
 */
void view_set_minute(int minute)
{
	s_info.minute = minute;
}

/*
 * @brief Updates the second hand edje part state
 * @param second - Current second
 */
void view_set_second(int second)
{
	Edje_Message_Int msg = {0,};

	Evas_Object *edje_obj = elm_layout_edje_get(s_info.layout);
	if (!edje_obj) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] edje_obj == NULL", __FILE__, __LINE__);
		return;
	}

	msg.val = second;
	edje_object_message_send(edje_obj, EDJE_MESSAGE_INT, MSG_ID_SECOND_PART, &msg);
}

/*
 * @brief Begins the minute change animation
 */
void view_start_minute_update(void)
{
	elm_layout_signal_emit(s_info.layout, SIGNAL_MINUTE_CHANGE_ANIM_START, "");
}

/*
 * @brief Updates the hour and minute values displayed by the edje layout
 * @param hour - New hour value
 * @param minute - New minute value
 */
void view_update_display_time(int hour, int minute)
{
	_part_text_set(hour, PART_HOUR_TEXT);
	_part_text_set((minute > 0) ? minute - 1 : 59, PART_MINUTE_PREV);
	_part_text_set((minute + 0), PART_MINUTE_MAIN);
	_part_text_set((minute + 1) % 60, PART_MINUTE_NEXT);
	_part_text_set((minute + 2) % 60, PART_MINUTE_OUTSIDE);
}


/*
 * @brief Sets the edje layout's ambient mode
 * @param ambient - Ambient mode state
 */
void view_set_ambient_mode(Eina_Bool ambient)
{
	if (ambient)
		elm_layout_signal_emit(s_info.layout, SIGNAL_SECOND_STATE_HIDDEN, "");
	else
		elm_layout_signal_emit(s_info.layout, SIGNAL_SECOND_STATE_VISIBLE, "");
}

/*
 * @brief Sets the hour value displayed by the edje layout
 * @param hour - Hour value
 */
void view_reset_display_state(void)
{
	elm_layout_signal_emit(s_info.layout, "load", "");
}


/*
 * @brief Callback invoked when a message from the edje is received. Used to update the value of the last 'minute' part
 * @param data - User data
 * @param obj - Edje object which sent the message
 * @param type - The message type
 * @param id - The message id
 * @param msg - The message value
 */
static void _message_outside_object_cb(void *data, Evas_Object *obj, Edje_Message_Type type, int id, void *msg)
{
	Edje_Message_String *edje_msg = NULL;

	if (type != EDJE_MESSAGE_STRING || id != MSG_ID_MINUTE_PART_NEW_OUTSIDE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "[%s:%d] Unknown message", __FILE__, __LINE__);
		return;
	}

	edje_msg = (Edje_Message_String *)msg;
	_part_text_set((s_info.minute + 2) % 60, edje_msg->str);
}

/*
 * @brief: Creates path to the given resource file by concatenation of the basic resource path and the given file_name.
 * @param[file_name]: File name or path relative to the resource directory.
 * @return: The absolute path to the resource with given file_name is returned.
 */
static char *_create_resource_path(const char *file_name)
{
	static char res_path_buff[PATH_MAX] = {0,};
	char *res_path = NULL;

	res_path = app_get_resource_path();
	if (res_path == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get resource path.");
		return NULL;
	}

	snprintf(res_path_buff, PATH_MAX, "%s%s", res_path, file_name);
	free(res_path);

	return &res_path_buff[0];
}

/*
 * @brief: Creates the application's layout.
 * @return: The Evas_Object of the layout created.
 */
static Evas_Object *_create_layout(void)
{
	char *edj_path = NULL;

	edj_path = _create_resource_path(MAIN_EDJ);

	Evas_Object *layout = view_create_layout_for_part(s_info.win, edj_path, LAYOUT_GROUP_MAIN, "default");
	if (!layout)
		return NULL;

	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_min_set(layout, s_info.w, s_info.h);
	evas_object_resize(layout, s_info.w, s_info.h);
	evas_object_show(layout);

	return layout;
}


/*
 * @brief Sets a number to be displayed by the given edje part
 * @param val - Value to display
 * @param part - Part to update
 */
static void _part_text_set(int val, const char *part)
{
	Eina_Stringshare *str = eina_stringshare_printf("%.2d", val);
	elm_layout_text_set(s_info.layout, part, str);
	eina_stringshare_del(str);
}
