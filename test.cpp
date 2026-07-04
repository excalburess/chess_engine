#include <iostream>
using namespace std;

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

double test()
{
	int width, height, channels;
	unsigned char* data = stbi_load("sprites.jpg", &width, &height, &channels, 0);

	if (data == nullptr)
	{
		std::cout << "Failed to load image!" << std::endl;
		return 1;
	}

	std::cout << "Loaded image: " << width << "x" << height
		<< " with " << channels << " channels" << std::endl;

	stbi_image_free(data);
	return 0;
}

// Add this main function to serve as the entry point
int main()
{
	cout << "Starting chess DL test..." << endl;
	test();

	// Keeps the console window open so you can read the output
	cout << "\nPress Enter to exit..." << endl;
	cin.get();
	return 0;
}