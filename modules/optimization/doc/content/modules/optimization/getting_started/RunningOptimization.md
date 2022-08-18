# Obtaining and Running the MOOSE optimization module

## Executing the MOOSE optimization module

When first starting out with the MOOSE optimization module, it is recommended to start from an
example problem similar to the problem that you are trying to solve.
Multiple examples can be found in `modules/optimization/test/tests/`.
It may be worth running the example problems to see how the code works
and modifying input parameters to see how the run time, results and
convergence behavior change.

To demonstrate running the MOOSE optimization module, consider running one of the regression tests:

```bash
cd ~/projects/moose/modules/optimization/test/tests/objective_minimize/bc_load_constant
# To run with one processor
~/projects/moose/modules/optimization/optimization-opt -i main.i
# To run in parallel (2 processors)
mpiexec -n 2 ~/projects/moose/modules/optimization/optimization-opt -i main.i
```

Note that the procedure for running this model in parallel is shown only
for illustrative purposes. This particular model is quite small, and would
not benefit from being run in parallel, although it can be run that way.

## Input to the MOOSE optimization module

Optimization simulation models are defined by the user through a text file
that defines the parameters of the run.  This text file specifies the
set of code objects that are composed together to simulate a physical
problem, and provides parameters that control how those objects behave
and interact with each other.  This text file can be prepared using any
text editor.

In addition to the text file describing the model parameters, MOOSE also
requires a definition of the finite element mesh on which the physics
equations are solved. The mesh can be generated internally by MOOSE using
parameters defined in MOOSE's input file for very simple geometries, or can
be read from a file as defined in the MOOSE input file.

## Post Processing

MOOSE typically writes solution data to an ExodusII file. Data may also
be written in other formats, a simple comma separated file giving global
data being the most common.

Several options exist for viewing ExodusII results files. These include
commercial as well as open-source tools. One good choice is ParaView,
which is open-source.

ParaView is available on a variety of platforms. It is capable of
displaying node and element data in several ways. It will also produce
line plots of global data or data from a particular node or element.
Detailed information on ParaView is available on its project
[website](https://www.paraview.org).
