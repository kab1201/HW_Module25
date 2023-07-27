#include "Chat.h"

#if defined(_WIN32)
// Colors for the console				analogs for Linux
const size_t colorWhite = 7;		// \033[37m ... \033[0m
const size_t colorLightBlue = 11;	// \033[96m ... \033[0m
const size_t colorLightGreen = 10;	// \033[92m ... \033[0m
const size_t colorYellow = 14;		// \033[93m ... \033[0m
const size_t colorDarkBlue = 19;	// \033[94m ... \033[0m

// Declare and initialize variables to change the color of text in the console
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
size_t concolColor = colorWhite;
#endif

// Reserve place for users in unordered_map
const size_t countChatUsers = 200;
// Reserve place for masseges in vector
const size_t countMessages = 1000;

Chat::Chat()
{
	isWork_ = true;
}

bool Chat::getIsWork() const { return isWork_; }
bool Chat::getIsToolsMenu() const { return isToolsMenu_; }

void Chat::setPtrDatabase(shared_ptr<Database> ptr_database) { ptr_database_ = ptr_database; }

int Chat::getIDCurrentUser() const { return id_currentUser_; }
void Chat::setIDCurrentUser(int id_currentUser) { id_currentUser_ = id_currentUser; }

int Chat::getUserIDByLogin(const string& login) const
{
	string& res = ptr_database_->getOneValue("SELECT id FROM Users WHERE login = '" + login + "' AND deleted = 0");
	
	return atoi(res.c_str());
}

int Chat::getUserIDByName(const string& name) const
{
	string& res = ptr_database_->getOneValue("SELECT id FROM Users WHERE name = '" + name + "'");

	return atoi(res.c_str());
}

string Chat::getUserName(const int userID) const
{
	string& res = ptr_database_->getOneValue("SELECT name FROM Users WHERE id = " + to_string(userID));
	return res;
}

uint Chat::getHashPassword(const int userID) const
{ 
	string& res = ptr_database_->getOneValue("SELECT hash FROM Hash WHERE id = " + to_string(userID));
	return stoll(res.c_str());
}

string Chat::getSalt(const int userID) const
{
	string& res = ptr_database_->getOneValue("SELECT salt FROM Hash WHERE id = " + to_string(userID));
	return res;
}

bool Chat::isAlphaNumeric(const string &str)
{
	auto it = find_if_not(str.begin(), str.end(), ::isalnum);
	return it == str.end();
}

void Chat::checkNewLogin(const string& login)
{
	if (!isAlphaNumeric(login))
		throw string("The login must contain letters and numbers only!\n");

	if (login == "all")
		throw string("This login is reserved!\n");

	auto userID = getUserIDByLogin(login);
	if (userID > 0)
		throw string("This login already exists!\n");
}

void Chat::checkNewPassword(const string& psw)
{
	if (psw.length() < minPswLen)
		throw string("\nPassword must be equal to or more than 5 characters long!\n");

	if (psw.length() > maxPswLen)
		throw string("\nPassword must not exceed 20 characters long!\n");

	const char* pattern = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789"
		"`~!@#$%^&*()_-+=|/[]{}:;',.<>\?\ ";

	if (psw.find_first_not_of(pattern) != string::npos)
		throw string("\nPassword contains invalid characters!\n");
}

void Chat::checkLogin(const string& login, const string& psw)
{

	auto userID = getUserIDByLogin(login);
	if (!userID)
		throw string("Invalid login! Try again, please.\n");

	string salt = getSalt(userID);
	/*uint get_hash = getHashPassword(userID);
	string v = psw + salt;
	uint create_hash = createHashPassword(v);*/
	if (getHashPassword(userID) != createHashPassword(psw + salt))
		throw string("Invalid password! Try again, please.\n");

	setIDCurrentUser(userID);
	if (ptr_database_->query("UPDATE Users SET last_login_date = CURRENT_TIMESTAMP WHERE id = " + to_string(userID)) != 0)
		throw string("Cann't update the field in database:") + ptr_database_->getMySQLError();
}

#if defined(_WIN32)
string Chat::passwordField()
{
	const size_t size_password = 25;
	char password[size_password] = { 0 };
	char ch;
	size_t k = 0;
	while (k < size_password - 1)
	{
		ch = _getch();
		password[k] = ch;
		if (password[k] == 13) {
			password[k] = '\0';
			break;
		}
		else if (password[k] == 8) {
			cout << "\b \b";
			--k;
		}
		else {
			++k;
			cout << '*';
		}
	}

	return string(password);
}
#endif

uint Chat::createHashPassword(const string& psw)
{
	// Convert string to char*[]
	size_t lenPsw = psw.length();
	char* userPsw = new char[lenPsw];
	memcpy(userPsw, psw.c_str(), lenPsw);

	// Hash password (including salt)
	uint* userHashPsw = sha1(userPsw, lenPsw);

	// Clear
	delete[] userPsw;

	return *userHashPsw;
}

