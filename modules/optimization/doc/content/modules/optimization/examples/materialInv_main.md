# Material Inversion Examples

In a material inversion problem, inverse optimization is used to control material parameters to match the experimental data.  The examples in this section are for steady state heat conduction where material inversion will be used to determine thermal conductivity.  Material inversion problems result in a nonlinear optimization problem and are difficult to solve.  To make these problems easier for the optimization algorithms to solve, a close initial guess and tight bounds for the thermal conductivity are needed.  Methods for computing a Hessian for material inversion problems has not been implemented and only gradient based optimization algorithms can be used. The following examples do not use regularization which would help with convergence issues during optimization.  The theory for these types of PDE constrained inversion problems are given on the theory page for [convective boundary condition inversion](theory/InvOptTheory.md#id=sec:robinInv) and [material inversion](theory/InvOptTheory.md#sec:material_inversion).  Two material inversion problems will be presented in this section for steady state heat conduction where the following types of material properties are parameterized:

- [Theory](theory/InvOptTheory.md)
- [Example 1: Convective Boundary Conditions](materialInv_ConvectiveBC.md)
- [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md)
- [debuggingHelp.md]
