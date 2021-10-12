# Electrostatic Contact Verification (Two Block Test)

This document describes the two block 1-D verification test for the
[ElectrostaticContactCondition.md] object. Below is a summary of the test, along
with a derivation of the analytic solution used for comparison and the relevant
test input file for review.

## Summary

!style halign=left
A visual summary of the two block verification test domain, as well as relevant
boundary and interface conditions is shown below (click to zoom):

!media two_block.png
       style=width:100%;
       id=two-block-summary
       caption=Visual summary of the two block verification test with boundary and interface conditions.

It is important to note that in [two-block-summary]:

- $\sigma_{i}$ is the electrical conductivity of material $i$,
- $C_E$ is the electrical contact conductance at the interface, and
- $\phi_i$ is the electrostatic potential of material $i$.

See the [ElectrostaticContactCondition.md] documentation for more information
about the particular definition of $C_E$. As the [ElectrostaticContactCondition.md]
object is intended for electrostatic field solves, the PDE being solved within
each domain is Poisson's Equation for electrostatic potential. In this case, we
are assuming a zero total charge density, which leads to

!equation id=poissons
\nabla \cdot (\sigma_i \nabla \phi_i) = 0

Material properties being used in this case are constants in each block, and
they are summarized below in [two-block-mat-props]. All material properties were
evaluated at a temperature of ~300 K.

!table id=two-block-mat-props caption=Material properties for the two block electrostatic contact verification case.
| Property (unit) | Value | Source |
| - | - | - |
| Stainless Steel (304) Electrical Conductivity (S / m) | $1.41867 \times 10^6$ | [!citep](cincotti2007sps) |
| Stainless Steel (304) Hardness  (Pa) | $1.92 \times 10^9$ | [!citep](cincotti2007sps) |
| Graphite (AT 101) Electrical Conductivity (S / m) | $73069.2$ | [!citep](cincotti2007sps) |
| Graphite (AT 101) Hardness (Pa) | $3.5 \times 10^9$ | [!citep](cincotti2007sps) |

The hardness values shown in [two-block-mat-props] are used in the
[ElectrostaticContactCondition.md] object as a harmonic mean of the two values.
For reference, the harmonic mean calculation for two values, $V_a$ and $V_b$, is
given by

!equation
V_{Harm} = \frac{2 V_a V_b}{V_a + V_b}

In the input file, the harmonic mean of hardness for stainless steel and graphite
was calculated and set to be $2.4797 \times 10^9$ Pa.

## Analytic Solution Derivation

!style halign=left
In 1-D, [!eqref](poissons) becomes

!equation
\sigma_ i \frac{\text{d}^2 \phi_i}{\text{d} x^2} = 0

since $\sigma_i$ is constant in both domains. This equation means a reasonable
guess for a generic solution function for $\phi_i$ would be

!equation
\phi_i(x) = A_i x + C_i

where $A_i$ and $C_i$ are to-be-determined constant coefficients.

### Apply boundary conditions

!style halign=left
Using the boundary conditions in [two-block-summary], we can determine $C$ for
both the stainless steel and graphite regions:

!row!
!col! small=12 medium=6 large=6
!style halign=center
+Stainless Steel+

\begin{equation}
\begin{aligned}
\phi_S (0) &= A_S(0) + C_S \\
A_S(0) + C_S &= 1 \\
C_S &= 1
\end{aligned}
\end{equation}

!col-end!

!col! small=12 medium=6 large=6

!style halign=center
+Graphite+

\begin{equation}
\begin{aligned}
\phi_G (2) &= A_G(2) + C_G \\
A_G(2) + C_G &= 0 \\
C_G &= -2A_G
\end{aligned}
\end{equation}

!col-end!
!row-end!

### Apply interface conditions

!style halign=left
Now, the interface conditions can be applied from [two-block-summary]. To begin,
let's focus on the current density ($-\sigma_i \nabla \phi_i$) equivalence
condition

