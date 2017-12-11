// demonstrates a simnple physics simulation


#include "PhysicsSim.h"


using namespace std;


PhysicsSim::PhysicsSim()
{

}
PhysicsSim::~PhysicsSim()
{
	/*for (int i = 0; i < NBODIES; i++)
	{
		if (Bodies[i] != 0)
			delete Bodies[i];
			*/
}


// initialize variables of RBs and links
void PhysicsSim::InitializeSim(float mass, enum BODYTYPE bt, float xdim, float ydim,
	float zdim, vec3 pos, float linkLen)
{
	// initialize bodies from left to right (ascending x)
	for (int i = 0; i < NBODIES; i++)
	{
		Bodies[i].mass = mass;
		if (bt == BODYTYPE::BODYTYPE_BLOCK)
			Bodies[i].Ibody = GetBlockInertiaTensor(xdim, ydim, zdim);
		else
			throw("no other body types supported yet");
		Bodies[i].Ibodyinv = inverse(Bodies[i].Ibody);
		Bodies[i].x = pos + vec3(2.f*i,0,0);
		Bodies[i].q = quat();
		Bodies[i].P = vec3();
		Bodies[i].L = vec3();
		Bodies[i].R = mat3();
		Bodies[i].Iinv = Bodies[i].R * Bodies[i].Ibodyinv * glm::transpose(Bodies[i].R);
		Bodies[i].v = vec3();
		Bodies[i].omega = vec3(); // maybe should be an axis normalized
		Bodies[i].force = vec3();
		Bodies[i].torque = vec3();
	}

	// initialize links
	for (int i = 0; i < NBODIES - 1; i++)
	{
		Links[i].length = linkLen;
		Links[i].rbl = &Bodies[i];
		Links[i].rbr = &Bodies[i + 1];
	}

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
	*y++ = rb->q.w;
	*y++ = rb->q.x;
	*y++ = rb->q.y;
	*y++ = rb->q.z;

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
	rb->R = glm::toMat3(((quat)normalize(rb->q)));

	/* Compute auxiliary variables... */
	/* v(t) = P(t)
	M */
	rb->v = (rb->P / rb->mass);
	/* I−1(t) = R(t)I−1
	bodyR(t)T*/
	rb->Iinv = rb->R * rb->Ibodyinv * glm::transpose(rb->R);
	/* ω(t) = I−1(t)L(t) */
	rb->omega = rb->Iinv * rb->L;
}


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

	//todo need to remove matrix conversion here, and use what the paper said or i think the matrix
	// could cause problems as a quaternion can be 2 different matrices right? or at least can have
	// an accumulation of error which causes skewing.
	mat3 rdot = Star(rb->omega) * rb->R;
	quat qdot = normalize(glm::toQuat(rdot));
//	quat qdot = Star(rb->omega) * rb->q;
//	quat qdot = (rb->omega * rb->q)* rb->q;

	*xdot++ = qdot.w;
	*xdot++ = qdot.x;
	*xdot++ = qdot.y;
	*xdot++ = qdot.z;

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
	m[2][0] = -a[1]; m[2][1] = a[0]; m[2][2] = 0;
	return m;
}

// return inertia tensor matrix of a block with x y and z dimensions
mat3 GetBlockInertiaTensor(float x, float y, float z)
{
	mat3 m;
	m[0][0] = y*y+z*z; m[0][1] = 0; m[0][2] =0;
	m[1][0] = 0; m[1][1] = x*x+z*z; m[1][2] =0;
	m[2][0] = 0; m[2][1] = 0; m[2][2] = x*x+y*y;
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
