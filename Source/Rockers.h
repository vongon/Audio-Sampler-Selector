/*
 * Rockers.h
 *
 *  Created on: Nov 9, 2014
 *      Author: rmcgill
 */

#ifndef SOURCE_ROCKERS_H_
#define SOURCE_ROCKERS_H_
#include <glib.h>

class Rockers {
public:
Rockers();
static gboolean onButtonEvent( GIOChannel *channel, GIOCondition condition, gpointer user_data );


};

#endif /* SOURCE_ROCKERS_H_ */
