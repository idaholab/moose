# Fracture flow using a MultiApp approach: Porous flow in a simple fracture-matrix system

## Background

PorousFlow can be used to simulate flow through fractured porous media in the case when the fracture network is so complicated it cannot be incorporated into the porous-media mesh.  The fundamental premise is *the fractures can be considered as lower dimensional entities within the higher-dimensional porous media*.  This page is part of a set that describes a MOOSE MultiApp approach to simulating such models:

- [Introduction](multiapp_fracture_flow_introduction.md)
- [Mathematics and physical interpretation](multiapp_fracture_flow_equations.md)
- [Transfers](multiapp_fracture_flow_transfers.md)
- [MultiApp primer](multiapp_fracture_flow_primer.md): the diffusion equation with no fractures, and quantifying the errors introduced by the MultiApp approach
- [Diffusion in mixed dimensions](multiapp_fracture_flow_diffusion.md)
- [Porous flow in a single matrix system](multiapp_fracture_flow_PorousFlow_2D.md)
- [Porous flow in a small fracture network](multiapp_fracture_flow_PorousFlow_3D.md)

## Porous flow through a 3D fracture network

A sample PorousFlow simulation through the small 3D fracture network shown in [orbit_fracture] is presented in this section.  The methodology is described in the above pages.  It is assumed that

- the physics is fully-saturated, non-isothermal porous flow with heat conduction and convection;
- the porepressure is initially hydrostatic, around 10$\,$MPa corresponding to a depth of around 1$\,$km;
- the temperature is 200$^{\circ}$C;
- injection is into the fracture network only, through one point, at a rate of $10\,$kg.s$^{-1}$ and temperature of 100$^{\circ}$C;
- production is from the fracture network only, through one point, at a rate of approximately $10\,$kg.s$^{-1}$ (it cannot be exactly $10\,$kg.s$^{-1}$ initially because this causes large porepressure reductions due to thermal contraction of water and because the aperture increases in response to the injection);
- the fracture aperture dilates elastically in response to enhanced porepressure;
- only heat energy is transferred between the fracture and the matrix: the matrix heats the cool water injected into the fracture network.


!media porous_flow/examples/multiapp_flow/orbit_fracture.mp4
	style=width:60%;margin:auto;padding-top:2.5%;
	id=orbit_fracture
	caption=Fracture network.  The injection point is coloured green.  The production point is red.

## No heat transfer

Before introducing the matrix, consider a model containing just the fracture network.

### Physics

The physics is captured by the [PorousFlowFullySaturated](PorousFlowFullySaturated.md) Action:

!listing 3dFracture/fracture_only_aperture_changing.i block=PorousFlowFullySaturated

!alert note
[Kuzmin-Turek](/porous_flow/kt.md) stabilization cannot typically be used for the fracture flow.  This is because multiple fracture elements will meet along a single finite-element edge when fracture planes intersect, while for performance reasons, libMesh assumes only one or two elements share an edge.  This prevents the KT approach from quantifying flows in all neighboring elements, which prevents the scheme from working.

Because large changes of temperature and pressure are experienced in this model, the Water97 equation of state is employed:

!listing 3dFracture/fracture_only_aperture_changing.i block=FluidProperties


### Material properties

The insitu fracture aperture is assumed to be $a_{0} = 0.1\,$mm for all the fracture planes.  The fractures are assumed to dilate due to increasing porepressure:

\begin{equation}
\label{eqn.frac.open}
a = a_{0} + A(P - P_{0}) \ ,
\end{equation}

where $A = 10^{-3}\,$m.MPa$^{-1}$.  [eqn.frac.open] could easily be generalised to include a temperature-dependent term (which would still be modelled using [PorousFlowPorosityLinear](PorousFlowPorosityLinear.md)), or more complicated physics introduced through the use of other [PorousFlow porosity](porous_flow/porosity.md) classes.  Using $A = 10^{-3}\,$m.MPa$^{-1}$ means that a pressure increase of 1$\,$MPa dilates the fracture by 1$\,$mm.

