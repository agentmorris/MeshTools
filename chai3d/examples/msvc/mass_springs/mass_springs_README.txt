/****

 CHAI Example: 05_mass_springs
 
 Author: Dan Morris

****/

This example demonstrates some basic dynamics in CHAI, and a custom 
force loop.  Hold on tight when you enable haptics; the simulation 
of course flips out for just a bit while it gets used to where your
haptic device is.

Relevant items to look at in the code (besidres what was already discussed
in the README.txt file for the previous examples) are :

* mass_springs.cpp is the main application file, which includes
  initialization and the haptic and graphic rendering loops.

  mass_springs.h defines the CBall and CSpring classes, which are
  standard models of masses that can be connected by linear springs.

* The compute_springs() function defines the dynamic integration, where
  linear spring forces, gravity, and damping forces are applied to 
  each ball.  This function is called from the graphic rendering loop
  if the haptic loop is running; once haptics are enabled, this function
  is called from the haptic loop.

* The mass_springs_haptic_iteration() function gets the current haptic
  device position by calling updatePose(), invokes compute_springs(), then
  takes the position applied to ball #0 and applies it directly to the
  haptic device.
