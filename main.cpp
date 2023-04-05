/*
	--- Noita Summit Text Maker ---

	Author: Priskip

	Libraries Used:
	Lodepng - https://github.com/lvandeve/lodepng
	TinyXML2 - https://github.com/leethomason/tinyxml2
	ImGUI - https://github.com/ocornut/imgui

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.
*/

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cmath>
#include "lodepng.h"
#include "tinyxml2.h"
#include "imageClass.h"
#include "utilities.h"

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_4.h>
#include <tchar.h>

#ifdef _DEBUG
#define DX12_ENABLE_DEBUG_LAYER
#endif

#ifdef DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

struct FrameContext
{
	ID3D12CommandAllocator* CommandAllocator;
	UINT64 FenceValue;
};

// Data
static int const NUM_FRAMES_IN_FLIGHT = 3;
static FrameContext g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
static UINT g_frameIndex = 0;
static int const NUM_BACK_BUFFERS = 3;
static ID3D12Device* g_pd3dDevice = NULL;
static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
static ID3D12Fence* g_fence = NULL;
static HANDLE g_fenceEvent = NULL;
static UINT64 g_fenceLastSignaledValue = 0;
static IDXGISwapChain3* g_pSwapChain = NULL;
static HANDLE g_hSwapChainWaitableObject = NULL;
static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
static D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

// Forward declarations of imgui's helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void WaitForLastSubmittedFrame();
FrameContext* WaitForNextFrameResources();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declaration of Priskip's helper functions


