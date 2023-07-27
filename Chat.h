#pragma once

#if defined(_WIN32)
#include <conio.h>
#define NOMINMAX
#include <windows.h>
#else
#include <unistd.h>
#include <cstring>
#endif

#include "sha1.h"
#include "Database.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <random>
#include <chrono>
#include <algorithm>
#include <filesystem>

using namespace std;

class Chat
{
private:
	const size_t cntAttempts = 3; // count of attempts to Sign In
	const size_t minPswLen = 5; // minimum password length
	const size_t maxPswLen = 20; // maximum password length

	bool isWork_{ false };
	bool isToolsMenu_{ false };

	shared_ptr<Database> ptr_database_ = nullptr;
	int id_currentUser_{NULL};

	void setIDCurrentUser(int id_currentUser);

	int getUserIDByLogin(const string& login) const;
	int getUserIDByName(const string& name) const;
	string getUserName(const int userID) const;
	uint getHashPassword(const int userID) const;
	string getSalt(const int userID) const;

	bool isAlphaNumeric(const string &str);
	void checkNewLogin(const string& login);
	void checkNewPassword(const string& psw);
	void checkLogin(const string& login, const string& psw);

	string passwordField();

	uint createHashPassword(const string& psw);
	string createSalt();

	void signUp();
	void signIn();
	void Exit();

	void showAllUsers();
	void showAllMessages();
	void sendMessage();

	void changePassword();
	void changeName();

public:
	Chat();
	~Chat() = default;

	bool getIsWork() const;
	bool getIsToolsMenu() const;
	int getIDCurrentUser() const;
	void setPtrDatabase(shared_ptr<Database> ptr_database);
	void chatMenu();
	void userMenu();
	void toolsMenu();
};

