lumail
======

lumail is a modal console-based email client, which has built in support for scripting
via Lua.

The email client is a modal application, which means that you're *always* in one of three
states:

* Interacting with lists of mailboxes.
   * This is the mode you'll start in.
   * Move around via `j`/`k`/`/`.
   * You may open the single selected folder by pressing `RETURN`.
   * Or you may toggle the selected state by pressing `SPACE` and jumping into index mode with `I` when you've selected all the folders you care about.
* Interacting with lists of messages.
   * Press `a` to view all messages.
   * Press `n` to view new messages only.
* Interacting with a single message.

You'll find [a quick introduction to using lumail](http://lumail.org/getting-started/) on
the [lumail website](http://lumail.org).


Code
----

The application is developed in C++ and has intentionally minimal dependencies:

* lua 5.1 - The scripting language
* ncurses - The console input/graphics library.
* mimetic - The MIME-library.

Upon a Debian GNU/Linux system you may install all required packages with:

     # apt-get install libncurses-dev liblua5.1-0-dev lua5.1 libmimetic-dev

Although we might become more complex in the future the code currently builds
via a simple `Makefile`, and running `make` with no arguments should be sufficient.

Once compiled the client may be executed directly, but you will need to pass the
path to a configuration file:

     $ ./lumail --rcfile ./lumail.lua

For coding-style we use the following [Emacs](http://www.gnu.org/software/emacs/) settings:

    (setq c-default-style "linux" c-basic-offset 4)
    (c-set-offset 'substatement-open 0)



Current Status
--------------

Because lumail is modal application the coding has been split into sections:

* Code the display/manipulation of the Maildir folders.
    * This is complete.
    * You may scroll/search/limit the display of folders.
* Code the display/manipulation of the message-indexes.
    * This is functional.
    * You may scroll/search/limit the display of messages.  (But such searches are slow.)
* Code the display/manipulation of a single mail message.
    * You may view the first screen-ful of a message.  If it is text/plain.
    * Once viewed a message is marked as read.

Missing functionality largely relates to using this client for real.

The following features are missing:

* The ability to reply to a message.


Screenshots
-----------

* [Showing all folders](img/all.png).
* [Showing all folders with new mail](img/new.png).
* [Showing all folders which match a pattern](img/lj.png).


Configuration & Lua-Primitives
------------------------------

If you examine the supplied [lumail.lua](https://raw.github.com/skx/lumail/master/lumail.lua)
configuration file you'll get a flavour for the configuration.

The main part of the configuration is to point the mail-client at your local Maildir
location, from which all sub-folders will be determined at run-time.

At startup the following three lua files are evaluated, if present:

* `/etc/lumail.lua`
* `~/.lumail/config.lua`


Further Information
-------------------

You may find further information upon the lumail website:

* http://lumail.org/
    * This website is built automatically from the [lumail.org website repository](https://github.com/skx/lumail.org/).


Steve
--