The page on [mathematics and physical interpretation](multiapp_fracture_flow_equations.md) demonstrated that the porosity in the 2D fracture simulation should be multiplied by $a$.  Assuming that the porosity of the fracture is 1, the simulation's porosity is $\phi = a$:

!listing 3dFracture/fracture_only_aperture_changing.i block=porosity

The insitu fracture permeability is assumed to be $10^{-11}\,$m$^{2}$.  It is assumed that this is $r a_{0}^{2}/12$, where $r=0.012$ is a factor capturing the roughness of the fracture surfaces.  It is also assumed that when the fracture dilates, this becomes $ra^{2}/12$.  Because of the multplication by $a$ (see [here](multiapp_fracture_flow_equations.md)), the value of permeability used in the simulations is $ra^{3}/12$:

\begin{equation}
\label{eqn.frac.perm}
k = ra^{3}/12 = \frac{ra_{0}^{3}}{12} \frac{a^{3}}{a_{0}^{3}} = 10^{-15} \left(\frac{\phi}{\phi_{0}}\right)^{3} \ .
\end{equation}

Hence a [PorousFlowPermeabilityKozenyCarman](PorousFlowPermeabilityKozenyCarman.md) Material can be used:

!listing 3dFracture/fracture_only_aperture_changing.i block=permeability

PorousFlow's "matrix internal energy" quantifies the internal energy of the rock material.  Since there is essentially no rock material within the fracture, the matrix internal energy for the fracture App is zero.  It is assumed that the thermal conductivity is constant and independent of $a$ (in reality, this too should increase with $a$, but PorousFlow currently lacks this capability, and thermal conductivity is unlikely to be important within the fracture):

!listing 3dFracture/fracture_only_aperture_changing.i block=internal_energy

!listing 3dFracture/fracture_only_aperture_changing.i block=aq_thermal_conductivity

### Injection and production

