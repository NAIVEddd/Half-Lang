#include<vector>
#include "gif.h"

const int width = 256;
const int height = 256;
static uint8_t image[ width * height * 4 ];
static int sbuffer[ width * height ];
static int sbuffer2[ width * height ];

// one dimentional array as 2d array
// calculate the index of array using row and col
extern "C" int Index2D(int row, int col, int row_size);

// count the number of live neighbors
extern "C" int CountLiveNeighbors(int* array, int row, int col, int row_size);

// check is the cell alive or dead
extern "C" int IsAlive(int* array, int row, int col, int row_size);

// update the array
extern "C" void UpdateArray(int* array, int* new_array, int row_size, int col_size);

void set_pixel_white(size_t x, size_t y)
{
    size_t index = (x + y * width) * 4;
    image[index + 0] = 255;
    image[index + 1] = 255;
    image[index + 2] = 255;
    image[index + 3] = 255;
}

void set_pixel_black(size_t x, size_t y)
{
    size_t index = (x + y * width) * 4;
    image[index + 0] = 0;
    image[index + 1] = 0;
    image[index + 2] = 0;
    image[index + 3] = 255;
}

void copy_buffer_to_image(int* buffer, size_t row_size, size_t col_size)
{
    for (size_t i = 0; i < row_size; i++)
    {
        for (size_t j = 0; j < col_size; j++)
        {
            if (buffer[Index2D(i, j, row_size)] == 1)
            {
                set_pixel_white(i, j);
            }
            else
            {
                set_pixel_black(i, j);
            }
        }
    }
}

int main(int argc, const char * argv[])
{
    const char* filename = "./MyGif.gif";
    if( argc > 1 )
    {
        filename = argv[1];
    }
    
    // Create a gif
    GifWriter writer = {};
    GifBegin( &writer, filename, width, height, 2, 8, true );
    
    // Set the initial state of the game
    int* buffer = sbuffer;
    int* new_buffer = sbuffer2;
    
    for (size_t i = 0; i < width; i++)
    {
        for (size_t j = 0; j < height; j++)
        {
            int v = std::rand() % 2;
            buffer[Index2D(i, j, width)] = v;
            new_buffer[Index2D(i, j, width)] = v;
        }
    }

    for( int frame=0; frame<256; ++frame )
    {
        std::swap( buffer, new_buffer );
        UpdateArray(buffer, new_buffer, width, height);
        
        copy_buffer_to_image(buffer, width, height);
        
        // Write the frame to the gif
        GifWriteFrame( &writer, image, width, height, 2, 8, true );
    }
    
    // Write EOF
    GifEnd( &writer );
    
    return 0;
}