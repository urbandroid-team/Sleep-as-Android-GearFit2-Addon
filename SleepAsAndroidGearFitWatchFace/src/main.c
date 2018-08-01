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

#include <tizen.h>
#include <system_settings.h>
#include <watch_app.h>
#include "sleepasandroidgearfitwatchface.h"
#include "view_defines.h"
#include <app_manager.h>

#define MAIN_EDJ "icon/main.edj"


typedef struct appdata {
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *nf;
	Evas_Object *table;
	Evas_Object *label_time;
	Evas_Object *bg;
	Evas_Object *gest;
	Evas_Object *label_tracking;

} appdata_s;

bool is_tracking = false;
int g_width;
int g_height;

#define TEXT_BUF_SIZE 256
#define IMAGE_PATH "images/unnamed.png"
#define ServiceID "com.urbandroid.sleep.gearfit.service"

static void
update_watch(appdata_s *ad, watch_time_h watch_time, int ambient)
{
	char watch_text[TEXT_BUF_SIZE];
	int hour24, minute, second;

	if (watch_time == NULL)
		return;

	watch_time_get_hour24(watch_time, &hour24);
	watch_time_get_minute(watch_time, &minute);
	watch_time_get_second(watch_time, &second);
	if (!ambient) {
		snprintf(watch_text, TEXT_BUF_SIZE, "<align=center valign=middle font_size=50><br/>%02d:%02d:%02d</align>",
				hour24, minute, second);
	} else {
		snprintf(watch_text, TEXT_BUF_SIZE, "<align=center valign=middle font_size=50><br/>%02d:%02d</align>",
				hour24, minute);
	}

	elm_object_text_set(ad->label_time, watch_text);
}

static void return_to_base_UI(appdata_s *ad){
	dlog_print(DLOG_INFO, LOG_TAG, "Watchface: Return to Base GUI");
	elm_naviframe_item_pop(ad->nf);
}

static void tracking_updater(appdata_s *ad, bool tracking){
	is_tracking = tracking;

	if (is_tracking){
		dlog_print(DLOG_INFO, LOG_TAG, "UI: Tracking on");
		elm_object_text_set(ad->label_tracking,"<align=center valign=middle>Tracking</align>");
	} else{
		dlog_print(DLOG_INFO, LOG_TAG, "UI: Tracking off");
		elm_object_text_set(ad->label_tracking,"<align=center valign=middle>Not Tracking</align>");
	}

}

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

static void send_service_command(const char* command) {
	app_control_h app_control;
	if (app_control_create(&app_control) == APP_CONTROL_ERROR_NONE) {
		int res1 = 0, res2 = 0, res3 = 0;
		if (((res1 = app_control_set_app_id(app_control, "com.urbandroid.sleep.gearfit.service")) == APP_CONTROL_ERROR_NONE)
				&& ((res2 = app_control_add_extra_data(app_control, "app_action", command)) == APP_CONTROL_ERROR_NONE)
				&& ((res3 = app_control_send_launch_request(app_control, NULL, NULL)) == APP_CONTROL_ERROR_NONE)) {
			dlog_print(DLOG_INFO, LOG_TAG, "WatchFace: App command request sent: %s", command);
		} else {
			dlog_print(DLOG_ERROR, LOG_TAG, "WatchFace: App command request sending failed! Err: %d %d %d", res1, res2, res3);
		}
		if (app_control_destroy(app_control) == APP_CONTROL_ERROR_NONE) {
			dlog_print(DLOG_INFO, LOG_TAG, "WatchFace: App control destroyed.");
		}
	} else {
		dlog_print(DLOG_ERROR, LOG_TAG, "WatchFace: App control creation failed!");
	}
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	return EINA_TRUE;
}

static Evas_Event_Flags
tracking_button_clicked(void *data, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG,"Watchface: Tracking Button Double Clicked");

	//tracking_updater(data, !is_tracking );
	send_service_command("start_tracking");



	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
touch_event(void *ad, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG,"Touch Event");

	Elm_Gesture_Taps_Info *p = (Elm_Gesture_Taps_Info *) event_info;
	dlog_print(DLOG_INFO, LOG_TAG,"N tap started <%p> x,y=<%d,%d> count=<%d> timestamp=<%d> \n",
	           event_info, p->x, p->y, p->n, p->timestamp);

	int snz_btn_x,snz_btn_y,snz_btn_w,snz_btn_h;
	int dis_btn_x,dis_btn_y;

	dlog_print(DLOG_INFO, LOG_TAG,"snzbutton position: x: %d, y:%d",snz_btn_x,snz_btn_y);
	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
