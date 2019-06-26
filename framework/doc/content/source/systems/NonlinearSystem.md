# NonlinearSystem

The NonlinearSystem object holds the equation system created by the normal FEM process
(e.g. the Matrix and RHS vector) to be solved. Normally MOOSE uses PETSc to store and
solve this system. This object is where you will find the callback routines used
by the PETSc solvers.