# Simulation

Simulation is one of the two base classes of a [THMProblem.md]. It implements a number of the specificities
of a thermal hydraulics solve, notably the reliance on [Components](syntax/Components/index.md) to form the problem.

It is in charge of :

- building the mesh, each part being created by each [Component](syntax/Components/index.md)
- creating the variables
- setting up initial conditions for the variables, whether from file or from user input
- setting up the quadrature

It also has APIs, that are called by various actions when parsing the input file, to:

- add components
- initialize components
- add MOOSE objects created by components
- identify component loops
- set up the coordinate system


It also performs integrity checks for:

- the coupling matrix, making sure it sufficiently captures inter-variable dependencies
  for convergence
- the components, making sure they all have a single inlet and a single outlet, and calling
  their own integrity checks
- control data, from [ControlLogic](syntax/ControlLogic/index.md) objects, making sure all the request control data
  does exist, and adding the proper dependencies
