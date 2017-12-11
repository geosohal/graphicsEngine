// demonstrates a simnple physics simulation

#include <glm/glm.hpp>
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
	struct RigidBody {
		/* Constant quantities */
		float mass; /* mass M */
		mat4 Ibody; /* Ibody */
		mat4 Ibodyinv; /* I−1 body (inverse of Ibody) */
					 
			/* State variables */
		vec3 x; /* x(t) */
		quat q; /* R(t) */
		vec3 P; /* P(t) */
		vec3 L; /* L(t) */

			/* Derived quantities (auxiliary variables) */
		mat4 Iinv; /* I−1(t) */
		mat4 R;	/* R(t) */
		vec3 v; /* v(t) */
		vec3 omega; /* ω(t) */

			/* Computed quantities */
		vec3 force; /* F(t) */
		vec3 torque; /* τ(t) */
	};

	PhysicsSim();
	~PhysicsSim();

	// initialize state variables of RBs and convert bodies to array so we can run simulation
	void InitializeSim();
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

	// ODE ordrinary differential equation solver
	void eulerstep(float *x0, float *xFinal, int arrSize, float t0, float t1);

	RigidBody Bodies[];

};