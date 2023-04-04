#ifndef imageClass
#define imageClass

#include <string>
#include <vector>

class Image {
private:
	//Data Values
	std::vector<unsigned char> pixel_data_backup; //Contains the original rbga values of the image.


	

public:
	//Data Values
	std::vector<unsigned char> pixel_data; //Contains all the rbga values of the image.
	std::vector<unsigned int> rgba; //A single pixel of rgba data.
	std::vector<double> hsv; //A single pixel of hsv data
	std::vector<std::string> colors;//RGBA Hex Strings of all the colors that appear in the image.
	std::vector<unsigned int> colors_occurances; //The number of times these hex strings occur.
	std::string most_common_color; //The most common color in the image stored as a hexidecimal string.
	std::string rgba_hex; //A pixel of rgba data represented by a hexidecimal string.
	unsigned int width; //Width of the image.
	unsigned int height; //Height of the image.
	unsigned error; //Lodepng Error Codes

	//Functions
	//Constructors
	Image();
	Image(unsigned int image_width, unsigned int image_height);

	//Stores Image obj.pixel_data to private pixel_data_backup
	void storePixelDataBackup();

	//Resizes pixel data using Image obj.width and Image obj.height.
	void resizePixelData();

	//Converts Image obj.rgba into an 8 character wide hexidecimal string and stores it in obj.rgba_hex
	void rgbaVecToHexString();

	//Converts Image obj.rgba_hex 8 character wide hexidecimal string into an RGBA vector and stores it in obj.rgba
	void hexStringToRgbaVec();

	//Converts Image obj.rgba RGBA data to HSV data and stores it in obj.hsv
	void rgbaVecToHsvVec();

	//Converts Image obj.hsv HSV data to RGBA data and stores it in obj.rgba
	//Note this function will always return A at 255.
	void hsvVectoRgbaVec();

	//Reads the data from a file into Image obj.pixel_data using lodepng library
	//Also sets obj.width and obj.height
	void readImageIntoPixelData(std::string image_path);

	//Writes to file from Image obj.pixel_data using lodepng library
	void writePixelDataIntoImage(std::string image_path);

	//Find the most common color in Image obj.pixel_data and stores the result into obj.most_common_color.
	void findMostCommonColor();

	//Color Shifts the Image obj.pixel_data to the desired hex color string.
	void colorShiftToRGBAHex(std::string rgb_hex_input);

	//Gray shifts the obj.pixel_data
	void colorShiftToGrayScale(double saturation, double value);

	//Stores private obj.pixel_data_backup to public obj.pixel_data
	void restorePixelData();

	//Writes Image obj.rbga into obj.pixel_data at a specific x, y of the image
	void insertRGBAtoSpecificPixel(unsigned int pos_x, unsigned int pos_y);

	//Reads what color is in obj.pixel_data at position x,y and stores it to obj.rgba
	void readRGBAatSpeicificPixel(unsigned int pos_x, unsigned int pos_y);

	//Writes a given image into image.
	void insertImage(imageClass::Image image_to_write, unsigned int pos_x, unsigned int pos_y, bool ingore_transparency);
	void insertImage(imageClass::Image image_to_write, unsigned int pos_x, unsigned int pos_y, bool ingore_transparency, unsigned int alpha_transparency);

	//Returns an image with a subselection of pixels from image calling this function.
	imageClass::Image getSubselectionOfImage(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

	//Returns bool if RGBA in obj.rgba is the same as the rgba provided.
	bool compareRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a);

	//Crops image to size
	void cropImage(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);

	//Sets every element in obj.pixel_data to 0
	void zeroPixelData();
};
#endif