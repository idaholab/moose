# Fracture Integrals

## J-Integral

A finite element calculation in which a crack is represented in the mesh geometry can be used to calculate the displacement and stress fields in the vicinity of the crack.  One straightforward way to evaluate the stress intensity from a finite element solution is through the $J$-integral [!citep](rice_path_1968), which provides the mechanical energy release rate. If the crack is subjected to pure mode-$I$ loading, $K_I$ can be calculated from $J$ using the following relationship:
\begin{equation}
J=
\begin{cases}
\frac{1-\nu^2}{E}K_I^2 & \text{plane strain}\\
\frac{1}{E}K_I^2 & \text{plane stress}
\end{cases}
\end{equation}
where $E$ is the Young's modulus and $\nu$ is the Poisson's ratio.
The $J$-integral was originally formulated as an integral over a closed curve around the crack tip in 2D.  It can alternatively be expressed as an integral over a surface in 2D or a volume in 3D using the method of [!cite](li_comparison_1985), which is more convenient for implementation within a finite element code.

Following the terminology of [!cite](warp3d_17.3.1), the integrated value of the $J$-integral over a segment of a crack front, represented as $\bar{J}$, can be expressed as a summation of four terms:
\begin{equation}
\bar{J}=\bar{J}_1+\bar{J}_2+\bar{J}_3+\bar{J}_4
\end{equation}
where the individual terms are defined as:
\begin{equation}
\bar{J}_1=\int_{V_0}\biggl(
P_{ji}\frac{\partial u_i}{\partial X_k}\frac{\partial q_k}{\partial X_j}
-W\frac{\partial q_k}{\partial X_k}
\biggr) dV_0
\end{equation}

\begin{equation}
\bar{J}_2=-\int_{V_0}\biggl(
\frac{\partial W}{\partial X_k} -
P_{ji}\frac{\partial^2u_i}{\partial X_j\partial X_k}
\biggr) q_k dV_0
\end{equation}

\begin{equation}
\bar{J}_3=-\int_{V_0}\biggl(
T\frac{\partial q_k}{\partial X_k} -
\rho \frac{\partial^2 u_i}{\partial t^2} \frac{\partial u_i}{\partial X_k} q_k +
\rho \frac{\partial u_i}{\partial t} \frac{\partial^2 u_i}{\partial t \partial X_k} q_k
\biggr) dV_0
\end{equation}

\begin{equation}
\bar{J}_4=-\int_{A_0}
t_i\frac{\partial u_i}{\partial X_k} q_k
dA_0
\end{equation}
In these equations, $V_0$ is the undeformed volume, $A_0$ is the combined area of the two crack faces, $P_{ji}$ is the 1st Piola-Kirchoff stress tensor, $u_i$ is the displacement vector, $X_k$ is the undeformed coordinate vector, $W$ is the stress-work density, $T$ is the kinetic energy density, $t$ is time, $\rho$ is the material density, and $t_i$ is the vector of tractions applied to the crack face.

The vector field $q_k$ is a vector field that is oriented in the crack normal direction. This field has a magnitude of 1 between the crack tip and the inner radius of the ring over which the integral is to be performed, and drops from 1 to 0 between the inner and outer radius of that ring, and has a value of 0 elsewhere. In 3D, $J$ is evaluated by calculating the integral $\bar{J}$ over a segment of the crack front. A separate $q$ field is formed for each
segment along the crack front, and for each ring over which the integral is to be evaluated. Each of those $q$ fields is multiplied by a weighting function that varies from a value of 0 at the ends to a finite value in the middle of the segment. The value of $J$ at each point on the curve is evaluated by dividing $\bar{J}$ by the integrated value of the weighting function over the segment containing that point.

The first term in $\bar{J}$, $\bar{J}_1$, represents the effects of strain energy in homogeneous material in the absence of thermal strains or inertial effects. The second term, $\bar{J}_2$, accounts for the effects of material inhomogenieties and gradients of thermal strain.  For small strains, this term can be represented as:
\begin{equation}
\bar{J}_2 = \int_{V_0} \sigma_{ij} \left [ \alpha_{ij} \frac{\partial \bar{\theta}}{\partial X_k} + \frac{\partial \alpha_{ij}}{\partial X_k} \bar{\theta} \right] q_k dV_0
\end{equation}
where $\sigma_{ij}$, $\alpha_{ij}$ and $\bar{\theta}$ are the Cauchy stress, thermal conductivity, and difference between the current temperature and a reference temperature, respectively.

The third term in $\bar{J}$, $\bar{J}_3$, accounts for inertial effects in a dynamic analysis, and the fourth term, $\bar{J}_4$ accounts for the effects of surface tractions on the crack face.

The MOOSE implementation of the $J$-integral calculator can be used for arbitrary curved 3D crack fronts, and includes the $\bar{J}_1$ and $\bar{J}_2$ terms to account for the effects of quasistatic mechanically and thermally induced strains. The last two terms have not been implemented. For quasistatic loading conditions, there are no inertial effects, so $\bar{J}_3=0$. $\bar{J}_4$ would be nonzero for pressurized cracks, and should be implemented in the future to consider such cases.

