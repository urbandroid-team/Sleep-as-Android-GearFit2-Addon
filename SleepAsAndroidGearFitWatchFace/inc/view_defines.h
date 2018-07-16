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

#if !defined(VIEW_DEFINES_H_)
#define VIEW_DEFINES_H_

#include <watch_app.h>
#include <watch_app_efl.h>
#include <Elementary.h>
#include <dlog.h>

#define PART_HOUR_TEXT "hour_text"
#define PART_MINUTE_PREV "minute_prev"
#define PART_MINUTE_MAIN "minute_main"
#define PART_MINUTE_NEXT "minute_next"
#define PART_MINUTE_OUTSIDE "minute_outside"
#define PART_SECOND_HAND "second_hand"
#define SIGNAL_MINUTE_CHANGE_ANIM_START "minute,change,anim,start"
#define SIGNAL_SECOND_STATE_VISIBLE "second,state,visible"
#define SIGNAL_SECOND_STATE_HIDDEN "second,state,hidden"
#define LAYOUT_GROUP_MAIN "layout_group_main"
#define MSG_ID_MINUTE_PART_NEW_OUTSIDE 1
#define MSG_ID_SECOND_PART 2
#define MINUTE_PART_ANIMATION_TIME 1.0

#endif
