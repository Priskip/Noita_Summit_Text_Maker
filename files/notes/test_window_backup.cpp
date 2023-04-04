//TEST WINDOW
if (test_window) {
	ImGui::Begin("Test Window", &test_window);
	ImGui::SliderInt("scale", &preview_display_scale, 1, 10);
	ImGui::InputText("Streamer Name", streamer_name, 256, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);
	//Create Image
	//Cast to string to use .length function
	std::string test_name = streamer_name;

	//clean canvas mat
	canvas_mat.zeroPixelData();
	canvas_mat.width = 64 * test_name.length();
	canvas_mat.height = 87;
	canvas_mat.resizePixelData();

	//clean canvas color
	canvas_color.zeroPixelData();
	canvas_color.width = 64 * test_name.length();
	canvas_color.height = 87;
	canvas_color.resizePixelData();

	//Variable to store positions where we splat our brush on the canvas
	std::vector<int> brush_positions;
	brush_positions.push_back(0); //for 0 indexing

	for (int char_num = 0; char_num < test_name.length(); char_num++) {
		//Get Character position in list.
		int char_pos = -1;
		for (int i = 0; i < character_list.length(); i++) {
			if (character_list[i] == test_name[char_num]) {
				char_pos = i;
				break;
			}
		}
		int chars_per_row = 26;
		int char_row = floor(char_pos / chars_per_row);
		int char_column = char_pos % chars_per_row;
		int char_width = (boundaries[char_row][char_column + 1]) - (boundaries[char_row][char_column] + 1);

		//Get Brushes
		brush_mat = font_mat.getSubselectionOfImage(
			boundaries[char_row][char_column] + 1, 1 + 90 * char_row,
			boundaries[char_row][char_column + 1], 88 + 90 * char_row
		);
		brush_color = font_color.getSubselectionOfImage(
			boundaries[char_row][char_column] + 1, 1 + 90 * char_row,
			boundaries[char_row][char_column + 1], 88 + 90 * char_row
		);

		//insert letter into image
		int brush_position = brush_positions[char_num];
		if (char_num > 0 && test_name[char_num] != ' ') { //If character is not the first, we will smoosh it back
			bool too_close = false;
			while (!too_close) {
				//Find all red pixels in brush
				std::vector<std::vector<int>> red_pixels;
				std::vector<int> red_pixel_position(2, 0);
				for (int y = 0; y < brush_mat.height; y++) {
					for (int x = 0; x < brush_mat.width; x++) {
						brush_mat.readRGBAatSpeicificPixel(x, y);
						if (brush_mat.compareRGBA(255, 0, 0, 255)) {
							red_pixel_position[0] = x;
							red_pixel_position[1] = y;
							red_pixels.push_back(red_pixel_position);
						}
					}
				}

				//For each red pixel found, see if there is a blue pixel immediately to it's left
				for (int i = 0; i < red_pixels.size(); i++) {
					int red_x_pos = brush_position + red_pixels[i][0];
					int red_y_pos = red_pixels[i][1];

					for (int y_offset = -1; y_offset <= 1; y_offset++) {
						if (red_y_pos + y_offset >= 0 && red_y_pos + y_offset <= 87) {//bounds for making sure we're not searching outside the side of the brush
							canvas_mat.readRGBAatSpeicificPixel(red_x_pos - 1, red_y_pos + y_offset);
							if (canvas_mat.compareRGBA(0, 0, 255, 255)) {
								//This pixel is blue.
								//This is bad
								too_close = true;
							}
						}
					}
				}

				//If we're not too close, decrement
				if (!too_close) brush_position--;
			}
			brush_position++;
		}
		canvas_mat.insertImage(brush_mat, brush_position, 0, true);
		canvas_color.insertImage(brush_color, brush_position, 0, true);
		brush_positions.push_back(brush_position + char_width + 5);
	}

	int crop_x = 1;
	bool breaky = false;
	for (int x = canvas_mat.width; x >= 0; x--) {
		for (int y = 0; y < canvas_mat.height; y++) {
			canvas_mat.readRGBAatSpeicificPixel(x, y);
			if (canvas_mat.compareRGBA(0, 0, 255, 255)) {
				breaky = true;
				crop_x = x;
				break;
			}
		}
		if (breaky) break;
	}

	canvas_mat.cropImage(0, 0, crop_x, canvas_mat.height);
	canvas_color.cropImage(0, 0, crop_x, canvas_color.height);
	//END Create Image


	//Draw Image
	ImGui::Checkbox("Mat Preview", &toggle_preview);
	if (toggle_preview) {
		im_ptr = &canvas_mat;
	}
	else {
		im_ptr = &canvas_color;
	}

	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	static ImVec4 colf = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	const ImVec2 p = ImGui::GetCursorScreenPos();
	ImU32 color = ImColor(colf);
	float x = p.x + 4.0f;
	float y = p.y + 44.0f;

	//Draw a white box around image preview
	draw_list->AddRect(
		ImVec2(x - 1, y - 1),
		ImVec2(x + preview_display_scale * im_ptr->width + 1, y + preview_display_scale * im_ptr->height + 1),
		color, 0.0f, ImDrawFlags_None, 1.0f);

	//Draw canvas
	for (int row = 0; row < im_ptr->height; row++) {
		for (int column = 0; column < im_ptr->width; column++) {
			im_ptr->readRGBAatSpeicificPixel(column, row);
			color = ImColor(ImVec4(
				(float)(im_ptr->rgba[0]) / 255,
				(float)(im_ptr->rgba[1]) / 255,
				(float)(im_ptr->rgba[2]) / 255,
				(float)(im_ptr->rgba[3]) / 255)
			);

			draw_list->AddRectFilled(
				ImVec2(column * preview_display_scale + x, row * preview_display_scale + y),
				ImVec2(column * preview_display_scale + x + preview_display_scale, row * preview_display_scale + y + preview_display_scale),
				color);
		}
	}
	//END Draw Image


	ImGui::End();
}