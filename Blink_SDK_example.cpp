#include "Blink_C_wrapper.h"  // Relative path to SDK header.
#include "ImageGen.h"
#include "math.h"
#include <Windows.h>
#include <iostream>
using namespace std;

#include <stdint.h>

#define STB_IMAGE_IMPLEMENTATION  // For loading images
#include "stb_image.h"  // For loading images

// ------------------------- Blink_SDK_example --------------------------------
// Simple example using the Blink_SDK DLL to send a sequence of phase targets
// to a single SLM.
// To run the example, ensure that Blink_SDK.dll is in the same directory as
// the Blink_SDK_example.exe.
// ----------------------------------------------------------------------------
int main()
{
  int board_number;
  // Construct a Blink_SDK instance
  unsigned int bits_per_pixel = 12U;   //12U is used for 1920x1152 SLM, 8U used for the small 512x512
  bool         is_nematic_type = true;
  bool         RAM_write_enable = true;
  bool         use_GPU_if_available = true;
  unsigned int n_boards_found = 0U;
  int         constructed_okay = true;




  //if bits per pixel is wrong, the lower level code will figure out
  //what it should be and construct properly.
  Create_SDK(bits_per_pixel, &n_boards_found, &constructed_okay, is_nematic_type, RAM_write_enable, use_GPU_if_available, 10U, 0);

  cout << "# of SLM Found = " << n_boards_found << endl;

  // return of 0 means okay, return -1 means error
  if (constructed_okay == 0)
  {
	board_number = n_boards_found;
	
	int height = Get_image_height(board_number);
	int width = Get_image_width(board_number);

	double pitch =  Get_pixel_pitch(board_number);

		
	double temp1 = Read_SLM_temperature(1);
	
	
	if (board_number == 2) {
		double temp2 = Read_SLM_temperature(2);
		cout << "SLM1 (SN5500) Temperature = " << temp1 << endl;
		cout << "SLM2 (SN5495) Temperature = " << temp2 << endl;
	}
	else {
		cout << "SLM Temperature = " << temp1 << endl;
	}

	cout << "SLM Pixel Pitch = " << pitch << endl;
	cout << "SLM Size = " << width << "x" << height << endl;

	const char* info = Get_version_info();
    
	cout << "SLM Info = " << info << endl << endl;

    //***you should replace *bit_linear.LUT with your custom LUT file***
	//but for now open a generic LUT that linearly maps input graylevels to output voltages
	//***Using *bit_linear.LUT does NOT give a linear phase response***
	/*char* lut_file;
	if(width == 512)
		lut_file = "C:\\Program Files\\Meadowlark Optics\\Blink OverDrive Plus\\LUT Files\\8bit_linear.LUT";
	else 
		lut_file = "C:\\Program Files\\Meadowlark Optics\\Blink OverDrive Plus\\LUT Files\\12bit_linear.LUT";
	Load_LUT_file(board_number, lut_file);*/

	char* lut_file_2 = "C:\\Program Files\\Meadowlark Optics\\Blink OverDrive Plus\\LUT Files\\slm5495_at1550_amplitude.LUT";
	char* lut_file_1 = "C:\\Program Files\\Meadowlark Optics\\Blink OverDrive Plus\\LUT Files\\slm5500_at1550_amplitude.LUT";
	Load_LUT_file(1, lut_file_1);
	if (board_number == 2) {
		Load_LUT_file(2, lut_file_2);
		cout << "LUT File_1 = " << lut_file_1 << endl;
		cout << "LUT File_2 = " << lut_file_2 << endl;
	}
	else {
		cout << "LUT File = " << lut_file_1 << endl;
	}

	// Create two vectors to hold values for two SLM images with opposite ramps.
	unsigned char* ImageOne = new unsigned char[width*height];
	unsigned char* ImageTwo = new unsigned char[width*height];
	// Generate phase gradients
	//int VortexCharge = 5;
	//Generate_LG(ImageOne, width, height, VortexCharge, width / 2.0, height / 2.0, false);
	//VortexCharge = 3;
	//Generate_LG(ImageTwo, width, height, VortexCharge, width / 2.0, height / 2.0, false);

	//Generate_Solid(ImageOne, width, height, 0);
	//Generate_Solid(ImageTwo, width, height, 255);

	//uint8_t* rgb_image = stbi_load("image.png", &width, &height, &bpp, 3);

	//stbi_image_free(rgb_image);
	
	int n = 1;
	//unsigned char* data = stbi_load("1920white.bmp", &width, &height, &n, 0);
	ImageOne = stbi_load("C:\\Program Files\\Meadowlark Optics\\Blink OverDrive Plus\\Image Files\\frame rate\\1920black.bmp", &width, &height, &n, 0);
	ImageTwo = stbi_load("C:\\Program Files\\Meadowlark Optics\\Blink OverDrive Plus\\Image Files\\frame rate\\1920white.bmp", &width, &height, &n, 0);
	//int s = sizeof(ImageTwo);
	//cout << "bmp size = " << s << endl;

	bool ExternalTrigger = false;
	bool OutputPulse = false;

	int framerate = 20;
	int loopnumber = 200;

	cout << "External Trigger = " << ExternalTrigger << endl << endl;
	cout << "Frame Rate = " << framerate << " Hz" << endl;
	cout << "Dwell Time = " << 1000 / framerate << " ms" << endl;
	cout << "Loop Number = " << loopnumber << endl << endl;

	cout << "Current Loop = ";

	for (int i = 0; i < loopnumber; i++)
	{
		cout << i+1; cout << "; ";
		//write image returns on DMA complete, ImageWriteComplete returns when the hardware
		//image buffer is ready to receive the next image. Breaking this into two functions is 
		//useful for external triggers. It is safe to apply a trigger when Write_image is complete
		//and it is safe to write a new image when ImageWriteComplete returns
		Write_image(1, ImageOne, width * height, ExternalTrigger, OutputPulse, 5000);
		Write_image(board_number, ImageOne, width*height, ExternalTrigger, OutputPulse, 5000);
		ImageWriteComplete(1, 5000);
		ImageWriteComplete(board_number, 5000);
		//Sleep(500);
		Sleep(1000 / framerate);

		Write_image(1, ImageTwo, width * height, ExternalTrigger, OutputPulse, 5000);
		Write_image(board_number, ImageTwo, width*height, ExternalTrigger, OutputPulse, 5000);
		ImageWriteComplete(1, 5000);
		ImageWriteComplete(board_number, 5000);
		//Sleep(500);
		Sleep(1000 / framerate);
	}
	cout << endl << endl;
	cout << "Display is finished!" << endl;

	//delete[]ImageOne;
	//delete[]ImageTwo;

	stbi_image_free(ImageOne);
	stbi_image_free(ImageTwo);

	SLM_power(false);
	Delete_SDK();
	return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}