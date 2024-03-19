#pragma once
#include "IDataAccess.h"
#include "DatabaseAcses.h"
#include "sqlite3.h"
#include <list>
#include <vector>
#include <io.h>

int loadIntoAlbums(void* data, int argc, char** argv, char** azColName);
int loadIntoPictures(void* data, int argc, char** argv, char** azColName);
int loadIntoUsers(void* data, int argc, char** argv, char** azColName);

class DatabaseAccess : public IDataAccess
{
public:
	static std::list<Album> albums;	
	static std::list<Picture> pictures;
	static std::vector<User> users;

	DatabaseAccess() = default;
	virtual ~DatabaseAccess() = default;

	// album related
	const std::list<Album> getAlbums() override;
	const std::list<Album> getAlbumsOfUser(const User& user) override;
	void createAlbum(const Album& album) override;
	void deleteAlbum(const std::string& albumName, int userId) override;
	bool doesAlbumExists(const std::string& albumName, int userId) override;
	Album openAlbum(const std::string& albumName) override;
	void closeAlbum(Album& pAlbum) override;
	void printAlbums() override;

	// picture related
	void addPictureToAlbumByName(const std::string& albumName, const Picture& picture) override;
	void removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName) override;
	void tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;
	void untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId) override;

	// user related
	void printUsers() override;
	void createUser(User& user) override;
	void deleteUser(const User& user) override;
	bool doesUserExists(int userId) override;
	User getUser(int userId) override;

	// user statistics
	int countAlbumsOwnedOfUser(const User& user) override;
	int countAlbumsTaggedOfUser(const User& user) override;
	int countTagsOfUser(const User& user) override;
	float averageTagsPerAlbumOfUser(const User& user) override;

	// queries
	User getTopTaggedUser() override;
	Picture getTopTaggedPicture() override;
	std::list<Picture> getTaggedPicturesOfUser(const User& user) override;

	bool open() override;
	void close() override {};
	void clear() override;

private:
	bool runCommand(const std::string& sqlStatement, sqlite3* db, int (*callback)(void*, int, char**, char**) = nullptr, void* secondParam = nullptr);
	Picture getPictureFromAlbum(const std::string& albumName, const std::string& picture);
	sqlite3* _db;
};
