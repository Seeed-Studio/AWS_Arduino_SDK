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

#ifndef config_h
#define config_h

#define MAX_BUF_SIZE 256                // maximum number of bytes to publish/receive
#define MAX_SUB 15                // maximum number of subscribe
#define CMD_TIME_OUT 100                // maximum time to wait for feedback from AR9331, 100 = 10 sec
#define MAX_SHADOW_TOPIC_LEN 64               // maximum length for shadow topic, the metadata length for shadow topic is 32, make sure your thing name length plus that does not exceed this limit

#endif
