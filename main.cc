/**
 * main.cc - Driver/Wrapper for our lumail script.
 *
 * This file is part of lumail: http://lumail.org/
 *
 * Copyright (c) 2013 by Steve Kemp.  All rights reserved.
 *
 **
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 dated June, 1991, or (at your
 * option) any later version.
 *
 * On Debian GNU/Linux systems, the complete text of version 2 of the GNU
 * General Public License can be found in `/usr/share/common-licenses/GPL-2'
 */

#include <algorithm>
#include <cstdlib>
#include <curses.h>
#include <iostream>
#include <fstream>
#include <getopt.h>

#include "lua.h"
#include "message.h"
#include "maildir.h"
#include "screen.h"
#include "version.h"


/**
 * Entry point to our code.
 */
int main(int argc, char *argv[])
{
    /**
     * Parse command-line arguments
     */
    int c;

    bool version = false;       /* show version */
    std::string rcfile = "";    /* load rc file */
    std::string folder = "";    /* open folder */

    while (1)
    {
	static struct option long_options[] =
            {
                {"version", no_argument, 0, 'v'},
                {"rcfile", required_argument, 0, 'r'},
                {"folder", required_argument, 0, 'f'},
                {0, 0, 0, 0}
            };

	/* getopt_long stores the option index here. */
	int option_index = 0;

	c = getopt_long(argc, argv, "vr:f:", long_options, &option_index);

	/* Detect the end of the options. */
	if (c == -1)
	    break;

	switch (c)
        {
	case 'r':
	    rcfile = optarg;
	    break;
	case 'f':
	    folder = optarg;
	    break;
	case 'v':
	    version = true;
	    break;
	case '?':
	    /* getopt_long already printed an error message. */
	    break;

	default:
	    std::cerr << "Unknown argument" << std::endl;
	    return (2);
	}
    }

    if (version)
    {
	std::cout << "lumail v" << LUMAIL_VERSION << std::endl;
	return 0;
    }

    /**
     * Initialize the screen.
     */
    CScreen screen = CScreen();
    screen.Init();

    /**
     * Number of init-files we've loaded.
     */
    int init = 0;

    /**
     * Create the lua intepreter.
     */
    CLua *lua = CLua::Instance();
    if ( lua->load_file("/etc/lumail.lua") )
        init += 1;

    /**
     * Load the init-file from the users home-directory, if we can.
     */
    std::string home = getenv( "HOME" );
    if ( CMaildir::is_directory( home + "/.lumail" ) )
        if ( lua->load_file( home + "/.lumail/config.lua") )
            init += 1;

    /**
     * If we have any init file specified then load it up too.
     */
    if (!rcfile.empty())
    {
        if ( lua->load_file(rcfile.c_str()) )
            init += 1;
    }


    /**
     *  Ensure we've loaded something.
     */
    if ( init == 0 )
    {
        endwin();
        std::cerr << "No init file was loaded!" << std::endl;
        std::cerr << "We try to load both /etc/lumail.lua and ~/.lumail/config.lua if present." << std::endl;
        std::cerr << "You could try: ./lumail --rcfile ./lumail.lua" << std::endl;
        return -1;
    }


    /**
     * We're starting, so call the on_start() function.
     */
    lua->call_function("on_start");

    /**
     * If we have a starting folder, select it.
     */
    if ( !folder.empty() )
    {
        if ( CMaildir::is_directory( folder ) )
        {
            /**
             * This should be simplifiable.
             */
            lua->execute( "global_mode( \"index\" );" );
            lua->execute( "maildir_limit( \"all\" );" );
            lua->execute( "clear_selected_folders();");
            lua->execute( "scroll_maildir_to( \"" + folder + "\");" );
            lua->execute( "add_selected_folder()");
            lua->execute( "global_mode( \"index\" );" );
        }
        else
        {
            lua->execute("msg(\"Startup folder doesn't exist!\");" );
        }
    }


    /**
     * Now enter our event-loop
     */
    while (true)
    {
	char key = getch();
	if (key == ERR)
        {
	    /*
	     * Timeout - so we go round the loop again.
	     */
	    lua->call_function("on_idle");

	}
        else
        {
            /**
             * The human-readable version of the key which has
             * been pressed.
             *
             * i.e. Ctrl-r -> ^R.
             */
            const char *name = keyname( key );

	    if (!lua->on_keypress(name))
            {
		std::string foo = "msg(\"Unbound key: ";
                foo += std::string(name) + "\");";
		lua->execute(foo);
	    }
	}

	screen.refresh_display();
    }

    /**
     * We've been terminated.
     *
     * We call the lua-version of exit, because this will run our
     * on-exit hook/function, after ending the curses window(ing)
     * routine(s).
     *
     */
    lua->call_function("exit");

    /**
     * This code is never reached.
     */
    exit(0);
    return 0;
}
