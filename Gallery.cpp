#include <iostream>
#include <string>
#include "MemoryAccess.h"
#include "AlbumManager.h"
#include <chrono>
#include <ctime>


int getCommandNumberFromUser()
{
	std::string message("\nPlease enter any command(use number): ");
	std::string numericStr("0123456789");
	
	std::cout << message << std::endl;
	std::string input;
	std::getline(std::cin, input);
	
	while (std::cin.fail() || std::cin.eof() || input.find_first_not_of(numericStr) != std::string::npos) {

		std::cout << "Please enter a number only!" << std::endl;

		if (input.find_first_not_of(numericStr) == std::string::npos) {
			std::cin.clear();
		}

		std::cout << std::endl << message << std::endl;
		std::getline(std::cin, input);
	}
	
	return std::atoi(input.c_str());
}

int main(void)
{
	// initialization data access
	MemoryAccess dataAccess;

	// initialize album manager
	AlbumManager albumManager(dataAccess);


	std::string albumName;
	// Get current time as time_point
	auto current_time_point = std::chrono::system_clock::now();

	// Convert time_point to time_t
	std::time_t current_time = std::chrono::system_clock::to_time_t(current_time_point);

	// Convert time_t to tm struct for easy access to year, month, day, etc.
	struct tm* ptm = std::localtime(&current_time);

	std::cout << "Craeted by Yoel Weisberg - The time is: ";
	std::cout << 1900 + ptm->tm_year << "-"
		<< 1 + ptm->tm_mon << "-"
		<< ptm->tm_mday << " "
		<< ptm->tm_hour << ":"
		<< ptm->tm_min << ":"
		<< ptm->tm_sec << std::endl;
	std::cout << "Welcome to Gallery!" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
	
	do {
		int commandNumber = getCommandNumberFromUser();
		
		try	{
			albumManager.executeCommand(static_cast<CommandType>(commandNumber));
		} catch (std::exception& e) {	
			std::cout << e.what() << std::endl;
		}
	} 
	while (true);
}


