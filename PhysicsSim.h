// demonstrates a simnple physics simulation

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <math.h>
#include <glbinding/Binding.h>
#include <glbinding/gl/gl.h>
#include "Quaternion.h"	//
#include "mesh.h"

using namespace glm;
using namespace std;
#define STATE_SIZE 13
#define NBODIES 5
#define BLOCKSPACING 5.5f
class PhysicsSim
{
public:

	enum BODYTYPE
	{
		BODYTYPE_BLOCK,
		BODYTYPE_SPHERE
	};

// todo notes rb v in ComputeForceAndTorque(float t, RigidBody *rb, int i)
	// may be wrong because its not using the end point velocity, just the
	// rb velocity. we need to use omega.

	struct RigidBody {
		/* Constant quantities */
		float mass; /* mass M */
		mat3 Ibody; /* Ibody */
		mat3 Ibodyinv; /* I−1 body (inverse of Ibody) */
					 
			/* State variables */
		vec3 x; /* x(t) */
		glm::quat q; /* R(t) */
		vec3 P; /* P(t) */
		vec3 L; /* L(t) */

			/* Derived quantities (auxiliary variables) */
		mat3 Iinv; /* I−1(t) */
		mat3 R;	/* R(t) */
		vec3 v; /* v(t) */
		vec3 omega; /* ω(t) */

			/* Computed quantities */
		vec3 force; /* F(t) */
		vec3 torque; /* τ(t) */
		float xhExtent; // sticks are connected along x-extends. 
						// this is the half extent
		vec3 lEndPt;	// left end point where spring attaches
		vec3 rEndPt;	// right "
		vec3 lEndPtVel;	// left end point velocity
		vec3 rEndPtVel;	// right "
	};
	struct Link
	{
		float length;	// spring rest length
		RigidBody *rbl;
		RigidBody *rbr;
		vec3 forceL;	// force on leftside (-x) of spring
		vec3 forceR;	// force on rightside (+x) of spring
	};

	const float g = 9.1f;
	const float springStiffness =9.09f;
	const float damperCoeff =9.55f;

	PhysicsSim();
	~PhysicsSim();

	// initialize state variables of RBs and lnks, blocks are initialized from left to right
	// left is -x, right is +x, this assume horizontal layout but oh well
	void InitializeSim(float mass, enum BODYTYPE bt, float xdim, float ydim,
		float zdim, vec3 pos);
	void StateToArray(RigidBody *rb, float *y);
	void ArrayToState(RigidBody *rb, float *y);
	void ArrayToBodies(float x[]);
	void BodiesToArray(float x[]);
	//ComputeForceAndTorquetakes into account all forces and torques: gravity, wind, interaction with other bodies etc
	void ComputeForceAndTorque(float t, RigidBody *rb, int i);
	// dxdt is called by numerical solver ode (euler or RK) and is responsible for allocating enough space for the
	//arrays x, and xdot(STATE_SIZE · NBODIES worth for each).
	void Dxdt(float t, float x[], float xdot[]);
	// The function which does the real work of computing d/dt X(t) and storing it in the array xdot
	void DdtStateToArray(RigidBody *rb, float *xdot);
	// returns matrix form of angular vel which is used to calculate Rdot
	glm::mat3 Star(vec3 a);

	void UpdateSim(float elapsedSec, float timeSinceUpdate);
	quat ReflectedQuaternion(int i);

	vec3 anchorLeft;
	vec3 anchorRight;
	RigidBody Bodies[NBODIES];
	float x0[STATE_SIZE * NBODIES];
	float xFinal[STATE_SIZE * NBODIES];
	// runge kutta copies of state variables:
	float k1[STATE_SIZE * NBODIES];
	float k2[STATE_SIZE * NBODIES];
	float k3[STATE_SIZE * NBODIES];
	float k4[STATE_SIZE * NBODIES];
	float xtemp[STATE_SIZE * NBODIES];
private:
	// ODE ordrinary differential equation solver
	void eulerstep(float *x0, float *xFinal, int arrSize, float t0, float t1);
	void rungekuttastep(float *x0, float *xFinal, int arrSize, float t0, float t1);
	mat3 GetBlockInertiaTensor(float xScale, float yScale, float zScale);
	vec3 GetEndPt(bool isRight, RigidBody* rb);
	void StateMultT(float *state, float t);
	void StateAdd(float *state1, float *state2, float state2Coeff, float *results);
	void StateAdd(float *x1, float *x2, float *x3, float *x4, float *xFinal);
	void ClearStates();	// clears all state arrays from last update
	
	Link Links[NBODIES-1];



};