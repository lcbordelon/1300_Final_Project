/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Lindsay Bordelon

- All project requirements fully met? (YES or NO):
    <ANSWER>

- If no, please explain what you could not get to work:
    <ANSWER>

- Did you do any optional enhancements? If so, please explain:
    No, but below is a list of things I learned/cemented during this project:
    -Why variable type matters (string to int, int to double, etc)
    -I need to compile the code after every change and then run the code
    
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string>
#include <sstream>

using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */
int get_int(fstream &stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    cout << "in read_image function, filename:" << filename << endl;

    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    // cout << "file size" << file_size;
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        cout << "we're getting here" << endl;
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel>(width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();

    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset + i] = (unsigned char)(value >> (i * 8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>> &image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header, 0, 1, 'B');                                             // ID field
    set_bytes(bmp_header, 1, 1, 'M');                                             // ID field
    set_bytes(bmp_header, 2, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE + array_bytes); // Size of BMP file
    set_bytes(bmp_header, 6, 2, 0);                                               // Reserved
    set_bytes(bmp_header, 8, 2, 0);                                               // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE + DIB_HEADER_SIZE);              // Pixel array offset

    // DIB Header
    set_bytes(dib_header, 0, 4, DIB_HEADER_SIZE); // DIB header size
    set_bytes(dib_header, 4, 4, width_pixels);    // Width of bitmap in pixels
    set_bytes(dib_header, 8, 4, height_pixels);   // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);              // Number of color planes
    set_bytes(dib_header, 14, 2, 24);             // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);              // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);    // Size of raw bitmap data (including padding)
    set_bytes(dib_header, 24, 4, 2835);           // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);           // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);              // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);              // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char *)bmp_header, sizeof(bmp_header));
    stream.write((char *)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char *)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//

//
// YOUR FUNCTION DEFINITIONS HERE
//

//PROCESS 1 - ADDS VIGNETTE
vector<vector<Pixel>> process_1(const vector<vector<Pixel>> &image)
{
    int num_rows = image.size();
    int num_columns = image[0].size();
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_columns));

    for (int row = 0; row < num_rows; row++)
    {
        for (int col = 0; col < num_columns; col++)
        {
            int red_color = image[row][col].red;
            int green_color = image[row][col].green;
            int blue_color = image[row][col].blue;

            int distance = sqrt(pow((col - num_columns / 2), 2) + pow((row - num_rows / 2), 2));
            double scaling_factor = (num_rows - distance) / double(num_rows);

            int new_red = red_color * scaling_factor;
            int new_green = green_color * scaling_factor;
            int new_blue = blue_color * scaling_factor;

            new_image[row][col].red = new_red;
            new_image[row][col].green = new_green;
            new_image[row][col].blue = new_blue;
        }
    }

    return new_image;
}

int main()
{

    //
    // YOUR CODE HERE
    //
    cout << "CSPB 1300 Image Processing Application" << endl;
    cout << "Enter input BMP filename" << endl;
    string file_name;
    cin >> file_name;
    cout << file_name << endl;

    cout << "IMAGE PROCESSING MENU" << endl
         << "0) Change image (current: " << file_name << ")" << endl
         << "1) Vignette" << endl
         << "2) Clarendon" << endl
         << "3) Grayscale" << endl
         << "4) Rotate 90 degrees" << endl
         << "5) Rotate multiple 90 degrees" << endl
         << "6) Enlarge" << endl
         << "7) High contrast" << endl
         << "8) Lighten" << endl
         << "9) Darken" << endl
         << "10) Black, white, red, green, blue" << endl;

    cout << "Enter menu selection (Q to quit): " << endl;
    string menu_number;
    cin >> menu_number;
    stringstream geek(menu_number);
    int menu_choice = 0;
    geek >> menu_choice;
    cout << "Menu Choice: " << menu_choice << endl;

    if (menu_choice == 1)
    {
        vector<vector<Pixel>> image = read_image(file_name);
        vector<vector<Pixel>> new_image = process_1(image);
        bool success = write_image("new_sample.bmp", new_image);
    }

    // Read in BMP image file into a 2D vector (using read_image function)

    // Output modified red and blue pixel image, results
    // cout << "The dimensions of the image are: " << image.rows << " rows by " << image.cols << " columns." << endl;

    // Call process_1 function using the input 2D vector and save the result returned to a new 2D vector

    // Write the resulting 2D vector to a new BMP image file (using write_image function)

    return 0;
}