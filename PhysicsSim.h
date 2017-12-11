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
class PhysicsSim
{
public:

	enum BODYTYPE
	{
		BODYTYPE_BLOCK,
		BODYTYPE_SPHERE
	};

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
	};
	struct Link
	{
		float length;
		RigidBody *rbl;
		RigidBody *rbr;
	};

	const float gravity = 9.81f;

	PhysicsSim();
	~PhysicsSim();

	// initialize state variables of RBs and lnks
	void InitializeSim(float mass, enum BODYTYPE bt, float xdim, float ydim,
		float zdim, vec3 pos, float linkLen);
	void StateToArray(RigidBody *rb, float *y);
	void ArrayToState(RigidBody *rb, float *y);
	void ArrayToBodies(float x[]);
	void BodiesToArray(float x[]);
	//ComputeForceAndTorquetakes into account all forces and torques: gravity, wind, interaction with other bodies etc
	void ComputeForceAndTorque(float t, RigidBody *rb);
	// dxdt is called by numerical solver ode (euler or RK) and is responsible for allocating enough space for the
	//arrays x, and xdot(STATE_SIZE · NBODIES worth for each).
	void Dxdt(float t, float x[], float xdot[]);
	// The function which does the real work of computing d/dt X(t) and storing it in the array xdot
	void DdtStateToArray(RigidBody *rb, float *xdot);
	// returns matrix form of angular vel which is used to calculate Rdot
	glm::mat3 Star(vec3 a);

	void UpdateSim(float elapsedSec, float timeSinceUpdate);


private:
	// ODE ordrinary differential equation solver
	void eulerstep(float *x0, float *xFinal, int arrSize, float t0, float t1);
	mat3 GetBlockInertiaTensor(float xScale, float yScale, float zScale);

	RigidBody Bodies[NBODIES];
	Link Links[NBODIES-1];


};