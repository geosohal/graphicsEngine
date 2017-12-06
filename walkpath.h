// WalkPath.h
// class generates curve based on predetermined control points using the
// DeBoor algorithm which has C2 continuity. 
// class also handles arc length parameterization in a table that is built
// as a preprocessor step. The table elements are stepped through by
// the user and they are interpolated as the users animation moves from
// one point to the next

#include <glm/glm.hpp>
#include <math.h>
#include <vector>
#include <glbinding/Binding.h>
#include <glbinding/gl/gl.h>
#include "Quaternion.h"	// rotating walker while on path
#include "mesh.h"
using namespace glm;
using namespace std;
class WalkPath
{
public:
	struct altEntry	// arc length table entry
	{
		float paramatric;	// parametric entry, starts at t = degree, rather than 0
		float arcLength;	// cumulative arc length for this parametric entry
		vec2 pos;			// position of point on curve
	};


	WalkPath();
	~WalkPath();
	void InitializeControlPoints(float z, float scale, vec2 translate);
	void Draw(int shaderId);	// draws path lines and control points
	void DrawControlPts(int shaderId, BasicMesh* mesh, MAT4& vp);	// use different shader for ctrl pt spheres
	// given an arc length returns the previous and next points, and the 
	// t value for the curve is returned as an interpolation
	float GetParametericFromArcLen(float s);
	vec2 GetPointAndDirFromArcLen(float s, vec2& dir);
	float GetMaxArcLen();
	float GetMinArcLen();	// starting arc length. 
	void UpdateWalker(float timeElapsed);
	vector<vec2> ctrlPts;	// control points with x and y coords
	float zValue;			// the common z value of all control points
	float GetT();	// returns t, currtime/totaltime in [0,1]
	void StartOver(); // start path over from beginning
	// last value used to index into table to retrieve pos. ex: if lastindex = 1,
	// then the current arc length value is in the range of the positions
	// from index 0 to index 1
	int lastTableIndex;
	float timeToWalk = 27.f;	// time required to finish walk around the path
	float currTime = 0;	//current time spent walking on path
	float currArcLengthDist; //current arc length position of walker
	float walkerSpeed;	// in range from 0 to 1, where 1 is peak speed 
	MAT4 walkerTransform;
	float distance;
private:
	void BuildALTable();
	void ResetKnotSeq();// resets knots after ctrl point is added
	int GetDeBoorInterval(float t);	// deboor curve helper function
	vec2 DeBoor(float t, int p, int i);	// returns deboor curve pos at parameter t

	// for binary search by arc length (elements ordered by arcLength value)
	bool CompareALEntries(altEntry l, altEntry r); 


	vector<vector<int>> pascalsTriangle; // used for creating deboor curve
	vector<float> knot;		// knot seq for deboor
	vector<altEntry> alTable;	// arc length table
	int degree;	// degree of deboor subsplines
	int alTableSize;	// initialize arclength table to this size. (not actual size)
	float alpha;	// amount to increment parameter for each element in AL table


};