The [Thiem equation](https://en.wikipedia.org/wiki/Aquifer_test) may be used to estimate flows to and from the fracture network.  Imagine a single well piercing the fracture network in a normal direction to a fracture plane.  The Thiem equation is

\begin{equation}
Q = \frac{2\pi \rho k_{\mathrm{3D}} L \Delta P}{\mu \log(R/r_{\mathrm{well}})}
\end{equation}

Here

- Q is the flow rate to the well, with SI units kg.s$^{-1}$.
- $\rho$ is the fluid density: in this case $\rho \approx 870\,$kg.m$^{-3}$.
- $k_{\mathrm{3D}}$ is the fracture permeability: in this case $k_{\mathrm{3D}} = ra^{2}/12$ (it is not the 2D version with the "extra" factor of $a$)
- $L$ is the length of well piercing the fracture network: in this case $L=a$.
- $\Delta P$ is the change in pressure resulting from the well pumping: $\Delta P = P_{\mathrm{well}} - P_{0}$.
- $\mu$ is the fluid viscosity: in this case $\mu \approx 1.4\times 10^{-10}\,$MPa.s$^{-1}$
- $R$ is the radius of influence of the well: in this case it is appropriate to choose $R\sim 200\,$m since that is the size of the fracture network
- $r_{\mathrm{well}}$ is the radius of the borehole.  Assume this is $r = 0.075\,$m

[eqn.frac.open] may be used to write $a$ in terms of $\Delta P$.  This analysis is only approximate, but it provides a rough idea of the flow rates to expect, as shown in [table:flowrates].

!table id=table:flowrates caption=Indicative flow rate, aperture and permeability, depending on well pressure
| $\Delta P = P_{\mathrm{well}} - P_{0}$ (MPa) | Q (kg.s$^{-1}$) | a (mm) | permeability (m$^{2}$) |
| --- | --- | --- | --- |
| 0.1 | 0.004 | 0.2 | $4\times 10^{-11}$ |
| 0.2 | 0.03 | 0.3 | $9\times 10^{-11}$ |
| 0.5 | 0.5 | 0.6 | $4\times 10^{-10}$ |
| 1 | 7 | 1 | $10^{-9}$ |
| 2 | 90 | 2 | $4\times 10^{-9}$ |



Economically-viable flow rates are usually greater than about 10$\,$kg.s$^{-1}$, which is the amount prescribed in the MOOSE input file, below.  This means a pressure change of $\sim 1\,$MPa is expected, and apertures will be around 1$\,$mm.  Note that the porepressure around the *producer* should be around 1$\,$MPa *higher* than insitu, in order for the fluid to flow from the fracture to the production well.  If the production well reduces pressure too much, then according to [eqn.frac.open], the fracture will close in its vicinity, resulting in limited fluid production.  Therefore, the numerical model relies on the injector increasing the porepressure throughout the system (by greater than 1$\,$MPa in most places) and the producer removes excess fluid.

PorousFlow has many different types of [sinks](porous_flow/sinks.md) and [boundary conditions](porous_flow/boundaries.md) that can be used to model injection and production wells.  To most accurately represent the physics around these points, appropriate boundary conditions should be chosen that closely match the operating parameters of the pump infrastructure employed.  However, before obsessing over such details, it is worth noting that the fundamental assumption underpinning the PorousFlow module --- that the flow is slow --- is likely to be violated close to the wells.  For instance, if 10$\,$kg.s$^{-1}$ (approximately $10^{-2}\,$m$^{3}$.s$^{-1}$) is injected into a fracture of aperture 2$\,$mm through a borehole of diameter 15$\,$cm, the fluid velocity is approximately $10^{-2}/0.002/(\pi\times 0.15) \approx 11\,$m.s$^{-1}$, which is certainly turbulent and not laminar Darcy flow.  This also means that the injection and production pressures predicted by the PorousFlow model are likely to be inaccurate (the true injection pressure is likely to be higher).

For this reason, the current model implements the injection and production rather simply.  The injection is implemented using a [DirichletBC](DirichletBC.md) for the temperature at the injection node:

!listing 3dFracture/fracture_only_aperture_changing.i block=BCs

and a [PorousFlowPointSourceFromPostprocessor](PorousFlowPointSourceFromPostprocessor.md) DiracKernel for the fluid injection:

!listing 3dFracture/fracture_only_aperture_changing.i block=inject_fluid

As noted above, the production cannot always be 10$\,$kg.s$^{-1}$ (although it assumes this value at steady-state) so it is implemented using two [PorousFlowPeacemanBoreholes](PorousFlowPeacemanBorehole.md): one withdrawing fluid and the other heat energy.  There is a minor subtlety that can arise when using [PorousFlowPeacemanBoreholes](PorousFlowPeacemanBorehole.md) in lower-dimensional input files.  Peaceman [write the flux](porous_flow/sinks.md) as

\begin{equation}
f = W \frac{k_{r}\rho}{\mu}(P - P_{\mathrm{well}}) \ .
\end{equation}

The prefactor, $k_{r}\rho/\mu$ is automatically included when `use_mobility = true`, but Peaceman's "well constant", $W$, is

\begin{equation}
W = 2\pi k_{\mathrm{3D}}L / \log(r_{e}/r_{\mathrm{well}})
\end{equation}

Here $k_{\mathrm{3D}} = k/a$ is the "3D" permeability, and $L = a$ is the length of borehole that is piercing the fracture system.  Hence

\begin{equation}
W = 2\pi k / \log(r_{e}/r_{\mathrm{well}})
\end{equation}

is the appropriate lower-dimensional form.  This is easily obtained in the MOOSE model by setting `line_length = 1` in the input file, and then MOOSE will compute the well constant correctly:

!listing 3dFracture/fracture_only_aperture_changing.i block=DiracKernels

As mentioned above `bottom_p_or_t` is chosen so that the borehole doesn't "suck" water: instead they rely on the injection to increase the porepressure and pump water into the production borehole.  The reason for this is that if the porepressure reduces around the production well (due to a "suck") then the fracture aperture reduces, making it increasingly difficult to extract water.

### Results

Typically 3-5 fracture time-steps are taken in a single matrix step in this model.  In other models this number could be quite different, but here the fracture and matrix are both approaching steady-state at a similar rate.

The pressure at the injection point rises from its insitu value of around 9.4$\,$MPa to around 12.2$\,$MPa: an increase of 3$\,$MPa, which results in a fracture aperture of around 3$\,$mm according to [eqn.frac.open].  This results in permeability increasing by a factor of around 27000.

Some results are shown in [fracture_only_aperture_changing_P_out], [fracture_only_aperture_changing_T_out] and [fracture_only_aperture_changing_T_2hrs].  Production commences around 1.75 hours after injection starts.  These simulations were run using different mesh sizes (by choosing `Mesh/uniform_refine` appropriately) to illustrate the impact of different meshes in this problem.  A straightfoward analysis of simulation error as a function of mesh size is not possible, because each simulation also uses a different time-step size.  Nevertheless, some observations are:

- There is an interesting increase of temperature around the production well in the finer-resolution cases.  The reason for this is that a thin layer of insitu hot fluid is "squashed" against the fracture ends as the cold fluid invades the fracture (the thin layer is not resolved in the low-resolution cases).  This is also clearly seen in [orbit_T].  The hot fluid cannot escape, and it becomes pressurized, leading to an increase in temperature.  Eventually the porepressure increases to 10.6$\,$MPa and the production well activates (indicated by the dot in the figures), withdrawing the hot fluid, and the near-well area cools.  In regions where there is no production well, the high temperature eventually diffuses.  If the production well were in the middle of a fracture, this interesting phenomenon wouldn't be seen.
- As mesh resolution is increased, the results appear to be converging to an "infinite-resolution" case.  Given the likely uncertainties in parameter values and the physics of aperture dilation, a mesh with element side-lengths of 10$\,$m is likely to be perfectly adequate for this type of problem.

!media porous_flow/examples/multiapp_flow/fracture_only_aperture_changing_P_out.png
	style=width:70%;margin:auto;padding-top:2.5%;
	id=fracture_only_aperture_changing_P_out
	caption=Porepressure at the production point in the case where there is no matrix.  The legend's numbers indicate the size of elements used in the simulation.  Dotted lines: before production commences.  Dot: production commences.  Solid lines: during production.

!media porous_flow/examples/multiapp_flow/fracture_only_aperture_changing_T_out.png
	style=width:70%;margin:auto;padding-top:2.5%;
	id=fracture_only_aperture_changing_T_out
	caption=Temperature at the production point in the case where there is no matrix.  The legend's numbers indicate the size of elements used in the simulation.  Dotted lines: before production commences.  Dot: production commences.  Solid lines: during production.

!media porous_flow/examples/multiapp_flow/fracture_only_aperture_changing_T_2hrs.png
	style=width:70%;margin:auto;padding-top:2.5%;
	id=fracture_only_aperture_changing_T_2hrs
	caption=Temperature at the production point 2 hours after injection commences, in the case where there is no matrix, for meshes with different-sized elements.  A straightforward error analysis is not possible because each simulation takes different time-steps.

Some animations are shown in [orbit_T] and [orbit_aperture].  One month is simulated, but steady state is rapidly approached within the first few hours of simulation.   The cold injectate invades most of the fracture network: hot pockets of fluid only remain at the tops of some fractures, due to buoyancy.

!media porous_flow/examples/multiapp_flow/fracture_only_aperture_changing_T.mp4
	style=width:60%;margin:auto;padding-top:2.5%;
	id=orbit_T
	caption=Temperature in the fracture for the case where there is no matrix, during the first month of simulation.  The cold injectate invades most of the fracture network: hot pockets of fluid only remain at the tops of some fractures, due to buoyancy.  Time is indicated by the green bar: most of the temerature changes occur within the first few hours

!media porous_flow/examples/multiapp_flow/fracture_only_aperture_changing_aperture.mp4
	style=width:60%;margin:auto;padding-top:2.5%;
	id=orbit_aperture
	caption=Fracture aperture for the case where there is no matrix, during the first month of simulation.  Time is indicated by the green bar: most of the dilation occurs within the first few hours


## Including the matrix

### Matrix physics and material properties

Most of the matrix input file is standard by now.  The physics is governed by [PorousFlowFullySaturated](PorousFlowFullySaturated.md)

!listing 3dFracture/matrix_app.i block=PorousFlowFullySaturated

along with the familiar [ReporterPointSource](ReporterPointSource.md):

!listing 3dFracture/matrix_app.i block=DiracKernels

It is assumed the rock matrix has small porosity of 0.1 and permeability of $10^{-18}\,$m$^{2}$.  The rock density is 2700$\,$kg.m$^{-3}$ with specific heat capacity of 800$\,$J.kg$^{-1}$.K$^{-1}$ and isotropic thermal conductivity of 5$\,$W.m$^{-1}$.K$^{-1}$.  Hence, the `Materials` block is:

!listing 3dFracture/matrix_app.i block=Materials

The matrix input file transfers data to the same fracture input file used in the previous section for the fully insulated fracture.

### Heat transfer coefficients, matrix mesh sizes and time scales

As mentioned in the [mathematical and physical introduction](multiapp_fracture_flow_equations.md) the use of heat transfer coefficients such as

\begin{equation}
\label{eqn.suggested.h.L}
h = \frac{2h_{\mathrm{s}}\lambda_{\mathrm{m}}^{nn}L}{h_{\mathrm{s}}L^{2} + 2\lambda_{\mathrm{m}}^{nn}L} \ .
\end{equation}

is only justified if the matrix element sizes are small enough to resolve the physics of interest.  (For large elements, not enough heat will be transferred between the fracture and matrix, but the correct short-term behavior could be produced by choosing a larger heat-transfer coefficient than [eqn.suggested.h.L].  Naturally, this will result in incorrect long-term behaviour.)  The time taken for a pulse of heat to travel through the matrix over half-element distance $L$ is

\begin{equation}
t \sim \frac{c\rho}{\lambda}L^{2} \ .
\end{equation}

This equation provides a rough idea of the element size needed to accurately resolve physical phenomena.  (It also suggests a time-step size: perhaps $c \rho L^{2}/\lambda/10$ might be a good choice, but it is likely that the fracture App will need smaller time-steps.)  [table:time_scales] enumerates the time-scales for the case in hand: if $t$ is smaller than the enumerated time-scale then [eqn.suggested.h.L] is inappropriate, so other choices must be made, or the matrix mesh made finer.  (If the matrix mesh is made finer then small time-steps will be needed.)

!table id=table:time_scales caption=Minimum time-scales for which [eqn.suggested.h.L] will be appropriate, as a function of matrix mesh size
| $L$ (m) | matrix mesh size (m) | time scale (days) |
| --- | --- | --- |
| 10 | 20 | 500 |
| 5 | 10 | 125 |
| 2.5 | 5 | 30 |

The heat-transfer is implemented as a [PorousFlowHeatMassTransfer](PorousFlowHeatMassTransfer.md) Kernel in the fracture input file:

!listing 3dFracture/fracture_only_aperture_changing.i block=Kernels

with $h$ being calculated by the following AuxKernel that implements [eqn.suggested.h.L]:

!listing 3dFracture/fracture_only_aperture_changing.i block=heat_transfer_coefficient_auxk


### Coupling and transfers

The simulation's coupling involves the following steps (see also the [page on transfers](multiapp_fracture_flow_transfers.md)).

1. Each fracture element must be prescribed with a normal direction, using a [PorousFlowElementNormal](PorousFlowElementNormal.md) AuxKernel, such as

!listing 3dFracture/fracture_only_aperture_changing.i block=normal_dirn_x_auxk

2. Each matrix element must retrieve the fracture-normal information from the nearest fracture element, which is implemented using a [MultiAppNearestNodeTransfer](MultiAppNearestNodeTransfer.md) Transfer.

!listing 3dFracture/matrix_app.i block=normal_x_from_fracture

3. Each matrix element must be prescribed with a normal length, $L$, using a [PorousFlowElementLength](PorousFlowElementLength.md) AuxKernel and the fracture-normal direction sent to it.  As described in the [mathematical theory page](multiapp_fracture_flow_equations.md), this procedure assumes $L_{\mathrm{left}} = L_{\mathrm{right}} = L$.  If $L_{\mathrm{right}} \neq L_{\mathrm{left}}$ then this procedure corresponds to making a shift of the fracture position by an amount less than the finite-element size.  Since the accuracy of the finite-element scheme is governed by the element size, such small shifts introduce errors that are smaller than the finite-element error.  If a matrix element contains multiple fractures then this procedure only chooses one of their directions.  In that case, if the thermal conductivity is anisotropic then the incorrect $\lambda_{\mathrm{m}}^{nn}$ would be used for all but one of the fractures.

!listing 3dFracture/matrix_app.i block=element_normal_length_auxk

4. Each matrix element must be prescribed with a normal thermal conductivity, $\lambda_{\mathrm{m}}^{nn}$, using the fracture-normal direction, $\boldsymbol{n}$, received from the transfer using $\lambda_{\mathrm{m}}^{nn}=\boldsymbol{n}\cdot\lambda_{\mathrm{m}}\cdot\boldsymbol{n}$, where $\lambda_{\mathrm{m}}$ is the matrix material's $3\times 3$ anisotropic thermal conductivity tensor.  The below input block assumes an isotropic matrix thermal conductivity.

!listing 3dFracture/matrix_app.i block=normal_thermal_conductivity_auxk

5. Each fracture element must retrieve $L$ and $\lambda_{\mathrm{m}}^{nn}$ from its nearest matrix element using a [MultiAppNearestNodeTransfer](MultiAppNearestNodeTransfer.md), with blocks such as

!listing 3dFracture/matrix_app.i block=element_normal_length_to_fracture

6. Each fracture element must calculate $h$ using [eqn.suggested.h.L].

!listing 3dFracture/fracture_only_aperture_changing.i block=heat_transfer_coefficient_auxk

These steps could be performed during the simulation initialization, however, it is more convenient to perform them at each time-step.  When these steps have been accomplished, each time-step involves the following (which is also used in the sections above).

1. The matrix temperature, `matrix_T`, is sent to the fracture nodes using a [MultiAppGeometricInterpolationTransfer](MultiAppGeometricInterpolationTransfer.md) Transfer.
2. The fracture physics is solved.
3. The heat flowing between the fracture and matrix is transferred using a [MultiAppReporterTransfer](MultiAppReporterTransfer.md) Transfer.
4. The matrix physics is solved.

The `Transfers` described above are:

!listing 3dFracture/matrix_app.i block=Transfers


### Results

[matrix_app_T_short] and [matrix_app_T] show the temperature at the production bore.  It is clear that the matrix provides substantial heat-energy to the injectate, when these results are compared with [fracture_only_aperture_changing_T_out].  However, as time proceeds, the cold injectate cools the surrounding matrix, leading to cooler production temperatures.  These figures show how the results depend on the matrix and fracture mesh sizes.  Keep in mind [table:time_scales], which tabulates the time scale at which the results should become accurate (eg, the "20m, 9.2m" case is not expected to be accurate for time-scales less than about 500 days).  Convergence to the infinite-resolution limit is obviously achieved.

!media porous_flow/examples/multiapp_flow/matrix_app_T_short.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=matrix_app_T_short
	caption=Short-term temperature at the production well.  The first number in the legend is the mesh element size, while the second is the fracture element size.

!media porous_flow/examples/multiapp_flow/matrix_app_T.png
	style=width:60%;margin:auto;padding-top:2.5%;
	id=matrix_app_T
	caption=Long-term temperature at the production well.  The first number in the legend is the mesh element size, while the second is the fracture element size.

[matrix_T] shows the evolution of the cooled matrix material.  By 1000 days, an envelope of 10--20m around the fracture system has cooled by more than 10degC.  Some parts of the fracture are not cooled at all by the injectate, most particularly those at the top of the network.

!media porous_flow/examples/multiapp_flow/matrix_T.mp4
	style=width:60%;margin:auto;padding-top:2.5%;
	id=matrix_T
	caption=Evolution of the cooled matrix material.  Colors on the fracture system show fracture temperature.  The small boxes are matrix elements that have cooled by more than 10degC.  The green color-bar shows the time simulated.

## Extensions and comments

This page has described a sample model and workflow for simulating mixed-dimensional, fractured porous systems with PorousFlow.  Various simplifying assumptions have been made, and these may need to be modified in a more sophisticated approach.

- One of the key assumptions is that fracture aperture is proportional to porepressure change, [eqn.frac.open], and the value of $A = 10^{-3}\,$m.MPa$^{-1}$ was simply chosen to make the results "look reasonable".  Ideally, this assumption and value of $A$ should be checked experimentally.  Alternatively, modellers may implement other well-known equations, which may require a small amount of code development in PorousFlow.

- It is likely that the aperture is temperature-dependent, since when the cooled matrix contracts, the fracture will dilate.  This effect (if linear) can easily be modelled using [PorousFlowPorosityLinear](PorousFlowPorosityLinear.md).

- No mechanical effects have been included, except via [eqn.frac.open].  A different approach would be to treat the matrix as a THM system, transferring the fracture porepressure as an "external" normal stress, $\sigma_{nn}$, applied in matrix elements containing the fracture.  This could be applied as [ReporterPointSources](ReporterPointSource.md).  The matrix deforms in response to this as well as changes in matrix temperature.  The normal component of the strain, $\epsilon_{nn}$, could be interpreted as an aperture changed, and transferred back to the fracture.  This approach contains a few subtleties:

  - Is matrix strain a good measure of fracture opening?
  - The matrix element containing the fracture probably needs to be prescribed a modified stiffness, to ensure it adequately models a "fractured element"
  - Fracture porepressure is a nodal quantity, so for non-planar fractures the normal direction is undefined, so creating an elemental representation of porepressure might be advantageous
  - Careful consideration of fracture area contained by a matrix element might be advantageous.
  - Attributing strain in cases where a single matrix element contains multiple fracture elements might be complicated
  - If fracture elements are huge compared with matrix elements, only the matrix element that happens to contain a fracture node (or element centroid if using elemental porepressure) will experience the additional $\sigma_{nn}$, while only the matrix element that lies nearest the fracture centroid will provide $\epsilon_{nn}$.

- An alternate way of directly including mechanics could be to use the XFEM module and explicitly break matrix elements when they contain fracture elements.  The advantage over the previous approach is that it's easy to attribute strain to fracture dilation, but the disadvantage is the complexity of this approach.

- Only heat transfer between the fracture and matrix has been considered.  Using the methodology outlined in the [mathematics page](multiapp_fracture_flow_PorousFlow_3D.md), both fluid transfer and advective heat transfer could be included.

- Each fracture has been assumed to have an insitu aperture of 0.1$\,$mm.  Instead, each fracture could have a unique insitu aperture, and a unique scaling with pressure, by using different material blocks (subdomains).

- Fracture porosity has been assumed to be independent of $a$ (and hence the 2D version is $\propto a$), while permeability has been assumed to scale as $a^{2}$ (hence the 2D version is $\propto a^{3}$).  More elaborate versions are certainly possible, but possibly need to be coded into PorousFlow.

- The matrix porosity and permeability have been assumed constant and isotropic.  Instead, pressure, temperature and strain-dependent versions already coded into PorousFlow could be used.

- It is likely that the near-well physics is not accurately captured by PorousFlow, and different physics could be used instead if near-well phenomena and accurate computations of pump pressures are of interest.

- [table:time_scales] and the results make it clear that large matrix elements result in "short"-time inaccuracies.  It is therefore tempting to refine the matrix mesh around the fracture network.  As described in the [transfers page](multiapp_fracture_flow_transfers.md), it might be advantageous to use different Transfers if the relative mesh sizes are altered.  In the case where matrix elements are smaller than fracture elements, only those few matrix elements that contain fracture nodes will receive heat energy from the fracture system, while only those that lie at the centroid of a fracture element contribute to the aperture calculation.

- The injection and production pumps have been modelled in a very simplistic way, and they only act on the fracture system.  It would be relatively straightforward to use different pumping strategies.