// Main code
int main(int, char**)
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"ImGui Example", NULL };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Summit Text Maker", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
	//io.ConfigViewportsNoAutoMerge = true;
	//io.ConfigViewportsNoTaskBarIcon = true;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(g_pd3dDevice, NUM_FRAMES_IN_FLIGHT,
		DXGI_FORMAT_R8G8B8A8_UNORM, g_pd3dSrvDescHeap,
		g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
		g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart());

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);


	// -------------------- PUT ALL UI CODE BETWEEN HERE AND NEXT ONE OF THESE COMMENTS -------------------- //
	//Background Color
	ImVec4 background_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//Window Booleans
	bool show_demo_window = false;
	bool show_font_config_window = false;
	bool show_input_window = true;
	bool show_output_config_window = true;

	//List of names
	static char name[256];
	static char list_of_names_file_location[256] = "files/list.txt";
	static std::vector<std::string> list_of_names;

	//Allowed Character List
	std::string allowed_characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890+-()/?,.\"\'_ ";
	struct TextFilters
	{
		static int FilterImGuiLetters(ImGuiInputTextCallbackData* data)
		{
			if (data->EventChar < 256 && strchr("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890+-()/?,.\"\'_ ", (char)data->EventChar))
				return 0;
			return 1;
		}
	};

	//Output Types
	bool basic = true;
	bool one_player = false;
	bool two_player_left = false;
	bool two_player_right = false;

	//Config for full screen positionings
	static char output_generation_config_file_location[256] = "files/config.xml";
	static int one_player_x = 0;
	static int one_player_y = 0;
	static int two_player_left_x = 0;
	static int two_player_left_y = 0;
	static int two_player_right_x = 0;
	static int two_player_right_y = 0;

	//Read output generation config file before program runs.
	static tinyxml2::XMLDocument output_config;
	output_config.LoadFile(output_generation_config_file_location);
	tinyxml2::XMLElement* pRootElement = output_config.RootElement();

	for (const tinyxml2::XMLAttribute* attribute = pRootElement->FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
		std::string parameter = attribute->Name();

		if (parameter == "one_player_x") {
			one_player_x = decStringToUnsignedInt(attribute->Value());
		}
		if (parameter == "one_player_y") {
			one_player_y = decStringToUnsignedInt(attribute->Value());
		}
		if (parameter == "two_player_left_x") {
			two_player_left_x = decStringToUnsignedInt(attribute->Value());
		}
		if (parameter == "two_player_left_y") {
			two_player_left_y = decStringToUnsignedInt(attribute->Value());
		}
		if (parameter == "two_player_right_x") {
			two_player_right_x = decStringToUnsignedInt(attribute->Value());
		}
		if (parameter == "two_player_right_y") {
			two_player_right_y = decStringToUnsignedInt(attribute->Value());
		}
	}

	// Main loop
	bool done = false;
	while (!done)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Start the Dear ImGui frame
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//Main Window
		{
			ImGui::Begin("Main");
			//ImGui::Checkbox("Demo Window", &show_demo_window);
			ImGui::Checkbox("Input Window", &show_input_window);
			ImGui::Checkbox("Output Config Window", &show_output_config_window);
			ImGui::Checkbox("Font Config Window", &show_font_config_window);
			ImGui::End();
		}

		//Demo Window
		/*
		if (show_demo_window) {
			ImGui::ShowDemoWindow(&show_demo_window);
		}
		*/

		//Font Config Window
		if (show_font_config_window) {
			ImGui::Begin("Font Config");
			ImGui::Text("Coming Soon!");
			ImGui::Text("This will be for making your own custom png font thingies work with this tool.");
			ImGui::End();
		}

		//Input Window
		if (show_input_window) {
			ImGui::Begin("Inputs");

			if (ImGui::BeginTabBar("##Inputs_Tab_Bar"))
			{
				if (ImGui::BeginTabItem("Input Name"))
				{
					ImGui::Text("Add a single element to the list.");
					ImGui::Text("Allowed Characters: \"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890 +-()/?,.\"\'_\"");
					ImGui::InputText("Input", name, 256, ImGuiInputTextFlags_CallbackCharFilter, TextFilters::FilterImGuiLetters);

					if (ImGui::Button("Add to list")) {
						int table_pos = getVectorPosition(list_of_names, name); //returns -1 if element does not exist in vector 
						if (table_pos == -1) {
							list_of_names.push_back(name); //streamer is not in list, add it to list 
						}
					}

					//Sort List
					if (list_of_names.size() > 1) {
						std::sort(list_of_names.begin(), list_of_names.end());
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Read Data from Text File"))
				{
					ImGui::InputText("File Location", list_of_names_file_location, 256);
					if (ImGui::Button("Read File Contents")) {
						//Open File
						std::ifstream file;
						file.open(list_of_names_file_location);
						std::string line;

						if (file.is_open()) {
							while (file) {
								getline(file, line); //read line in file

								bool allowed = false;
								for (int i = 0; i < line.size(); i++) {
									allowed = false;
									for (int j = 0; j < allowed_characters.size(); j++) {
										if (line[i] == allowed_characters[j]) {
											allowed = true;
											break;
										}
									}
									if (!allowed) {
										break;
									}
								}

								//if allowed == false, then list contains a character not present in the allowed characters
								if (allowed) {
									//Check to see if element is already in table
									int table_pos = getVectorPosition(list_of_names, line); //returns -1 if the element does not exist in the vector
									if (table_pos == -1) {
										list_of_names.push_back(line);
									}
								}
							}
							//Sort List
							if (list_of_names.size() > 1) {
								std::sort(list_of_names.begin(), list_of_names.end());
							}

						}
						else
						{
							//Error - could not open file
							//TODO: write error outputting code
						}
						//Display Errors
						//ImGui::Text("Error/s:");
						//TODO: Display Errors
					}

					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("The List"))
				{
					if (list_of_names.size() > 0) {
						ImGui::Text("Click on element to remove from list.");
						ImGui::Text("Click");
						ImGui::SameLine();
						if (ImGui::Button("Here")) {
							list_of_names.clear();
						}
						ImGui::SameLine();
						ImGui::Text("to clear the entire list.");

						static ImGuiTableFlags flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersOuter |
							ImGuiTableFlags_BordersV | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoHostExtendX;

						int columns = 5;
						int rows = list_of_names.size() / columns;
						if (list_of_names.size() % columns != 0) {
							rows++;
						}

						if (ImGui::BeginTable("table1", columns, flags))
						{
							int count = 0;
							for (int row = 0; row < rows; row++)
							{
								ImGui::TableNextRow();
								for (int column = 0; column < columns; column++) {
									ImGui::TableSetColumnIndex(column);
									if (count < list_of_names.size()) {
										if (ImGui::Button(list_of_names[count].c_str())) {
											list_of_names.erase(list_of_names.begin() + count);
										}
									}
									count++;
								}

							}
							ImGui::EndTable();
						}
					}
					else
					{
						ImGui::Text("List is Empty");
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}

			ImGui::End();
		}


		//Output Config Window
		if (show_output_config_window) {
			ImGui::Begin("Output Config");


			if (ImGui::BeginTabBar("##Inputs_Tab_Bar"))
			{
				if (ImGui::BeginTabItem("Generate Outputs"))
				{
					ImGui::Text("Select which types of outputs to generate.");
					ImGui::Checkbox("Basic", &basic);
					ImGui::SameLine();
					ImGui::Checkbox("1 Player", &one_player);
					ImGui::SameLine();
					ImGui::Checkbox("2 Player Left", &two_player_left);
					ImGui::SameLine();
					ImGui::Checkbox("2 Player Right", &two_player_right);

					if (ImGui::Button("Generate Images")) {
						if ((basic || one_player || two_player_left || two_player_right) && list_of_names.size() >= 1) {
							for (int i = 0; i < list_of_names.size(); i++) {
								//Cast to string to use .length function
								std::string name_to_gen = list_of_names[i];

								//Image Variables
								Image canvas_mat;
								Image canvas_color;
								Image brush_mat;
								Image brush_color;
								Image font_mat;
								font_mat.readImageIntoPixelData("files/images/fonts/noita/matte.png");
								Image font_color;
								font_color.readImageIntoPixelData("files/images/fonts/noita/color.png");

								//Boundaries
								std::vector<std::vector<int>> boundaries;
								boundaries.resize(3);

								for (int row = 0; row < 3; row++) {
									for (int x = 0; x < font_mat.width; x++) {
										font_mat.readRGBAatSpeicificPixel(x, 1 + 90 * row);
										if (font_mat.compareRGBA(0, 255, 0, 255)) {
											boundaries[row].push_back(x);
										}
									}
								}

								//clean canvas mat
								canvas_mat.zeroPixelData();
								canvas_mat.width = 64 * name_to_gen.length();
								canvas_mat.height = 87;
								canvas_mat.resizePixelData();

								//clean canvas color
								canvas_color.zeroPixelData();
								canvas_color.width = 64 * name_to_gen.length();
								canvas_color.height = 87;
								canvas_color.resizePixelData();

								//Variable to store positions where we splat our brush on the canvas
								std::vector<int> brush_positions;
								brush_positions.push_back(0); //for 0 indexing

								for (int char_num = 0; char_num < name_to_gen.length(); char_num++) {
									//Get Character position in list.
									int char_pos = -1;
									for (int i = 0; i < allowed_characters.length(); i++) {
										if (allowed_characters[i] == name_to_gen[char_num]) {
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
									if (char_num > 0 ) { //&& name_to_gen[char_num] != ' '
										//If character is not the first, we will smoosh it back
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

								//"?" and "." can be messy with file names...
								for (int i = 0; i < name_to_gen.length(); i++) {
									if (name_to_gen[i] == '?') {
										name_to_gen[i] = '-';
									}
									if (name_to_gen[i] == '.') {
										name_to_gen[i] = '_';
									}
								}

								if (basic) {
									
									canvas_color.writePixelDataIntoImage("files/images/basic/"+ name_to_gen +".png");
								}
								if (one_player) {
									Image big_canvas(1920, 1080);
									big_canvas.insertImage(canvas_color, one_player_x, one_player_y, true);
									big_canvas.writePixelDataIntoImage("files/images/1_player/" + name_to_gen + ".png");
								}
								if (two_player_left) {
									Image big_canvas(1920, 1080);
									big_canvas.insertImage(canvas_color, two_player_left_x, two_player_left_y, true);
									big_canvas.writePixelDataIntoImage("files/images/2_players/left/" + name_to_gen + ".png");
								}
								if (two_player_right) {
									Image big_canvas(1920, 1080);
									big_canvas.insertImage(canvas_color, two_player_right_x, two_player_right_y, true);
									big_canvas.writePixelDataIntoImage("files/images/2_players/right/" + name_to_gen + ".png");
								}
							
							}
						}
					}
					ImGui::EndTabItem();
				}

				if (ImGui::BeginTabItem("Config Outputs"))
				{
					ImGui::Text("Configure Output Generation.");
					ImGui::PushItemWidth(100);
					ImGui::Text("      1 Player:");
					ImGui::SameLine();
					ImGui::InputInt("X (1P) ", &one_player_x);
					ImGui::SameLine();
					ImGui::InputInt("Y (1P) ", &one_player_y);

					ImGui::Text(" 2 Player Left:");
					ImGui::SameLine();
					ImGui::InputInt("X (2PL)", &two_player_left_x);
					ImGui::SameLine();
					ImGui::InputInt("Y (2PL)", &two_player_left_y);

					ImGui::Text("2 Player Right:");
					ImGui::SameLine();
					ImGui::InputInt("X (2PR)", &two_player_right_x);
					ImGui::SameLine();
					ImGui::InputInt("Y (2PR)", &two_player_right_y);
					ImGui::PopItemWidth();

					ImGui::Text("");

					ImGui::InputText("Config File Location", output_generation_config_file_location, 256);

					if (ImGui::Button("Read Config Values")) {
						output_config.LoadFile(output_generation_config_file_location);
						tinyxml2::XMLElement* pRootElement = output_config.RootElement();

						for (const tinyxml2::XMLAttribute* attribute = pRootElement->FirstAttribute(); attribute != nullptr; attribute = attribute->Next()) {
							std::string parameter = attribute->Name();

							if (parameter == "one_player_x") {
								one_player_x = decStringToUnsignedInt(attribute->Value());
							}
							if (parameter == "one_player_y") {
								one_player_y = decStringToUnsignedInt(attribute->Value());
							}
							if (parameter == "two_player_left_x") {
								two_player_left_x = decStringToUnsignedInt(attribute->Value());
							}
							if (parameter == "two_player_left_y") {
								two_player_left_y = decStringToUnsignedInt(attribute->Value());
							}
							if (parameter == "two_player_right_x") {
								two_player_right_x = decStringToUnsignedInt(attribute->Value());
							}
							if (parameter == "two_player_right_y") {
								two_player_right_y = decStringToUnsignedInt(attribute->Value());
							}
						}

					}

					if (ImGui::Button("Store Config Values")) {
						output_config.LoadFile("files/config.xml");
						tinyxml2::XMLElement* pRootElement = output_config.RootElement();
						pRootElement->SetAttribute("one_player_x", one_player_x);
						pRootElement->SetAttribute("one_player_y", one_player_y);
						pRootElement->SetAttribute("two_player_left_x", two_player_left_x);
						pRootElement->SetAttribute("two_player_left_y", two_player_left_y);
						pRootElement->SetAttribute("two_player_right_x", two_player_right_x);
						pRootElement->SetAttribute("two_player_right_y", two_player_right_y);
						output_config.SaveFile(output_generation_config_file_location);
					}

					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}


			ImGui::End();
		}

		// -------------------- PUT ALL UI CODE BETWEEN HERE AND THE PREVIOUS ONE OF THESE COMMENTS -------------------- //

		// Rendering
		ImGui::Render();

		FrameContext* frameCtx = WaitForNextFrameResources();
		UINT backBufferIdx = g_pSwapChain->GetCurrentBackBufferIndex();
		frameCtx->CommandAllocator->Reset();

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = g_mainRenderTargetResource[backBufferIdx];
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		g_pd3dCommandList->Reset(frameCtx->CommandAllocator, NULL);
		g_pd3dCommandList->ResourceBarrier(1, &barrier);

		// Render Dear ImGui graphics
		const float background_color_with_alpha[4] = { background_color.x * background_color.w, background_color.y * background_color.w, background_color.z * background_color.w, background_color.w };
		g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[backBufferIdx], background_color_with_alpha, 0, NULL);
		g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[backBufferIdx], FALSE, NULL);
		g_pd3dCommandList->SetDescriptorHeaps(1, &g_pd3dSrvDescHeap);
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		g_pd3dCommandList->ResourceBarrier(1, &barrier);
		g_pd3dCommandList->Close();

		g_pd3dCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&g_pd3dCommandList);

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault(NULL, (void*)g_pd3dCommandList);
		}

		g_pSwapChain->Present(1, 0); // Present with vsync
		//g_pSwapChain->Present(0, 0); // Present without vsync

		UINT64 fenceValue = g_fenceLastSignaledValue + 1;
		g_pd3dCommandQueue->Signal(g_fence, fenceValue);
		g_fenceLastSignaledValue = fenceValue;
		frameCtx->FenceValue = fenceValue;
	}//End Main Loop

	WaitForLastSubmittedFrame();

	// Cleanup
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);


	std::cout << "Done!" << std::endl;

	return 0;
}

// --- BEGIN IMGUI HELPER FUNCTIONS --- //
bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC1 sd;
	{
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = NUM_BACK_BUFFERS;
		sd.Width = 0;
		sd.Height = 0;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		sd.Scaling = DXGI_SCALING_STRETCH;
		sd.Stereo = FALSE;
	}

	// [DEBUG] Enable debug interface
#ifdef DX12_ENABLE_DEBUG_LAYER
	ID3D12Debug* pdx12Debug = NULL;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&pdx12Debug))))
		pdx12Debug->EnableDebugLayer();
