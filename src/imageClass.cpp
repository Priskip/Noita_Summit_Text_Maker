#include <string>
#include <vector>
#include <stdio.h>
#include <cmath>
#include <iostream>
#include "imageClass.h"
#include "utilities.h"
#include "lodepng.h"

//CONSTRUCTORS
Image::Image() {
	rgba.resize(4);
	hsv.resize(3);
	colors.resize(1);
	colors_occurances.resize(1);
	most_common_color = "FFFFFFFF";
	width = 0;
	height = 0;
	error = 0;
}

Image::Image(unsigned int image_width, unsigned int image_height) {
	rgba.resize(4);
	hsv.resize(3);
	colors.resize(1);
	colors_occurances.resize(1);
	most_common_color = "FFFFFFFF";
	width = image_width;
	height = image_height;
	error = 0;
	pixel_data.resize(4 * width * height);
}

//Stores Image obj.pixel_data to private pixel_data_backup
void Image::storePixelDataBackup() {
	pixel_data_backup.clear();
	pixel_data_backup = pixel_data;
}

//Stores private obj.pixel_data_backup to public obj.pixel_data
void Image::restorePixelData() {
	pixel_data = pixel_data_backup;
}

//Resizes pixel data using Image obj.width and Image obj.height.
void Image::resizePixelData() {
	pixel_data.resize(4 * width * height);
}

//Converts Image obj.rgba into an 8 character wide hexidecimal string and stores it in obj.rgba_hex
void Image::rgbaVecToHexString() {
	rgba_hex = "xxxxxxxx";
	for (int i = 0; i < 4; i++) {
		rgba_hex[2 * i] = unsignedIntToHexString(rgba[i], 2)[0];
		rgba_hex[2 * i + 1] = unsignedIntToHexString(rgba[i], 2)[1];
	}
}

//Converts Image obj.rgba_hex 8 character wide hexidecimal string into an RGBA vector and stores it in obj.rgba
void Image::hexStringToRgbaVec() {
	for (int i = 0; i < 4; i++) {
		rgba[i] = hexStringToUnsignedInt(rgba_hex.substr(2 * i, 2));
	}
}

//Converts Image obj.rgba RGBA data to HSV data and stores it in obj.hsv
void Image::rgbaVecToHsvVec() {

	//stored vector for rgba divided by 255
	std::vector<double> rgb_prime(3, 0);
	for (int i = 0; i < 3; i++) {
		rgb_prime[i] = (double)rgba[i] / 255;
	}

	double c_max = fmax(fmax(rgb_prime[0], rgb_prime[1]), rgb_prime[2]);
	double c_min = fmin(fmin(rgb_prime[0], rgb_prime[1]), rgb_prime[2]);
	double delta = c_max - c_min;

	//Calc H
	if (delta == 0) {
		hsv[0] = 0;
	}
	else {
		if (c_max == rgb_prime[0]) {
			hsv[0] = 60 * ((rgb_prime[1] - rgb_prime[2]) / delta);
		}

		if (c_max == rgb_prime[1]) {
			hsv[0] = 60 * ((rgb_prime[2] - rgb_prime[0]) / delta + 2);
		}

		if (c_max == rgb_prime[2]) {
			hsv[0] = 60 * ((rgb_prime[0] - rgb_prime[1]) / delta + 4);
		}
	}

	//Make sure to return H in range of [0,360)
	while (hsv[0] < 0 || hsv[0] >= 360) {
		if (hsv[0] < 0) {
			hsv[0] = hsv[0] + 360;
		}

		if (hsv[0] >= 360) {
			hsv[0] = hsv[0] - 360;
		}
	}

	//Calc S
	if (c_max == 0) {
		hsv[1] = 0;
	}
	else {
		hsv[1] = delta / c_max;
	}

	//Calc V
	hsv[2] = c_max;
}

