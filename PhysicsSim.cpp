// demonstrates a simnple physics simulation


#include "PhysicsSim.h"


using namespace std;

#define ZERO_MEM(a) memset(a, 0, sizeof(a))


PhysicsSim::PhysicsSim()
{
	ZERO_MEM(Bodies);
	ZERO_MEM(x0);
	ZERO_MEM(xFinal);
	ZERO_MEM(Links);
}

PhysicsSim::~PhysicsSim()
{
	/*for (int i = 0; i < NBODIES; i++)
	{
		if (Bodies[i] != 0)
			delete Bodies[i];
			*/
}


// initialize variables of RBs and links, assumes: models have CoM at their origin,
// spacing between rb's will be done on x axis by BLOCKSPACING, the first rb
// wil lbe placed furthest left
void PhysicsSim::InitializeSim(float mass, enum BODYTYPE bt, float xdim, float ydim,
	float zdim, vec3 pos)
{
	// initialize bodies from left to right (ascending x)
	for (int i = 0; i < NBODIES; i++)
	{
		Bodies[i].mass = mass;
		if (bt == BODYTYPE::BODYTYPE_BLOCK)
			Bodies[i].Ibody = mass* GetBlockInertiaTensor(xdim, ydim, zdim);
		else
			throw("no other body types supported yet");
		Bodies[i].Ibodyinv = (1/mass)* inverse(Bodies[i].Ibody);
		Bodies[i].x = pos + vec3(BLOCKSPACING*i+xdim,0,0);
		Bodies[i].q = quat();
		Bodies[i].P = vec3();
		Bodies[i].L = vec3();
		Bodies[i].R = mat3();
		Bodies[i].Iinv = Bodies[i].R * Bodies[i].Ibodyinv * glm::transpose(Bodies[i].R);
		Bodies[i].v = vec3();
		Bodies[i].omega = vec3(); // maybe should be an axis normalized
		Bodies[i].force = vec3();
		Bodies[i].torque = vec3();
		Bodies[i].xhExtent = xdim;
	}
	anchorLeft = Bodies[0].x + vec3(-BLOCKSPACING-xdim, 0, 0);
	anchorRight = Bodies[NBODIES-1].x + vec3(BLOCKSPACING+xdim, 0, 0);
	float linkLen = BLOCKSPACING - xdim;
	// initialize links
	for (int i = 0; i < NBODIES - 1; i++)
	{
		Links[i].length = linkLen;
		Links[i].rbl = &Bodies[i];
		Links[i].rbr = &Bodies[i + 1];
	}
	// copy body to state variables array so we can begin running simulation
	BodiesToArray(xFinal);
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
	rb->q.w = *y++;
	rb->q.x = *y++;
	rb->q.y = *y++;
	rb->q.z = *y++;

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


vec3 PhysicsSim::GetEndPt(bool isRight, RigidBody* rb)
{
	if (isRight)
		return rb->x + rb->q * (rb->x+vec3(rb->xhExtent, 0, 0));
	else
		return rb->x + rb->q * (rb->x+vec3(-rb->xhExtent, 0, 0));
}

//ComputeForceAndTorquetakes into account all forces and torques: gravity, wind, interaction with other bodies etc
// todo minimize half extent quaternion mults
void PhysicsSim::ComputeForceAndTorque(float t, RigidBody *rb, int i)
{
	
	// calculate linear force applied on stick's two end points by adjacent spring(s)
	vec3 forceEndL;	// linear force on stick's left end pt
	// force from stretch
	forceEndL += springStiffness * ( (i == 0 ? anchorLeft : Bodies[i - 1].rEndPt)-rb->lEndPt );	
	forceEndL += .5f * rb->mass * g * vec3(0, 0, -1) ;	// force from gravity
	// force from velocities
	forceEndL += damperCoeff * ((i == 0 ? vec3(0, 0, 0) : Bodies[i - 1].rEndPtVel) - Bodies[i].lEndPtVel);
	vec3 forceEndR;	// linear force on stick's left end pt
	forceEndR += springStiffness * ((i == NBODIES-1 ? anchorRight : Bodies[i + 1].lEndPt) - rb->rEndPt);
	forceEndR += .5f * rb->mass * g * vec3(0, 0, -1) ;	// force from gravity
	forceEndR += damperCoeff * ((i == NBODIES-1 ? vec3(0, 0, 0) : Bodies[i + 1].lEndPtVel) - Bodies[i].rEndPtVel);

	rb->force = (forceEndR + forceEndL);

	// angular force or torque exerted on the sticks two endpoints by adjacent spring(s)
	vec3 leftTorque = cross(rb->lEndPt - rb->x, forceEndL);
	vec3 rightTorque = cross(rb->rEndPt - rb->x, forceEndR);
	rb->torque = (leftTorque + rightTorque);

}

// dxdt is called by numerical solver ode (euler or RK) and is responsible for allocating enough space for the
//arrays x, and xdot(STATE_SIZE · NBODIES worth for each).
void PhysicsSim::Dxdt(float t, float x[], float xdot[])
{
	/* put data in x[] into Bodies[] */
	ArrayToBodies(x);

	//update end point positions of rb's where springs attach
	for (int i = 0; i < NBODIES; i++)
	{
		Bodies[i].lEndPt = GetEndPt(0, &Bodies[i]);
		Bodies[i].rEndPt = GetEndPt(1, &Bodies[i]);
	}

	// update ent point velocities
	for (int i = 0; i < NBODIES; i++)
	{
		// calculate velocities of sticks end points, accounting for both linear and angular components
		Bodies[i].lEndPtVel = Bodies[i].v + cross(Bodies[i].omega, Bodies[i].lEndPt - Bodies[i].x);
		Bodies[i].rEndPtVel = Bodies[i].v + cross(Bodies[i].omega, Bodies[i].rEndPt - Bodies[i].x);
	}

	// compute torques and forces and store data in array xdot
	for (int i = 0; i < NBODIES; i++)
	{
		ComputeForceAndTorque(t, &Bodies[i], i);
		DdtStateToArray(&Bodies[i],
			&xdot[i * STATE_SIZE]);
	}
	// compute spring forces

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
	quat rdotq = normalize(glm::toQuat(rdot));	// quaternion version of rdot
//	quat qdot = Star(rb->omega) * rb->q;
//	quat qdot = (rb->omega * rb->q)* rb->q;

	*xdot++ = rdotq.w;
	*xdot++ = rdotq.x;
	*xdot++ = rdotq.y;
	*xdot++ = rdotq.z;

	*xdot++ = rb->force[0]; /* derivative of P(t) = F(t) */
	*xdot++ = rb->force[1];
	*xdot++ = rb->force[2];
	*xdot++ = rb->torque[0]; /* derivative of L(t) = τ(t) */
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
mat3 PhysicsSim::GetBlockInertiaTensor(float x, float y, float z)
{
	mat3 m;
	m[0][0] = y*y+z*z; m[0][1] = 0; m[0][2] =0;
	m[1][0] = 0; m[1][1] = x*x+z*z; m[1][2] =0;
	m[2][0] = 0; m[2][1] = 0; m[2][2] = x*x+y*y;
	return m;
}


void PhysicsSim::UpdateSim(float elapsedSec, float timeSinceUpdate)
{


//	for (double t = 0; t < 10.0; t += 1. / 24.)
	{
		/* copy xFinal back to x0, as x0 is the last state to begin at */
		for (int i = 0; i < STATE_SIZE * NBODIES; i++)
		{
			x0[i] = xFinal[i];
		}

		eulerstep(x0, xFinal, STATE_SIZE * NBODIES,
			elapsedSec, elapsedSec + timeSinceUpdate);

		/* copy ddtX(t + 1/24 ) into state variables */
		ArrayToBodies(xFinal);
	}
	
	

}

// ODE ordrinary differential equation solver
void PhysicsSim::eulerstep(float *x, float *xdot, int arrSize, float t0, float t1)
{
	Dxdt(t1 - t0, x0, xFinal);
	float t = t1 - t0;
	for (int i = 0; i < NBODIES; i++)
	{
		/* position  */
		*xdot++ = (*x++) +(*xdot)*t; 
		*xdot++ = (*x++) + (*xdot)*t;
		*xdot++ = (*x++) + (*xdot)*t;

		quat qf;// todo apply same to rotation
		qf.w = *xdot;
		qf.x = *(xdot+1);
		qf.y = *(xdot + 2);
		qf.z = *(xdot + 3);
		quat q0;
		q0.w = *x++;
		q0.x = *x++;
		q0.y = *x++;
		q0.z = *x++;
		qf = normalize(q0 * qf);

		*xdot++ = qf.w;	// quaternion
		*xdot++ = qf.x;
		*xdot++ = qf.y;
		*xdot++ = qf.z;
// P, P(t)
		*xdot++ = (*x++) + (*xdot)*t;
		*xdot++ = (*x++) + (*xdot)*t;
		*xdot++ = (*x++) + (*xdot)*t;
// L
		*xdot++ = (*x++) + (*xdot)*t;
		*xdot++ = (*x++) + (*xdot)*t;
		*xdot++ = (*x++) + (*xdot)*t;

	}
}
