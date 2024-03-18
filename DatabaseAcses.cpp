#include "DatabaseAcses.h"
#include "sqlite3.h"

bool DatabaseAccess::open()
{
	std::cout << "Started " << std::endl;
	sqlite3* db;
	std::string dbFileName = "Gallery.sqlite";
	int file_exist = _access(dbFileName.c_str(), 0);
	int res = sqlite3_open(dbFileName.c_str(), &db);

	if (res != SQLITE_OK) {
		db = nullptr;
		std::cout << "Failed to open DB" << std::endl;
		return -1;
	}

	// cleaning the datbase 
	char** errMessage = nullptr;
	const char* sqlStatement = "DROP TABLE PERSONS; DROP TABLE Phone; DROP TABLE PhonePrefix; ";
	runCommand(sqlStatement, db);
	sqlStatement = "CREATE TABLE IF NOT EXISTS PERSONS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, LAST_NAME TEXT NOT NULL, FIRST_NAME TEXT NOT NULL, EMAIL TEXT NOT NULL);";
	runCommand(sqlStatement, db);

	sqlStatement = "CREATE TABLE IF NOT EXISTS Phone ( PhoneID	INTEGER, PhonePrefixID INTEGER, PhoneNumber INTEGER, PersonID INTEGER, FOREIGN KEY(PhonePrefixID) REFERENCES PhonePrefix(PhonePrefixID), FOREIGN KEY(PersonID) REFERENCES  PERSONS(ID) PRIMARY KEY (\"PhoneID\" AUTOINCREMENT)); ";
	runCommand(sqlStatement, db);
}
