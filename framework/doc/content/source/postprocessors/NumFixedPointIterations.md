# NumFixedPointIterations

!syntax description /Postprocessors/NumFixedPointIterations

Many problems, often multiphysics, need to be iterated to convergence. For steady state problem, it
can be done using a pseudo-transient, where the time step is incremented for each iteration. For problems that
are already transient in nature, it can also be done using a fixed point process. Each solve is iterated to
convergence. This postprocessor serves to measure the number of iterations needed to reach convergence.

## Example Input File Syntax

In this input file, we have a two-level multiphysics coupling, meaning that the parent app is
coupled with a sub-app, which is itself coupled with a sub-sub-app. We count the number of iterations
to converge the top level coupling with this postprocessor. The iterations taken to converge the bottom
level coupling are not counted, they would be counted by a similar postprocessor in the intermediate
sub-application.

!listing tests/multiapps/picard_multilevel/picard_parent.i block=Postprocessors/picard_its

!syntax parameters /Postprocessors/NumFixedPointIterations

!syntax inputs /Postprocessors/NumFixedPointIterations

!syntax children /Postprocessors/NumFixedPointIterations
