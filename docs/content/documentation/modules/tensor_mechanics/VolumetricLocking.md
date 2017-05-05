#Volumetric locking correction

Volumetric locking is the over stiffening of elements when the material is close to being incompressible (Poisson's ratio $\nu$ nearing 0.5). This happens when a fully integrated element (such as Hex8 elements with 8 quadrature points or Quad4 elements with 4 quadrature points) is used. This is a numerical artifact introduced because shape functions used in finite element analysis cannot properly approximate the incompressibility condition throughout the element. To avoid this locking of elements, B-bar correction \citep{hughes1987finite} is implemented in MOOSE.

In this method, both the strain ($\epsilon$) and the virtual strain ($\delta \epsilon$) in an element are separated into volumetric and deviatoric components. The volumetric component is then replaced with an element averaged volumetric strain. This ensures that the volumetric strain remains constant throughout the element.

For example, in the case of small strain linear elasticity, the equation of motion is:

\begin{align*}
\int_V \sigma(\epsilon)\delta \epsilon dV - \int_V b \delta v dV - \int_{\partial V} t \delta v dA = 0
\end{align*}

The element averaged volumetric strain (assuming small strain formulation) is:

\begin{align*}
 w=\frac{1}{V_e} \int_{V_e} \frac{1}{3} tr(\epsilon) dV,
\end{align*}

where $V_e$ is the volume of the element and tr(.) is the trace of the matrix.

The strain in each element is replaced by the approximation:

\begin{align*}
\bar{\epsilon} = \epsilon +(w - \frac{tr(\epsilon)}{3})I,
\end{align*}

where $I$ is the $3 \times 3$ identity matrix. Similarly, the virtual strain is also approximated by:

\begin{align*}
\bar{\delta \epsilon} = \delta \epsilon + (\delta w - \frac{tr(\delta \epsilon)}{3})I
\end{align*}

The modified equation of motion is:
\begin{align*}
\int_V \sigma(\bar{\epsilon})\bar{\delta \epsilon} dV - \int_V b \delta v dV - \int_{\partial V} t \delta v dA = 0
\end{align*}

More details about this method can be found in section 8.6 of \citet{bower2009applied}.

When finite strain formulation is used, the volumetric component of the strain is separated using the determinant of the deformation matrix.

### Usage

Volumetric locking correction is set to false by default in tensor mechanics. When dealing with problems involving plasticity or incompressible materials, it can be turned on by setting `volumetic_locking_correction=true` in both the stress divergence kernel (or TensorMechanicsAction) and the strain calculator. It can also be turned on by setting `volumetric_locking_correction=true` in the GlobalParams.

When volumetric locking correction is turned on, using a SMP preconditioner with coupled displacement variables may help with convergence. For a 3-D problem with only displacement as unknown variables, the following pre-conditioner block may be used:

!listing modules/tensor_mechanics/tests/finite_strain_elastic/elastic_rotation_test.i start=Preconditioning end=Executioner

\bibliography{docs/bib/tensor_mechanics.bib}
