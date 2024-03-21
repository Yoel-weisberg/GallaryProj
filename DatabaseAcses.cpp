#include "DatabaseAcses.h"
#include "sqlite3.h"
#include <algorithm>
#include <io.h>

#define CREATE_USERS "CREATE TABLE IF NOT EXISTS USERS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL);"
#define CREATE_ALBUMS "CREATE TABLE IF NOT EXISTS ALBUMS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, CREATION_DATE INTEGER NOT NULL, USER_ID INTEGER, FOREIGN KEY (USER_ID) REFERENCES USERS (ID));"
#define CREATE_PICTURES "CREATE TABLE IF NOT EXISTS PICTURES (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, NAME TEXT NOT NULL, LOCATION TEXT NOT NULL,CREATION_DATE INTEGER NOT NULL, ALBUM_ID INTEGER, FOREIGN KEY (ALBUM_ID) REFERENCES ALBUMS (ID));"
#define CREATE_TAGS "CREATE TABLE IF NOT EXISTS TAGS (ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, PICTURE_ID INTEGER NOT NULL, USER_ID INTEGER NOT NULL, FOREIGN KEY (USER_ID) REFERENCES USERS (ID), FOREIGN KEY (PICTURE_ID) REFERENCES PICTURES (ID));"
#define ID "ID"
#define NAME "NAME"
#define CREATION_DATE "CREATION_DATE"
#define USER_ID "USER_ID"
#define LOCATION "LOCATION"
#define ALBUM_ID "ALBUM_ID"

std::list<Album> DatabaseAccess::albums;
std::list<Picture> DatabaseAccess::pictures;
std::vector<User> DatabaseAccess::users;

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

void DatabaseAccess::clear() {}

int DatabaseAccess::getTheNextId(const std::string& tableName)
{
	std::string command = "SELECT max(ID) FROM " + tableName + " ;" ;
	int currentMax = 0;

	this->runCommand(command, this->_db, countCallback, &currentMax);
	return currentMax == NULL ? 1 : currentMax + 1;
}

bool DatabaseAccess::doesPictureExistsInAlbum(const std::string& albumName, const std::string& pictureName)
{
	int albumId = this->openAlbum(albumName).getId();
	std::string query = "SELECT * FROM PICTURES WHERE ALBUM_ID = " + std::to_string(albumId) + " AND NAME = \" " + pictureName + " \" ;";
	this->runCommand(query, this->_db, loadIntoPictures);
	return DatabaseAccess::pictures.empty() ? false : true;
}

std::string DatabaseAccess::removeWhiteSpacesBeforeAndAfter(const std::string& str)
{
	// Find the first non-whitespace character from the beginning
	size_t start = str.find_first_not_of(" \t\n\r");

	// If the string is all white spaces, return an empty string
	if (start == std::string::npos)
		return "";

	// Find the last non-whitespace character from the end
	size_t end = str.find_last_not_of(" \t\n\r");

	// Return the trimmed string
	return str.substr(start, end - start + 1);
}

bool DatabaseAccess::runCommand(const std::string& sqlStatement, sqlite3* db, int(*callback)(void*, int, char**, char**), void* secondParam)
{
	if (!DatabaseAccess::users.empty())
		DatabaseAccess::users.clear();

	if(!DatabaseAccess::albums.empty())
		DatabaseAccess::albums.clear();

	if(!DatabaseAccess::pictures.empty())
		DatabaseAccess::pictures.clear();

	char** errMessage = nullptr;
	int res = sqlite3_exec(db, sqlStatement.c_str(), callback, secondParam, errMessage);
	if (res != SQLITE_OK)
	{
		std::cout << "1: " << res;
		return false;
	}
}

Picture DatabaseAccess::getPictureFromAlbum(const std::string& albumName, const std::string& picture)
{
	int albumId = this->openAlbum(albumName).getId();
	std::string command = "SELECT * FROM PICTURES WHERE ALBUM_ID = " + std::to_string(albumId) + " AND NAME = \" " + this->removeWhiteSpacesBeforeAndAfter(picture) + " \" ;";

	this->runCommand(command, this->_db, loadIntoPictures);

	if (DatabaseAccess::pictures.size() != 0)
	{
		return *DatabaseAccess::pictures.begin();
	}
	else
	{
		throw std::invalid_argument("Picture with that name was not found ");
	}
}

