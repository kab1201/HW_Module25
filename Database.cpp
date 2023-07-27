#include "Database.h"

Database::Database(
	const string &host,
	const string &user,
	const string &password,
	const string &database,
	unsigned int port) :
	_host(host),
	_user(user),
	_password(password),
	_database(database),
	_port(port) {}

Database::~Database()
{
	/*if (&_mysql != nullptr) {
		mysql_close(&_mysql);
	}*/
}

void Database::init()
{
	mysql_init(&_mysql);

	if (&_mysql == nullptr) 	{
		// Если дескриптор не получен — выводим сообщение об ошибке
		cout << "Error: can't create MySQL-descriptor" << endl;
	}
}

void Database::connect()
{
	if (!mysql_real_connect(&_mysql, _host.c_str(), _user.c_str(), _password.c_str(), _database.c_str(), _port, NULL, 0)) {
		// Если нет возможности установить соединение с БД выводим сообщение об ошибке
		cout << "Error: can't connect to database: " << mysql_error(&_mysql) << endl;
	}
	else {
		// Если соединение успешно установлено выводим фразу — "Success!"
		cout << "Success connect to database!" << endl;
	}
	mysql_set_character_set(&_mysql, "utf8mb4");
	//Смотрим изменилась ли кодировка на нужную, по умолчанию идёт latin1. 
	cout << "Connection characterset: " << mysql_character_set_name(&_mysql) << endl;
}

string Database::getMySQLError() 
{
	return mysql_error(&_mysql);
};

int Database::query(const string &sql_query)
{
	return mysql_query(&_mysql, sql_query.c_str());
}

void Database::getQueryResult(const string &sql_query)
{
	MYSQL_RES* res;
	MYSQL_ROW row;

	mysql_query(&_mysql, sql_query.c_str());
	//Выводим все, что есть в базе через цикл
	if (res = mysql_store_result(&_mysql)) {
		cout << endl;
		while (row = mysql_fetch_row(res)) {
			for (size_t i = 0; i < mysql_num_fields(res); i++) {
				cout << row[i] << "\t";
			}
			cout << endl;
		}
	}
	else
		cout << "Ошибка MySql номер " << mysql_error(&_mysql);
}

size_t Database::getRowCountOfQuery(const string &sql_query)
{
	MYSQL_RES* res;

	mysql_query(&_mysql, sql_query.c_str());
	if (res = mysql_store_result(&_mysql)) {
		return res->row_count;
	}
	else {
		cout << "Ошибка MySql номер " << mysql_error(&_mysql);
		return 0;
	}

}

string Database::getOneValue(const string &sql_query)
{
	MYSQL_RES* res;
	MYSQL_ROW row;

	mysql_query(&_mysql, sql_query.c_str());
	if (res = mysql_store_result(&_mysql)) {
		if (res->row_count != 0) {
			row = mysql_fetch_row(res);
			return row[0];
		}
	}
	return "";
}
