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

#include <system_settings.h>
#include <watch_app.h>
#include "digitalwatch.h"
#include "view.h"
#include "view_defines.h"

struct main_info {
	int hour;
	int minute;
	int second;
} s_info = {
	.hour = 0,
	.minute = 0,
	.second = 0,
};

static void _time_get(watch_time_h watch_time);
static void _curent_time_get(void);

/*
 * @brief The system language changed event callback function
 * @param[in] event_info The system event information
 * @param[in] user_data The user data passed from the add event handler function
 */
void lang_changed(app_event_info_h event_info, void* user_data)
{
	/*
	 * Takes necessary actions when language setting is changed
	 */
	char *locale = NULL;

	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	if (locale == NULL)
		return;

	elm_language_set(locale);
	free(locale);

	return;
}

/*
 * @brief The region format changed event callback function
 * @param[in] event_info The system event information
 * @param[in] user_data The user data passed from the add event handler function
 */
void region_changed(app_event_info_h event_info, void* user_data)
{
	/*
	 * Takes necessary actions when region setting is changed
	 */
}

/*
 * @brief The low battery event callback function
 * @param[in] event_info The system event information
 * @param[in] user_data The user data passed from the add event handler function
 */
void low_battery(app_event_info_h event_info, void* user_data)
{
	/*
	 * Takes necessary actions when system is running on low battery
	 */
	watch_app_exit();
}

/*
 * @brief The low memory event callback function
 * @param[in] event_info The system event information
 * @param[in] user_data The user data passed from the add event handler function
 */
void low_memory(app_event_info_h event_info, void* user_data)
{
	/*
	 * Takes necessary actions when system is running on low memory
	 */
	watch_app_exit();
}

/*
 * @brief The device orientation changed event callback function
 * @param[in] event_info The system event information
 * @param[in] user_data The user data passed from the add event handler function
 */
void device_orientation(app_event_info_h event_info, void* user_data)
{
	/*
	 * Takes necessary actions when device orientation is changed
	 */
}

static void send_service_command(const char* command) {
    app_control_h app_control;
	if (app_control_create(&app_control) == APP_CONTROL_ERROR_NONE) {
		int res1 = 0, res2 = 0, res3 = 0;
		if (((res1 = app_control_set_app_id(app_control, "com.urbandroid.sleep.service")) == APP_CONTROL_ERROR_NONE)
			&& ((res2 = app_control_add_extra_data(app_control, "app_action", command)) == APP_CONTROL_ERROR_NONE)
			&& ((res3 = app_control_send_launch_request(app_control, NULL, NULL)) == APP_CONTROL_ERROR_NONE)) {
			dlog_print(DLOG_INFO, LOG_TAG, "App command request sent: %s", command);
		} else {
			dlog_print(DLOG_ERROR, LOG_TAG, "App command request sending failed! Err: %d %d %d", res1, res2, res3);
		}
		if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE) {
			dlog_print(DLOG_INFO, LOG_TAG, "App control destroyed.");
		}
	} else {
		dlog_print(DLOG_ERROR, LOG_TAG, "App control creation failed!");
	}
}

/*
 * @brief Called when the application starts.
 * @param[in] width The width of the window of idle screen that will show the watch UI
 * @param[in] height The height of the window of idle screen that will show the watch UI
 * @param[in] user_data The user data passed from the callback registration function
 */
static bool app_create(int width, int height, void* user_data)
{
	/*
	 * Hook to take necessary actions before main event loop starts
	 * Initialize UI resources and application's data
	 */

	app_event_handler_h handlers[5] = {NULL, };

	/*
	 * Register callbacks for each system event
	 */
	if (watch_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, lang_changed, NULL) != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_add_event_handler () is failed");

	if (watch_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, region_changed, NULL) != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_add_event_handler () is failed");

	if (watch_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, low_battery, NULL) != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_add_event_handler () is failed");

	if (watch_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, low_memory, NULL) != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_add_event_handler () is failed");

	if (watch_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, device_orientation, NULL) != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_add_event_handler () is failed");

	view_create_with_size(width, height);

	_curent_time_get();


	send_service_command("start");

	return true;
}

/*
 * @brief: This callback function is called when another application
 * sends the launch request to the application
 */
static void app_control(app_control_h app_control, void *user_data)
{
	/*
	 * Handle the launch request.
	 */
}

/*
 * @brief: This callback function is called each time
 * the application is completely obscured by another application
 * and becomes invisible to the user
 */
static void app_pause(void *user_data)
{
	/*
	 * Take necessary actions when application becomes invisible.
	 */
}

/*
 * @brief: This callback function is called each time
 * the application becomes visible to the user
 */
static void app_resume(void *user_data)
{
	/*
	 * Take necessary actions when application becomes visible.
	 */

	view_reset_display_state();
	_curent_time_get();
}

/*
 * @brief: This callback function is called once after the main loop of the application exits
 */
static void app_terminate(void *user_data)
{
	view_destroy();
}

/*
 * @brief Called at each second. This callback is not called while the app is paused or the device is in ambient mode.
 * @param[in] watch_time The watch time handle. watch_time will not be available after returning this callback. It will be freed by the framework.
 * @param[in] user_data The user data to be passed to the callback functions
 */
static void app_time_tick(watch_time_h watch_time, void* user_data)
{
	_time_get(watch_time);

	if (s_info.second == 60 - (int)MINUTE_PART_ANIMATION_TIME) {
		view_set_minute(s_info.minute);
		view_start_minute_update();
	} else if (s_info.second == 0 && s_info.minute == 0) {
		view_set_hour(s_info.hour);
	}

	view_set_second(s_info.second);
}

/*
 * @brief Called at each minute when the device in the ambient mode.
 * @param[in] watch_time The watch time handle. watch_time will not be available after returning this callback. It will be freed by the framework.
 * @param[in] user_data The user data to be passed to the callback functions
 */
void app_ambient_tick(watch_time_h watch_time, void* user_data)
{
	int prev_min = s_info.minute;
	_time_get(watch_time);

	if (s_info.minute != prev_min) {
		view_set_minute(s_info.minute - 1);
		view_start_minute_update();

		if (s_info.minute == 0)
			view_set_hour(s_info.hour);
	}
}

/*
 * @brief: This function will be called when the ambient mode is changed
 */
static void app_ambient_changed(bool ambient_mode, void* user_data)
{
	/*
	 * Take necessary actions when application goes to/from ambient state
	 */
	view_set_ambient_mode(ambient_mode);
}


/*
 * @brief: Main function of the application
 */
int main(int argc, char *argv[])
{
	int ret = 0;

	watch_app_lifecycle_callback_s event_callback = {0, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;
	event_callback.time_tick = app_time_tick;
	event_callback.ambient_tick = app_ambient_tick;
	event_callback.ambient_changed = app_ambient_changed;

	ret = watch_app_main(argc, argv, &event_callback, NULL);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_main() is failed. err = %d", ret);

	return ret;
}


/*
 * @brief Gets the time from a watch_time_h handler and stores it in the s_info structure
 * @param watch_time - watch_time_h structure to read the time from
 */
static void _time_get(watch_time_h watch_time)
{
	watch_time_get_second(watch_time, &s_info.second);
	watch_time_get_minute(watch_time, &s_info. minute);
	watch_time_get_hour(watch_time, &s_info.hour);
}

/*
 * @brief Reads the current time.
 */
static void _curent_time_get(void)
{
	watch_time_h watch_time;

	watch_time_get_current_time(&watch_time);
	_time_get(watch_time);
	view_update_display_time(s_info.hour, s_info.minute);
	watch_time_delete(watch_time);
}