!equation
\sigma_S \left. \frac{\text{d} \phi_S}{\text{d} x} \right|_{x = 1} = \sigma_G \left. \frac{\text{d} \phi_G}{\text{d} x} \right|_{x = 1}

Taking into account our initial guess for the solution function, this becomes

!equation id=condition-one
\sigma_S A_S = \sigma_G A_G

Next, we can apply the conductance condition from [two-block-summary], which is

!equation
\sigma_S \left. \frac{\text{d} \phi_S}{\text{d} x} \right|_{x = 1} = -C_E (\phi_S(1) - \phi_G(1))

Taking into account our initial guess for the solution function and the constant
coefficients solved for above, this becomes

\begin{equation}
\begin{aligned}
\sigma_S A_S &= -C_E (A_S(1) + 1 - A_G (1 - 2)) \\
\sigma_S A_S &= -C_E (A_S + 1 + A_G)
\end{aligned}
\end{equation}

!equation id=condition-two
\sigma_S A_S = -C_E A_S - C_E - C_E A_G

### Find the remaining coefficients

!style halign=left
Using the Elimination Method, we can combine [!eqref](condition-one) and
[!eqref](condition-two) in order to solve for each remaining unknown coefficient.
If [!eqref](condition-one) is multiplied through on both sides by $C_E$ and
[!eqref](condition-two) is multiplied through on both sides by $\sigma_G$, we have

!equation id=condition-one-a
C_E \sigma_S A_S - C_E \sigma_G A_G = 0

and

!equation
\sigma_G \sigma_S A_S = -C_E \sigma_G A_S - C_E \sigma_G - C_E \sigma_G A_G

Grouping terms, we then have

!equation id=condition-two-a
(\sigma_G \sigma_S + C_E \sigma_G) A_S + C_E \sigma_G A_G = -C_E \sigma_G

Combining [!eqref](condition-one-a) and [!eqref](condition-two-a) together via
addition yields

!equation
(\sigma_G \sigma_S + C_E \sigma_G) A_S + C_E \sigma_S A_S = -C_E \sigma_G

which can then be solved for $A_S$

!equation
A_S = \frac{-C_E \sigma_G}{\sigma_G \sigma_S + C_E \sigma_G + C_E \sigma_S}

Using [!eqref](condition-one), $A_G$ can now be solved for

\begin{equation}
\begin{aligned}
\sigma_G A_G &= \sigma_S \left[ \frac{-C_E \sigma_G}{\sigma_G \sigma_S + C_E \sigma_G + C_E \sigma_S} \right] \\
A_G &= \frac{-C_E \sigma_S}{\sigma_G \sigma_S + C_E \sigma_G + C_E \sigma_S}
\end{aligned}
\end{equation}

### Summarize

!style halign=left
Now our determined coefficients can be combined to form the complete solutions for
both stainless steel and graphite. To summarize, the derived analytical solutions
for each domain given the boundary and interface conditions described in
[two-block-summary] is

\begin{equation}
\begin{aligned}
\phi_S (x) &= \frac{-C_E \sigma_G}{\sigma_G \sigma_S + C_E \sigma_G + C_E \sigma_S} x + 1 \\
\phi_G (x) &= \frac{-C_E \sigma_S}{\sigma_G \sigma_S + C_E \sigma_G + C_E \sigma_S} (x - 2)
\end{aligned}
\end{equation}

This is implemented in source code as `ElectricalContactTestFunc.C` and is
located within the test source code directory located at `modules/electromagnetics/test/src`.

## Input File

!listing analytic_solution_test_two_block.i

## Results

!style halign=left
Results from the input file shown above (with `Mesh/line/nx=30` and
`Outputs/exodus=true`) compared to the analytic function are shown below in
[two-block-results]. Note that the number of points shown in the plot has been
down-sampled compared to the solved number of elements for readability.

!media two_block_results.png
       style=width:50%;margin:auto;
       id=two-block-results
       caption=Results of electrostatic contact two block validation case.
