#include <string>
#include <vector>

//Returns the index of an element of type string within a vector of strings.
//Returns -1 if element is not present in vector.
int getVectorPosition(std::vector<std::string> vector, std::string element);

//Converts unsigned integer value into a hexidecimal string.
//Width specifies how many characters wider the string will be.
//Example unsignedIntToHexString(255, 8) --> "000000FF"
std::string unsignedIntToHexString(unsigned int value, unsigned int width);

//Converts unsigned integer value into a decimal string.
//Width specifies how many characters wider the string will be.
//Example unsignedIntToHexString(255, 8) --> "00000255"
std::string unsignedIntToDecString(unsigned int value, unsigned int width);

//Converts a hexidecimal string value into an unsigned integer.
unsigned int hexStringToUnsignedInt(std::string hex_string);

//Converts a decimal string value into an unsigned integer.
unsigned int decStringToUnsignedInt(std::string dec_string);

//Generates a string of blank spaces
std::string blankSpaces(unsigned int spaces);

//Gets windows username for the machine running the program
std::string getWindowsUsername();

//Reads a gun_actions.lua file and stores all the entity xml's to vector tables
int parseGunActionsFile(char* filename, 
	std::vector<std::string>& projectiles, 
	std::vector<std::string>& related_projectiles, 
	std::vector<std::string>& extra_entities, 
	std::vector<std::string>& related_extra_entities);

//Adds a string to a vector if the string wasn't in the vector already.