string Chat::createSalt()
{
	// Using the computer’s internal clock to generate the seed.
	const auto p = std::chrono::system_clock::now() - std::chrono::hours(24);
	auto seed = p.time_since_epoch().count();
	// Quickly generating pseudo random integers between 1 and 2147483646
	default_random_engine randomEngine(seed);
	// Converting those random integers into the range [33, 126] such that they’re uniformly distributed.
	uniform_int_distribution<int> distribution(0x21, 0x7E);
	auto randomReadableCharacter = [&]() { return distribution(randomEngine); };

	size_t size = 10;
	string salt;
	generate_n(back_inserter(salt), size, randomReadableCharacter);

	return salt;
}

void Chat::signUp()
{
	string userLogin;
	string userPassword;
	string userName;

	do
	{
		try
		{
			// User sign in attempt 
			cout << "Login: ";
			cin >> userLogin;
			checkNewLogin(userLogin);

#if defined(_WIN32)
			cout << endl << "Password: ";
			userPassword = passwordField();
#else
			userPassword = getpass("Password: ");
#endif
			checkNewPassword(userPassword);

			cout << endl << "Name (optional): ";
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			getline(cin, userName);
			cout << endl;

			if (userName.length() == 0)
				userName = userLogin;

			string salt = createSalt();
			if (ptr_database_->query("INSERT INTO Users (name, login) VALUES ('" + userName + "','" + userLogin + "')") != 0)
				throw string("Cann't insert new user: " + ptr_database_->getMySQLError());
			string res = ptr_database_->getOneValue("SELECT LAST_INSERT_ID() AS id");
			int idNewUser = atoi(res.c_str());
			if (ptr_database_->query("UPDATE Hash SET hash = " + to_string(createHashPassword(userPassword + salt)) + ", salt = '" + salt + "' WHERE id = " + to_string(idNewUser)) != 0)
				throw string("Cann't update hash: " + ptr_database_->getMySQLError());
			setIDCurrentUser(idNewUser);
		}
		catch (const string& error_message)
		{
			id_currentUser_ = NULL;
			cerr << error_message;
		}
	} while (id_currentUser_ == NULL);

	cout << userName << ", you have successfully registered!\n" << endl;
}

void Chat::signIn()
{
	string userLogin;
	string userPassword;
	size_t cntAttemptsLogin = 0;

	do
	{
		cout << "Login: ";
		cin >> userLogin;
#if defined(_WIN32)
		cout << endl << "Password: ";
		userPassword = passwordField();
#else
		userPassword = getpass("Password: ");
#endif
		cout << endl;

		try
		{
			// User sign in attempt 
			++cntAttemptsLogin;
			checkLogin(userLogin, userPassword);
		}
		catch (const string& error_message)
		{
			cerr << error_message;
		}
	} while (id_currentUser_ == NULL && (cntAttemptsLogin < cntAttempts));

	if (id_currentUser_ == NULL)
		cout << "You have made " << cntAttempts << " attempts! Sign Up, please.\n";
}

void Chat::Exit()
{
	cout << "Goodby, " << getUserName(id_currentUser_) << "!" << endl;
	if (id_currentUser_)
		id_currentUser_ = NULL;
}

void Chat::showAllUsers()
{
	// change the color of the text
#if defined(_WIN32)
	concolColor = colorDarkBlue;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
	cout << "*****   USERS   *****";
	// reset the color of the text
	concolColor = colorWhite;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
#else
	cout << "\033[94m*****   USERS   *****\033[0m" << endl;
#endif 
	ptr_database_->getQueryResult("SELECT CASE WHEN id = " + to_string(id_currentUser_) + " THEN CONCAT(name, ' *') ELSE name END AS userName FROM Users ORDER BY 1");
}

void Chat::showAllMessages()
{
	// change the color of the text
#if defined(_WIN32)
	concolColor = colorDarkBlue;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
	cout << "*****   MESSAGES   *****" << endl;
	// change the color of the text
	concolColor = colorWhite;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
#else
	cout << "\033[94m*****   MESSAGES   *****\033[0m" << endl;
#endif 
	cout << "in|out \tstatus \tfrom \tto \tmessage";
	ptr_database_->query("UPDATE Messages SET status = status + 1 WHERE status IN (0, 1) AND recipient_id IN (1, " + to_string(id_currentUser_) + ") ");
	ptr_database_->getQueryResult("SELECT CASE WHEN m.sender_id = " + to_string(id_currentUser_) + " THEN '<-  ' ELSE '->  ' END AS in_out, "
		"CASE WHEN m.status != 2 THEN 'new' ELSE '' END AS status, "
		"u1.name AS 'from', u2.name AS 'to', m.message "
		"FROM Messages AS m "
		"INNER JOIN Users AS u1 ON m.sender_id = u1.id "
		"INNER JOIN Users AS u2 ON m.recipient_id = u2.id "
		"WHERE m.sender_id = " + to_string(id_currentUser_) + " OR m.recipient_id IN (1, " + to_string(id_currentUser_) + ") "
		"ORDER BY creation_date");
}