bool DatabaseAccess::isUserTaggedInPicture(const User& user, const Picture& picture)
{
	std::string query = "SELECT COUNT(*) FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + " AND PICTURE_ID = " + std::to_string(picture.getId()) + " ;";
	int times = 0;
	this->runCommand(query, this->_db, countCallback, &times);
	return times != 0 ? true : false;
}

std::list<User> DatabaseAccess::getUsersTaggedInPicture(const Picture& picture)
{
	std::string query = "SELECT USERS.ID, USERS.NAME FROM USERS INNER JOIN TAGS ON USERS.ID = TAGS.USER_ID WHERE PICTURE_ID = " + std::to_string(picture.getId()) + " ;";
	this->runCommand(query, this->_db, loadIntoUsers);
	std::list<User> res;

	for (const auto& user : DatabaseAccess::users)
	{
		res.push_back(user);
	}
	if (!res.empty())
		res.erase(std::unique(res.begin(), res.end()));
	return res;
}


Picture DatabaseAccess::getPicture(const int& id)
{
	std::string query = "SELECT * FROM PICTURES WHERE ID = " + std::to_string(id) + " ;";
	this->runCommand(query, this->_db, loadIntoPictures);
	if (DatabaseAccess::pictures.empty())
	{
		throw std::invalid_argument("Picture not found with that id");
	}
	else
	{
		return *DatabaseAccess::pictures.begin();
	}
}

int DatabaseAccess::timesAlbumsOfUserGotTagged(const User& user)
{
	std::string query = "SELECT count(*) FROM TAGS  INNER JOIN PICTURES  ON PICTURES.ID = TAGS.PICTURE_ID INNER JOIN ALBUMS ON ALBUMS.ID = PICTURES.ALBUM_ID INNER JOIN USERS ON USERS.ID = ALBUMS.USER_ID WHERE USERS.ID = " + std::to_string(user.getId()) + " ;";
	int times = 0;
	this->runCommand(query, this->_db, countCallback, &times);
	return times;
}


const std::list<Album> DatabaseAccess::getAlbums()
{
	std::string sqlCommand = "SELECT * FROM ALBUMS;";
	this->runCommand(sqlCommand, this->_db, loadIntoAlbums);
	return DatabaseAccess::albums;
}

const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{
	std::string command = "SELECT * FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + " ;";
	this->runCommand(command, this->_db, loadIntoAlbums);
	return DatabaseAccess::albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
	std::string command = "INSERT INTO ALBUMS (name, CREATION_DATE, USER_ID) VALUES ( \" " + album.getName() + " \", \" " + album.getCreationDate() + " \" , " + std::to_string(album.getOwnerId()) + " );";
	this->runCommand(command, this->_db);
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId)
{
	int albumId = this->openAlbum(albumName).getId();
	std::string command = "DELETE FROM PICTURES WHERE ALBUM_ID = " + std::to_string(albumId) + ";";
	this->runCommand(command, this->_db);


	command = "DELETE FROM ALBUMS WHERE name = \" " + albumName + " \" AND USER_ID = " + std::to_string(userId)  + ";";
	this->runCommand(command, this->_db);
}


bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId)
{
	std::string command = "SELECT * FROM ALBUMS WHERE NAME =  \" " + albumName + " \" AND USER_ID = " + std::to_string(userId) + " ;";
	this->runCommand(command, this->_db, loadIntoAlbums);
	return DatabaseAccess::albums.size() != 0 ? true : false;
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{
	std::string albumNameWithNoSpaces = this->removeWhiteSpacesBeforeAndAfter(albumName);
	std::string command = "SELECT * FROM ALBUMS WHERE NAME = \" " + albumNameWithNoSpaces + " \" ;";
	this->runCommand(command, this->_db, loadIntoAlbums);

	if (DatabaseAccess::albums.size() != 0)
	{
		auto begin = DatabaseAccess::albums.begin();
		return *begin;
	}
	else
	{
		throw std::invalid_argument("No album with name " + albumName + " exists");
	}
}

void DatabaseAccess::closeAlbum(Album& pAlbum) {}

void DatabaseAccess::printAlbums()
{
	this->getAlbums();
	if (DatabaseAccess::albums.empty()) {
		throw std::invalid_argument("There are no existing albums.");
	}
	std::cout << "Album list:" << std::endl;
	std::cout << "-----------" << std::endl;
	for (const Album& album : DatabaseAccess::albums) {
		std::cout << std::setw(5) << "* " << album;
	}
}

void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
	int id = this->openAlbum(albumName).getId();
	std::string command = "INSERT INTO PICTURES (name, LOCATION, CREATION_DATE, ALBUM_ID) VALUES ( \" " + picture.getName() + " \" , \" " + picture.getPath() + " \" , \" " + picture.getCreationDate() + " \" , " + std::to_string(id) + " );";
	this->runCommand(command, this->_db);
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
	int albumId = this->openAlbum(albumName).getId();
	std::string command = "DELETE FROM PICTURES WHERE NAME = \" " + pictureName + " \" AND ALBUM_ID = " + std::to_string(albumId) + " ;";
	this->runCommand(command, this->_db);
}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	int pictureId = this->getPictureFromAlbum(albumName, pictureName).getId();
	std::string command = "DELETE FROM TAGS WHERE PICTURE_ID = " + std::to_string(pictureId) + " AND USER_ID = " + std::to_string(userId) + " ;";
	this->runCommand(command, this->_db);
}

void DatabaseAccess::printUsers()
{
	std::string command = "SELECT * FROM USERS;";
	this->runCommand(command, this->_db, loadIntoUsers);

	std::cout << "Users list:" << std::endl;
	std::cout << "-----------" << std::endl;
	for (const auto& user : DatabaseAccess::users) {
		std::cout << user << std::endl;
	}
}

void DatabaseAccess::createUser(User& user)
{
	std::string command = "INSERT INTO USERS (NAME, ID) VALUES (\" " + user.getName() + "\", " + std::to_string(user.getId()) + "); ";
	this->runCommand(command, this->_db);
}

void DatabaseAccess::deleteUser(const User& user)
{
	std::string command = "DELETE FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + " ;";
	this->runCommand(command, this->_db);
	command = "DELETE FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + " ;";
	this->runCommand(command, this->_db);
	command = "DELETE FROM USERS WHERE ID = " + std::to_string(user.getId()) + " ;";
	this->runCommand(command, this->_db);
}

bool DatabaseAccess::doesUserExists(int userId)
{
	std::string command = "SELECT * FROM USERS WHERE ID = " + std::to_string(userId) + " ;";
	this->runCommand(command, this->_db, loadIntoUsers);

	return DatabaseAccess::users.empty() ? false : true;
}

User DatabaseAccess::getUser(int userId)
{
	std::string command = "SELECT * FROM USERS WHERE ID = " + std::to_string(userId) + " ;";
	this->runCommand(command, this->_db, loadIntoUsers);

	if (DatabaseAccess::users.empty())
	{
		throw std::invalid_argument("User does not exist ");
	}
	else
	{
		return DatabaseAccess::users[0];
	}
}

int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
	std::string command = "SELECT COUNT (*) FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + " ;";
	int count = 0;
	this->runCommand(command, this->_db, countCallback, &count);
	return count;
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
	std::string command = "SELECT COUNT(DISTINCT ALBUMS.ID) FROM TAGS INNER JOIN PICTURES ON TAGS.PICTURE_ID = PICTURES.ID INNER JOIN ALBUMS ON PICTURES.ALBUM_ID = ALBUMS.ID WHERE TAGS.USER_ID = " + std::to_string(user.getId()) + " ;";
	int count = 0;
	this->runCommand(command, this->_db, countCallback, &count);
	return count;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
	std::string command = "SELECT COUNT (*) FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + " ;";
	int count = 0;
	this->runCommand(command, this->_db, countCallback, &count);
	return count;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
	if (this->countAlbumsOwnedOfUser(user) == 0)
	{
		return 0;
	}
	return (float)this->getTaggedPicturesOfUser(user).size() / this->countAlbumsOwnedOfUser(user);
}

