#ifndef _SPHERICALHARMONICS_
#define _SPHERICALHARMONICS_
// reads in hdr image and write irradiance map image
#include "math.h"
#include <vector>
#include <iostream>
#include <string.h>
#include <assert.h> 


using namespace std;
const float MPI = 3.14159265359f;
const float MPI2 = 6.28318530718f;
class SphericalHarmonics
{
public:
	SphericalHarmonics();
	void shread(const string inName, std::vector<float>& image,
		int& width, int& height);

	void shwrite(const string outName, std::vector<float>& image,
		const int width, const int height);
	void InitializeSH();

private:

	float WidthIndexToPhi(int wi, int totalWidth)
	{
		return MPI2 * ((float)wi + .5f) / (float)totalWidth;
	}

	float HeightIndexToTheta(int hi, int totalHeight)
	{
		return MPI * ((float)hi + .5f) / (float)totalHeight;
	}

	void DirVectorFromThetaPhi(float& x, float& y, float& z, float theta, float phi)
	{
		x = sin(theta) * cos(phi);
		y = sin(theta) * sin(phi);
		z = cos(theta);
	}
	void YfromThetaPhi(float& y, float theta, float phi)
	{
		y = sin(theta) * sin(phi);
	}
	void ZfromThetaPhi(float& z, float theta, float phi)
	{
		z = cos(theta);
	}
	void XfromThetaPhi(float& x, float theta, float phi)
	{
		x = sin(theta) * cos(phi);
	}
};

#endif
