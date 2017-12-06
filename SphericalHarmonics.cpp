// Win32Project1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"




#include "SphericalHarmonics.h"
#include "rgbe.h"

#include <glm/glm.hpp>

SphericalHarmonics::SphericalHarmonics()
{
	;
}
// Read an HDR image in .hdr (RGBE) format.
void SphericalHarmonics::shread(const string inName, std::vector<float>& image,
	int& width, int& height)
{
	rgbe_header_info info;
	char errbuf[100] = { 0 };

	// Open file and read width and height from the header
	FILE* fp;
	fopen_s(&fp, inName.c_str(), "rb");
	if (!fp) {
		printf("Can't open file: %s\n", inName.c_str());
		exit(-1);
	}
	int rc = RGBE_ReadHeader(fp, &width, &height, &info, errbuf);
	if (rc != RGBE_RETURN_SUCCESS) {
		printf("RGBE read error: %s\n", errbuf);
		exit(-1);
	}

	// Allocate enough memory
	image.resize(3 * width*height);

	// Read the pixel data and close the file
	rc = RGBE_ReadPixels_RLE(fp, &image[0], width, height, errbuf);
	if (rc != RGBE_RETURN_SUCCESS) {
		printf("RGBE read error: %s\n", errbuf);
		exit(-1);
	}
	fclose(fp);

	printf("Read %s (%dX%d)\n", inName.c_str(), width, height);
}

// Write an HDR image in .hdr (RGBE) format.
void SphericalHarmonics::shwrite(const string outName, std::vector<float>& image,
	const int width, const int height)
{
	rgbe_header_info info;
	char errbuf[100] = { 0 };

	// Open file and rite width and height to the header
	FILE* fp;
	fopen_s(&fp, outName.c_str(), "wb");
	int rc = RGBE_WriteHeader(fp, width, height, NULL, errbuf);
	if (rc != RGBE_RETURN_SUCCESS) {
		printf("RGBE write error: %s\n", errbuf);
		exit(-1);
	}

	// Writ the pixel data and close the file
	rc = RGBE_WritePixels_RLE(fp, &image[0], width, height, errbuf);
	if (rc != RGBE_RETURN_SUCCESS) {
		printf("RGBE write error: %s\n", errbuf);
		exit(-1);
	}
	fclose(fp);

	printf("Wrote %s (%dX%d)\n", outName.c_str(), width, height);
}




