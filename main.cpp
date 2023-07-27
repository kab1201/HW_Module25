#include "Database.h"
#include "Chat.h"

void createDatabase(Database &db)
{
	db.query("CREATE TABLE IF NOT EXISTS Users("
		"id INT NOT NULL AUTO_INCREMENT,"
		"name VARCHAR(255) NOT NULL,"
		"login VARCHAR(255) NOT NULL UNIQUE,"
		"email VARCHAR(255),"
		"regdate TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"last_login_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"deleted INT DEFAULT 0,"
		"CONSTRAINT pk_users PRIMARY KEY(id))");

	db.query("CREATE TABLE IF NOT EXISTS Hash("
		"id INT NOT NULL UNIQUE,"
		"hash INT UNSIGNED NULL,"
		"salt VARCHAR(10) NULL,"
		"CONSTRAINT fk_hash_id FOREIGN KEY (id) REFERENCES Users(id) ON DELETE CASCADE ON UPDATE CASCADE)");

	if (db.getRowCountOfQuery("SELECT 'True' FROM Users LIMIT 1") == 0) {
		db.query("CREATE TRIGGER NewUser_Insert_to_Hash AFTER INSERT ON Users FOR EACH ROW INSERT INTO Hash SET id = NEW.id");

		db.query("INSERT INTO Users(name, login, email)"
			"VALUES "
			"('all', 'all', 'all@mail.ru'),"
			"('TEST1', 'test1', 'test1@mail.ru'),"
			"('TEST2', 'test2', 'test2@mail.ru'),"
			"('TEST3', 'test3', 'test3@mail.ru'),"
			"('TEST4', 'test4', 'test4@mail.ru'),"
			"('TEST5', 'test5', 'test5@mail.ru')");

		db.query("UPDATE Hash "
			"SET hash = 1583067709,"
			"salt = 'Sqncgj3OFx'"
			"WHERE id = 1");

		db.query("UPDATE Hash "
			"SET hash = 3430300287,"
			"salt = 'vL@g`A6Gxp' "
			"WHERE id = 2");

		db.query("UPDATE Hash "
			"SET hash = 2451056565,"
			"salt = 'Zms(2<JT/>' "
			"WHERE id = 3");

		db.query("UPDATE Hash "
			"SET hash = 3353989826,"
			"salt = '-WC@4H!,[8' "
			"WHERE id = 4");

		db.query("UPDATE Hash "
			"SET hash = 2870157325,"
			"salt = 'gI~>.u7`X>' "
			"WHERE id = 5");

		db.query("UPDATE Hash "
			"SET hash = 1958071822,"
			"salt = 'b_T~W1T:R~'"
			"WHERE id = 6");
	}

	db.query("CREATE TABLE IF NOT EXISTS Messages("
		"id INT NOT NULL AUTO_INCREMENT,"
		"sender_id INT NOT NULL,"
		"recipient_id INT NOT NULL,"
		"message TEXT NOT NULL,"
		"creation_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP,"
		"status TINYINT NOT NULL DEFAULT 0 CHECK(status IN (0, 1, 2)),"
		"CONSTRAINT pk_messages PRIMARY KEY(id),"
		"CONSTRAINT fk_sender_id FOREIGN KEY (sender_id) REFERENCES Users (id) ON UPDATE CASCADE,"
		"CONSTRAINT fk_recipient_id FOREIGN KEY (recipient_id) REFERENCES Users(id) ON UPDATE CASCADE)");

	if (db.getRowCountOfQuery("SELECT 'True' FROM Messages LIMIT 1") == 0) {
		db.query("INSERT INTO Messages(sender_id, recipient_id, message, status)"
			"VALUES "
			"(2, 5, 'Hi, TEST4! How are you?', 2),"
			"(2, 3, 'I invite you to a meeting at 4:00 p.m.', 2),"
			"(3, 2, 'Hi, TEST1. Ok!', 2),"
			"(3, 1, 'Who lost their keys?', 1),"
			"(3, 1, 'Good day, all!', 1),"
			"(3, 5, 'Good day, TEST4!', 1)");
	}
}

int main()
{
	setlocale(LC_ALL, "");

	Database mysql_db(static_cast<string>("localhost"), static_cast<string>("root"), static_cast<string>("bdfqyj7MyS"), static_cast<string>("Chat_DB"), 3306);
	mysql_db.init();
	mysql_db.connect();
	createDatabase(mysql_db);

	Chat chat{};
	chat.setPtrDatabase(make_shared<Database>(mysql_db));

	do
	{
		chat.chatMenu();

		while (chat.getIDCurrentUser())
		{
			// Depending on the user's choice, the User menu or the Settings menu is displayed.
			if (!chat.getIsToolsMenu())
				chat.userMenu();
			else
				chat.toolsMenu();
		}

	} while (chat.getIsWork());

	return 0;
}