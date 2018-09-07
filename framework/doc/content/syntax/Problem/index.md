# Problem system overview

The Problem class is one of the core extension points in MOOSE. Problems
are designed to hold the `EquationSystems` objects (from libMesh) that
house the numerical systems we are ultimately solving for our computing.

Fortunately when you are first getting started with MOOSE you normally
don't have to worry or do anything special with the Problem object. MOOSE
automatically constructs a suitable Problem for your simulation type
taking into account the other types of objects present in the input file.
Most simulations use the `FEProblem` class, which contains a single
`NonlinearSystem` and single `AuxiliarySystem` or a single `MooseEigenSystem`
and single `AuxiliarySystem`. The `NonlinearSystem` or `MooseEigenSystem`
contains the matrix and vectors used for solving the equations implemented
through a combination of other objects in your simulations (`Kernels`, `BCs`,
etc.). The `AuxiliarySystem` houses the solution vectors use to hold
computed solutions or values.

As your application grows in complexity, you may find it useful or
necessary to create your own problems to extend default behavior provided
by the core MOOSE framework. Common examples include, specialized
convergence tests, etc.

# Automatic Problem Creation

The automatic problem creation is handled for you by MOOSE. In a normal
input file that does not contain a special `[Problem]` block, MOOSE
will create a suitable Problem for you. If however, you need to change
specific system related parameters you may find yourself adding a
`[Problem]` block with name/value pairs. Different types of Problems
may be instantiated by using the `/Problem/type` whose default value is
`FEProblem`.

# Problem System

!syntax list /Problem objects=True actions=False subsystems=False

!syntax list /Problem objects=False actions=False subsystems=True

!syntax list /Problem objects=False actions=True subsystems=False
