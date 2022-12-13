# Force Inversion Examples

In a force inversion problem, we use inverse optimization to match the primary
variable (e.g. displacement or temperature) by parameterizing gradients of the
primary variables (e.g. force or heat flux).  The theory behind force inversion is given in this [section](theory/InvOptTheory.md#sec:forceInv) on the theory page.
Force inversion is a linear optimization or
linear programming problem where the parameters being controlled are not dependent on the solution, see [!eqref](theory/InvOptTheory.md#eq:bodyLoads).
Examples for the following force inversion problems are given on the below pages for steady state
heat conduction:

- [Theory](theory/InvOptTheory.md)
- [Example 1: Point Loads](forceInv_pointLoads.md)
- [Example 2: Neumann Boundary Condition](forceInv_NeumannBC.md)
- [Example 3: Distributed Body Load](forceInv_BodyLoad.md)
- [debuggingHelp.md]