Pointwise values $J(s)$ are calculated from the $J$-integral over a crack front segment $\bar{J}$ using the expression
\begin{equation}
J(s) = \frac{\bar{J}(s)}{\int q(s) ds}
\end{equation}
To use the $J$-integral capability in MOOSE, a user specifies a set of nodes along the crack front, information used to calculate the crack normal directions along the crack front, and the inner and outer radii of the set of rings over which the integral is to be performed. The code automatically defines the full set of $q$ functions for each point along the crack front, and outputs either the value of $J$ or $K_I$ at each of those points. In addition, the user can ask for any other variable in the model to be output at points corresponding to those where $J$ is evaluated along the crack front.

## Interaction Integral

The interaction integral method is based on the $J$-integral and makes it possible to evaluate mixed-mode stress intensity factors $K_I$, $K_{II}$ and $K_{III}$, as well as the T-stress, in the vicinity of three-dimensional cracks. The formulation relies on superimposing Williams' solution for stress and displacement around a crack (in this context called 'auxiliary fields') and the computed finite element stress and displacement fields (called 'actual fields'). The total superimposed $J$ can be separated into three parts: the $J$ of the actual fields, the $J$ of the auxiliary fields, and an interaction part containing the terms with both actual and auxiliary field quantities. The last part is called the interaction integral and for a fairly straight crack without body forces, thermal loading or crack face tractions, the interaction integral over a crack front segment can be written [!citep](walters_interaction_2005):
\begin{equation}
\bar{I}(s) = \int_V \left[ \sigma_{ij} u_{j,1}^{(aux)} + \sigma_{ij}^{(aux)} u_{j,1} - \sigma_{jk} \epsilon_{jk}^{(aux)} \delta_{1i} \right] q_{,i} \, \mathrm{d}V + \int_V \left[ \epsilon^*_{ij,1} \sigma_{ij}^{(aux)} - b_i u_{i,1}^{(aux)} \right] q \ + \mathrm{d}V +\int_V \left[ \sigma_{ij,j}^{(aux)}u_{i,1}^{(aux)} - C_{ijkl,1}\epsilon_{kl} \epsilon_{ij}^{(aux)} \right] q \, \mathrm{d}V,
\end{equation}
where $\sigma$ is the stress, $u$ is the displacement, and $q$ is identical to the $q$-functions used for $J$-integrals. The second integral in this equation accounts for the effects of the gradient of the eigenstrain, $\epsilon^*$, and for the effects of the body force, $b$. If the eigenstrain is a thermal strain, its gradient is computed using the gradient of the temperature field and the temperature derivative of the eigenstrain. In that case, the user supplies the temperature and eigenstrain name through the `temperature` and `eigenstrain_name` parameters, respectively. If the eigenstrain is from another source, its gradient must be computed by the eigenstrain material model in the form of a RankThreeTensor object, whose name is specified for the fracture integrals using the `eigenstrain_gradient` parameter. The last integral accounts for spatially-varying elastic properties in the material (such as for functionally graded materials) using a "non-equilibrium" formulation.

The treatment for functionally graded materials is currently limited to cases for which the gradient of the Young's modulus is in the direction of crack extension. For example, this could be used to represent a crack that is perpendicular to an interface between two materials, where the transition between the materials is assumed to be approximated by a smooth function such as $E(X_{1}) = E_{0} \exp{\beta X_{1}}$, where $X_{1}$ is the position in the direction of crack extension, $E_{0}$ is the stiffness of the material behind the crack tip, and $\beta$ is a parameter that governs the rate of the material property change. To specify the spatially varying properties, the user must define the `functionally_graded_youngs_modulus` and `functionally_graded_youngs_modulus_crack_dir_gradient` parameters, which reference scalar material properties that define the spatially varying Young's modulus and its gradient in the crack direction, respectively.  These material properties can be defined arbitrarily by the user, but it is up to the user to ensure that the gradient is indeed in the crack direction. Note that the user must still also specify `youngs_modulus` to define that moduls at the crack tip. See the "non-equilibrium" formulation in [!cite](Jeong2004functionallygraded) for details.


The integration integral is automatically adapted to axisymmetric solids if an axisymmetric coordinate system is chosen. For example,
the interaction integral for homogeneous materials can be defined as follows:
\begin{equation}
\bar{I}(s) = \frac{1}{R} \int_V \left[ \left[ \sigma_{ij} u_{j,1}^{(aux)} + \sigma_{ij}^{(aux)} u_{j,1} - \sigma_{jk} \epsilon_{jk}^{(aux)} \delta_{1i} \right] q_{,i} + \left[ (\frac{u_{1}}{r} \sigma_{\theta\theta}^{(aux)} + \frac{u_{1}^{(aux)}}{r} \sigma_{\theta\theta} - \sigma_{jk} \epsilon_{jk}^{(aux)})\frac{q}{r} + \frac{1}{r}(\sigma_{1j}^{(aux)} u_{j,1} - \sigma_{\theta\theta}^{(aux)} u_{1,1} +  \sigma_{\theta\theta}(u_{1,1}^{(aux)} - \frac{u_{1}^{(aux)}}{r}))  \right] q \right] r \, \mathrm{d}V,
\end{equation}
where $R$ is the radial location of the crack, $r$ is a relative location with respect to the crack tip, $u^{(aux)}$ is the auxiliary displacement
field, and $1$ refers to the local direction of crack advancement, which, in the case of a radial crack, it would coindide with the radial direction.
See [!cite](NAHTA19932027) for details. Note that this form of the interaction integral can be integrated with other features such as its application to functionally graded materials.