void Chat::sendMessage()
{
	string to{};
	string text{};

	cout << "To (name or all): ";
	cin >> to;
	cout << endl << "Text: ";
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	getline(cin, text);
	cout << endl;

	try
	{
		int idUser = getUserIDByName(to);
		if (!idUser && to != "all")
			throw string("Invalid user name (to)! Choose another user, please.\n");

		if (ptr_database_->query("INSERT INTO Messages (sender_id, recipient_id, message) VALUES (" + to_string(id_currentUser_) + "," + to_string(idUser) + ",'" + text + "')") != 0)
			throw string("Cann't insert message to database: " + ptr_database_->getMySQLError());
	}
	catch (const string& error_message)
	{
		cerr << error_message;
	}
}

void Chat::changePassword()
{
	string newPassword;

#if defined(_WIN32)
	cout << endl << "Enter new password: ";
	newPassword = passwordField();
#else
	newPassword = getpass("Password: ");
#endif
	cout << endl;

	try
	{
		checkNewPassword(newPassword);
	}
	catch (const string& error_message)
	{
		cerr << error_message;
	}

	if (ptr_database_->query("UPDATE Hash SET hash = " + to_string(createHashPassword(newPassword + getSalt(id_currentUser_))) + " WHERE id = " + to_string(id_currentUser_)) != 0)
		throw string("Cann't update hash: " + ptr_database_->getMySQLError());
}

void Chat::changeName()
{
	string newName;

	cout << "Enter new nick name: ";
	cin.ignore(numeric_limits<streamsize>::max(), '\n');
	getline(cin, newName);
	cout << endl;

	if (ptr_database_->query("UPDATE Users SET name = '" + newName + "' WHERE id = " + to_string(id_currentUser_)) != 0)
		throw string("Cann't update name: " + ptr_database_->getMySQLError());
}

void Chat::chatMenu()
{
	int choiceSign = 1;

	// change the color of the text
#if defined(_WIN32)
	concolColor = colorYellow;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
	cout << string(47, '-') << endl;
	cout << "| Sign In (1) || Sign Up (2) || Stop chat (0) |" << endl;
	cout << string(47, '-') << endl;
	// reset the color of the text
	concolColor = colorWhite;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
#else
	cout << "\033[93m" << string(47, '-') << endl;
	cout << "| Sign In (1) || Sign Up (2) || Stop chat (0) |" << endl;
	cout << string(47, '-') << "\033[0m" << endl;
#endif
	cin >> choiceSign;

	switch (choiceSign)
	{
	case 1:
		signIn();
		break;
	case 2:
		signUp();
		break;
	case 0:
		isWork_ = false;
		break;
	default:
		cout << "Wrong choice!" << endl << endl;
		break;
	}
}

void Chat::userMenu()
{
	int choice = 1;

#if defined(_WIN32)
	// change the color of the text
	concolColor = colorLightBlue;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
	cout << string(74, '-') << endl;
	cout << "| Send Message (1) || Messages (2) || Users (3) || Tools (4) || Exit (0) |" << endl;
	cout << string(74, '-') << endl;
	// reset the color of the text
	concolColor = colorWhite;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
#else
	cout << "\033[96m" << string(74, '-') << endl;
	cout << "| Send Message (1) || Messages (2) || Users (3) || Tools (4) || Exit (0) |" << endl;
	cout << string(74, '-') << "\033[0m" << endl;
#endif
	cin >> choice;

	switch (choice)
	{
	case 1:
		sendMessage();
		break;
	case 2:
		showAllMessages();
		break;
	case 3:
		showAllUsers();
		break;
	case 4:
		isToolsMenu_ = true;
		break;
	case 0:
		Exit();
		break;
	default:
		cout << "Wrong choice!" << endl << endl;
		break;
	}
}

void Chat::toolsMenu()
{
	int choice = 1;

#if defined(_WIN32)
	// change the color of the text
	concolColor = colorLightGreen;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
	cout << string(56, '-') << endl;
	cout << "| Change Name (1) || Change password (2) || Return (0) |" << endl;
	cout << string(56, '-') << endl;
	// change the color of the text
	concolColor = colorWhite;
	SetConsoleTextAttribute(hConsole, concolColor);
	// ----------
#else
	cout << "\033[92m" << string(56, '-') << endl;
	cout << "| Change Name (1) || Change password (2) || Return (0) |" << endl;
	cout << string(56, '-') << "\033[0m" << endl;
#endif
	cin >> choice;

	switch (choice)
	{
	case 1:
		changeName();
		break;
	case 2:
		changePassword();
		break;
	case 0:
		isToolsMenu_ = false;
		break;
	default:
		cout << "Wrong choice!" << endl << endl;
		break;
	}
}