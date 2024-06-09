# Electrostatic Contact Verification (Three Block Test)

This document describes the three block 1-D verification test for the
[ElectrostaticContactCondition.md] object. Below is a summary of the test, along
with a derivation of the analytic solution used for comparison and the relevant
test input file for review.

## Summary

!style halign=left
A visual summary of the three block verification test domain, as well as relevant
boundary and interface conditions is shown below (click to zoom):

!media three_block.png
       style=width:100%;
       id=three-block-summary
       caption=Visual summary of the three block verification test with boundary and interface conditions.

It is important to note that in [three-block-summary]:

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
they are summarized below in [three-block-mat-props]. All material properties were
evaluated at a temperature of ~300 K.

!table id=three-block-mat-props caption=Material properties for the three block electrostatic contact verification case.
| Property (unit) | Value | Source |
| - | - | - |
| Stainless Steel (304) Electrical Conductivity (S / m) | $1.41867 \times 10^6$ | [!citep](cincotti2007sps) |
| Stainless Steel (304) Hardness  (Pa) | $1.92 \times 10^9$ | [!citep](cincotti2007sps) |
| Graphite (AT 101) Electrical Conductivity (S / m) | $73069.2$ | [!citep](cincotti2007sps) |
| Graphite (AT 101) Hardness (Pa) | $3.5 \times 10^9$ | [!citep](cincotti2007sps) |

The hardness values shown in [three-block-mat-props] are used in the
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

since $\sigma_i$ is constant in all domains. This equation means a reasonable
guess for a generic solution function for $\phi_i$ would be

!equation
\phi_i(x) = A_i x + C_i

where $A_i$ and $C_i$ are to-be-determined constant coefficients.

### Apply boundary conditions

!style halign=left
Using the boundary conditions in [three-block-summary], we can determine $C_i$
for both of the outer stainless steel regions:

!row!
!col! small=12 medium=6 large=6
!style halign=center
+Stainless Steel (left)+

\begin{equation}
\begin{aligned}
\phi_{S1} (0) &= A_{S1}(0) + C_{S1} \\
A_{S1}(0) + C_{S1} &= 1 \\
C_{S1} &= 1
\end{aligned}
\end{equation}

!col-end!

!col! small=12 medium=6 large=6

!style halign=center
+Stainless Steel (right)+

\begin{equation}
\begin{aligned}
\phi_{S3} (3) &= A_{S3} (3) + C_{S3} \\
A_{S3} (3) + C_{S3} &= 0 \\
C_{S3} &= -3A_{S3}
\end{aligned}
\end{equation}

!col-end!
!row-end!

### Apply interface conditions

!style halign=left
Now, the interface conditions can be applied from [three-block-summary]. To begin,
let's focus on the current density ($-\sigma_i \nabla \phi_i$) equivalence
condition on the left interface (at $x = 1$):

!equation
\sigma_S \left. \frac{\text{d} \phi_{S1}}{\text{d} x} \right|_{x = 1} = \sigma_G \left. \frac{\text{d} \phi_G}{\text{d} x} \right|_{x = 1}

Taking into account our initial guess for the solution function, this becomes

!equation id=condition-one
\sigma_S A_{S1} = \sigma_G A_G

Next, we can apply the conductance condition from [three-block-summary], which is

!equation
\sigma_S \left. \frac{\text{d} \phi_{S1}}{\text{d} x} \right|_{x = 1} = -C_E (\phi_{S1}(1) - \phi_G(1))

Taking into account our initial guess for the solution function and the constant
coefficient $C_{S1}$ solved for above, this becomes

\begin{equation}
\begin{aligned}
\sigma_S A_{S1} &= -C_E (A_{S1}(1) + 1 - A_G (1) - C_G) \\
\sigma_S A_{S1} &= -C_E (A_{S1} + 1 - A_G - C_G) \\
\sigma_S A_{S1} &= -C_E A_{S1} - C_E + C_E A_G + C_E C_G \\
\sigma_S A_{S1} + C_E A_{S1} - C_E A_G &= - C_E + C_E C_G
\end{aligned}
\end{equation}

Grouping terms, we have

!equation id=condition-two
(\sigma_S + C_E) A_{S1} - C_E A_G = - C_E + C_E C_G

Next, let's focus on the current density ($-\sigma_i \nabla \phi_i$) equivalence
condition on the right interface (at $x = 2$):

!equation
\sigma_G \left. \frac{\text{d} \phi_G}{\text{d} x} \right|_{x = 2} = \sigma_S \left. \frac{\text{d} \phi_{S3}}{\text{d} x} \right|_{x = 2}

Taking into account our initial guess for the solution function, this becomes

