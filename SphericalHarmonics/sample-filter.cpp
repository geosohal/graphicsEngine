#include "math.h"
#include <vector>
#include <iostream>
#include <string.h>
#include <assert.h> 
using namespace std;

#include "rgbe.h"

const float PI = 3.1415927;

// Read an HDR image in .hdr (RGBE) format.
void read(const string inName, std::vector<float>& image, 
          int& width, int& height)
{
    rgbe_header_info info;
    char errbuf[100] = {0};
    
    // Open file and read width and height from the header
    FILE* fp = fopen(inName.c_str(), "rb");
    if (!fp) {
        printf("Can't open file: %s\n", inName.c_str());
        exit(-1); }
    int rc = RGBE_ReadHeader(fp, &width, &height, &info, errbuf);
    if (rc != RGBE_RETURN_SUCCESS) {
        printf("RGBE read error: %s\n", errbuf);
        exit(-1); }

    // Allocate enough memory
    image.resize(3*width*height);

    // Read the pixel data and close the file
    rc = RGBE_ReadPixels_RLE(fp, &image[0], width, height, errbuf);
    if (rc != RGBE_RETURN_SUCCESS) {
        printf("RGBE read error: %s\n", errbuf);
        exit(-1); }
    fclose(fp);
    
    printf("Read %s (%dX%d)\n", inName.c_str(), width, height);
}

// Write an HDR image in .hdr (RGBE) format.
void write(const string outName, std::vector<float>& image, 
           const int width, const int height)
{
    rgbe_header_info info;
    char errbuf[100] = {0};

    // Open file and rite width and height to the header
    FILE* fp  =  fopen(outName.c_str(), "wb");
    int rc = RGBE_WriteHeader(fp, width, height, NULL, errbuf);
    if (rc != RGBE_RETURN_SUCCESS) {
        printf("RGBE write error: %s\n", errbuf);
        exit(-1); }

    // Writ the pixel data and close the file
    rc = RGBE_WritePixels_RLE(fp, &image[0], width,  height, errbuf);
    if (rc != RGBE_RETURN_SUCCESS) {
        printf("RGBE write error: %s\n", errbuf);
        exit(-1); }
    fclose(fp);
    
    printf("Wrote %s (%dX%d)\n", outName.c_str(), width, height);
}

int main(int argc, char** argv)
{    
    // Read in-file name from command line, create out-file name
    string inName = argv[1];
    string outName = inName.substr(0,inName.length()-4) + "-linear.hdr";

    std::vector<float> image;
    int width, height;
    read(inName, image, width, height);

    // For no good reason other than to demonstrate the manipulation
    // of an image, I'll Gamma correct the image to linear color
    // space.  Use gamma=2.2 if you have no specific gamma
    // information. Skip this step if you know the image is already in
    // linear color space.

    // This is included to demonstrate the magic of OpenMP: This
    // pragma turns the following loop into a multi-threaded loop,
    // making use of all the cores your machine may have.
    #pragma omp parallel for schedule(dynamic, 1) // Magic: Multi-thread y loop
    for (int j=0;  j<height;  j++) {
        for (int i=0;  i<width; i++) {
            int p = (j*width+i);
            for (int c=0;  c<3;  c++) {
                image[3*p+c] *= pow(image[3*p+c], 1.8); } } }

    // Write out the processed image.
    write(outName, image, width, height); 
}
