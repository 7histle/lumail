/**
 * global.cc - Singleton interface to store global data
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

#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include "global.h"

/**
 * Instance-handle.
 */
CGlobal *CGlobal::pinstance = NULL;

/**
 * Get access to our singleton-object.
 */
CGlobal *CGlobal::Instance()
{
    if (!pinstance)
	pinstance = new CGlobal;

    return pinstance;
}

/**
 * Constructor - This is private as this class is a singleton.
 */
CGlobal::CGlobal()
{
  /**
   * Defaults.
   */
    m_mode          = new std::string("maildir");
    m_maildir_limit = new std::string("all");
    m_index_limit   = new std::string("all");
    m_index_format  = new std::string( "[$FLAGS] $FROM - $SUBJECT" );

    std::string user = "UNKNOWN";
    if ( getenv( "USER" ) )
      user = getenv( "USER" );

    m_from_address   = new std::string( user + "@localhost" );
    m_sendmail_path  = new std::string( "/usr/lib/sendmail -t" );
    m_maildir_prefix = NULL;
    m_cur_folder     = 0;
    m_cur_message    = 0;
    m_messages       = NULL;
}

/**
 * Set the new mode for the client.
 */
void CGlobal::set_mode(std::string * mode)
{
    if (m_mode != NULL)
	delete(m_mode);

    m_mode = new std::string(mode->c_str());
}

/**
 * Get the current mode the client is in: index, maildir or message.
 */
std::string * CGlobal::get_mode()
{
    return (m_mode);
}

/**
 * Set the maildir limit.
 */
void CGlobal::set_maildir_limit(std::string * limit)
{
    if (m_maildir_limit)
	delete(m_maildir_limit);

    m_maildir_limit = limit;
}

/**
 * Get the maildir limit
 */
std::string * CGlobal::get_maildir_limit()
{
    return (m_maildir_limit);
}


/**
 * Set the index-limit.
 */
void CGlobal::set_index_limit( std::string *limit)
{
    if (m_index_limit)
	delete(m_index_limit);

    m_index_limit = limit;
}

/**
 * Get the index-limit.
 */
std::string *CGlobal::get_index_limit()
{
  return( m_index_limit );
}

/**
 * Set the prefix for our maildir folders.
 */
void CGlobal::set_maildir_prefix(std::string * prefix)
{
    if (m_maildir_prefix != NULL)
	delete(m_maildir_prefix);

    m_maildir_prefix = new std::string(prefix->c_str());
}

/**
 * Get the prefix for the maildir folders.
 */
std::string * CGlobal::get_maildir_prefix()
{
    return (m_maildir_prefix);
}


/**
 * Set the index format.
 */
void CGlobal::set_index_format(std::string * fmt)
{
    if (m_index_format != NULL)
	delete(m_index_format);

    m_index_format = new std::string(fmt->c_str());
}

/**
 * Get the index format.
 */
std::string * CGlobal::get_index_format()
{
    return (m_index_format);
}


/**
 * Get all selected folders.
 */
std::vector<std::string> CGlobal::get_selected_folders()
{
    return (m_selected_folders);
}

/**
 * Get all folders.
 */
std::vector<CMaildir> CGlobal::get_all_folders()
{
    std::vector<CMaildir> maildirs;

    std::vector<std::string> folders =
	CMaildir::getFolders(*m_maildir_prefix);
    std::vector < std::string >::iterator it;
    for (it = folders.begin(); it != folders.end(); ++it) {
	maildirs.push_back(CMaildir(*it));
    }

    return (maildirs);
}

/**
 * Get folders matching the current mode.
 */
std::vector < CMaildir > CGlobal::get_folders()
{
  CGlobal *global = CGlobal::Instance();
  std::vector<CMaildir> folders = global->get_all_folders();
  std::vector<CMaildir> display;
  std::string * filter = global->get_maildir_limit();

  /**
   * Filter the folders to those we can display
   */
  std::vector<CMaildir>::iterator it;
  for (it = folders.begin(); it != folders.end(); ++it) {
    CMaildir x = *it;

    if ( x.matches_filter( filter ) )
      display.push_back(x);
  }

  return (display);
}

/**
 * My sort function: sort CMessages by most recent to oldest.
 */
bool my_sort(CMessage *a, CMessage *b)
{
  /**
   * Stat both files.
   */
    struct stat us;
    struct stat them;

    std::string us_path = a->path();
    std::string them_path = b->path();

    if (stat(us_path.c_str(), &us) < 0)
	return 0;

    if (stat(them_path.c_str(), &them) < 0)
	return 0;

    return (us.st_mtime < them.st_mtime);

}

/**
 * Get all messages from the currently selected folders.
 */
std::vector<CMessage *>* CGlobal::get_messages()
{
  return( m_messages );
}


/**
 * Update the list of global messages, using the index_limit string set by lua.
 */
void CGlobal::update_messages()
{
  /**
   * If we have items already then free each of them.
   */
  if ( m_messages != NULL ) {
    std::vector<CMessage *>::iterator it;
    for (it = m_messages->begin(); it != m_messages->end(); ++it) {
      delete( *it );
    }
    delete( m_messages );
  }

  /**
   * create a new store.
   */
  m_messages = new std::vector<CMessage *>;


  /**
   * Get the selected maildirs.
   */
  CGlobal *global = CGlobal::Instance();
  std::vector<std::string> folders = global->get_selected_folders();
  std::string * filter = global->get_index_limit();


  /**
   * For each selected maildir read the messages.
   */
  std::vector<std::string>::iterator it;
  for (it = folders.begin(); it != folders.end(); ++it) {

    /**
     * get the messages from this folder.
     */
    CMaildir tmp = CMaildir(*it);
    std::vector<CMessage *> contents = tmp.getMessages();

    /**
     * Append to the list of messages combined.
     */
    std::vector<CMessage *>::iterator mit;
    for (mit = contents.begin(); mit != contents.end(); ++mit) {
      if ( (*mit)->matches_filter( filter ) )
        m_messages->push_back(*mit) ;
    }
  }

  /*
   * Sort?
   */
  std::sort(m_messages->begin(), m_messages->end(), my_sort);

}

/**
 * Remove all selected folders.
 */
void CGlobal::unset_folders()
{
    m_selected_folders.clear();
}

/**
 * Add a folder to the selected set.
 */
void CGlobal::add_folder(std::string path)
{
    m_selected_folders.push_back(path);
}

/**
 * Remove a folder from the selected set.
 */
bool CGlobal::remove_folder(std::string path)
{
    std::vector<std::string>::iterator it;

  /**
   * Find the folder.
   */
    it = std::find(m_selected_folders.begin(), m_selected_folders.end(), path);

  /**
   * If we found it reemove it.
   */
    if (it != m_selected_folders.end()) {
	m_selected_folders.erase(it);
	return true;
    }

  /**
   * Failed to find it.
   */
    return false;

}

/**
 * Get the from address.
 */
std::string * CGlobal::get_default_from()
{
  return( m_from_address );
}

/**
 * Set the from-address.
 */
void CGlobal::set_default_from( std::string *address)
{
    if (m_from_address)
	delete(m_from_address);

    m_from_address = address;
}

/**
 * Get the sendmail path
 */
std::string * CGlobal::get_sendmail_path()
{
    return( m_sendmail_path );
}

/**
 * Set the sendmail path
 */
void CGlobal::set_sendmail_path( std::string *path)
{
    if ( m_sendmail_path )
        delete( m_sendmail_path );

    m_sendmail_path = path;
}
