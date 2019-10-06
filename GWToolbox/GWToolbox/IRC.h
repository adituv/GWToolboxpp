/*
	cpIRC - C++ class based IRC protocol wrapper
	Copyright (C) 2003 Iain Sheppard

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

	Contacting the author:
	~~~~~~~~~~~~~~~~~~~~~~

	email:	iainsheppard@yahoo.co.uk
	IRC:	#magpie @ irc.quakenet.org
*/

#include <stdio.h>
#include <stdarg.h>
#include <WinSock2.h>
#include <thread>

#define __CPIRC_VERSION__	0.1
#define __IRC_DEBUG__ 0

#define IRC_USER_VOICE	1
#define IRC_USER_HALFOP	2
#define IRC_USER_OP		4

struct irc_reply_data
{
	char* nick;
	char* ident;
	char* host;
	char* target;
};

struct irc_command_hook
{
	char* irc_command;
	int (*function)(const char*, irc_reply_data*, void*);
	irc_command_hook* next;
};

struct channel_user
{
	char* nick;
	char* channel;
	char flags;
	channel_user* next;
};

class IRC
{
public:
	IRC();
	~IRC();
	int start(char* server, int port, char* nick, char* user, char* name, char* pass);
	void disconnect();
	int privmsg(char* target, char* message);
	int privmsg(char* fmt, ...);
	int notice(char* target, char* message);
	int notice(char* fmt, ...);
    
	int part(char* channel);
	int kick(char* channel, char* nick, char* message);
	int mode(char* modes);
	int mode(char* channel, char* modes, char* targets);
	int nick(char* newnick);
	int quit(char* quit_message);
	int raw(char* fmt, ...);
    int raw(wchar_t* fmt, ...);
    int join(char* channel) { return raw("JOIN %s\r\n", channel); }
    int kick(char* channel, char* nick) { return raw("KICK %s %s\r\n", channel, nick); }
	void hook_irc_command(const char* cmd_name, int (*function_ptr)(const char*, irc_reply_data*, void*));
	int message_loop();
    int message_fetch();
	int is_op(char* channel, char* nick);
	int is_voice(char* channel, char* nick);
	char* current_nick();
private:
    void error(int err);
	void call_hook(char* irc_command, char*params, irc_reply_data* hostd);
	/*void call_the_hook(irc_command_hook* hook, char* irc_command, char*params, irc_host_data* hostd);*/
	void parse_irc_reply(char* data);
	void split_to_replies(char* data);
	void insert_irc_command_hook(irc_command_hook* hook, const char* cmd_name, int (*function_ptr)(const char*, irc_reply_data*, void*));
	void delete_irc_command_hook(irc_command_hook* cmd_hook);
    // int irc_socket; // This fails when using winsock2.h in Windows. Define as SOCKET to fix?
	SOCKET irc_socket;
    char message_buffer[1024] = { 0 };
	bool connected;
	bool pending_disconnect;
	bool sentnick;
	bool sentpass;
	bool sentuser;
	char* cur_nick;
	FILE* dataout;
	FILE* datain;
	channel_user* chan_users;
	irc_command_hook* hooks;
    std::thread t;
};
