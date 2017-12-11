// demonstrates a simnple physics simulation


#include "PhysicsSim.h"


using namespace std;


PhysicsSim::PhysicsSim()
{

}
PhysicsSim::~PhysicsSim()
{


}



// initialize state variables of RBs and convert bodies to array so we can run simulation
void PhysicsSim::InitializeSim()
{


	// todo initialize rb variables
}
void PhysicsSim::StateToArray(RigidBody *rb, float *y)
{
	*y++ = rb->x[0]; /* x component of position */
	*y++ = rb->x[1]; /* etc. */
	*y++ = rb->x[2];

	/*
	* Assume that a quaternion is represented in
	* terms of elements ‘r’ for the real part,
	* and ‘i’, ‘j’, and ‘k’ for the vector part.
	*/
	*y++ = rb->q.s;
	*y++ = rb->q.v.x;
	*y++ = rb->q.v.y;
	*y++ = rb->q.v.z;

	*y++ = rb->P[0];
	*y++ = rb->P[1];
	*y++ = rb->P[2];
	*y++ = rb->L[0];
	*y++ = rb->L[1];
	*y++ = rb->L[2];
}
void PhysicsSim::ArrayToState(RigidBody *rb, float *y)
{
	rb->x[0] = *y++;
	rb->x[1] = *y++;
	rb->x[2] = *y++;
	rb->P[0] = *y++;
	rb->P[1] = *y++;
	rb->P[2] = *y++;
	rb->L[0] = *y++;
	rb->L[1] = *y++;
	rb->L[2] = *y++;
	rb->R = rb->q.Normalize().ToMat4g();
}
	/* Compute auxiliary variables... */
	/* v(t) = P(t)
	M */
	rb->v = rb->P / rb->mass;
	/* I−1(t) = R(t)I−1
	bodyR(t)T*/
	rb->Iinv = R * Ibodyinv * Transpose(R);
	/* ω(t) = I−1(t)L(t) */
	rb->omega = rb->Iinv * rb->L;


void PhysicsSim::ArrayToBodies(float x[])
{
	for (int i = 0; i < NBODIES; i++)
		ArrayToState(&Bodies[i], &x[i * STATE_SIZE]);
}

void PhysicsSim::BodiesToArray(float x[])
{
	for (int i = 0; i < NBODIES; i++)
		StateToArray(&Bodies[i], &x[i * STATE_SIZE]);
}

//ComputeForceAndTorquetakes into account all forces and torques: gravity, wind, interaction with other bodies etc
void PhysicsSim::ComputeForceAndTorque(float t, RigidBody *rb)
{

}
// dxdt is called by numerical solver ode (euler or RK) and is responsible for allocating enough space for the
//arrays x, and xdot(STATE_SIZE · NBODIES worth for each).
void PhysicsSim::Dxdt(float t, float x[], float xdot[])
{
	/* put data in x[] into Bodies[] */
	ArrayToBodies(x);
	for (int i = 0; i < NBODIES; i++)
	{
		ComputeForceAndTorque(t, &Bodies[i]);
		DdtStateToArray(&Bodies[i],
			&xdot[i * STATE_SIZE]);
	}
}

// The function which does the real work of computing d/dt X(t) and storing it in the array xdot
void PhysicsSim::DdtStateToArray(RigidBody *rb, float *xdot)
{
	/* copy d
	dt x(t) = v(t) into xdot */
	*xdot++ = rb->v[0];
	*xdot++ = rb->v[1];
	*xdot++ = rb->v[2];
	/* Compute R˙(t) = ω(t)∗R(t) */
	Quaternion qdot = .5 * (rb->omega * rb->q);
	*xdot++ = qdot.r;
	*xdot++ = qdot.i;
	*xdot++ = qdot.j;
	*xdot++ = qdot.k;

	*xdot++ = rb->force[0]; /* d
							dt P(t) = F(t) */
	*xdot++ = rb->force[1];
	*xdot++ = rb->force[2];
	*xdot++ = rb->torque[0]; /* d
							 dt L(t) = τ(t) */
	*xdot++ = rb->torque[1];
	*xdot++ = rb->torque[2];
}

// returns matrix form of angular vel which is used to calculate Rdot
glm::mat3 PhysicsSim::Star(vec3 a)
{
	glm::mat3 m;
	m[0][0] = 0; m[0][1] = -a[2]; m[0][2] = a[1];
	m[1][0] = a[2]; m[1][1] = 0 ; m[1][2] = -a[0];
	m[2][0] = -a[1]; m[2][1] = a[0]; m[2][2] = 0
}

void PhysicsSim::UpdateSim(float elapsedSec, float timeSinceUpdate)
{
	float x0[STATE_SIZE * NBODIES],
		xFinal[STATE_SIZE * NBODIES];

	// todo init rb state varaibles
	BodiesToArray(xFinal);

	/* copy xFinal back to x0 */
	for (int i = 0; i < STATE_SIZE * NBODIES; i++)
	{
		// todo add spring forces too rb's
		x0[i] = xFinal[i];
		eulerstep(x0, xFinal, STATE_SIZE * NBODIES,
			elapsedSec, elapsedSec+ timeSinceUpdate);

		/* copy ddtX(t + 1/24 ) into state variables */
		ArrayToBodies(xFinal);
	}
	

}

// ODE ordrinary differential equation solver
void PhysicsSim::eulerstep(float *x0, float *xFinal, int arrSize, float t0, float t1)
{

}
