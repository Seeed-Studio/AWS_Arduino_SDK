/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
 
#ifndef aws_iot_error_h
#define aws_iot_error_h

typedef enum {
	NONE_ERROR = 0,
	GENERIC_ERROR = -1,
	NULL_VALUE_ERROR = -2,
	OVERFLOW_ERROR = -3,
	SET_UP_ERROR = -4,
	CONNECT_ERROR = -5,
	PUBLISH_ERROR = -6,
	SUBSCRIBE_ERROR = -7,
	UNSUBSCRIBE_ERROR = -8,
	YIELD_ERROR = -9,
	DISCONNECT_ERROR = -10,
	SHADOW_INIT_ERROR = -11,
	SHADOW_UPDATE_ERROR = -12,
	SHADOW_GET_ERROR = -13,
	SHADOW_DELETE_ERROR = -14,
	CONFIG_ERROR = -15,
	OUT_OF_SKETCH_SUBSCRIBE_MEMORY = -16
} IoT_Error_t;

#endif