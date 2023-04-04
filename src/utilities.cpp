#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <iostream>

//Returns the index of an element of type string within a vector of strings.
//Returns -1 if element is not present in vector.
int getVectorPosition(std::vector<std::string> vector, std::string element) {
	unsigned int i;
	int position = -1;
	for (i = 0; i < vector.size(); i++) {
		if (vector[i] == element) {
			position = i;
			break;
		}
	}
	return position;
}

//Converts unsigned integer value into a hexidecimal string.
//Width specifies how many characters wider the string will be.
//Example unsignedIntToHexString(255, 8) --> "000000FF"
std::string unsignedIntToHexString(unsigned int value, unsigned int width) {
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(width) << std::hex << (value | 0);
	return ss.str();
}

//Converts unsigned integer value into a decimal string.
//Width specifies how many characters wider the string will be.
//Example unsignedIntToHexString(255, 8) --> "00000255"
std::string unsignedIntToDecString(unsigned int value, unsigned int width) {
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(width) << std::dec << (value | 0);
	return ss.str();
}

//Converts a hexidecimal string value into an unsigned integer.
unsigned int hexStringToUnsignedInt(std::string hex_string) {
	unsigned int i;
	std::istringstream iss(hex_string);
	iss >> std::hex >> i;

	return i;
}

//Converts a decimal string value into an unsigned integer.
unsigned int decStringToUnsignedInt(std::string dec_string) {
	unsigned int i;
	std::istringstream iss(dec_string);
	iss >> std::dec >> i;

	return i;
}

//Generates a string of blank spaces
std::string blankSpaces(unsigned int spaces) {
	std::string str = "";
	if (spaces > 0) {
		for (unsigned int i = 0; i < spaces; i++) {
			str = str + " ";
		}
	}
	return str;
}

//Gets windows username for the machine running the program
std::string getWindowsUsername() {
	char* username;
	size_t lengthOfUsername;
	errno_t err = _dupenv_s(&username, &lengthOfUsername, "USERNAME");
	return std::string(username);
}