#endif

	// Create device
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;
	if (D3D12CreateDevice(NULL, featureLevel, IID_PPV_ARGS(&g_pd3dDevice)) != S_OK)
		return false;

	// [DEBUG] Setup debug interface to break on any warnings/errors
#ifdef DX12_ENABLE_DEBUG_LAYER
	if (pdx12Debug != NULL)
	{
		ID3D12InfoQueue* pInfoQueue = NULL;
		g_pd3dDevice->QueryInterface(IID_PPV_ARGS(&pInfoQueue));
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		pInfoQueue->Release();
		pdx12Debug->Release();
	}
#endif

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = NUM_BACK_BUFFERS;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 1;
		if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dRtvDescHeap)) != S_OK)
			return false;

		SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
		for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
		{
			g_mainRenderTargetDescriptor[i] = rtvHandle;
			rtvHandle.ptr += rtvDescriptorSize;
		}
	}

	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (g_pd3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&g_pd3dSrvDescHeap)) != S_OK)
			return false;
	}

	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 1;
		if (g_pd3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&g_pd3dCommandQueue)) != S_OK)
			return false;
	}

	for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
		if (g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_frameContext[i].CommandAllocator)) != S_OK)
			return false;

	if (g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_frameContext[0].CommandAllocator, NULL, IID_PPV_ARGS(&g_pd3dCommandList)) != S_OK ||
		g_pd3dCommandList->Close() != S_OK)
		return false;

	if (g_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&g_fence)) != S_OK)
		return false;

	g_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (g_fenceEvent == NULL)
		return false;

	{
		IDXGIFactory4* dxgiFactory = NULL;
		IDXGISwapChain1* swapChain1 = NULL;
		if (CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)) != S_OK)
			return false;
		if (dxgiFactory->CreateSwapChainForHwnd(g_pd3dCommandQueue, hWnd, &sd, NULL, NULL, &swapChain1) != S_OK)
			return false;
		if (swapChain1->QueryInterface(IID_PPV_ARGS(&g_pSwapChain)) != S_OK)
			return false;
		swapChain1->Release();
		dxgiFactory->Release();
		g_pSwapChain->SetMaximumFrameLatency(NUM_BACK_BUFFERS);
		g_hSwapChainWaitableObject = g_pSwapChain->GetFrameLatencyWaitableObject();
	}

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->SetFullscreenState(false, NULL); g_pSwapChain->Release(); g_pSwapChain = NULL; }
	if (g_hSwapChainWaitableObject != NULL) { CloseHandle(g_hSwapChainWaitableObject); }
	for (UINT i = 0; i < NUM_FRAMES_IN_FLIGHT; i++)
		if (g_frameContext[i].CommandAllocator) { g_frameContext[i].CommandAllocator->Release(); g_frameContext[i].CommandAllocator = NULL; }
	if (g_pd3dCommandQueue) { g_pd3dCommandQueue->Release(); g_pd3dCommandQueue = NULL; }
	if (g_pd3dCommandList) { g_pd3dCommandList->Release(); g_pd3dCommandList = NULL; }
	if (g_pd3dRtvDescHeap) { g_pd3dRtvDescHeap->Release(); g_pd3dRtvDescHeap = NULL; }
	if (g_pd3dSrvDescHeap) { g_pd3dSrvDescHeap->Release(); g_pd3dSrvDescHeap = NULL; }
	if (g_fence) { g_fence->Release(); g_fence = NULL; }
	if (g_fenceEvent) { CloseHandle(g_fenceEvent); g_fenceEvent = NULL; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }

#ifdef DX12_ENABLE_DEBUG_LAYER
	IDXGIDebug1* pDebug = NULL;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
	{
		pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
		pDebug->Release();
}
#endif
}

void CreateRenderTarget()
{
	for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
	{
		ID3D12Resource* pBackBuffer = NULL;
		g_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pBackBuffer));
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, g_mainRenderTargetDescriptor[i]);
		g_mainRenderTargetResource[i] = pBackBuffer;
	}
}

void CleanupRenderTarget()
{
	WaitForLastSubmittedFrame();

	for (UINT i = 0; i < NUM_BACK_BUFFERS; i++)
		if (g_mainRenderTargetResource[i]) { g_mainRenderTargetResource[i]->Release(); g_mainRenderTargetResource[i] = NULL; }
}

void WaitForLastSubmittedFrame()
{
	FrameContext* frameCtx = &g_frameContext[g_frameIndex % NUM_FRAMES_IN_FLIGHT];

	UINT64 fenceValue = frameCtx->FenceValue;
	if (fenceValue == 0)
		return; // No fence was signaled

	frameCtx->FenceValue = 0;
	if (g_fence->GetCompletedValue() >= fenceValue)
		return;

	g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
	WaitForSingleObject(g_fenceEvent, INFINITE);
}

FrameContext* WaitForNextFrameResources()
{
	UINT nextFrameIndex = g_frameIndex + 1;
	g_frameIndex = nextFrameIndex;

	HANDLE waitableObjects[] = { g_hSwapChainWaitableObject, NULL };
	DWORD numWaitableObjects = 1;

	FrameContext* frameCtx = &g_frameContext[nextFrameIndex % NUM_FRAMES_IN_FLIGHT];
	UINT64 fenceValue = frameCtx->FenceValue;
	if (fenceValue != 0) // means no fence was signaled
	{
		frameCtx->FenceValue = 0;
		g_fence->SetEventOnCompletion(fenceValue, g_fenceEvent);
		waitableObjects[1] = g_fenceEvent;
		numWaitableObjects = 2;
	}

	WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

	return frameCtx;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			WaitForLastSubmittedFrame();
			CleanupRenderTarget();
			HRESULT result = g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT);
			assert(SUCCEEDED(result) && "Failed to resize swapchain.");
			CreateRenderTarget();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
// --- END IMGUI HELPER FUNCTIONS --- //

// --- BEGIN PRISKIP'S HELPER FUNCTIONS --//

// --- END PRISKIP'S HELPER FUNCTIONS --//