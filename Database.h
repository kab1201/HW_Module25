#pragma once
#include <iostream>
#include <string>
#include "mysql.h"
#pragma comment(lib, "libmysql.lib")

using namespace std;

class Database
{
public:
	Database(
		const string &host,
		const string &user,
		const string &password,
		const string &database,
		unsigned int port);

	~Database();

	void init();
	void connect();
	int query(const string &sql_query);
	string getMySQLError();
	void getQueryResult(const string &sql_query);
	size_t getRowCountOfQuery(const string &sql_query);
	string getOneValue(const string &sql_query);

private:
	MYSQL _mysql;
	const string _host;
	const string _user;
	const string _password;
	const string _database;
	unsigned int _port{ 0 };
};