snz_button_clicked(void *data, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG,"Watchface: Snooze Clicked");
	send_service_command("snooze");
	return_to_base_UI(data);
	return EVAS_EVENT_FLAG_ON_HOLD;
}

static Evas_Event_Flags
dis_button_clicked(void *data, void *event_info)
{
	dlog_print(DLOG_INFO, LOG_TAG,"Watchface: Dismiss Clicked");
	send_service_command("dismiss");
	return EVAS_EVENT_FLAG_ON_HOLD;
}

static void
create_base_gui(appdata_s *ad, int width, int height)
{
	dlog_print(DLOG_INFO, LOG_TAG, "Start Base GUI");

	int ret;


	/* Window */
	ret = watch_app_get_elm_win(&ad->win);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get window. err = %d", ret);
		return;
	}

	evas_object_resize(ad->win, width, height);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Naviframe */
	ad->nf = elm_naviframe_add(ad->conform);
	elm_object_content_set(ad->conform, ad->nf);
	evas_object_show(ad->nf);

	dlog_print(DLOG_INFO, LOG_TAG, "Finished Base GUI");
}

static void
watchface_gui(appdata_s *ad)
{
	dlog_print(DLOG_INFO, LOG_TAG, "Started Watch Face GUI");

	Evas_Object *watch_bg = elm_bg_add(ad->nf);
	elm_bg_color_set(watch_bg,20,20,20);
	evas_object_show(watch_bg);

	ad->table = elm_table_add(ad->nf);
	evas_object_size_hint_weight_set(ad->table,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->table,EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(ad->table);

	elm_object_content_set(watch_bg,ad->table);


	/* Label Time */
	ad->label_time = elm_label_add(ad->table);
	evas_object_color_set(ad->label_time,230,230,230,255);
	elm_object_text_set(ad->label_time,"<align=center valign=top> TIME </align> ");
	evas_object_size_hint_min_set(ad->label_time, g_width, g_height*.75);
	evas_object_size_hint_align_set(ad->label_time,EVAS_HINT_FILL,EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->label_time,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_table_pack(ad->table,ad->label_time,0,0,1,1);
	evas_object_show(ad->label_time);

	/* Tracking Background*/
	Evas_Object *bg_track = elm_bg_add(ad->table);
	elm_bg_color_set(bg_track,0,140,0);
	evas_object_size_hint_min_set(bg_track, g_width, g_height*.25);
	evas_object_size_hint_weight_set(bg_track,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg_track,EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(bg_track);
	elm_table_pack(ad->table,bg_track,0,1,1,1);

	/* Label Tracking */
	ad->label_tracking = elm_label_add(ad->table);
	elm_object_content_set(bg_track,ad->label_tracking);
	evas_object_size_hint_align_set(ad->label_tracking,EVAS_HINT_FILL,EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->label_tracking,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_object_text_set(ad->label_tracking,"<align=center valign=middle> Tracker </align> ");
	elm_table_pack(ad->table,ad->label_tracking,0,1,1,1);
	evas_object_show(ad->label_tracking);
	tracking_updater(ad, is_tracking);

	/* Gesture */
	Evas_Object *gest_track = elm_gesture_layer_add(ad->win);
	elm_gesture_layer_attach(gest_track, bg_track);
	elm_gesture_layer_cb_set(gest_track, ELM_GESTURE_N_DOUBLE_TAPS, ELM_GESTURE_STATE_END, tracking_button_clicked, NULL);



	watch_time_h watch_time = NULL;
	int ret = watch_time_get_current_time(&watch_time);
	if (ret != APP_ERROR_NONE)
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get current time. err = %d", ret);

	update_watch(ad, watch_time, 0);
	watch_time_delete(watch_time);

	elm_naviframe_item_push(ad->nf, NULL,NULL, NULL,watch_bg, NULL);

	dlog_print(DLOG_INFO, LOG_TAG, "Finished Watch Face GUI");


}

static void
alarm_gui(appdata_s *ad)
{
	dlog_print(DLOG_INFO, LOG_TAG, "Alarm GUI started");

	Evas_Object *alarm_bg = elm_bg_add(ad->nf);
	elm_bg_color_set(alarm_bg,240,240,240);
	evas_object_show(alarm_bg);


	ad->table = elm_table_add(ad->nf);
	evas_object_size_hint_weight_set(ad->table,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(ad->table,EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(ad->table);

	elm_object_content_set(alarm_bg,ad->table);


	/* Dismiss Background */
	Evas_Object *bg_dis = elm_bg_add(ad->table);
	elm_bg_color_set(bg_dis,250,0,0);
	evas_object_size_hint_weight_set(bg_dis,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg_dis,EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(bg_dis);
	elm_table_pack(ad->table,bg_dis,0,0,1,3);

	/* Label Dismiss */
	Evas_Object *label_dis = elm_label_add(ad->table);
	elm_object_content_set(bg_dis,label_dis);
	evas_object_size_hint_align_set(label_dis,EVAS_HINT_FILL,EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(label_dis,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_object_text_set(label_dis,"<align=center valign=middle> DISMISS </align> ");
	elm_table_pack(ad->table,bg_dis,0,1,1,1);
	evas_object_show(label_dis);

	/* Label Time */
	ad->label_time = elm_label_add(ad->table);
	evas_object_color_set(ad->label_time,30,30,30,255);
	elm_object_text_set(ad->label_time,"<align=center color=#303030  valign=middle> TIME </align> ");
	evas_object_size_hint_align_set(ad->label_time,EVAS_HINT_FILL,EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->label_time,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_table_pack(ad->table,ad->label_time,0,4,1,1);
	evas_object_show(ad->label_time);

	/* Background Snooze */
	Evas_Object *bg_snz = elm_bg_add(ad->table);
	elm_bg_color_set(bg_snz,0,140,0);
	evas_object_size_hint_weight_set(bg_snz,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(bg_snz,EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(bg_snz);
	elm_table_pack(ad->table,bg_snz,0,6,1,3);

	/* Label Snooze */
	Evas_Object *label_snz = elm_label_add(ad->table);
	elm_object_content_set(bg_snz,label_snz);
	evas_object_size_hint_align_set(label_snz,EVAS_HINT_FILL,EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(label_snz,EVAS_HINT_EXPAND,EVAS_HINT_EXPAND);
	elm_object_text_set(label_snz,"<align=center valign=middle> Snooze </align> ");
	elm_table_pack(ad->table,label_snz,0,7,1,1);
	evas_object_show(label_snz);

	/* Gestures */
	Evas_Object *gest_snz = elm_gesture_layer_add(bg_snz);
	elm_gesture_layer_attach(gest_snz,bg_snz);
	elm_gesture_layer_cb_set(gest_snz, ELM_GESTURE_N_TAPS, ELM_GESTURE_STATE_END, snz_button_clicked,NULL);

	Evas_Object *gest_dis = elm_gesture_layer_add(bg_dis);
	elm_gesture_layer_attach(gest_dis,bg_dis);
	elm_gesture_layer_cb_set(gest_dis,ELM_GESTURE_N_TAPS,ELM_GESTURE_STATE_END,dis_button_clicked,NULL);

	elm_naviframe_item_pop_cb_set(ad->nf, naviframe_pop_cb, ad);

	elm_naviframe_item_push(ad->nf, NULL,NULL, NULL, alarm_bg, NULL);

	dlog_print(DLOG_INFO, LOG_TAG, "Alarm GUI Finished");
}

/*
 * @brief The system language changed event callback function
 * @param[in] event_info The system event information
 * @param[in] data The user data passed from the add event handler function
 */
void lang_changed(app_event_info_h event_info, void* data)
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
 * @param[in] data The user data passed from the add event handler function
 */
void region_changed(app_event_info_h event_info, void* data)
{
	/*
	 * Takes necessary actions when region setting is changed
	 */
}

/*
 * @brief The low battery event callback function
 * @param[in] event_info The system event information
 * @param[in] data The user data passed from the add event handler function
 */
void low_battery(app_event_info_h event_info, void* data)
{
	/*
	 * Takes necessary actions when system is running on low battery
	 */
//	watch_app_exit();
}

/*
 * @brief The low memory event callback function
 * @param[in] event_info The system event information
 * @param[in] data The user data passed from the add event handler function
 */
void low_memory(app_event_info_h event_info, void* data)
{
	/*
	 * Takes necessary actions when system is running on low memory
	 */
//	watch_app_exit();
}

/*
 * @brief The device orientation changed event callback function
 * @param[in] event_info The system event information
 * @param[in] data The user data passed from the add event handler function
 */
void device_orientation(app_event_info_h event_info, void* data)
{
	/*
	 * Takes necessary actions when device orientation is changed
	 */
}



/*
 * @brief Called when the application starts.
 * @param[in] width The width of the window of idle screen that will show the watch UI
 * @param[in] height The height of the window of idle screen that will show the watch UI
 * @param[in] data The user data passed from the callback registration function
 */
static bool app_create(int width, int height, void *data)
{
	/*
	 * Hook to take necessary actions before main event loop starts
	 * Initialize UI resources and application's data
	 */

	g_width = width;
	g_height = height;

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

	appdata_s *ad = data;
	create_base_gui(ad, width, height);
	watchface_gui(ad);

	alarm_gui(ad);

	evas_object_show(ad->win);

//	send_service_command("start");

	return true;
}

/*
 * @brief: This callback function is called when another application
 * sends the launch request to the application
 */
static void app_control(app_control_h app_control, void *data){
	dlog_print(DLOG_INFO, LOG_TAG, "WatchFace: app control received");
	char *caller_id = NULL;
	if (app_control_get_caller(app_control, &caller_id) == APP_CONTROL_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "WatchFace: Caller: %s", caller_id);
		free(caller_id);
	}

	char *action_value = NULL;
	if (app_control_get_extra_data(app_control, "app_action", &action_value) == APP_CONTROL_ERROR_NONE) {
		dlog_print(DLOG_INFO, LOG_TAG, "WatchFace: App control action: %s", action_value);

		// Add start?
		if (action_value != NULL && strcmp(action_value, "alarm_started") == 0) {
			alarm_gui(data);
		} else if (action_value != NULL && strcmp(action_value, "alarm_finished") == 0) {
//			tracking_updater(data, true);
		} else if (action_value != NULL && strcmp(action_value, "tracking_on") == 0) {
			tracking_updater(data, true);
		} else if (action_value != NULL && strcmp(action_value, "tracking_off") == 0) {
			tracking_updater(data, false);
		} else{
			dlog_print(DLOG_INFO, LOG_TAG, "WatchFace: Unsupported action! Doing nothing...");
			free(action_value);
		}
	} else {
		dlog_print(DLOG_ERROR, LOG_TAG, "WatchFace: Failed to get app control attribute");
	}

}

/*
 * @brief: This callback function is called each time
 * the application is completely obscured by another application
 * and becomes invisible to the user
 */
static void app_pause(void *data)
{
	/*
	 * Take necessary actions when application becomes invisible.
	 */
}

/*
 * @brief: This callback function is called each time
 * the application becomes visible to the user
 */
static void app_resume(void *data)
{
	/*
	 * Take necessary actions when application becomes visible.
	 */
}

/*
 * @brief: This callback function is called once after the main loop of the application exits
 */
static void app_terminate(void *data)
{
	send_service_command("terminate");
}

/*
 * @brief Called at each second. This callback is not called while the app is paused or the device is in ambient mode.
 * @param[in] watch_time The watch time handle. watch_time will not be available after returning this callback. It will be freed by the framework.
 * @param[in] data The user data to be passed to the callback functions
 */
static void app_time_tick(watch_time_h watch_time, void *data)
{
	/* Called at each second while your app is visible. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, watch_time, 0);
}

/*
 * @brief Called at each minute when the device in the ambient mode.
 * @param[in] watch_time The watch time handle. watch_time will not be available after returning this callback. It will be freed by the framework.
 * @param[in] data The user data to be passed to the callback functions
 */
static void app_ambient_tick(watch_time_h watch_time, void *data)
{
	/* Called at each minute while the device is in ambient mode. Update watch UI. */
	appdata_s *ad = data;
	update_watch(ad, watch_time, 1);
}

/*
 * @brief: This function will be called when the ambient mode is changed
 */
static void app_ambient_changed(bool ambient_mode, void* data)
{
	/*
	 * Take necessary actions when application goes to/from ambient state
	 */
}

static void
watch_app_lang_changed(app_event_info_h event_info, void *data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/
	char *locale = NULL;
	app_event_get_language(event_info, &locale);
	elm_language_set(locale);
	free(locale);
	return;
}

static void
watch_app_region_changed(app_event_info_h event_info, void *data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

/*
 * @brief: Main function of the application
 */
int main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	watch_app_lifecycle_callback_s event_callback = {0, };
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;
	event_callback.time_tick = app_time_tick;
	event_callback.ambient_tick = app_ambient_tick;
	event_callback.ambient_changed = app_ambient_changed;

	watch_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED],
			APP_EVENT_LANGUAGE_CHANGED, watch_app_lang_changed, &ad);
	watch_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED],
			APP_EVENT_REGION_FORMAT_CHANGED, watch_app_region_changed, &ad);

	ret = watch_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		dlog_print(DLOG_ERROR, LOG_TAG, "watch_app_main() is failed. err = %d", ret);
	}

	return ret;
}