!equation id=condition-three
\sigma_G A_G = \sigma_S A_{S3}

Next, we can apply the conductance condition from [three-block-summary], which is

!equation
\sigma_G \left. \frac{\text{d} \phi_G}{\text{d} x} \right|_{x = 2} = -C_E (\phi_G(2) - \phi_{S3}(2))

Taking into account our initial guess for the solution function and the constant
coefficient $C_{S3}$ solved for above, this becomes

\begin{equation}
\begin{aligned}
\sigma_G A_G &= -C_E (A_G (2) + C_G - A_{S3} (2 - 3)) \\
\sigma_G A_G &= -C_E (2A_G + C_G + A_{S3}) \\
\sigma_G A_G &= -2 C_E A_G - C_E C_G - C_E A_{S3} \\
\sigma_G A_G + 2 C_E A_G + C_E A_{S3} &= - C_E C_G \\
\end{aligned}
\end{equation}

Grouping terms, we have

!equation id=condition-four
(\sigma_G + 2 C_E) A_G + C_E A_{S3} = - C_E C_G

### Find the remaining coefficients

!style halign=left
Using the Elimination Method and some algebra, we can combine [!eqref](condition-one),
[!eqref](condition-two), [!eqref](condition-three), and [!eqref](condition-four)
in order to solve for each remaining unknown coefficient. To begin, let's focus
on [!eqref](condition-one) and [!eqref](condition-two). Multiplying
[!eqref](condition-one) through by $C_E$ and [!eqref](condition-two) through by
$-\sigma_G$ yields

!equation id=condition-one-a
C_E \sigma_S A_{S1} - C_E \sigma_G A_G = 0

and

!equation id=condition-two-a
-\sigma_G (\sigma_S + C_E) A_{S1} + \sigma_G C_E A_G =  \sigma_G C_E - \sigma_G C_E C_G

Combining [!eqref](condition-one-a) and [!eqref](condition-two-a) together via
addition yields

!equation
\sigma_S C_E A_{S1} - \sigma_G (\sigma_S + C_E) A_{S1} =  \sigma_G C_E - \sigma_G C_E C_G

which can then be solved for $A_{S1}$:

\begin{equation}
\begin{aligned}
(\sigma_S C_E - \sigma_G (\sigma_S + C_E)) A_{S1} &=  \sigma_G C_E - \sigma_G C_E C_G \\
(\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E) A_{S1} &=  \sigma_G C_E - \sigma_G C_E C_G
\end{aligned}
\end{equation}

!equation id=a-stainless-one
A_{S1} =  \frac{\sigma_G C_E - \sigma_G C_E C_G}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E}

Next, we focus on [!eqref](condition-three) and [!eqref](condition-four).
Multiplying [!eqref](condition-three) through by $C_E$ and
[!eqref](condition-four) through by $\sigma_S$ yields

!equation id=condition-three-a
\sigma_G C_E A_G - \sigma_S C_E A_{S3} = 0

and

!equation id=condition-four-a
\sigma_S (\sigma_G + 2 C_E) A_G + \sigma_S C_E A_{S3} = - \sigma_S C_E C_G

Combining [!eqref](condition-three-a) and [!eqref](condition-four-a) together via
addition yields

\begin{equation}
\begin{aligned}
\sigma_G C_E A_G + \sigma_S (\sigma_G + 2 C_E) A_G &= - \sigma_S C_E C_G \\
\sigma_G C_E A_G + \sigma_S \sigma_G A_G + 2 \sigma_S C_E A_G &= - \sigma_S C_E C_G
\end{aligned}
\end{equation}

which can then be solved for $A_G$:

!equation
(\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E) A_G = - \sigma_S C_E C_G

!equation id=a-graphite
A_G = \frac{-\sigma_S C_E C_G}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E}

Note that [!eqref](a-stainless-one) and [!eqref](a-graphite) still depend on
finding the coefficient $C_G$. We can now solve for $C_G$ by using
[!eqref](condition-one), which yields

