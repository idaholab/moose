# Free Energy Function Materials

The free energy functional is the basis of the evolution of the phase field variables. However, the
free energy itself is never used directly in the phase field equations, rather derivatives of the
free energy are required. In the case of a free energies involving multiple coupled variables,
different derivatives will be required for each residual equation.

In the free energy based model approach, a [Function Material](FunctionMaterials) class defines these
required free energy derivatives, which are used by the respective kernels. These derivatives can be
defined in the code by the user or they can be generated using automatic differentiation. The
derivatives can also come from CALPHAD type free energies. The required order of derivative that is
required for the residual are shown on the [Basic Phase Field Equations](phase_field/Phase_Field_Equations.md)
page for the Allen-Cahn equation and for the two solution approaches for the Cahn-Hilliard equations.
One additional derivative is required to define the Jacobian term used in the solution of the nonlinear
system of FEM equations. It is common to also define the full free energy, even when not required, for
visualization and debugging purposes.

To demonstrate this approach, we focus on a simple system with a single conserved concentration $c$
that varies from -1 to 1. The free energy in this case is

\begin{equation}
f_{loc} = \frac{1}{4}(1 + c)^2(1 - c)^2
\end{equation}

and its derivatives are

\begin{equation}
\begin{aligned}
  \frac{\partial f_{loc}}{\partial c} &=& c(c^2 - 1) \\
  \frac{\partial^2 f_{loc}}{\partial c^2} &=& 3 c^2 - 1 \\
  \frac{\partial^3 f_{loc}}{\partial c^3} &=& 6 c.
\end{aligned}
\end{equation}

The second and third derivatives would be required to use the direct solution method (via
[`CahnHilliard`](/CahnHilliard.md)) and the first and second derivatives would be required to use the
split solution method (via [`SplitCHParsed`](/SplitCHParsed.md)). This model has been implemented in
the [`MathFreeEnergy`](/MathFreeEnergy.md) material found in the phase field module of MOOSE,
inheriting from `DerivativeFunctionMaterialBase`. The code from
[`MathFreeEnergy`](/MathFreeEnergy.md) is shown below:

```cpp
Real
MathFreeEnergy::computeF()
{
  return 1.0/4.0*(1.0 + _c[_qp])*(1.0 + _c[_qp])*(1.0 - _c[_qp])*(1.0 - _c[_qp]);
}

Real
MathFreeEnergy::computeDF(unsigned int j_var)
{
    return _c[_qp]*(_c[_qp]*_c[_qp] - 1.0);
}

Real
MathFreeEnergy::computeD2F(unsigned int j_var, unsigned int k_var)
{
    return 3*_c[_qp]*_c[_qp] - 1.0;
}

Real
MathFreeEnergy::computeD3F(unsigned int j_var, unsigned int k_var, unsigned int l_var)
{
    return 6*_c[_qp];
}
```

An alternative to writing your own free energy materials is to take advantage of the
[`DerivativeParsedMaterial`](FunctionMaterials), where the free energy is entered in the input file
and all required derivatives are taken automatically. This approach is highly encouraged, as it
drastically simplifies model development.


## See also

- [Basic Phase Field Equations](phase_field/Phase_Field_Equations.md) - Basic information about the equations underlying the phase field module
- [Function Materials](FunctionMaterials) - The key component in the modular free energy phase field modeling approach. This page lists the available function materials and explains how to define a free energy function and combine multiple free energy contributions (including elastic energy) into a total free energy.
- [Function Material Kernels](FunctionMaterialKernels) - Kernels which utilize free energy densities provides by Function Material. These are the recommended phase field kernels.
- [ExpressionBuilder](ExpressionBuilder) - Use automatic differentiation with Free energies defined in the C++ code.
