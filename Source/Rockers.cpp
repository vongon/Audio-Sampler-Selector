#include <fcntl.h>
#include <glib.h>
#include "stdlib.h"
#include <unistd.h>
#include "Rockers.h"
#include <iostream>

using namespace std;

Rockers::Rockers()
{
	int sys=0;
	sys=system("sudo sh -c 'echo 30 > /sys/class/gpio/export'");
	sys=system("sudo sh -c 'echo in > /sys/class/gpio/gpio30/direction'");
	sys=system("sudo sh -c 'echo both > /sys/class/gpio/gpio30/edge'");
    GMainLoop* loop = g_main_loop_new( 0, 0 );
    int fd = open( "/sys/class/gpio/gpio30/value", O_RDONLY | O_NONBLOCK );
    GIOChannel* channel = g_io_channel_unix_new( fd );
    	if(!channel){g_error("Error creating new GIOChannel!\n");}

    GIOCondition cond = GIOCondition( G_IO_PRI );
    guint id = g_io_add_watch( channel, cond, onButtonEvent, 0 );
    	if(!id) g_error("Error creating watch!\n");

    g_main_loop_run( loop );
    cout << "~ Rockers setup has run ~"<< endl;
}

gboolean
Rockers::onButtonEvent( GIOChannel *channel,
               GIOCondition condition,
               gpointer user_data )
{
	cerr << "onButtonEvent" << endl;
	GError *error = 0;
	gchar buf[1] = {'B'};
	gsize buf_sz = 8;
	gsize bytes_read = 0;
	GIOStatus ret = g_io_channel_seek_position( channel, 0, G_SEEK_SET, 0 );

	GIOStatus rc = g_io_channel_read_chars( channel,
	                                         buf, buf_sz-1,
	                                         &bytes_read,
	                                         &error );

	cerr << "rc:" << rc << "  data:" << buf[0] << endl;

	// thank you, call again!
    return 1;

}
