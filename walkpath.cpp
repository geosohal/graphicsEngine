#include "walkpath.h"
#include <algorithm>	// binary search
#include "utility.h"	// lerp

using namespace gl;
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
//using namespace gl;
WalkPath::WalkPath()
{
	alTableSize = 100;
	degree = 3;
	lastTableIndex = 1;	// start walking path at beginning of curve
	// Initialize pascals triangle
	int numberOfRows = 16;
	pascalsTriangle.resize(numberOfRows);
	for (int i = 0; i < numberOfRows; i++)
		pascalsTriangle[i].resize(numberOfRows);
	pascalsTriangle[0][0] = 1;

	// complete rows of triangle
	for (int r = 2; r <= numberOfRows; r++)
	{
		// pad row with ones
		pascalsTriangle[r - 1][ 0] = 1;
		pascalsTriangle[r - 1][ r - 1] = 1;

		// fill in colums in between the ones
		for (int j = 0; j <= r - 3; j++)
		{
			pascalsTriangle[r - 1][ j + 1] =
				pascalsTriangle[r - 2][ j] + pascalsTriangle[r - 2][ j + 1];
		}
	}
}

WalkPath::~WalkPath()
{

}

void WalkPath::InitializeControlPoints(float z, float scale, vec2 t)
{
	zValue = z;
	// insert predetermined set of control points and scale them down
	// so they fit on the scene
	ctrlPts.push_back(vec2(231, 469) * scale+t);  ResetKnotSeq();
	ctrlPts.push_back(vec2(230, 469) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(156, 451) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(94,383 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(106,224 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(286,159 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(476,409 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(689,442 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(764,283 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(680,136 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(528,151 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(393,411 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(333,451 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(237,489 ) * scale+t); ResetKnotSeq();
	ctrlPts.push_back(vec2(35,375 ) * scale+t); ResetKnotSeq();


	BuildALTable();
	vec2 last;
	vec2 next;

}

void WalkPath::ResetKnotSeq()
{
	knot.clear();
	int s = ctrlPts.size() - 1;
	int N = s + degree + 1;

	for (int i = degree; i < degree + N + 1; i++)
		knot.push_back((float)(i - degree));
}



int WalkPath::GetDeBoorInterval(float t)
{
	for (int q = 1; q < knot.size() - 1; q++)
	{
		if (t < knot[q])
			return q - 1;
		else if (t >= knot[knot.size() - 1])
			return knot.size() - 1;
	}
	return -1;
}

void WalkPath::Draw(int shaderId)
{
	float steps = ctrlPts.size() * 6;
	float alpha = 1 / steps;

	if (degree < ctrlPts.size())
	{
		int loc = gl::glGetUniformLocation(shaderId, "red");
		gl::glUniform1fARB(loc, .1);
		gl::glBegin(gl::GL_LINES);
		vec2 current_left;
		vec2 current_right = ctrlPts.at(0);

		int i;	// deboor interval
		for (float t = degree; t < ctrlPts.size(); t += alpha)
		{
			current_left = current_right;
			i = GetDeBoorInterval(t);
			current_right = DeBoor(t, degree, i);
			gl::glVertex3f(current_left[0], current_left[1], zValue);
			gl::glVertex3f(current_right[0], current_right[1], zValue);
		}
		gl::glEnd();
	}
	int loc = gl::glGetUniformLocation(shaderId, "red");
	gl::glUniform1fARB(loc, 1);
}

void WalkPath::DrawControlPts(int shaderId, BasicMesh* sphere, MAT4& VPmatrix)
{
	vec4 custColor = vec4(.6, .1, .1, .1);
	int loc = glGetUniformLocation(shaderId, "custColor");
	glUniform4fv(loc, 1, &custColor[0]);;

	for (int i = 0; i < ctrlPts.size(); i++)
	{
		MAT4 t = Translate(ctrlPts[i].x, ctrlPts[i].y, -4.f)* Scale(.2f, .2f, .2f);
		loc = glGetUniformLocation(shaderId, "gWVP");
		glUniformMatrix4fv(loc, 1, GL_TRUE, (VPmatrix*t).Pntr());
		sphere->Render();
	}

	custColor = vec4(.1, .1, .4, .1);
	loc = glGetUniformLocation(shaderId, "custColor");
	glUniform4fv(loc, 1, &custColor[0]);
}

float WalkPath::GetMaxArcLen()
{
	return alTable[alTable.size() - 1].arcLength;
}

float WalkPath::GetMinArcLen()
{
	return alTable[0].arcLength;
}


vec2 WalkPath::DeBoor(float t, int p, int i)
{
	if (p == 0)
	{
		return ctrlPts[i];
	}
	return DeBoor(t, p - 1, i) * ((t - knot[i]) / (knot[i + degree - p + 1] - knot[i]))
		+ DeBoor(t, p - 1, i - 1) * ((knot[i + degree - p + 1] - t) / (knot[i + degree - p + 1] - knot[i]));
}

vec2 WalkPath::GetPointAndDirFromArcLen(float s, vec2& dir)
{
	float t = GetParametericFromArcLen(s);
	int i = GetDeBoorInterval(t);
	vec2 currPos = DeBoor(t, degree, i);
	// calculate direction by getting the next point on the curve
	t = (t+.04 >= ctrlPts.size()) ? ctrlPts.size() : t+.04;
	vec2 nextPos = DeBoor(t, degree, i);
	dir = nextPos - currPos;
	return currPos;
}

// precondition: s is always in range [0, highest arc length]
float WalkPath::GetParametericFromArcLen(float s)
{
	// if the table index got passed the last element
	if (lastTableIndex >= alTable.size())
		lastTableIndex = 1;
	// if s value has exceeded the arc length of last known table index, increment index
	if (s > alTable[lastTableIndex].arcLength)
		lastTableIndex++;

	int nei;	// next table element index
	int pei;	// previous element index
	pei = lastTableIndex-1;
	nei = lastTableIndex;

	
	// arc length falls between the interval of previous and next indices into table
	if (s > alTable[pei].arcLength && s <= alTable[nei].arcLength)
	{
		float u = (s - alTable[pei].arcLength) / (alTable[nei].arcLength - alTable[pei].arcLength);
		return (1-u) * alTable[pei].paramatric + u * alTable[nei].paramatric;
	}
	else if (pei == 0 && s < alTable[nei].arcLength) 
	{	// if we're on the first index and arc length is less than first index
		return alTable[pei].paramatric;	// to debug
	}
	else
	{	// a row in the table was skipped over
		return GetParametericFromArcLen(s);
	}
}

// returns t, currtime/totaltime in [0,1]
float WalkPath::GetT()
{
	return currTime / timeToWalk;
}
// reset walker back to beginning curve position
void WalkPath::StartOver()
{
	currArcLengthDist = 1.1f;
	lastTableIndex = 1;
	currTime = 0;
}

void WalkPath::UpdateWalker(float timeElapsed)
{
	currTime += timeElapsed / 1000.f;	// update amount of time elapsed



	float t = GetT();	// get time on scale from 0 to 1
	if (t >= 1)	// if arc length reaches end
	{
		//printf("starting over\n");
		StartOver();
		t = 0;
	}
	float s = EaseInOut(t, .2f, .8f);	// get arclength position for current time in [0,1]
	// actual arclength distance on curve
	currArcLengthDist = (GetMaxArcLen()-GetMinArcLen()) * s + GetMinArcLen();	
	vec2 dir;	// direction walker is facing
	vec2 newPos = GetPointAndDirFromArcLen(currArcLengthDist, dir);
	dir = glm::normalize(dir);
	MAT4 r = Quaternion::FromAngleAxis(atan2(-dir.x, -dir.y), vec3(0, 0, 1)).ToMat4();
	vec2 lastPos = vec2(walkerTransform[0][3], walkerTransform[1][3]);
	distance = length(lastPos - newPos);	// distance moved on this update
	if (distance > .05)	// typical update moves walker by .05
		walkerSpeed = 1;
	else
		walkerSpeed = distance / .05;	// current speed ratio of the max speed
	walkerTransform = Translate(newPos.x, newPos.y, zValue + 3.96f) * r;
}

bool WalkPath::CompareALEntries(altEntry l, altEntry r)
{
	return l.arcLength < r.arcLength;
}

void WalkPath::BuildALTable()
{
	assert(ctrlPts.size() > degree);
	vec2 lastPoint;
	vec2 currPoint = ctrlPts.at(0);
	int i;	// deboor interval
	float arcLenSum = 0;
	alpha = (float)(ctrlPts.size()-degree) / alTableSize;

	// add first table element
	//altEntry firstElement;
	//firstElement.paramatric = degree;
	//firstElement.arcLength = arcLenSum;
	//firstElement.pos = currPoint;
	//alTable.push_back(firstElement);

	// add all other arc length table elements
	for (float t = degree ; t < ctrlPts.size(); t += alpha)
	{
		lastPoint = currPoint;
		i = GetDeBoorInterval(t);
		currPoint = DeBoor(t, degree, i);
		arcLenSum += glm::distance(lastPoint, currPoint);

		altEntry tableElement;
		tableElement.paramatric = t;
		tableElement.arcLength = arcLenSum;
		tableElement.pos = currPoint;
		alTable.push_back(tableElement);
	}
}