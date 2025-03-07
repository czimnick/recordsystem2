// Copyright (c) 2012 q3df-team. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "RecordsystemDaemon.h"

#include <google/protobuf/rpc/rpc_server.h>
#include <google/protobuf/rpc/rpc_client.h>
#include <boost/program_options.hpp>
#include <sstream>

#include "Q3dfApiImpl.h"
#include "../client/StringTokenizer.h"

using namespace ::google::protobuf;
using namespace ::google::protobuf::rpc;
using namespace ::service;

namespace po = boost::program_options;

extern "C" {
	const char *va( const char *format, ... ) {
		va_list		argptr;
		static char		string[2][32000];	// in case va is called by nested functions
		static int		index = 0;
		char	*buf;

		buf = string[index & 1];
		index++;

		va_start (argptr, format);
		vsprintf (buf, format, argptr);
		va_end (argptr);

		return buf;
	}
}

static sql::Driver *driver = NULL;
GlobalObject *RS = NULL;


GlobalObject::GlobalObject() {
	this->con_ = Console::Create();
	this->clientList_ = new ClientList();
	this->env_ = new Q3dfEnv();
	this->cmdList_ = new vector<CommandBase*>();
}


GlobalObject::~GlobalObject() {
	delete this->mysqlPool_;
	delete this->clientList_;
	delete this->env_;
	delete this->con_;

	while(!this->cmdList_->empty()) {
		auto pBase = this->cmdList_->back();
		this->cmdList_->pop_back();
		delete pBase;
	}

	delete this->cmdList_;
}

void GlobalObject::Initialize(string mysql_hostname, string mysql_username, string mysql_password, string mysql_database, int poolSize) {
	int i;
	this->con_->PrintInfo("Initialize MysqlPool to database server '^5%s^7' with ^5%i^7 connections...\n", mysql_hostname.c_str(), poolSize);
	this->mysqlPool_ = new MysqlPool(poolSize, [&mysql_hostname, &mysql_username, &mysql_password, &mysql_database]() -> sql::Connection* {
		if(driver == NULL)
			driver = sql::mysql::get_driver_instance();

		/* Using the Driver to create a connection */
		sql::Connection *con = driver->connect(mysql_hostname, mysql_username, mysql_password);

		auto_ptr< sql::Statement > stmt(con->createStatement());
		stmt->execute(mysql_database);

		return con;
	});

	for(i=0; i<gCommandStore.GetCount(); i++) {
		auto pBase = gCommandStore.GetAt(i)->Create();
		this->con_->PrintInfo("Command [^5%s^7] initialied...\n", pBase->Name().c_str());
		this->cmdList_->push_back(pBase);
	}
}

ClientList* GlobalObject::Clients() {
	return this->clientList_;
}


SettingsMap GlobalObject::Settings() {
	return this->settings_;
}


void GlobalObject::SetSetting(string key, string value) {
	settings_[key].clear();
	settings_[key].append(value);
}


Console* GlobalObject::Con() {
	return this->con_;
}


string const& GlobalObject::GetSetting(string key) {
	return settings_[key];
}


bool GlobalObject::HasSettingKey(string key) {
	return settings_.find(key) != settings_.end();
}


void GlobalObject::PrintSettingsList() {
	SettingsMapIterator it;
	this->Con()->Print("  Settings-List\n");
	this->Con()->Print(" ^3---------------------------------------^7\n");
	for (it=settings_.begin(); it!=settings_.end(); ++it)
		this->Con()->Print("    %s = '%s'\n", it->first.c_str(), it->second.c_str());
}


bool GlobalObject::HandleCommand(string const& cmd, const vector<string>* args, bool fromApi, Conn *contextCon, string *output) {
	vector<CommandBase *>::iterator it;
	bool haveToExecute = false;

	for(it = this->cmdList_->begin(); it != this->cmdList_->end(); ++it) {
		if((*it)->Name() == cmd) {
			if(fromApi && (*it)->IsAlsoProxymodCommand())
				haveToExecute = true;
			else if(!fromApi)
				haveToExecute = true;

			if(haveToExecute)
				return (*it)->Execute(args, contextCon, output);

			output->clear();
			output->append("unknown command");
			return false;
		}
	}

	output->clear();
	output->append("unknown command");
	return false;
}


MysqlPool* GlobalObject::SqlPool() {
	return this->mysqlPool_;
}


Q3dfEnv* GlobalObject::Env() {
	return this->env_;
}


/*********************************************
 * Recordsystem Daemon
 * ENTRY FUNCTION ;)
 */