User DatabaseAccess::getTopTaggedUser()
{
	std::string query = "SELECT USERS.ID, USERS.NAME FROM USERS INNER JOIN TAGS  ON USERS.ID = TAGS.USER_ID GROUP BY USERS.ID ORDER BY count(*) DESC LIMIT 1;";
	this->runCommand(query, this->_db, loadIntoUsers);
	if (DatabaseAccess::users.size() == 0)
	{
		throw std::invalid_argument("There are no users at all \n");
	}
	return *DatabaseAccess::users.begin();
}

Picture DatabaseAccess::getTopTaggedPicture()
{
	std::string query = "SELECT PICTURE_ID, COUNT(PICTURE_ID) AS Count FROM TAGS GROUP BY PICTURE_ID ORDER BY Count DESC LIMIT 1 ;";
	int pictureId;
	this->runCommand(query, this->_db, countCallback, &pictureId);
	
	return this->getPicture(pictureId);
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
	std::string query = "SELECT PICTURES.ALBUM_ID, PICTURES.CREATION_DATE, PICTURES.ID, PICTURES.LOCATION, PICTURES.NAME FROM PICTURES  INNER JOIN  TAGS ON PICTURES.ID = TAGS.PICTURE_ID INNER JOIN ALBUMS ON ALBUMS.ID = PICTURES.ALBUM_ID INNER JOIN USERS ON USERS.ID = ALBUMS.USER_ID WHERE USERS.ID = " + std::to_string(user.getId()) + " ;";
	this->runCommand(query, this->_db, loadIntoPictures);

	return DatabaseAccess::pictures;
}

void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
	int pictureId = this->getPictureFromAlbum(albumName, pictureName).getId();

	std::string command = "INSERT INTO TAGS (PICTURE_ID, USER_ID) VALUES ( " + std::to_string(pictureId) + ", " + std::to_string(userId) + " );";
	this->runCommand(command, this->_db);
}

int loadIntoAlbums(void* data, int argc, char** argv, char** azColName)
{
	// Create a new Album object for each row fetched
	Album albumObj (0, "");

	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == USER_ID) {
			albumObj.setOwner(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == CREATION_DATE) {
			albumObj.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == NAME) {
			albumObj.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == ID) {
			albumObj.setId(std::stoi(argv[i]));
		}
		// Add the newly created Album object to the passed-in vector
	}
	DatabaseAccess::albums.push_back(albumObj);
	return 0; // Return 0 to indicate success
}

int loadIntoPictures(void* data, int argc, char** argv, char** azColName)
{
	// Create a new Album object for each row fetched
	Picture picture(0, "");

	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == NAME) {
			picture.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == CREATION_DATE) {
			picture.setCreationDate(argv[i]);
		}
		else if (std::string(azColName[i]) == LOCATION) {
			picture.setLocation(argv[i]);
		}
		else if (std::string(azColName[i]) == ALBUM_ID) {
			picture.setAlbumId(std::stoi(argv[i]));
		}
		else if (std::string(azColName[i]) == ID) {
			picture.setId(std::stoi(argv[i]));
		}
		// Add the newly created Album object to the passed-in vector
	}
	DatabaseAccess::pictures.push_back(picture);


	return 0; // Return 0 to indicate success
}

int loadIntoUsers(void* data, int argc, char** argv, char** azColName)
{
	User user (0, "");

	for (int i = 0; i < argc; i++) {
		if (std::string(azColName[i]) == NAME) {
			user.setName(argv[i]);
		}
		else if (std::string(azColName[i]) == ID) {
			user.setId(std::stoi(argv[i]));
		}
	}
	DatabaseAccess::users.push_back(user);

	return 0;
}

int countCallback(void* data, int argc, char** argv, char** azColName)
{
	int* count = static_cast<int*>(data);
	if (argv[0]) {
		*count = std::stoi(argv[0]);
	}
	return 0;
}
