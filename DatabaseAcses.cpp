#include "DatabaseAcses.h"
#include "sqlite3.h"
#include <io.h>

#define CREATE_USERS "CREATE TABLE IF NOT EXISTS USERS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);"
#define CREATE_ALBUMS "CREATE TABLE IF NOT EXISTS ALBUMS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, CREATION_DATE INTEGER NOT NULL, USER_ID INTEGER, FOREIGN KEY (USER_ID) REFERENCES USERS (ID));"
#define CREATE_PICTURES "CREATE TABLE IF NOT EXISTS PICTURES (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL,CREATION_DATE INTEGER NOT NULL, ALBUM_ID INTEGER, FOREIGN KEY (ALBUM_ID) REFERENCES ALBUMS (ID));"
#define CREATE_TAGS "CREATE TABLE IF NOT EXISTS TAGS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY (USER_ID) REFERENCES USERS (ID), FOREIGN KEY (PICTURE_ID) REFERENCES PICTURES (ID));"
#define ID "ID"
#define NAME "NAME"
#define CREATION_DATE "CREATION_DATE"
#define USER_ID "USER_ID"

std::list<Album> DatabaseAccess::albums;

bool DatabaseAccess::open()
{
	std::string dbFileName = "Gallery.sqlite";
	int file_exist = _access(dbFileName.c_str(), 0);
	int res = sqlite3_open(dbFileName.c_str(), &this->_db);

	if (res != SQLITE_OK) {
		this->_db = nullptr;
		std::cout << "Failed to open DB" << std::endl;
		return -1;
	}

	char** errMessage = nullptr;
	const char* sqlStatement = nullptr;
	
	this->runCommand(CREATE_USERS, this->_db);
	this->runCommand(CREATE_ALBUMS, this->_db);
	this->runCommand(CREATE_PICTURES, this->_db);
	this->runCommand(CREATE_TAGS, this->_db);
}

void DatabaseAccess::clear()
{

}

bool DatabaseAccess::runCommand(const char* sqlStatement, sqlite3* db, int(*callback)(void*, int, char**, char**), void* secondParam)
{
	char** errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement, callback, secondParam, errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << "1: " << res;
		return false;
	}
}


const std::list<Album> DatabaseAccess::getAlbums()
{
	char* sqlCommand = "SELECT * FROM ALBUMS;";
	this->runCommand(sqlCommand, this->_db, loadIntoAlbums);
	return DatabaseAccess::albums;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::string command = ""
	this->runCommand(sqlCommand, this->_db, loadIntoAlbums);
	return DatabaseAccess::albums;
}

int loadIntoAlbums(void* data, int argc, char** argv, char** azColName)
{
	// Create a new Album object for each row fetched
	Album albumObj (0, "");

	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == "USER_ID") {
			albumObj.setOwner(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == "CREATION_DATE") {
			albumObj.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == "NAME") {
			albumObj.setName(argv[i]);
		}
	}

	// Add the newly created Album object to the passed-in vector
	DatabaseAccess::albums.push_back(albumObj);

	return 0; // Return 0 to indicate success
}
