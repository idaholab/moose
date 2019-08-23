# Volumetric Locking Correction

Volumetric locking is the over stiffening of elements when the material is close to being
incompressible (Poisson's ratio $\nu$ nearing 0.5). This stiffening happens when a fully integrated element
(such as Hex8 elements with 8 quadrature points or Quad4 elements with 4 quadrature points) is
used. This is a numerical artifact introduced because shape functions used in finite element analysis
cannot properly approximate the incompressibility condition throughout the element. To avoid this
locking of elements, B-bar correction [!cite](hughes1987finite) is implemented in MOOSE.

## Theory

In this method, both the strain ($\epsilon$) and the virtual strain ($\delta \epsilon$) in an element
are separated into volumetric and deviatoric components. The volumetric component is then replaced
with an element averaged volumetric strain. This ensures that the volumetric strain remains constant
throughout the element.

For example, in the case of small strain linear elasticity, the equation of motion is:
\begin{equation}
\begin{aligned}
\int_V \sigma(\epsilon)\delta \epsilon dV - \int_V b \delta v dV - \int_{\partial V} t \delta v dA = 0
\end{aligned}
\end{equation}
The element averaged volumetric strain (assuming small strain formulation) is:
\begin{equation}
\begin{aligned}
 w=\frac{1}{V_e} \int_{V_e} \frac{1}{3} tr(\epsilon) dV,
\end{aligned}
\end{equation}
where $V_e$ is the volume of the element and tr(.) is the trace of the matrix.

The strain in each element is replaced by the approximation:
\begin{equation}
\begin{aligned}
\bar{\epsilon} = \epsilon +(w - \frac{tr(\epsilon)}{3})I
\end{aligned}
\end{equation}
where $I$ is the $3 \times 3$ identity matrix. Similarly, the virtual strain is also approximated by:
\begin{equation}
\begin{aligned}
\bar{\delta \epsilon} = \delta \epsilon + (\delta w - \frac{tr(\delta \epsilon)}{3})I
\end{aligned}
\end{equation}
The modified equation of motion is:
\begin{equation}
\begin{aligned}
\int_V \sigma(\bar{\epsilon})\bar{\delta \epsilon} dV - \int_V b \delta v dV - \int_{\partial V} t \delta v dA = 0
\end{aligned}
\end{equation}
More details about this method can be found in [Section 8.6](http://solidmechanics.org/Text/Chapter8_6/Chapter8_6.php) of [!cite](bower2009applied).

When finite strain formulation is used, the volumetric component of the strain is separated using the
determinant of the deformation matrix.

## Usage

Volumetric locking correction is set to false by default in tensor mechanics. When dealing with
problems involving plasticity or incompressible materials, it can be turned on by setting
`volumetric_locking_correction=true` in both the stress divergence kernel and the strain calculator
or in the Tensor Mechanics master action.

When volumetric locking correction is turned on, using a SMP preconditioner with coupled displacement
variables may help with convergence. For a 3-D problem with only displacement as unknown variables,
the following pre-conditioner block may be used:

!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/elastic_rotation_test.i
         start=Preconditioning
         end=Executioner

## Verification of locking correction on Cook's membrane

A 2D trapezoidal membrane [fig_cook] with Poisson's ratio of 0.4999 is fixed at the left edge and sheared at the right edge. Locking behavior of the membrane is observed with the use of first order (Quad4) elements with no volumetric locking correction. This locking results in much lower vertical displacement at Point A than that observed in other scenarios, see [fig_cook_results] . Locking can be avoided with the use of Quad4 elements along with volumetric locking correction or with the use of second order elements (Quad8) with or without volumetric locking correction. These results match with that presented in Fig. 6 of [!cite](nakshatrala2008fem).

!row!

!media media/tensor_mechanics/cook_problem.png
      style=width:45%;float:left;
      id=fig_cook
      caption=2D problem to demonstrate volumetric locking.

!media media/tensor_mechanics/cook_results.png
      style=width:45%;float:right;
      id=fig_cook_results
      caption= Vertical displacement at Point A for different element types and mesh density. Locking behavior is observed when Quad4 elements with no volumetric locking correction are used.

!row-end!

Note that at least 20 nodes per edge are required to converge to the correct solution even when volumetric locking correction is used for this problem. Also, second order elements do not display locking behavior, so volumetric locking correction is not required for second order elements. Using volumetric locking correction with second order elements causes a different convergence pattern for the displacements [fig_cook_results] and it is also shown to result in noisy zigzag pattern in stress or strain profiles.

!bibtex bibliography