//Converts Image obj.hsv HSV data to RGBA data and stores it in obj.rgba
//Note this function will always return A at 255.
void Image::hsvVectoRgbaVec() {

	double c = hsv[1] * hsv[2]; //c = SV
	double m = hsv[2] - c; //m = V-c
	double x = c * (1 - fabs(fmod((hsv[0] / 60), 2) - 1));
	std::vector<double> rgb_prime(3, 0);

	if (hsv[0] >= 0 && hsv[0] < 60) {
		rgb_prime[0] = c;
		rgb_prime[1] = x;
		rgb_prime[2] = 0;
	}
	if (hsv[0] >= 60 && hsv[0] < 120) {
		rgb_prime[0] = x;
		rgb_prime[1] = c;
		rgb_prime[2] = 0;
	}
	if (hsv[0] >= 120 && hsv[0] < 180) {
		rgb_prime[0] = 0;
		rgb_prime[1] = c;
		rgb_prime[2] = x;
	}
	if (hsv[0] >= 180 && hsv[0] < 240) {
		rgb_prime[0] = 0;
		rgb_prime[1] = x;
		rgb_prime[2] = c;
	}
	if (hsv[0] >= 240 && hsv[0] < 300) {
		rgb_prime[0] = x;
		rgb_prime[1] = 0;
		rgb_prime[2] = c;
	}
	if (hsv[0] >= 300 && hsv[0] < 360) {
		rgb_prime[0] = c;
		rgb_prime[1] = 0;
		rgb_prime[2] = x;
	}

	for (int i = 0; i < 3; i++) {
		rgba[i] = (int)(255 * (rgb_prime[i] + m));
	}
	rgba[3] = 255;
}