//Reads a gun_actions.lua file and stores all the entity xml's to vector tables
int parseGunActionsFile(char* filename, 
	std::vector<std::string>& projectiles, 
	std::vector<std::string>& related_projectiles, 
	std::vector<std::string>& extra_entities, 
	std::vector<std::string>& related_extra_entities)
{
	//Error Bit
	int error = 0;

	//Open File
	std::ifstream gun_actions_file;
	gun_actions_file.open(filename);

	//Read stuff from file.
	if (gun_actions_file.is_open()) {
		//Variables for String Parsing
		std::size_t index = 0;
		bool is_block_comment = false;
		bool go_to_next_line = false;
		std::string gun_actions_line;

		//Parse File
		while (gun_actions_file) {
			getline(gun_actions_file, gun_actions_line); //read next line
			go_to_next_line = false; //reset value

			//clears all spaces and tabs from the line
			gun_actions_line.erase(remove(gun_actions_line.begin(), gun_actions_line.end(), ' '), gun_actions_line.end());
			gun_actions_line.erase(remove(gun_actions_line.begin(), gun_actions_line.end(), '\t'), gun_actions_line.end());

			while (!go_to_next_line) {
				/*	We're parsing Lua code here
				Line Comments are "--"
				Block Comments are "--[[ ... ]]--"
				Need to find these and ignore them. */

				//We will continue as normal unless we find a comment
				//If we find a comment, we will repeat operations on current line
				go_to_next_line = true;

				if (is_block_comment) {
					//Ignore everything until we find a closing block comment bracket.

					//Look for an opening block comment bracket.
					index = gun_actions_line.find("]]", 0);
					if (index != std::string::npos) {
						//Block comment ending found
						is_block_comment = false;
						go_to_next_line = false;
					}
				}
				else
				{
					//Look for an opening block comment bracket.
					index = gun_actions_line.find("--[[", 0);
					if (index != std::string::npos) {
						//Block comment starter found
						is_block_comment = true;
						go_to_next_line = false;
					}

					//Look for a line comment and remove them from line
					index = gun_actions_line.find("--", 0);
					if (index != std::string::npos) {
						gun_actions_line = gun_actions_line.substr(0, index);
						go_to_next_line = false;
					}


					//If we are not repeating, examine line for inportant information
					if (go_to_next_line) {
						/* What do we all need to find here?
							- add_projectile (This should get triggers and timers and the like
							- related_projectiles
							- c.extra_entities
							- related_extra_entities
						*/

						//Find add_projectile
						index = gun_actions_line.find("add_projectile", 0);
						if (index != std::string::npos) {
							index = gun_actions_line.find("\"", 0); //find first " mark
							gun_actions_line = gun_actions_line.substr(index + 1, gun_actions_line.length() - index - 1); //delete everything that comes before first " mark
							index = gun_actions_line.find("\"", 0); //find 2nd " mark
							gun_actions_line = gun_actions_line.substr(0, index); //delete everything that comes after 2nd " mark

							//Add "gun_actions_line" to "std::vector<std::string> projectiles;"
							int table_pos = getVectorPosition(projectiles, gun_actions_line); //returns -1 if element does not exist in vector
							index = gun_actions_line.find(".xml", 0); //Test if file is a .xml
							if (table_pos == -1 && index != std::string::npos) {
								projectiles.push_back(gun_actions_line); //Projectile is not in list, add it to list
							}
						}

						//Find related_projectiles
						index = gun_actions_line.find("related_projectiles={", 0);
						if (index != std::string::npos) {
							index = gun_actions_line.find("\"", 0); //find first " mark
							gun_actions_line = gun_actions_line.substr(index + 1, gun_actions_line.length() - index - 1); //delete everything that comes before first " mark
							index = gun_actions_line.find("\"", 0); //find 2nd " mark
							gun_actions_line = gun_actions_line.substr(0, index); //delete everything that comes after 2nd " mark

							//Add "gun_actions_line" to "std::vector<std::string> related_projectiles;"
							int table_pos = getVectorPosition(related_projectiles, gun_actions_line); //returns -1 if element does not exist in vector
							index = gun_actions_line.find(".xml", 0); //Test if file is a .xml
							if (table_pos == -1 && index != std::string::npos) {
								related_projectiles.push_back(gun_actions_line); //Related Projectile is not in list, add it to list
							}
						}

						//Find c.extra_entities
						index = gun_actions_line.find("c.extra_entities..", 0);
						if (index != std::string::npos) {
							index = gun_actions_line.find("\"", 0); //find first " mark
							gun_actions_line = gun_actions_line.substr(index + 1, gun_actions_line.length() - index - 1);  //delete everything that comes before first " mark
							index = gun_actions_line.find("\"", 0); //find 2nd " mark
							gun_actions_line = gun_actions_line.substr(0, index); //delete everything that comes after 2nd " mark

							//extra entities here can have multiple entites in one line
							// Ex.
							// c.extra_entities = c.extra_entities .. "data/entities/misc/nolla.xml,"
							// c.extra_entities = c.extra_entities .. "data/entities/misc/homing.xml,data/entities/particles/tinyspark_white.xml,"

							std::vector<std::string> extra_ents_to_add;
							std::vector<std::size_t> indecies(1, -1);
							std::size_t ind = 1;

							while (gun_actions_line.find(",", ind) != std::string::npos) {
								ind = gun_actions_line.find(",", ind + 1);
								if (ind != std::string::npos) {
									indecies.push_back(ind);
								}
							}

							for (unsigned int i = 0; i < indecies.size() - 1; i++) {
								extra_ents_to_add.push_back(gun_actions_line.substr(indecies[i] + 1, indecies[i + 1] - indecies[i] - 1));
							}

							//Add "extra_ents_to_add" to "std::vector<std::string> extra_entities"
							for (unsigned int i = 0; i < extra_ents_to_add.size(); i++) {
								int table_pos = getVectorPosition(extra_entities, extra_ents_to_add[i]); //returns -1 if element does not exist in vector
								index = extra_ents_to_add[i].find(".xml", 0); //Test if file is a .xml
								if (table_pos == -1 && index != std::string::npos) {
									extra_entities.push_back(extra_ents_to_add[i]); //Related Projectile is not in list, add it to list
								}
							}
						}

						//Find related_extra_entities
						index = gun_actions_line.find("related_extra_entities={", 0);
						if (index != std::string::npos) {
							//Find stuff between curly brackets
							index = gun_actions_line.find("{", 0); //find opening curly bracket
							gun_actions_line = gun_actions_line.substr(index + 1, gun_actions_line.length() - index - 1);  //delete everything that comes before first curly bracket
							index = gun_actions_line.find("}", 0); //find closing curly bracket
							gun_actions_line = gun_actions_line.substr(0, index); //delete everything that comes after 2nd curly bracket

							// related extra entities can have multiple entities split apart like shown below
							// Ex.
							// related_extra_entities = { "data/entities/misc/autoaim.xml" },
							// related_extra_entities = { "data/entities/misc/homing_short.xml", "data/entities/particles/tinyspark_white_weak.xml" },

							//Declare Vars Needed
							std::vector<std::string> related_ents_to_add;
							std::vector<std::size_t> indecies(1, 0);

							//Find all " marks in line
							index = 0;
							while (gun_actions_line.find("\"", index) != std::string::npos) {
								index = gun_actions_line.find("\"", index + 1);
								if (index != std::string::npos) {
									indecies.push_back(index);
								}
							}

							//Store the stuff between " marks
							for (unsigned int i = 0; i < indecies.size() / 2; i++) {
								related_ents_to_add.push_back(gun_actions_line.substr(indecies[2 * i] + 1, indecies[2 * i + 1] - indecies[2 * i] - 1));
							}

							//Add "related_ents_to_add" to "std::vector<std::string> related_extra_entities"
							for (unsigned int i = 0; i < related_ents_to_add.size(); i++) {
								int table_pos = getVectorPosition(extra_entities, related_ents_to_add[i]); //returns -1 if element does not exist in vector
								index = related_ents_to_add[i].find(".xml", 0); //Test if file is a .xml
								if (table_pos == -1 && index != std::string::npos) {
									related_extra_entities.push_back(related_ents_to_add[i]); //Related Projectile is not in list, add it to list
								}
							}
						}
					}//end if (go_to_next_line)
				}//end while (!go_to_next_line)
			}//end while (!go_to_next_line)
		}//end while (gun_actions_file)
	}//end if (gun_actions_file.is_open())
	else {
		//Could not open file. Return error code 1.
		error = 1;
	}

	//Close file
	gun_actions_file.close();

	return error;
}