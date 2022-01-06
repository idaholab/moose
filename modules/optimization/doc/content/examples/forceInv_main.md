# Force Inversion Example

In a force inversion problem, we use inverse optimization to match the primary
variable (e.g. displacement or temperature) by parameterizing gradients of the
primary variables (e.g. force or heat flux).  The theory behind force inversion is given on this [page](getting_started/InvOptTheory.md). 
Force inversion is a linear optimization or
linear programming problem because the parameters are not dependent on the solution.
Examples for the following force inversion problems are given on the below pages for steady state 
heat conduction:

- [Example 1: Point Loads](forceInv_pointLoads.md)
- [Example 2: Neumann Boundary Condition](forceInv_NeumannBC.md)
- [Example 3: Distributed Body Load](forceInv_BodyLoad.md)
- [Example 4: Dirichlet Boundary Condition](forceInv_DirichletBC.md)