\begin{equation}
\begin{aligned}
\sigma_S \left[ \frac{\sigma_G C_E - \sigma_G C_E C_G}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \right] &= \sigma_G \left[ \frac{-\sigma_S C_E C_G}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} \right] \\
\frac{\sigma_S \sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} - \left[ \frac{\sigma_S \sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \right] C_G + \left[ \frac{\sigma_G \sigma_S C_E}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} \right] C_G &= 0 \\
\left[ \frac{\sigma_G \sigma_S C_E}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} \right] C_G - \left[ \frac{\sigma_S \sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \right] C_G &= \frac{-\sigma_S \sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \\
\left[ \frac{\sigma_G \sigma_S C_E}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} - \frac{\sigma_S \sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \right] C_G &= \frac{-\sigma_S \sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \\
\left[ \frac{1}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} - \frac{1}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G C_E} \right] C_G &= \frac{-1}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \\
\left[ \frac{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G C_E}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} - \frac{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G C_E} \right] C_G &= -1 \\
\left[ \frac{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G C_E}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} - 1 \right] C_G &= -1 \\
\left[ \sigma_S C_E - \sigma_G \sigma_S - \sigma_G C_E - \left( \sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E \right) \right] C_G &= -\left( \sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E \right)\\
\left( -\sigma_S C_E - 2 \sigma_G \sigma_S - 2 \sigma_G C_E \right) C_G &= -\left( \sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E \right)
\end{aligned}
\end{equation}

and finally

!equation id=c-graphite-final
C_G = \frac{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E}{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G C_E}

Returning to [!eqref](a-graphite), we can now fully solve for $A_G$:

!equation
A_G = \frac{-\sigma_S C_E}{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E} \left[ \frac{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E}{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G C_E} \right]

Simplifying yields:

!equation id=a-graphite-final
A_G = \frac{-\sigma_S C_E}{2 \sigma_G C_E + 2 \sigma_G \sigma_S + \sigma_S C_E}

Returning to [!eqref](a-stainless-one), we can now fully solve for $A_{S1}$:

\begin{equation}
\begin{aligned}
A_{S1} &=  \frac{\sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \left[ 1 - \frac{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E}{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G C_E} \right] \\
A_{S1} &=  \frac{\sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \left[ \frac{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G  C_E}{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G C_E} - \frac{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E}{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G  C_E} \right] \\
A_{S1} &=  \frac{\sigma_G C_E}{\sigma_S C_E - \sigma_G \sigma_S - \sigma_G  C_E} \left[ \frac{-\sigma_S C_E + \sigma_G \sigma_S + \sigma_G  C_E}{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G C_E} \right]
\end{aligned}
\end{equation}

Simplifying yields:

!equation id=a-stainless-one-final
A_{S1} =  \frac{-\sigma_G C_E}{2 \sigma_G C_E + 2 \sigma_G \sigma_S + \sigma_S C_E}

Returning to [!eqref](condition-three), we can now fully solve for $A_{S3}$:

\begin{equation}
\begin{aligned}
\sigma_G A_G &= \sigma_S A_{S3} \\
A_{S3} &= \frac{\sigma_G}{\sigma_S} A_G \\
A_{S3} &= \frac{\sigma_G}{\sigma_S} \left[ \frac{-\sigma_S C_E}{2 \sigma_G C_E + 2 \sigma_G \sigma_S + \sigma_S C_E} \right]
\end{aligned}
\end{equation}

!equation id=a-stainless-three-final
A_{S3} = \frac{-\sigma_G C_E}{2 \sigma_G C_E + 2 \sigma_G \sigma_S + \sigma_S C_E}

### Summarize

!style halign=left
Now our determined coefficients can be combined to form the complete solutions for
both stainless steel and graphite. To summarize, the derived analytical solutions
for each domain given the boundary and interface conditions described in
[three-block-summary] is:

- For stainless steel from $x = 0$ to $x = 1$:

!equation
\phi_{S1}(x) = \frac{-\sigma_G C_E}{2 \sigma_G C_E + 2 \sigma_G \sigma_S + \sigma_S C_E} x + 1

- For graphite from $x = 1$ to $x = 2$:

!equation
\phi_G(x) = \frac{-\sigma_S C_E}{2 \sigma_G C_E + 2 \sigma_G \sigma_S + \sigma_S C_E} x + \frac{\sigma_G C_E + \sigma_S \sigma_G + 2 \sigma_S C_E}{\sigma_S C_E + 2 \sigma_G \sigma_S + 2 \sigma_G C_E}

- For stainless steel from $x = 2$ to $x = 3$:

!equation
\phi_{S3}(x) = \frac{-\sigma_G C_E}{2 \sigma_G C_E + 2 \sigma_G \sigma_S + \sigma_S C_E} (x - 3)

This is implemented in source code as `ElectricalContactTestFunc.C` and is
located within the test source code directory located at `modules/electromagnetics/test/src`.

## Input File

!listing analytic_solution_test_three_block.i

## Results

!style halign=left
Results from the input file shown above (with `Mesh/line/nx=60` and
`Outputs/exodus=true`) compared to the analytic function are shown below in
[three-block-results]. Note that the number of points shown in the plot has been
down-sampled compared to the solved number of elements for readability.

!media three_block_results.png
       style=width:50%;margin:auto;
       id=three-block-results
       caption=Results of electrostatic contact three block validation case.
