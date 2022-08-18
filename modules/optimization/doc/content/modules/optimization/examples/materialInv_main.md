# Material Inversion Examples

In a material inversion problem, inverse optimization is used to tune the material parameters to match the experimental data.  The examples in this section are for steady state heat conduction where material inversion will be used to determine thermal conductivity.  Material inversion problems result in a nonlinear optimization problem and are difficult to solve.  To make these problems easier for the optimization algorithms to solve, a close initial guess and tight bounds for the thermal conductivity are needed.  The theory for PDE constrained inverse optimization and its application thermal problems are available in the [theory section](getting_started/InvOptTheory.md).  Two material inversion problems will be presented in this section for steady state heat conduction where the following types of loadings are parameterized:

- [Example 1: Convective Boundary Conditions](materialInv_ConvectiveBC.md)
- [Example 2: Constant Thermal Conductivity](materialInv_ConstK.md)
- [Example 3: Temperature Dependent Thermal Conductivity](materialInv_TDepK.md)
