CS 560 Animation Project 4 Physically Based Simulation
By Harjott Sohal
geosohal@gmail.com

================================================

------------------------------------------------
Compiling
------------------------------------------------
1. open framework.sln with VS2015
2. Compile and run as Release and x86


------------------------------------------------
Keyboard and Mouse controls
------------------------------------------------
-Mouse click and drag to rotate camera
-WASD to move left anchor
-TFGH to move right anchor 
	(similar layout to WASD but different keys)
-Y to move right anchor towards camera
-R to move right anchor away from camera


------------------------------------------------
Algorithms and code
------------------------------------------------
Relevant files for this project are in the solution
directory, under names:
	PhysicsSim.cpp
	PhysicsSim.h
	Scene.cpp 
	
The program demonstrates a physics simulation with
5 sticks connected by springs. It uses Runge-Kutta 
integration method.

PhysicsSim Implementation details:
The simulation starts with InitializeSim() function
which initializes rigid body variables and copies
the bodies to their array form, where the state
variables can be used for later calculations.
UpdateSim() is then called to update the simulation.
from UpdateSim() all other functions are called,
including the integrator rungekuttastep(), which
then also calls dxdt() where forces and torques 
are calculated and stored in the xdot state array.


Extra credit attempt:
a few additions in scene lighting were made with
image-based-lighting and a skydome. It can be seen
when the right anchor is brought near the camera using
the Y key.