void SphericalHarmonics::InitializeSH()
{
	// Read in-file name from command line, create out-file name
	string inName = "Alexs_Apt_2k.hdr";
	string outName = inName.substr(0, inName.length() - 4) + "-irrad.hdr";
	printf("reading image..\n");
	std::vector<float> image;
	int width, height;
	shread(inName, image, width, height);
	printf("processing image..\n");



	std::vector<float> rSums;	// these sums are the coefficients for the respective basis function
	std::vector<float> gSums;
	std::vector<float> bSums;
	std::vector<float> Yconst;	// constant portion of the 9 basis functions
	Yconst.push_back(.5f * std::sqrt(1 / MPI));	// Y0,0
	rSums.push_back(0);
	gSums.push_back(0);
	bSums.push_back(0);
	// Y 1,-1
	Yconst.push_back(.5f*std::sqrt(3 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);
	// Y 1, 0
	Yconst.push_back(.5f*std::sqrt(3 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);
	// Y 1, 1
	Yconst.push_back(.5f*std::sqrt(3 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);
	// Y 2, -2
	Yconst.push_back(.5f*std::sqrt(15 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);
	// Y 2, -1
	Yconst.push_back(.5f*std::sqrt(15 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);
	// Y 2, 0
	Yconst.push_back(.25f*std::sqrt(5 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);
	// Y 2, 1
	Yconst.push_back(.5f*std::sqrt(15 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);
	// Y 2,2
	Yconst.push_back(.25f*std::sqrt(15 / MPI));
	rSums.push_back(0); gSums.push_back(0); bSums.push_back(0);

	// This is included to demonstrate the magic of OpenMP: This
	// pragma turns the following loop into a multi-threaded loop,
	// making use of all the cores your machine may have.
	//#pragma omp parallel for schedule(dynamic, 1) // Magic: Multi-thread y loop

	float spherearea = 0;
	//area of unit sphere
	for (int j = 0; j < height; j++)
	{
		float deltaTheta = MPI / height;
		float deltaPhi = MPI2 / width;
		for (int i = 0; i < width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			spherearea += sin(theta)* deltaTheta*deltaPhi;
		}
	}
	float max = .1234555555555555555232334234f;

	// Y 0,0
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			int p = (j*width + i);
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			rSums[0] += image[3 * p + 0] * Yconst[0] * sin(theta)* deltaTheta*deltaPhi;
			gSums[0] += image[3 * p + 1] * Yconst[0] * sin(theta)* deltaTheta*deltaPhi;
			bSums[0] += image[3 * p + 2] * Yconst[0] * sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");
	// Y 1,-1
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float y = 0;
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			YfromThetaPhi(y, theta, WidthIndexToPhi(i, width));
			int p = (j*width + i);
			rSums[1] += image[3 * p + 0] * Yconst[1] * y* sin(theta)* deltaTheta*deltaPhi;
			gSums[1] += image[3 * p + 1] * Yconst[1] * y* sin(theta)* deltaTheta*deltaPhi;
			bSums[1] += image[3 * p + 2] * Yconst[1] * y* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");// Y 1,0
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			float z = 0;
			ZfromThetaPhi(z, theta, WidthIndexToPhi(i, width));


			int p = (j*width + i);
			rSums[2] += image[3 * p + 0] * Yconst[2] * z* sin(theta)* deltaTheta*deltaPhi;
			gSums[2] += image[3 * p + 1] * Yconst[2] * z* sin(theta)* deltaTheta*deltaPhi;
			bSums[2] += image[3 * p + 2] * Yconst[2] * z* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");// Y 1,1
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			float x = 0;
			XfromThetaPhi(x, theta, WidthIndexToPhi(i, width));
			int p = (j*width + i);
			rSums[3] += image[3 * p + 0] * Yconst[3] * x* sin(theta)* deltaTheta*deltaPhi;
			gSums[3] += image[3 * p + 1] * Yconst[3] * x* sin(theta)* deltaTheta*deltaPhi;
			bSums[3] += image[3 * p + 2] * Yconst[3] * x* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");// Y 2,-2
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			float x = 0;
			float y = 0;
			XfromThetaPhi(x, theta, WidthIndexToPhi(i, width));
			YfromThetaPhi(y, theta, WidthIndexToPhi(i, width));
			int p = (j*width + i);
			rSums[4] += image[3 * p + 0] * Yconst[4] * x*y* sin(theta)* deltaTheta*deltaPhi;
			gSums[4] += image[3 * p + 1] * Yconst[4] * x*y* sin(theta)* deltaTheta*deltaPhi;
			bSums[4] += image[3 * p + 2] * Yconst[4] * x*y* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");// Y 2,-1
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			float z = 0;
			float y = 0;
			ZfromThetaPhi(z, theta, WidthIndexToPhi(i, width));
			YfromThetaPhi(y, theta, WidthIndexToPhi(i, width));
			int p = (j*width + i);
			rSums[5] += image[3 * p + 0] * Yconst[5] * z*y* sin(theta)* deltaTheta*deltaPhi;
			gSums[5] += image[3 * p + 1] * Yconst[5] * z*y* sin(theta)* deltaTheta*deltaPhi;
			bSums[5] += image[3 * p + 2] * Yconst[5] * z*y* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");// Y 2,0
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			float z = 0;
			ZfromThetaPhi(z, theta, WidthIndexToPhi(i, width));
			int p = (j*width + i);
			rSums[6] += image[3 * p + 0] * Yconst[6] * (z*z * 3 - 1)* sin(theta)* deltaTheta*deltaPhi;
			gSums[6] += image[3 * p + 1] * Yconst[6] * (z*z * 3 - 1)* sin(theta)* deltaTheta*deltaPhi;
			bSums[6] += image[3 * p + 2] * Yconst[6] * (z*z * 3 - 1)* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");//Y 2,1
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			float z = 0;
			float x = 0;
			ZfromThetaPhi(z, theta, WidthIndexToPhi(i, width));
			XfromThetaPhi(x, theta, WidthIndexToPhi(i, width));
			int p = (j*width + i);
			rSums[7] += image[3 * p + 0] * Yconst[7] * z*x* sin(theta)* deltaTheta*deltaPhi;
			gSums[7] += image[3 * p + 1] * Yconst[7] * z*x* sin(theta)* deltaTheta*deltaPhi;
			bSums[7] += image[3 * p + 2] * Yconst[7] * z*x* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf(".");//Y 2,2
	for (int j = 0; j<height; j++)
	{
		for (int i = 0; i<width; i++)
		{
			float theta = HeightIndexToTheta(j, height);
			float deltaTheta = MPI / height;
			float deltaPhi = MPI2 / width;
			float y = 0;
			float x = 0;
			YfromThetaPhi(y, theta, WidthIndexToPhi(i, width));
			XfromThetaPhi(x, theta, WidthIndexToPhi(i, width));
			int p = (j*width + i);
			rSums[8] += image[3 * p + 0] * Yconst[8] * (x*x - y*y)* sin(theta)* deltaTheta*deltaPhi;
			gSums[8] += image[3 * p + 1] * Yconst[8] * (x*x - y*y)* sin(theta)* deltaTheta*deltaPhi;
			bSums[8] += image[3 * p + 2] * Yconst[8] * (x*x - y*y)* sin(theta)* deltaTheta*deltaPhi;
		}
	}
	printf("coeffs done..\n");
	int radwidth = 400;
	int radheight = 200;
	float deltatheta = MPI / radheight;
	float deltaphi = MPI2 / radwidth;
	std::vector<float> irradimage;
	irradimage.resize(3 * radwidth*radheight);
	for (int j = 0; j < radheight; j++)
	{
		for (int i = 0; i < radwidth; i++)
		{
			float x, y, z;
			DirVectorFromThetaPhi(x, y, z, HeightIndexToTheta(j, radheight), WidthIndexToPhi(i, radwidth));
			glm::vec3 dir = glm::vec3(x, y, z);
			dir = glm::normalize(dir);
			x = dir.x;
			y = dir.y;
			z = dir.z;
			int p = (j*radwidth + i);
			for (int c = 0; c < 9; c++)
			{
				float hRatio = ((float)j / radheight);
				float wRatio = ((float)i / radwidth);
				int h = floor(hRatio*height);
				int w = floor(wRatio*width);
				int p2 = (h*width + w);

				float basisFactor = 1 * MPI;
				if (c == 1)
					basisFactor = y * (2.f / 3.f)*MPI;
				if (c == 2)
					basisFactor = z* (2.f / 3.f)*MPI;
				if (c == 3)
					basisFactor = x* (2.f / 3.f)*MPI;
				if (c == 4)
					basisFactor = x*y * (1.f / 4.f)*MPI;
				if (c == 5)
					basisFactor = y*z * (1.f / 4.f)*MPI;
				if (c == 6)
					basisFactor = (3.f * z*z - 1.f) * (1.f / 4.f)*MPI;
				if (c == 7)
					basisFactor = x*z * (1.f / 4.f)*MPI;
				if (c == 8)
					basisFactor = (x*x - y*y) * (1.f / 4.f)*MPI;


				irradimage[3 * p + 0] += rSums[c] * Yconst[c] * basisFactor;
				irradimage[3 * p + 1] += gSums[c] * Yconst[c] * basisFactor;
				irradimage[3 * p + 2] += bSums[c] * Yconst[c] * basisFactor;
			}
		}
	}
	printf("writing image..\n");
	// Write out the processed image.
	shwrite(outName, irradimage, radwidth, radheight);
	printf("complete..\n");
}