In the same way as for the $J$-integral, the pointwise value $I(s)$ at location $s$ is obtained from $\bar{I}(s)$ using:
\begin{equation}
I(s) = \frac{\bar{I}(s)}{\int q(s) \mathrm{d}s}
\end{equation}
Next, we relate the interaction integral $I(s)$ to stress intensity factors. By writing $J^S$, with actual and auxiliary fields superimposed, in terms of the mixed-mode stress intensity factors
\begin{equation}
\begin{aligned}
J^S(s) &= \frac{1-\nu^2}{E} \left[ \left( K_I+ K_I^{(aux)} \right)^2 + \left( K_{II} + K_{II}^{(aux)} \right)^2 \right] \\
&+ \frac{1+\nu}{E} \left( K_{III}+ K_{III}^{(aux)} \right)^2 \\
& = J(s) + J^{(aux)}(s) + I(s)
\end{aligned}
\end{equation}
the interaction integral part evaluates to
\begin{equation}
\begin{aligned}
I(s) &= \frac{1-\nu^2}{E} \left( 2 K_I K_I^{(aux)} + 2 K_{II} K_{II}^{(aux)} \right) \\
& + \frac{1+\nu}{E} \left( K_{III} K_{III}^{(aux)} \right)
\end{aligned}
\end{equation}
To obtain individual stress intensity factors, the interaction integral is evaluated with different auxiliary fields. For instance, by choosing $K_I^{(aux)} = 1.0$ and $K_{II}^{(aux)} = K_{III}^{(aux)} = 0$ and computing the volume integral $I(s)$ over the actual and auxiliary fields, $K_I$ can be solved for.

If all three interaction integral stress intensity factors are computed, there is an option to output an equivalent stress intensity factor computed by:
\begin{equation}
K_{eq} = \sqrt{ K_I^2 + K_{II}^2 + \frac{K_{III}^2}{1-\nu}}.
\end{equation}
The $T$-stress is the first second-order parameter in Williams' expansion of stress at a crack tip and is a constant stress parallel to the crack [!citep](larsson_influence_1973) [!citep](rice_limitations_1974). Contrary to $J$, $T$-stress depends on geometry and size and can give a more accurate description of the stresses and strains around a crack tip than $J$ alone. The $T$-stress characterizes the crack-tip constraint and a negative $T$-stress is associated with loss of constraint and a higher fracture toughness than would be predicted from a one-parameter $J$ description of the load on the crack. $T$-stresses can be calculated with the interaction integral methodology using appropriate auxiliary fields [!citep](toshio_determination_1992). The current implementation of the $T$-stress computation is valid for two-dimensional and three-dimensional cracks under Mode I loading.


## Usage

The MOOSE implementation of the capability to compute these fracture integrals is provided using a variety of MOOSE objects, which are quite complex to define manually, especially for 3D simulations. For this reason, the to compute $J$-integrals or interaction integrals, the [DomainIntegral Action](/DomainIntegralAction.md) should be used to set up all of the required objects.

!listing modules/tensor_mechanics/test/tests/j_integral/j_integral_3d.i block=DomainIntegral


## C-Integral

The C(t) integral is a fracture integral that is obtained in essentially the same way as the $J$-integral described above, but by replacing displacements and strains with displacement rates and strain rates:

\begin{equation}
{C}(t)=\int_{\Gamma}\biggl(
P_{ji}\frac{\partial \dot{u}_i}{\partial X_k}ds -\dot{W}dy
\biggr) d\Gamma
\end{equation}

This integral is computed in an analogous way to the J-integral: A domain integral is solved on the crack front geometry defined in the mesh. This integral becomes the $C^{*}$ integral as creep behavior becomes steady state. Then, creep crack growth behavior during secondary creep can be obtained from this integral. A number of approximate expressions for $C^{*}$ are available in the literature [!citep](bongyoon_2003).

### Usage

To compute $C$-integrals, the [DomainIntegral Action](/DomainIntegralAction.md) should be used to set up all of the required objects. In particular, two additional inputs are required `integrals = CIntegral` and `inelastic_models`. The former refers to the type of integral requested ('C') and the latter refers to the name of the power law creep model --only supported material model at present. The power law exponent in the material model is used to compute the strain energy rate density in the fracture integral under the assumption of a creep strain rate field subject to steady-state (secondary) crack growth.

!listing modules/tensor_mechanics/test/tests/j_integral_vtest/c_int_surfbreak_ellip_crack_sym_mm.i block=DomainIntegral