int main(int argc, char **argv) {
	string mysql_hostname;
	string mysql_username;
	string mysql_password;
	string mysql_database;

	int listenPort = 1234;
	int mysqlPoolSize = 10;

	po::options_description desc("Allowed options");
	desc.add_options()
	  ("help", "produce help message")
	  ("mysql_host",   po::value<string>(&mysql_hostname)->default_value(string("localhost")),  "MySQL host.")
	  ("mysql_user",   po::value<string>(&mysql_username)->default_value(string("root")),  "MySQL username.")
	  ("mysql_passwd", po::value<string>(&mysql_password)->default_value(string("root")),  "MySQL password.")
	  ("mysql_db",     po::value<string>(&mysql_database)->default_value(string("USE db_q3df")), "MySQL database name.")
	  ("api_listen_port", po::value<int>(&listenPort)->default_value(1234), "api listen on a specific port.")
	  ("mysql_pool_size", po::value<int>(&mysqlPoolSize)->default_value(10), "The pool size of the mysql connections.")
	;
	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
	po::notify(vm);

	if (vm.count("help")) {
		std::ostringstream tmpDesc;
		//desc.print(tmpDesc);

		printf("Usage: recordsystemd [options]\n");
		printf("%s", tmpDesc.str().c_str());
		return 0;
	}

	try {
		RS = new GlobalObject();
		RS->Initialize(mysql_hostname, mysql_username, mysql_password, mysql_database, mysqlPoolSize);
	} catch(sql::SQLException &e) {
		RS->Con()->PrintError("SQL: %s\n", e.what());
		return -1;
	}

	sql::Connection *mcon = RS->SqlPool()->Get();
	try {
		auto_ptr< sql::Statement > stmt(mcon->createStatement());
		/* Fetching again but using type convertion methods */
		auto_ptr< sql::ResultSet > res(stmt->executeQuery("SELECT * FROM q3_servers ORDER BY id"));
		RS->Con()->PrintInfo("Load apikeys of all servers\n");
		RS->Con()->PrintInfo("---------------------------\n");
		while (res->next()) {
			if(res->getString("apikey").length() > 0) {
				RS->Con()->Print("  - get apikey for '%s'\n", res->getString("name").c_str());
				string key(va("apikey-%i", res->getInt("id")));
				RS->SetSetting(key, string(res->getString("apikey")));
			}
		}
	} catch (sql::SQLException &e) {
		RS->Con()->PrintError("could not load servr api keys: '%s'\n", e.what());
	}

	RS->SqlPool()->Return(mcon);
	Server server(RS->Env());
	server.AddService(new Q3dfApiImpl(), true);
	RS->Con()->PrintInfo("API-Server starting listen on port %i.\n", listenPort);
	server.ListenTCP(1234);

	for(;;) {
		char *cmd = RS->Con()->Input();
		if(cmd && !strncmp(cmd, "exit", 4)) {
			break;

		} else if(cmd && !strncmp(cmd, "status", 5)) {
			RS->Clients()->PrintList();

		} else if(cmd && !strncmp(cmd, "settingslist", 12)) {
			RS->PrintSettingsList();

		} else if(cmd && !strncmp(cmd, "set", 3)) {
			StringTokenizer *cmdline = new StringTokenizer(cmd, false);
			if(cmdline->Argc() == 3) {
				string key(cmdline->Argv(1));
				RS->SetSetting(key, string(cmdline->Argv(2)));
				RS->Con()->Print("%s='%s' SAVED.\n", cmdline->Argv(1), RS->GetSetting(key).c_str());
			}else
				RS->Con()->PrintError("usage: set <varname> <value>\n");

			delete cmdline;

		} else if(cmd && !strncmp(cmd, "get", 3)) {
			StringTokenizer *cmdline = new StringTokenizer(cmd, false);
			string key(cmdline->Argv(1));
			if(cmdline->Argc() == 2 && RS->HasSettingKey(key)) {
				RS->Con()->Print("RESULT: %s='%s'\n", cmdline->Argv(1), RS->Settings()[key].c_str());
			}else
				RS->Con()->PrintError("RESULT: '%s' not found!\n", cmdline->Argv(1));

			delete cmdline;

		} else if(cmd) {
			StringTokenizer *cmdline = new StringTokenizer(cmd, false);
			vector<string> *t = new vector<string>();
			string output("");

			for(int i = 1; i<cmdline->Argc(); i++) {
				t->push_back(cmdline->Argv(i));
			}
						
			RS->HandleCommand(string(cmdline->Argv(0)), t, false, nullptr, &output);
			RS->Con()->Print("%s\n", output.c_str());

			delete cmdline;
			delete t;
		}

		Conn *conn = server.AcceptNonBlock();
		if(conn) {
			RS->Con()->PrintInfo("incoming connection from ^3%s^7\n", conn->RemoteIpAdress());
			server.Serve(conn);
		}

		Sleep(10);
	}

	RS->Con()->PrintInfo(va("Shutingdown now...\n"));
	delete RS;

	return 0;
}