//Reads the data from a file into pixel_data using lodepng library
void Image::readImageIntoPixelData(std::string image_path) {
	const char* filename = image_path.c_str();
	error = lodepng::decode(pixel_data, width, height, filename);
	storePixelDataBackup();

	if (error) {
		std::cout << "Image::readImageIntoPixelData - lodepng decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}
}

//Writes to file from Image obj.pixel_data using lodepng library
void Image::writePixelDataIntoImage(std::string image_path) {
	const char* filename = image_path.c_str();
	error = lodepng::encode(filename, pixel_data, width, height);

	if (error) {
		std::cout << "Image::writePixelDataIntoImage - lodepng encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
	}
}

//Find the most common color in Image obj.pixel_data and stores the result into obj.most_common_color.
void Image::findMostCommonColor() {
	int index = -1;

	for (int i = 0; i < pixel_data.size() / 4; i++) {
		//read current pixel
		for (int j = 0; j < 4; j++) {
			rgba[j] = pixel_data[4 * i + j];

		}

		//transform rgba vec to hex string
		rgbaVecToHexString();

		//get index of color in list
		index = getVectorPosition(colors, rgba_hex);

		if (index != -1) {
			//color was found in the list - increment occurance count
			colors_occurances[index]++;
		}
		else
		{
			//color was not found in the list - add to list
			colors.push_back(rgba_hex);
			colors_occurances.push_back(1);
		}
	}

	//organize lists
	bool sorted = false;
	while (!sorted) {
		sorted = true;
		int list_size = (int)colors_occurances.size();
		for (int i = 0; i < list_size - 1; i++) {
			if (colors_occurances[i + 1] > colors_occurances[i]) {
				std::swap(colors_occurances[i], colors_occurances[i + 1]);
				std::swap(colors[i], colors[i + 1]);
			}
		}
	}

	//set most common color
	most_common_color = colors[0];

	if (most_common_color.compare("00000000") == 0) {
		//If most common color is full transparency, get the next color in the list
		most_common_color = colors[1];
	}
}

//Color Shifts the Image obj.pixel_data to the desired hex color string.
void Image::colorShiftToRGBAHex(std::string rgb_hex_input) {

	//Transform rgb_hex_input string into target hsv vector.
	rgba_hex = rgb_hex_input;
	hexStringToRgbaVec();
	rgbaVecToHsvVec();
	std::vector<double> target_hsv = hsv;

	//Get Most Common Color as hsv vector.
	rgba_hex = most_common_color;
	hexStringToRgbaVec();
	rgbaVecToHsvVec();
	std::vector<double> most_common_color_hsv = hsv;

	for (int i = 0; i < pixel_data.size() / 4; i++) {
		//read current pixel
		for (int j = 0; j < 4; j++) {
			rgba[j] = pixel_data[4 * i + j];
		}

		//If not transparent pixel
		if (!(rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0 && rgba[3] == 0)) {
			//transform to rgba to hsv
			rgbaVecToHsvVec();

			//Color Transform
			hsv[0] = target_hsv[0] - (hsv[0] - most_common_color_hsv[0]);

			//Make sure to return H in range of [0,360)
			while (hsv[0] < 0 || hsv[0] >= 360) {
				if (hsv[0] < 0) {
					hsv[0] = hsv[0] + 360;
				}

				if (hsv[0] >= 360) {
					hsv[0] = hsv[0] - 360;
				}
			}

			//Convert back to RGBA and write to pixel_data
			hsvVectoRgbaVec();
			for (int j = 0; j < 4; j++) {
				pixel_data[4 * i + j] = rgba[j];

			}
		}
	}
}

//Gray shifts the obj.pixel_data
void Image::colorShiftToGrayScale(double saturation, double value) {
	for (int i = 0; i < pixel_data.size() / 4; i++) {
		//read current pixel
		for (int j = 0; j < 4; j++) {
			rgba[j] = pixel_data[4 * i + j];
		}

		//If not transparent pixel
		if (!(rgba[0] == 0 && rgba[1] == 0 && rgba[2] == 0 && rgba[3] == 0)) {
			////Calculate Gray Scale
			//int grayscale = round(0.3 * rgba[0] + 0.59 * rgba[1] + 0.11 * rgba[2]);

			////Set RGB to gray
			//rgba[0] = grayscale;
			//rgba[1] = grayscale;
			//rgba[2] = grayscale;

			//Convert to HSV
			rgbaVecToHsvVec();

			//Shift S and V values
			hsv[1] = saturation * hsv[1];
			hsv[2] = value * hsv[2];

			//clamp values
			hsv[1] = fmin(hsv[1], 1);
			hsv[1] = fmax(hsv[1], 0);
			hsv[2] = fmin(hsv[2], 1);
			hsv[2] = fmax(hsv[2], 0);

			//Convert back to rgba
			hsvVectoRgbaVec();

			//Set in pixel data
			for (int j = 0; j < 4; j++) {
				pixel_data[4 * i + j] = rgba[j];
			}
		}
	}
}

//Writes Image obj.rbga into obj.pixel_data at a specific x, y of the image
void Image::insertRGBAtoSpecificPixel(unsigned int pos_x, unsigned int pos_y) {
	for (unsigned int i = 0; i < 4; i++) {
		pixel_data[4 * width * pos_y + 4 * pos_x + i] = rgba[i];
	}
}

//Reads what color is in obj.pixel_data at position x,y and stores it to obj.rgba
void  Image::readRGBAatSpeicificPixel(unsigned int pos_x, unsigned int pos_y) {
	for (unsigned int i = 0; i < 4; i++) {
		rgba[i] = pixel_data[4 * width * pos_y + 4 * pos_x + i];
	}
}

//Writes a given image into image.
void Image::insertImage(imageClass::Image image_to_write, unsigned int pos_x, unsigned int pos_y, bool ingore_transparency) {
	bool will_fit = true; //bool to check if image_to_write will fit inside this image.

	if (pos_x + image_to_write.width > width) {
		will_fit = false;
	}

	if (pos_y + image_to_write.height > height) {
		will_fit = false;
	}

	if (will_fit) {
		unsigned int x = 0;
		unsigned int y = 0;

		for (unsigned int i = 0; i < image_to_write.pixel_data.size() / 4; i++) {
			//read rgba from image_to_write
			rgba[0] = image_to_write.pixel_data[4 * i];
			rgba[1] = image_to_write.pixel_data[4 * i + 1];
			rgba[2] = image_to_write.pixel_data[4 * i + 2];
			rgba[3] = image_to_write.pixel_data[4 * i + 3];

			if (!(ingore_transparency && rgba[3] == 0)) {
				//write to image
				insertRGBAtoSpecificPixel(x + pos_x, y + pos_y);
			}

			//counting
			x++;
			if (x == image_to_write.width) {
				x = 0;
				y++;
			}
		}
	}
	else
	{
		std::cout << "Error (Image::insertImage): image_to_write does not fit inside image." << std::endl;
	}
}

//Writes a given image into image with writing image having a transparency to it.
void Image::insertImage(imageClass::Image image_to_write, unsigned int pos_x, unsigned int pos_y, bool ingore_transparency, unsigned int alpha_transparency) {
	bool will_fit = true; //bool to check if image_to_write will fit inside this image.

	if (pos_x + image_to_write.width > width) {
		will_fit = false;
	}

	if (pos_y + image_to_write.height > height) {
		will_fit = false;
	}

	if (will_fit) {
		unsigned int x = 0;
		unsigned int y = 0;


		bool test = false;
		//std::cout << "--i --x --y --r --g --b --a --test" << std::endl;

		for (unsigned int i = 0; i < image_to_write.pixel_data.size() / 4; i++) {
			rgba[3] = pixel_data[4 * i + 3];
			test = false;

			//write to image
			if (!(ingore_transparency && rgba[3] == 0)) {
				//Calculate Color
				for (unsigned int j = 0; j < 3; j++) {
					double R = pixel_data[4 * i + j];
					double Rs = (double)image_to_write.pixel_data[4 * i + j];
					rgba[j] = (unsigned int)((Rs * alpha_transparency + R * (255 - alpha_transparency)) / 255); //(Rs*As+R*(255-As))/255
				}

				//Insert into image
				insertRGBAtoSpecificPixel(x + pos_x, y + pos_y);

				test = true;
			}

			/*std::cout << unsignedIntToDecString(i, 3) << " " << unsignedIntToDecString(x, 3) << " " << unsignedIntToDecString(y, 3)
				<< " " << unsignedIntToDecString(rgba[0], 3) << " " << unsignedIntToDecString(rgba[1], 3)
				<< " " << unsignedIntToDecString(rgba[2],3) << " " << unsignedIntToDecString(rgba[3],3)
				<< " " << test << std::endl;*/

				//counting
			x++;
			if (x == image_to_write.width) {
				x = 0;
				y++;
			}
		}
	}
	else
	{
		std::cout << "Error (Image::insertImage): image_to_write does not fit inside image." << std::endl;
	}
}

//Returns an image with a subselection of pixels from image calling this function.
imageClass::Image Image::getSubselectionOfImage(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
	/*    Error Checking
	Case 1: x2 < x1
	Case 2: y2 < y1
	Case 3: x1 out of bounds
	Case 4: y1 out of bounds
	Case 5: x2 out of bounds
	Case 6: y2 out of bounds
	*/
	bool is_error = false;
	std::vector<bool> errors(6, false);
	Image subImage;

	//Check error conditions
	if (x2 < x1) {
		is_error = true;
		errors[0] = true;
	}

	if (y2 < y1) {
		is_error = true;
		errors[1] = true;
	}

	if (x1 > width) {
		is_error = true;
		errors[2] = true;
	}

	if (y1 > height) {
		is_error = true;
		errors[3] = true;
	} //Secret comment because funny number and comment by OneTrueCube in twitch chat (twitch.tv/priskip)

	if (x2 > width) {
		is_error = true;
		errors[4] = true;
	}

	if (y2 > height) {
		is_error = true;
		errors[5] = true;
	}

	if (is_error) {
		//Display Errors
		std::string error_header = "Error in Image::getSubselectionOfImage - ";
		std::vector<std::string> error_messages(6, "");
		error_messages[0] = "x2 is less than x1";
		error_messages[1] = "y2 is less than y1";
		error_messages[2] = "x1 is not in range of image's size";
		error_messages[3] = "y1 is not in range of image's size";
		error_messages[4] = "x2 is not in range of image's size";
		error_messages[5] = "y2 is not in range of image's size";

		for (unsigned int i = 0; i < error_messages.size(); i++) {
			if (errors[i]) {
				std::cout << error_header << error_messages[i] << std::endl;
			}
		}

	}
	else {
		//No Errors - Good to go!
		subImage.width = x2 - x1;
		subImage.height = y2 - y1;
		subImage.resizePixelData();

		for (unsigned int x = 0; x < subImage.width; x++) {
			for (unsigned int y = 0; y < subImage.height; y++) {
				readRGBAatSpeicificPixel(x + x1, y + y1);
				subImage.rgba = rgba;
				subImage.insertRGBAtoSpecificPixel(x, y);
			}
		}
	}

	return subImage;
}

//Returns bool if RGBA in obj.rgba is the same as the rgba provided.
bool Image::compareRGBA(unsigned int r, unsigned int g, unsigned int b, unsigned int a) {
	bool same = false;

	if (rgba[0] == r && rgba[1] == g && rgba[2] == b && rgba[3] == a) {
		same = true;
	}

	return same;
}


//Crops image to size
void Image::cropImage(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
	/*    Error Checking
	Case 1: x2 < x1
	Case 2: y2 < y1
	Case 3: x1 out of bounds
	Case 4: y1 out of bounds
	Case 5: x2 out of bounds
	Case 6: y2 out of bounds
	*/
	bool is_error = false;
	std::vector<bool> errors(6, false);

	//Check error conditions
	if (x2 < x1) {
		is_error = true;
		errors[0] = true;
	}

	if (y2 < y1) {
		is_error = true;
		errors[1] = true;
	}

	if (x1 > width) {
		is_error = true;
		errors[2] = true;
	}

	if (y1 > height) {
		is_error = true;
		errors[3] = true;
	} //Secret comment because funny number and comment by OneTrueCube in twitch chat (twitch.tv/priskip)

	if (x2 > width) {
		is_error = true;
		errors[4] = true;
	}

	if (y2 > height) {
		is_error = true;
		errors[5] = true;
	}

	if (is_error) {
		//Display Errors
		std::string error_header = "Error in Image::cropImage - ";
		std::vector<std::string> error_messages(6, "");
		error_messages[0] = "x2 is less than x1";
		error_messages[1] = "y2 is less than y1";
		error_messages[2] = "x1 is not in range of image's size";
		error_messages[3] = "y1 is not in range of image's size";
		error_messages[4] = "x2 is not in range of image's size";
		error_messages[5] = "y2 is not in range of image's size";

		for (unsigned int i = 0; i < error_messages.size(); i++) {
			if (errors[i]) {
				std::cout << error_header << error_messages[i] << std::endl;
			}
		}

	}
	else {
		//No Errors - Good to go!
		//New Width and Height
		unsigned int old_width = width;
		width = x2 - x1;
		height = y2 - y1;

		//Store current pixel data as backup
		storePixelDataBackup();

		//Resize pixeldata
		resizePixelData();

		//load image data into cropped version from backup
		for (unsigned int x = 0; x < width; x++) {
			for (unsigned int y = 0; y < height; y++) {
				//get rbga out of pixel data backup
				for (unsigned int i = 0; i < 4; i++) {
					rgba[i] = pixel_data_backup[4 * old_width * y + 4 * x + i];
				}
				//insert into image
				insertRGBAtoSpecificPixel(x + x1, y + y1);
			}
		}

		//take new backup
		storePixelDataBackup();
	}
}

//Sets every element in obj.pixel_data to 0
void Image::zeroPixelData() {
	for (int i = 0; i < pixel_data.size(); i++) {
		pixel_data[i] = 0;
	}
}