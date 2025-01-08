# Groundwater models

Authors: Andy Wilkins and Ma&#235;lle Grass

The PorousFlow module may be used to simulate groundwater systems.   A typical example can be found in [!citet](herron).  Large and complex models may be built that include the effect of rainfall patterns, spatially and temporally varying evaporation and transpiration, seasonal river flows, realistic topography and hydrogeology, as well as human factors such as mines, water bores, dams, etc.

It is also possible to include the vadose zone (unsaturated flow), to track the transport of tracers and pollutants through the system (multi-component flow), to utilize high-precision equations of state for the groundwater (density and viscosity as functions of porepressure and temperature), to include heat flows, to include the impact of rock deformation (solid mechanics), to include gas flows (multi-phase flows) and geochemical reactions.  This page concentrates mostly on the construction of traditional groundwater models and only touches lightly on these more elaborate options, which are explained in the [other PorousFlow examples, tutorials and tests](porous_flow/index.md).

## Groundwater concepts

Because PorousFlow has been built as a general multi-component, multi-phase simulator, its models and physics are most conveniently and commonly expressed in terms of quantities that are alien to many experienced groundwater modellers, such as porepressure, permeability and porosity.  This section provides translations between more conventional concepts --- hydraulic head, hydraulic conductivity and storativity --- and the quantities usually used in PorousFlow simulations.  In the following sections, PorousFlow models are presented using the conventional groundwater quantities (hydraulic head, etc) as well as the "natural" quantities (porepressure, etc) but for the reasons mentioned below, users are encouraged to use the "natural" quantities in their models.

### Hydraulic head and porepressure

Many experienced groundwater modellers will be used to expressing their models in terms of [hydraulic head](https://en.wikipedia.org/wiki/Hydraulic_head).  For instance, if a borehole is drilled into an [aquifer](https://en.wikipedia.org/wiki/Aquifer) that has a hydraulic head of 5$\,$m, then the groundwater in the borehole will rise 5$\,$m above the water table.  (This is assuming the hydraulic head is measured relative to the local water table, which is the common convention.)  This is shown in [gw_hydraulic_head_fig].

!media porous_flow/gw_hydraulic_head.png caption=Hydraulic head, $h$, of an aquifer is measured by recording the height above the water table (the datum in this case) of groundwater in a borehole.  id=gw_hydraulic_head_fig

Hydraulic head determines the direction of groundwater flow.  For instance, if the hydraulic head of a sandstone aquifer is greater than a siltstone aquifer, groundwater will attempt to flow from the sandstone to the siltstone.  Or, if the hydraulic gradient increases from south to north --- that is, the hydraulic head is small in the southern regions and is large in the northern regions --- then groundwater will tend to flow from north to south.

PorousFlow models are more naturally expressed in terms of *porepressure*, which is the pressure of the groundwater.  This is related to hydraulic head through:
\begin{equation}
\label{eqn.p.h}
P = \rho g (h + d) \ .
\end{equation}
Here:

- $P$ is porepressure, measured in Pa
- $h$ is hydraulic head, with respect to a fixed datum such as the elevation above sea level of water table at a monitoring site, measured in m
- $d$ is depth below the fixed datum, measured in m
- $\rho$ is the water density (approximately 1000$\,$kg.m$^{-3}$) and $g$ is the acceleration due to gravity (approximately 10$\,$m.s$^{-2}$), so $\rho g \approx 10^{4}\,$Pa.m$^{-1}$.  (Recall 1$\,$Pa = 1$\,$N.m$^{-2}$ = 1$\,$kg.m$^{-1}$.s$^{-2}$.)

Consider a [confined aquifer](https://en.wikipedia.org/wiki/Aquifer) with constant hydraulic head, $h$.  This means that the groundwater in a borehole will rise $h$ metres above the fixed datum (e.g. the local groundwater level), irrespective of where the borehole is drilled and how deep it is (assuming the well intersects this aquifer and not another one).  [eqn.p.h] shows that porepressure increases with depth!  This may be conceptually challenging for modellers used to hydraulic head.  Remember, porepressure is the actual pressure of the groundwater.  It increases with depth because of the weight of the groundwater sitting above it.

Porepressure is advantageous to use in PorousFlow because users will often like to enhance their models using the advanced inbuilt features of PorousFlow, such as

- realistic equations of state for the water (or brine), in which density and viscosity are functions of porepressure
- vadose-zone physics (unsaturated flow) which is conventionally expressed in terms of porepressure
- coupling with solid mechanics through the effective stress, which is expressed in terms of porepressure

Below, models are expressed in terms of hydraulic head and porepressure, but in order to facilitate easy extension of their models, users are encouraged to utilize porepressure.

### Hydraulic conductivity and permeability

Imagine a pipe of cross-sectional area $A$ filled with porous material, as shown in [gw_darcy_flow_fig]  Assume there is a constant hydraulic gradient between its ends:
\begin{equation}
\frac{\mathrm{d}h}{\mathrm{d}x} = \frac{h_{\mathrm{right\ end}} - h_{\mathrm{left\ end}}}{\mathrm{pipe\ length}} \ .
\end{equation}
[Darcy's law](https://en.wikipedia.org/wiki/Darcy%27s_law) states that the flow rate of water, $Q$, is proportional to the hydraulic gradient:
\begin{equation}
\label{darcy.eqn.h}
Q = -KA\frac{\mathrm{d}h}{\mathrm{d}x} \ .
\end{equation}
In this equation:

- $Q$ is the flow rate of water through the pipe, measured in m$^{3}$.s$^{-1}$
- $A$ is the cross-sectional area of the pipe, measured in m$^{2}$
- $K$ is the *hydraulic conductivity* of the porous material inside the pipe, measured in m.s$^{-1}$
- $\mathrm{d}h/\mathrm{d}x$ is the hydraulic gradient, and the negative sign indicates water flows from regions of high head to low head.

!media porous_flow/gw_darcy_flow.png caption=Darcy flow, $Q$, through a pipe of cross-sectional area $A$ due to a hydraulic gradient.  id=gw_darcy_flow_fig

To provide extra context for the meaning of hydraulic conductivity, imagine a vertical column of saturated porous material that is open at both ends.  This could be the rock above an underground mine, for instance.  Gravity will cause the water to move downwards (and be replaced by air at the top end).  The hydraulic head in this situation is $h = z$, where $z$ is the elevation (and the porepressure is zero, or atmospheric pressure), so $\mathrm{d}h/\mathrm{d}z = 1$.  The flow of water per unit area is therefore
\begin{equation}
Q/A = K
\end{equation}
That is, under gravity-driven flow, the flow-rate of water controlled by $K$, measured in m$^{3}$(water)/m$^{2}$(area)/s.

The physics of PorousFlow is more naturally presented in terms of *permeability* rather than hydraulic conductivity.  Permeability is an intrinsic quantity of a porous material, in contrast to hydraulic conductivity, which only pertains to water.  Permeability is therefore useful in multi-phase systems that may not even include water, and in situations where the fluids do not have constant density.  Hence, permeability is usually used in the petroleum literature, geothermal scenarios (where the density of water is varying), unconventional-gas models, etc.

Writing [darcy.eqn.h] using the porepressure of [eqn.p.h] yields
\begin{equation}
\frac{Q}{A} = - \frac{K}{\rho g} \frac{\mathrm{d}P}{\mathrm{d}x} = - \frac{k}{\mu} \frac{\mathrm{d}P}{\mathrm{d}x}
\end{equation}
In these equations:

- it is assumed that depth, $d$, does not depend on $x$.  If the $x$ axis were oriented vertically then the equations would include $\mathrm{d}P/\mathrm{d}x - \rho g$ instead of just $\mathrm{d}P/\mathrm{d}x$.
- $k$ is the *permeability*, measured in m$^{2}$.
- $\mu$ is the water dynamic viscosity, measured in Pa.s.

The relationship between the permeability and hydraulic conductivity is therefore:
\begin{equation}
\label{eqn.hyd.cond.defn}
K = k \frac{\rho g}{\mu} \ ,
\end{equation}
where $\rho$ is the density of water and $\mu$ is its dynamic viscosity.

Permeability is often measured in [Darcys](https://en.wikipedia.org/wiki/Darcy_%28unit%29) instead of its natural unit of m$^{2}$.  The conversion is
\begin{equation}
k \approx 1\,\mathrm{Darcy} \Longleftrightarrow k\approx 10^{-12}\,\mathrm{m}^{2} \Longleftrightarrow K \approx 10^{-5}\,\mathrm{m}.\mathrm{s}^{-1} \Longleftrightarrow K \approx 1\,\mathrm{m}.\mathrm{day}^{-1} \ .
\end{equation}

Both permeability and hydraulic conductivity are tensors, so PorousFlow demands that users enter the full tensorial quantity, even though for most cases this is of the form

```
permeability = 'horizontal_x 0 0  0 horizontal_y 0  0 0 vertical'
```


### Storativity and porosity

A porous material like rock is conceptualised as consisting of solid rock grains and void space.  Groundwater flows through the void space.  Both [storativity](https://en.wikipedia.org/wiki/Specific_storage) and porosity parameterise the amount of groundwater present in a volume of porous material.

[Porosity](/porous_flow/porosity.md) is defined as:
\begin{equation}
\phi = \frac{V_{\mathrm{void}}}{V_{\mathrm{porous\ material}}} = \frac{V_{\mathrm{void}}}{V_{\mathrm{void}} + V_{\mathrm{solid\ grains}}}
\end{equation}
Here

- $\phi$ is the porosity, which is dimensionless
- $V_{\mathrm{void}}$ is the volume of connected void space, measured in m$^{3}$.  (There may also be unconnected void spaces, which are ignored in this presentation since they are unimportant for fluid flow and only impact the solid-mechanical deformations.)
- $V_{\mathrm{porous\ material}}$ is the volume of the porous material (rock) sample, measured in m$^{3}$
- $V_{\mathrm{solid\ grains}}$ is the volume of the solid grains (containing no void space), measured in m$^{3}$

Hence, if the porous material is fully saturated with water at atmospheric pressure, it contains a volume of $\phi V_{\mathrm{porous\ material}}$ of water at atmospheric pressure.

However, in the general situation, the volume of water that can be extracted from a volume of porous material is often not $\phi V_{\mathrm{porous\ material}}$ because of the following.

1. The porous material may not be fully saturated with water, meaning that the water volume is less than $\phi V_{\mathrm{porous\ material}}$
2. Even if fully saturated, the water may be under high pressure, and since it is slightly compressible and the rock grains are slightly compressible, the water volume at atmospheric pressure may be greater than $\phi V_{\mathrm{porous\ material}}$.  Similarly, because of the compressibilities, $\phi$ can depend on porepressure, so the $\phi$ that is measured at atmospheric pressure in a lab may be different than the in-situ $\phi$.
3. Due to unsaturated physics --- namely [capillarity](PorousFlowCapillaryPressureVG.md) and [relative permeability](PorousFlowRelativePermeabilityCorey.md) --- it may be impossible to extract all the water without resorting to extreme pressures, temperatures or chemical stimulants.

All these phenomena are handled automatically by PorousFlow, so porosity is usually employed in PorousFlow models.  Porosity is also a natural quantity to use in multi-phase flow scenarios.  In most groundwater models, [porosity](PorousFlowPorosityConst.md) or its cousin, the [BiotModulus](PorousFlowConstantBiotModulus.md), are simply specified as fixed quantities for each hydrologic unit.  In more elaborate models, porosity is specified at atmospheric pressure and temperature, at zero strain, and in-situ amount of mineralisation, and a [PorousFlowPorosity](PorousFlowPorosity.md) Material handles any changes due to changes in porepressure, temperature, deformation and dissolution/precipitation.

However, for the 3 reasons stated above, storativity, specific yield and specific storage are often used instead of porosity in groundwater models, reports and literature.  These are defined as follows.

#### Storativity or storage coefficient

\begin{equation}
S = S_{s}b + S_{y}
\end{equation}
Here

- $S$ is the storativity, which is sometimes called the storage coefficient, which is dimensionless
- $S_{s}$ is the specific storage, measured in m$^{-1}$
- $b$ is the aquifer thickness, measured in m
- $S_{y}$ is the specific yield, which is dimensionless

#### Specific storage

\begin{equation}
\label{eqn.ss.defn}
S_{s} \approx \rho g(C_{\mathrm{grains}} + \phi C_{\mathrm{water}}) \approx 10^{4}C_{\mathrm{grains}} + 5\times 10^{-6} \phi
\end{equation}
The $\approx$ symbol has been used instead of equality to emphasise that various assumptions have been made concerning the interaction between water pressure and solid-mechanical stresses and strains.  In this formula:

- $S_{s}$ is the specific storage, measured in m$^{-1}$
- $\rho$ is the water density (approximately 1000$\,$kg.m$^{-3}$) and $g$ is the acceleration due to gravity (approximately 10$\,$m.s$^{-2}$), so $\rho g \approx 10^{4}\,$Pa.m$^{-1}$.
- $C_{\mathrm{grains}}$ is the compressibility of the rock grains (the porous material without any void space), which is the reciprocal of its bulk modulus, measured in Pa$^{-1}$.  Typical values are $10^{-7}\,$Pa$^{-1}$ for clay, $10^{-9}\,$Pa$^{-1}$ for sandstone, $10^{-10}\,$Pa$^{-1}$ for shale, $<10^{-10}\,$Pa$^{-1}$ for limestone and granite.
- $C_{\mathrm{water}}$ is the compressibility of water (approximately $5\times 10^{-10}\,$Pa$^{-1}$), measured in Pa$^{-1}$.

#### Specific yield

Specific yield, $S_{y}$, is the volume of water that can be drained by gravity from a volume of porous material.  It is therefore close to the porosity, $S_{y} \approx \phi$.  However, due to unsaturated physics, it is typically a little less than $\phi$.

## Groundwater equations for confined situations

If you dislike equations as much as many groundwater hydrogeologists feel free to read the following note and skip to the next section!

!alert note
There are many different versions of the groundwater flow equations, and even more numerical implementations of them.  Each produces slightly different solutions.  The differences are tiny compared with the uncertainties in input parameters such as an aquifer's hydraulic conductivity.

Groundwater models often consider only fully-saturated scenarios such as confined aquifers, and do not consider the vadose zone.  This section contains a number of versions of the physical equations governing the groundwater flow in such situations.

These equations are sometimes used to model unsaturated flow (as in unconfined aquifers) however, strictly speaking their physics does not accurately describe such flows.  Using the equations below for both saturated and unsaturated zones means your model will be simple and will run rapidly.  The porepressure will be positive in fully-saturated regions, and negative in unsaturated regions.  If required, you will need to invent your own relationship between saturation and the negative porepressure.  Alternately, you may use a [PorousFlowUnsaturated](PorousFlowUnsaturated.md) Action instead of a [PorousFlow BasicTHM](PorousFlowBasicTHM.md) to accurately simulate models with both saturated and unsaturated zones, at the cost of providing extra information (the capillary and relative permeability functions) and computational speed.

### Presentation 1

Groundwater flow is often modelled by the [equation](porous_flow/governing_equations.md)
\begin{equation}
\label{eqn.full.sat}
\frac{\partial}{\partial t}(\phi \rho) = \sum_{i=1}^{3} \sum_{j=1}^{3}\nabla_{i} \left[\rho \frac{k_{ij}}{\mu} \left(\nabla_{j} P - \rho g_{j}\right) \right]\ .
\end{equation}
Here:

- $t$ is time, measured in s
- $\phi$ is the porosity, which is dimensionless.  It may vary spatially, corresponding to different aquifers and aquitards, for instance.
- $\rho$ is the water density, measured in kg.m$^{-3}$
- $\nabla_{i}$ is a vector of spatial derivatives, $(\partial/\partial x, \ \partial/\partial y, \ \partial/\partial z)$, measured in m$^{-1}$.  The index $i$ is 1, 2 or 3, depending on the spatial direction.  If the situation is 2D then the sums in [eqn.full.sat] run over 1 and 2 only.
- $k_{ij}$ is the permeability tensor, $(k_{xx},\  k_{xy},\  k_{xz},\  k_{yx},\  k_{yy},\  k_{yz},\  k_{zx},\  k_{zy},\  k_{zz})$, measured in m$^{2}$.  Usually this is transversely isotropic: $k_{ij} = (k_{\mathrm{horizontal}},\  0,\  0,\  0,\  k_{\mathrm{horizontal}},\  0,\  0,\  0,\  k_{\mathrm{vertical}})$
- $\mu$ is the viscosity of water, measured in Pa.s
- $P$ is the porepressure
- $g_{j}$ is the acceleration due to gravity, measured in kg.m$^{2}$.s$^{-1}$.  If the $z$ axis points upwards, this is $g_{j} = (0,\  0,\  -10)$ (i.e., $g_{j}$ points in the direction of falling).

### Presentation 2

[eqn.full.sat] may be extended to the situation in which the porous-material (rock) grains are compressible and may suffer volumetric expansion or compaction due to changes in groundwater porepressure.  In this case the equation reads
\begin{equation}
\label{eqn.full.sat.vol}
\frac{\partial}{\partial t}(\phi \rho) + \phi\rho\frac{\partial}{\partial t}\epsilon_{v} = \sum_{i=1}^{3} \sum_{j=1}^{3}\nabla_{i} \left[\rho \frac{k_{ij}}{\mu} \left(\nabla_{j} P - \rho g_{j}\right) \right]\ .
\end{equation}
In this equation $\epsilon_{v}$ is the volumetric strain of the porous material.

### Presentation 3

As described [here](PorousFlowFullySaturatedMassTimeDerivative.md), various standard assumptions allow the left-hand side of [eqn.full.sat.vol] to be re-written:
\begin{equation}
\label{eqn.introduce_biot}
\frac{\partial}{\partial t}(\phi \rho) + \phi\rho\frac{\partial}{\partial t}\epsilon_{v} = \rho \frac{1}{M}\frac{\partial}{\partial t}P
\end{equation}
In this equation $M$ is the so-called [Biot modulus](PorousFlowFullySaturatedMassTimeDerivative.md), measured in Pa:
\begin{equation}
\label{eqn.biotM.defn}
\frac{1}{M} = \phi C_{\mathrm{water}} + (1 - \alpha_{B})(\alpha_{B} - \phi)C_{\mathrm{grains}} \ .
\end{equation}
Here:

- $\phi$ is the porosity
- $C_{\mathrm{water}}$ is the compressibility of water (approximately $5\times 10^{-10}\,$Pa$^{-1}$), measured in Pa$^{-1}$.
- $C_{\mathrm{grains}}$ is the compressibility of the rock grains (the porous material without any void space), which is the reciprocal of its bulk modulus, measured in Pa$^{-1}$.
- $\alpha_{B}$ is the Biot coefficient, which is dimensionless, and describes the impact of the groundwater on solid-mechanical stresses.  $\alpha_{B}$ is usually chosen within the range $\phi \leq \alpha_{B}\leq 1$.  If $\alpha_{B} \approx 1$ then the groundwater has maximum impact on the solid-mechanical stresses.

In groundwater scenarios, the Biot modulus may be considered time-independent and only dependent on the porous material via material-dependent $\phi$ and $C_{\mathrm{grains}}$.

Using [eqn.introduce_biot] in [eqn.full.sat.vol] results in
\begin{equation}
\label{eqn.full.sat.biotM}
\rho \frac{1}{M}\frac{\partial}{\partial t}P = \sum_{i=1}^{3} \sum_{j=1}^{3}\nabla_{i} \left[\rho \frac{k_{ij}}{\mu} \left(\nabla_{j} P - \rho g_{j}\right) \right]\ .
\end{equation}

### Presentation 4

Assuming the density of water has very little spatial variation, it may be divided out from [eqn.full.sat.biotM] leaving a diffusion equation:
\begin{equation}
\label{eqn.full.sat.biotM.norho}
\frac{1}{M}\frac{\partial}{\partial t}P = \sum_{i=1}^{3} \sum_{j=1}^{3}\nabla_{i} \left[\frac{k_{ij}}{\mu} \left(\nabla_{j} P - \rho g_{j}\right) \right]\ .
\end{equation}

### Presentation 5

Notice the similarity of [eqn.ss.defn] and [eqn.biotM.defn].  To reasonable accuracy
\begin{equation}
\label{eqn.ss.biotM}
S_{s} = \frac{\rho g}{M} \ .
\end{equation}
Hence [eqn.full.sat.biotM] may be written
\begin{equation}
\label{eqn.full.sat.Ss}
S_{s}\frac{\partial}{\partial t}P = \sum_{i=1}^{3} \sum_{j=1}^{3}\nabla_{i} \left[\rho g \frac{k_{ij}}{\mu} \left(\nabla_{j} P - \rho g_{j}\right) \right]\ .
\end{equation}

### Presentation 6

Using the definition of hydraulic head, [eqn.p.h], and hydraulic conductivity, [eqn.hyd.cond.defn] in [eqn.full.sat.Ss], and making further assumptions based on the almost-incompressibility of water yields the
[usual groundwater flow equation](https://en.wikipedia.org/wiki/Groundwater_flow_equation)
\begin{equation}
\label{eqn.usual.gw}
S_{s}\frac{\partial}{\partial t}h = \sum_{i=1}^{3} \sum_{j=1}^{3}\nabla_{i} \left[K_{ij}\nabla_{j} h\right] \ .
\end{equation}


## Simulating a pumping test

To illustrate the basic setup of a PorousFlow groundwater model, an [aquifer pumping test](https://en.wikipedia.org/wiki/Aquifer_test).  This example forms part of the PorousFlow QA test suite and is documented from that perspective [here](tests/dirackernels/dirackernels_tests.md).

Place a constant volumetric sink of strength $q$ (m$^{3}$.m$^{-1}$.s$^{-1}$) acting along an infinite line in an isotropic 3D medium.  The situation is therefore two dimensional.  Assuming that groundwater head obeys [eqn.usual.gw], Theis provided the solution for the head
\begin{equation}
H = H_{0} + \frac{q}{4\pi K}\mathrm{Ei}\left( -\frac{r^{2}S_{s}}{4Kt} \right) \ ,
\label{eqn.theis_soln_eqn}
\end{equation}
which is frequently used in the groundwater literature.  "Ei" is the exponential integral function, with values $\mathrm{Ei}(-x) = \gamma + log(x) - x + O(x^{2})$ for small $x$ (where $\gamma$ is the Euler number $0.57722\ldots$), and $\mathrm{Ei}(-x) = e^{-x}\left(-\frac{1}{x} + \frac{1}{x^{2}} + O(\frac{1}{x^{3}})\right)$ for large $x$.

A sketch of the modelled situation is shown in [gw_theis_fig].

!media porous_flow/gw_theis.png caption=Sketch of a pumping test.  A well extracts water at a constant rate from an aquifer with initial porepressure 20$\,$MPa, which causes drawdown, as shown by the blue curve.  id=gw_theis_fig

The MOOSE input file may be broken down into a number of sections, as follows.

### The mesh

The situation is two dimensional, but radially symmetric, so can be simulated using a one-dimensional mesh, with a single radial coordinate:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i start=[Mesh] end=[GlobalParams]

### The PorousFlowDictator

All PorousFlow input files must include a [PorousFlowDictator](PorousFlowDictator.md).  This specifies the number of fluid phases (1 for groundwater models), number of fluid components (1 for groundwater models without tracers) and various other things that enable consistency checking throughout the input file.  Virtually all PorousFlow objects require a `PorousFlowDictator` input parameter.  Hence, to shorten input files, virtually all PorousFlow input files contain the PorousFlowDictator information in their GlobalParams block:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i block=GlobalParams

This simply means you do not have to type `PorousFlowDictator = dictator` in all the other PorousFlow objects in your input files.


### The physics

In order to replicate [eqn.theis_soln_eqn], the physics needs to be described by [eqn.usual.gw].  This may be achieved by employing a
[PorousFlowBasicTHM](PorousFlowBasicTHM.md) Action:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i block=PorousFlowBasicTHM

At this stage, it is probably a good idea for readers to skim-read the description of the [PorousFlowBasicTHM](PorousFlowBasicTHM.md) Action, as it will be used extensively in this page.

The Action implements [eqn.full.sat.biotM].  The Action also implements coupling with solid mechanics and heat flow, which are not of interest in simple groundwater models.  To ignore these couplings, simply do not specify the `coupling_type` input parameter, since it defaults to `hydro`.  Note the flags:

```
  gravity = '0 0 0'
  multiply_by_density = false
```

which mean that [eqn.full.sat.biotM] becomes (in 1D, without the $i$ and $j$ indices)
\begin{equation}
\frac{1}{M}\frac{\partial}{\partial t}P = \nabla \left[\frac{k}{\mu} \nabla P \right] \ .
\end{equation}
This is of the same form as [eqn.usual.gw] if the relationships mentioned above are used (with $\rho g = 10^{4}\,$Pa.m$^{-1}$ and $\mu = 10^{-3}\,$Pa.s), namely:

- $P = \rho g h = 10^{4} h$
- $M = \rho g / S_{s} = 10^{4} / S_{s}$
- $k = \mu K / (\rho g) = 10^{-7} K$

Hence, the action is implementing the desired equation, assuming the parameters $M$ and $k$ are chosen appropriately.

Some additional remarks are:

- The Action also implements [eqn.full.sat.biotM.norho] when the flag `multiply_by_density = false`.
- The Action also implements [eqn.full.sat.Ss] when the Biot Modulus (see below) is set to $M = \rho g S_{s}$ as in [eqn.ss.biotM]
- The Action does not have a "hydraulic head" input parameter.  Instead, it assumes that modellers are using "porepressure", users must supply the name of the porepressure variable using the `porepressure = XXXX` input (here `XXXX` is the name of the porepressure variable).
- In the above block `add_darcy_aux = false` is used simply because the [QA test](tests/dirackernels/dirackernels_tests.md) does not involve checking the Darcy-velocity directions.  Usually you will want to omit this line and PorousFlow will automatically add Darcy-velocity information to your output.

### Variables

In this input file, and most groundwater input files, there is just one `Variable` (unknown to solve for).  It is called `pp` here, although it may be interpreted as hydraulic head using the scaling $P = 10^{4}h$:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i block=Variables

### Equation of state for water

PorousFlow requires users to specify their "equation of state".  These are functional relationships that provide density and viscosity (and thermal conductivity, enthalpy, etc) as functions of porepressure and temperature.  In virtually all groundwater models a [SimpleFluidProperties](SimpleFluidProperties.md) object is sufficient:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i block=FluidProperties

Here:

- the `viscosity` is specified exactly, to make the $k = \mu K/(\rho g)$ relationship clearer in this example.
- the name `simple_fluid` is provided to the PorousFlowBasicTHM, above

### Material properties

The permeability and Biot Modulus need to be specified.  This is always done via a Materials block:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i block=Materials

In this case, setting `porosity = 0.05`, `fluid_bulk_modulus = 2E9` and `biot_coefficient = 1.0` means that the [BiotModulus](PorousFlowConstantBiotModulus.md) and the specific storage are
\begin{equation}
M = \frac{2}{0.05} = 40\,\mathrm{GPa} \Longrightarrow S_{s} = 2.5\times 10^{-7}\,\mathrm{m}^{-1}
\end{equation}
The permeability is isotropic:
\begin{equation}
k = 10^{-14}\,\mathrm{m}^{2} \Longrightarrow K = 10^{-7}\,\mathrm{m}.\mathrm{s}^{-1} \ .
\end{equation}

### The produced water flux

This is implemented by a [PorousFlowSquarePulsePointSource](PorousFlowSquarePulsePointSource.md):

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i block=DiracKernels

!alert note
In this 2D case with `multiply_by_density = false`, this means $0.16\times 10^{-3}\,$m$^{3}$ of water is removed from every 1$\,$m of aquifer height every second.  If `multiply_by_density = true` and the model was 2D then the `mass_flux` in a PorousFlowSquarePulsePointSource would be measured in kg.s$^{-1}$.m$^{-1}$, and if the model was 3D the units would be kg.s$^{-1}$.

### The entire input file

The remainder of the input file involves the usual MOOSE objects, such as an Executioner to define the end time and time-stepping, as well as a Preconditioning block, and some output objects.  The entire input file reads:

!listing modules/porous_flow/test/tests/dirackernels/theis_rz.i

### The results

MOOSE agrees with the Theis solution:

!media dirackernels/theis.png caption=Results of the aquifer pumping test in comparison with the Theis solution. id=theis_fig

## Abstracting from a groundwater system

In this section, a model of the groundwater system shown in [groundwater_ex01_mesh_fig] is studied.

!media groundwater_ex01_mesh.png caption=The mesh and properties used in the abstraction example.  id=groundwater_ex01_mesh_fig

The system consists of:

- An upper aquifer of thickness 10$\,$m with porosity, $\phi = 0.05$, and isotropic permeability $k = 10^{-12}\,$m$^{2}$.  With Biot coefficient $\alpha_{B} = 1$ and water bulk compliance $C_{\mathrm{water}} = 5\times 10^{-10}\,$Pa$^{-1}$, [eqn.biotM.defn] implies the Biot modulus is $M = 40\,$GPa, and [eqn.ss.biotM] implies the specific storage is $S_{s} =2.5\times 10^{-7}\,$m$^{-1}$.  This number is also produced by [eqn.ss.defn] if $C_{\mathrm{grains}} = 0$ ([eqn.ss.defn] does not contain the Biot coefficient: it approximates $(1-\alpha_{B})(\alpha_{B} - \phi) \approx 1$).  Using [eqn.hyd.cond.defn], the hydraulic conductivity is $K\approx 1\,$m.day$^{-1}$.
- An aquitard of thickness 10$\,$m with porosity $\phi = 0.2$, horizontal permeability $k_{\mathrm{hor}} = 10^{-16}\,$m$^{2}$ and vertical permeability $k_{\mathrm{ver}} = 10^{-17}\,$m$^{2}$.  Using the same manipulations as in the preceding paragraph, this yields Biot modulus $M = 10\,$GPa, specific storage $S_{s} \approx 10^{-6}\,$m$^{-1}$, horizontal permeability $K_{\mathrm{hor}} \approx 0.1\,$mm.day$^{-1}$ and $K_{\mathrm{ver}} \approx 0.01\,$mm.day$^{-1}$.
- A lower aquifer of thickness 10$\,$m with the same physical parameters as the upper aquifer.

This system is assumed to exist in isolation from any other aquifers and aquitards.  The roof of the upper aquifer is 70$\,$m below the water table.

A borehole abstracts water from either the lower aquifer or the aquitard (due to a mistake in the drilling or wellbore completion) and a model is built to investigate the drawdown in the upper aquifer due to these two scenarios.


### The finite-element mesh

Creating the mesh is the most difficult aspect of numerical modelling in almost all models, be they groundwater models, structural-mechanical models, models of fluid turbulence, electromagnetic models, etc.  For instance, the groundwater model in [!citet](herron) took about 3 months to construct since it contained over 100 underground mine workings, many aquifers and aquitards, streams and rivers and alluvium, zones of different evapotranspiration, the ocean, thousands of boreholes, etc.  For groundwater modelling, only conceptualisation and parameterisation are more difficult, but these also impact the mesh, for example, there is no need to distinguish between different aquifers if their hydraulic conductivities are the same, up to measurement accuracy.

In this case, the mesh is very simple and may be created using MOOSE's inbuilt commands with the result shown in [groundwater_ex01_mesh_fig]:

!listing modules/porous_flow/examples/groundwater/ex01.i block=Mesh


### Equation of state for water

PorousFlow requires users to specify their "equation of state".  These are functional relationships that provide density and viscosity (and thermal conductivity, enthalpy, etc) as functions of porepressure and temperature.  In virtually all groundwater models a [SimpleFluidProperties](SimpleFluidProperties.md) object is sufficient.  In this case, the fluid bulk modulus is chosen very large and the thermal expansion coefficient chosen to be zero so that the groundwater density is very close to 1000$\,$kg.m$^{-3}$ so that $\rho g = 10^{4}\,$Pa.m$^{-1}$.

!listing modules/porous_flow/examples/groundwater/ex01.i block=FluidProperties

### PorousFlowDictator specification

Most objects in PorousFlow models require the [PorousFlowDictator](PorousFlowDictator.md) to be specified, so this is most efficiently accomplished through the GlobalParams block:

!listing modules/porous_flow/examples/groundwater/ex01.i block=GlobalParams

### Variables

The only MOOSE variable (unknown) in this situation is the porepressure.

!listing modules/porous_flow/examples/groundwater/ex01.i block=Variables

### The physics

It is assumed that groundwater obeys [eqn.full.sat.biotM.norho].
This physics is implemented in PorousFlow by the [PorousFlowBasicTHM](PorousFlowBasicTHM.md) Action with `multiply_by_density = false`:

!listing modules/porous_flow/examples/groundwater/ex01.i block=PorousFlowBasicTHM

The physics is of the same form as [eqn.usual.gw] if the relationships mentioned above are used (with $\rho g = 10^{4}\,$Pa.m$^{-1}$ and $\mu = 10^{-3}\,$Pa.s), namely:

- $P = \rho g h = 10^{4} h$
- $M = \rho g / S_{s} = 10^{4} / S_{s}$
- $k = \mu K / (\rho g) = 10^{-7} K$

As mentioned in the section above, the Action does not have a "hydraulic head" input parameter.  Instead, it assumes that modellers are using "porepressure", users must supply the name of the porepressure variable using the `porepressure = XXXX` input (here `XXXX` is the name of the porepressure variable).

### In-situ conditions: hydraulic gradient

Assume that:

- That the porepressure is related to hydraulic head via [eqn.p.h] with $\rho g = 10^{4}$
- The average hydraulic head in the upper aquifer is 10$\,$m and there is a hydraulic gradient of $1/200$ from east to west in the upper aquifer
- The hydraulic head in the lower aquifer is 20$\,$m.  This means the model is not at [hydrostatic equilibrium](https://en.wikipedia.org/wiki/Hydrostatic_equilibrium) which could be due to recharge into the lower aquifer from a hilly region outside the model
- The hydraulic head in the aquitard is a linear combination of these two

\begin{equation}
h_{\mathrm{insitu}} = \left\{
\begin{array}{ll}
10 + \frac{x}{200} & \mathrm{if}\ z \geq -80 \\
\frac{(10 + \frac{x}{200})(z + 90) - 20(z + 80)}{10} & \mathrm{if}\ -80 < z < -90 \\
20 & \mathrm{if}\ z \leq -90
\end{array}
\right.
\end{equation}

The following functions then describe the in-situ conditions:

!listing modules/porous_flow/examples/groundwater/ex01.i start=[upper_aquifer_head] end=[l_rate]

### Initial and boundary conditions

Assume that the initial conditions are as specified by the `insitu_pp` Function describe above.  Assume also that porepressure is fixed on the model's boundaries.  This corresponds to a ready source of water that flows into the model from regions outside the model.  The relevant input-file blocks are:

!listing modules/porous_flow/examples/groundwater/ex01.i block=ICs

!listing modules/porous_flow/examples/groundwater/ex01.i block=BCs

### Prescribing material properties: porosity and permeability

The material properties are prescribed using the following block

!listing modules/porous_flow/examples/groundwater/ex01.i block=Materials

Note the aquitard has anisotropic permeability: $k_{\mathrm{hor}} = 10^{-16}\,$m$^{2}$ while $k_{\mathrm{ver}} = 10^{-17}\,$m$^{2}$.

### The abstraction well

An abstraction well may be modelled using a [PorousFlow sink](sinks.md).  The reader is encouraged to study the documentation on the [PorousFlow sinks page](sinks.md) in order to understand the following block, since boreholes are frequently used for injection or production in groundwater models.  A [PorousFlowPolyLineSink](PorousFlowPolyLineSink.md) is used in this example:

!listing modules/porous_flow/examples/groundwater/ex01.i block=DiracKernels

!listing modules/porous_flow/examples/groundwater/ex01.bh_lower

The `ex01.bh_lower` file contains just one line, and the sink sets `line_length = 10` and `line_direction = '0 0 1'` (by default) so this corresponds to a vertical borehole of length 10$\,$m.  The lines

```
    p_or_t_vals = '0 1E9'
    fluxes = '0 1'
```

mean that the borehole will extract a flux of 0$\,$m$^{3}$(water)/m(borehole)/s if the porepressure is zero or lower, and a flux of 1$\,$m$^{3}$(water)/m/s if the porepressure is 1$\,$GPa or higher.  In between these two porepressures, the flux varies linearly.  Hence, this will cause porepressure to reduce to close to zero before an equilibrium is found between the borehole extraction rate and the groundwater flux through the rock.  If `multiply_by_density = true` was used in the [PorousFlowBasicTHM](PorousFlowBasicTHM.md), then the units of `fluxes` would be kg(water)/m(borehole)/s.

If a fixed flux was required instead, then `fluxes = '1 1'` could be used, or another of the [PorousFlow sinks](sinks.md) could be employed.

!alert warning
The appropriate PorousFlow sink must be used for your model, because they represent significantly different physics, so produce significantly different results.  This example uses a `PorousFlowPolyLineSink` for a borehole, while virtually all real-life models would use a `PorousFlowPeacemanBorehole` instead.  A rule of thumb is to use a `PorousFlowPeacemanBorehole` for boreholes, and a `PorousFlowPolyLineSink` for rivers.  This is discussed further towards the bottom of this page.

It is common to refine the mesh around boreholes:

- a refinement in the plan mesh allows the $(x, y)$ extent of drawdown cones to be more accurately predicted;
- a refinement in the vertical mesh so that the screened section is captured by at least one finite element is recommended as it allows drawdown on vertical sections to be accurately simulated.

Such refinements are not mandatory.  They are not performed in this example because only MOOSE's simple inbuilt meshing functionality is used.  (Using the `PorousFlowPolyLineSink` just so happens to be fine in this model because the single Dirac point coincides with a node in the $(x,y)$ plane.)


### Recording the drawdown

AuxVariables and a VectorPostprocessor are used to record the head change throughout the model:

!listing modules/porous_flow/examples/groundwater/ex01.i block=AuxVariables

!listing modules/porous_flow/examples/groundwater/ex01.i block=AuxKernels

!listing modules/porous_flow/examples/groundwater/ex01.i block=VectorPostprocessors

### The entire input file

For reference, the entire input file is:

!listing modules/porous_flow/examples/groundwater/ex01.i

### Results

Water is abstracted from the lower aquifer when `point_file = ex01.bh_lower` in the PorousFlowPolyLineSink.  This production rate is approximately $9\,$l.s$^{-1}$, and the upper aquifer experiences very little drawdown.  Water is abstracted from the aquitard when `point_file = ex01.bh_aquitard` in the PorousFlowPolyLineSink, which results in production rate of about $6\,$l.s$^{-1}$ and significant drawdown in the upper aquifer.  The drawdown cone reaches steady state after about 4 days, and is shown in [groundwater_ex01_fig].

!media groundwater_ex01.png caption=Drawdown in the upper aquifer when water is abstracted from the lower aquifer (blue line) or the aquitard (red line).  id=groundwater_ex01_fig

The results are mesh-dependent, as expected.  A finer mesh produces more drawdown near to the well since it is more able to resolve spatial structure there.

The severe abstraction from the aquitard causes negative hydraulic head in the upper aquifer.  This does not mean the porepressure is negative in this case.  However, it is easy to build a model in which the porepressure becomes negative.  This corresponds physically to desaturation, and to properly model such situations a [PorousFlowUnsaturated](PorousFlowUnsaturated.md) Action should be used instead of a [PorousFlowBasicTHM](PorousFlowBasicTHM.md) Action.  In many cases, the extra computational cost of modelling unsaturated physics, and the difficulty of appropriately parameterising the model, mean that groundwater modellers choose to ignore partially-saturated physics and simply employ the equivalent of PorousFlowBasicTHM regardless of unsaturated zones.  In PorousFlow it is relatively straightforward to swap between PorousFlowBasicTHM and PorousFlowUnsaturated to investigate the impact of including partially-saturated physics.


## Baseflow, ET, recharge, unsaturated flow: impact of groundwater abstraction on baseflow to a river

In this example, the impact of groundwater abstraction on baseflow to a river is modelled.  The hydrostratigraphy is shown in [groundwater_ex02_mesh_fig].  It consists of:

- an upper, unconfined aquitard with topography of varying elevation representing a hill and valley
- an aquitard that outcrops in the valley region
- a lower aquifer that also outcrops in the valley region

The lower confined aquifer may be recharged by a perennial river that is assumed to exist at the valley's base.  Or, it may provide baseflow to the river.  Whether leakage or baseflow occurs depends on the groundwater porepressure: the position of the water table with respect to the river stage height.

A horizontal abstraction borehole is drilled into the lower aquifer below the river.  The question to answer is: how does the groundwater abstraction influence the groundwater-river interaction?  (Such horizontal boreholes are used in gas exploitation, but the main reason it is used here is to illustrate boreholes don't need to be vertical.)

To answer this question, the parameters defining the system will be described in the context of the PorousFlow input file, then the steady-state virgin system will be solved, then the borehole introduced and the impact assessed.

!media groundwater_ex02_mesh.png caption=The hydrostratigraphy and the mesh used to explore the impact of groundwater abstraction on baseflow to the river.  The unconfined aquifer is shown in brown-yellow, the aquitard in red and the confined aquifer in blue.  The river is shown as a bright green line.  Vertical exaggeration equals 2 in this figure.  The mesh was created by a closed-source script built by CSIRO for these types of problems.  id=groundwater_ex02_mesh_fig

### The mesh

The mesh in this case was created by an external program that defined the hydrogeology and the mesh, but not the boundaries that are required for this model.  Hence, MOOSE's in-built MeshModifiers are used to identify the appropriate boundaries:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i block=Mesh

### The physics

In this case, unsaturated flow and saturated flow are used, in order to use appropriate physics for the unconfined aquifer.  Hence, instead of a PorousFlowBasicTHM Action, a [PorousFlowUnsaturated](PorousFlowUnsaturated.md) action is used instead:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i block=PorousFlowUnsaturated

Readers are encouraged to read the documentation for the [PorousFlowUnsaturated](PorousFlowUnsaturated.md) action if they need to model unsaturated flow.  Usually, the above block would contain information concerning the relative permeability and the capillary pressure functions, sourced from site experiments, but in this case just the default values are used.

!alert note
Note the important facet of `PorousFlowUnsaturated`: it does not have a `multiply_by_density` flag.  Hence, all quantities are measured in *mass*, not volume.  For instance, the recharge quantities are measured in kg.s$^{-1}$, not m$^{3}$.s$^{-1}$.

### Porosity and permeability

The porosity is assumed to be 0.05 everywhere, while the permeabilities are assumed to be:

- aquifers: $10^{-12}\,$m$^{2}$ in the horizontal direction, $10^{-13}\,$m$^{2}$ in the vertical direction
- aquitard: $10^{-16}\,$m$^{2}$ in the horizontal direction, $10^{-17}\,$m$^{2}$ in the vertical direction

These properties are specified via:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i block=Materials

### Initial conditions

It is likely that the steady-state solution will be approximately hydrostatic.  Therefore, to help PorousFlow converge, the model is initialized with the hydrostatic condition.  This is non-trivial here because, for fixed elevation ($z$), the depth varies spatially (with $x$ and $y$).  The initial mesh, however, has depth information inserted into it, as the variable `cosflow_depth`.  Hence, it may be extracted using a SolutionUserObject:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i start=[initial_mesh] end=[baseflow]

and then used in a Function:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i start=[initial_pp] end=[baseflow_rate]

before finally being used in an initial condition:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i block=ICs

This kind of manipulation --- using a SolutionUserObject and a SolutionFunction --- is rather common in groundwater modelling.

### Boundary conditions

The boundary conditions are the most complicated aspect of this input file.

#### Rainfall recharge

It is assumed that rainfall recharges the groundwater system at the topography with a constant rate of 0.1$\,$mm.day$^{-1}$, uniformly over the entire topography.  This quantity needs to be converted to a mass flux:
\begin{equation}
0.1\,\mathrm{mm}.\mathrm{day}^{-1} = 10^{-4}\,\mathrm{m}^{3}.\mathrm{m}^{-2}.\mathrm{day}^{-1} \approx 0.1\,\mathrm{kg}.\mathrm{m}^{-2}.\mathrm{day}^{-1} \approx 10^{-6}\,\mathrm{kg}.\mathrm{m}^{-2}.\mathrm{s}^{-1}
\end{equation}
Hence the BC:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i start=[rainfall_recharge] end=[evapotranspiration]

In most groundwater models, rainfall recharge would vary spatially and temporally, since different surficial expressions and vegetation cover and seasonal patterns impact rainfall recharge.  This is easily incorporated through the `flux_function` or other parameters to the [PorousFlowSink](PorousFlowSink.md).

#### Evapotranspiration (ET)

A [PorousFlowHalfCubicSink](PorousFlowHalfCubicSink.md) is used to model ET:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i start=[evapotranspiration] end=[]

This block requires some explanation, but the reader is strongly encouraged to read the PorousFlow documentation on the [PorousFlowHalfCubicSink](PorousFlowHalfCubicSink.md) and [PorousFlow boundary conditions in general](boundaries.md) for further information.

It is assumed that:

- ET is zero if the groundwater head is more than 5$\,$m below the topography, corresponding to a maximum root depth of plants of 5$\,$m.
- ET is maximum if the groundwater table is at, or above, the topography.

It is common to make these types of assumptions, with the rooting depth being dependent on vegetation type and/or surficial geology.  In this case, it is assumed that the 5$\,$m cutoff is independent of time and does not vary spatially.  In the above input-file block, more sophisticated PorousFlow groundwater models would make use of the `flux_function` and `PT_shift` and make `cutoff` be a more complicated function to incorporate spatio-temporal variation.  The cubic in the [PorousFlowHalfCubicSink](PorousFlowHalfCubicSink.md) interpolates smoothly between the maximum ET and zero, but users may build their own ET functions of porepressure or saturation if data is available, employing a [PorousFlowPiecewiseLinearSink](PorousFlowPiecewiseLinearSink.md).

Furthermore, it is assumed that the pan evaporation is 4$\,$mm.day$^{-1}$, or, expressed in mass units:

\begin{equation}
4\,\mathrm{mm}.\mathrm{day}^{-1} = 4\times 10^{-3}\,\mathrm{m}^{3}.\mathrm{m}^{-2}.\mathrm{day}^{-1} \approx 4\,\mathrm{kg}.\mathrm{m}^{-2}.\mathrm{day}^{-1} \approx 4\times 10^{-5}\,\mathrm{kg}.\mathrm{m}^{-2}.\mathrm{s}^{-1}
\end{equation}

It is assumed that if the surficial geology has permeability $10^{-10}\,$m$^{2}$ and the water table is at the topography, then ET removes groundwater at this rate.  Because of the flag `use_mobility = true`, the `max` parameter obeys:

\begin{equation}
4\times 10^{-5}\,\mathrm{kg}.\mathrm{m}^{-2}.\mathrm{s}^{-1} = \mathrm{max} \times 10^{-10} \times \rho/\mu \ ,
\end{equation}
where $\rho \approx 1000\,$kg.m$^{-3}$ is the density of water, and $\mu \approx 10^{-3}\,$Pa.s is its viscosity.  Hence, $\mathrm{max} \approx 40$.

#### The river

The river is modelled using a [PorousFlowPolyLineSink](PorousFlowPolyLineSink.md):

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i block=DiracKernels

This object has already been encountered above, but readers are encouraged to read the detailed documentation on the [PorousFlowPolyLineSink](PorousFlowPolyLineSink.md) and [line sources in general](sinks.md).

In this case, it is assumed that the river is perennial and has:

- an incision depth of 1$\,$m, and
- a stage height of 1.5$\,$m, and

Therefore, its surface sits 0.5$\,$m above the topography.  It is assumed that this holds for all simulated times and over the whole river.  More sophisticated PorousFlow models can modify the inputs of the PorousFlowPolyLineSink in order to achieve spatio-temporal variation in these parameters.  In the case at hand, if the porepressure at the topography is 5$\,$kPa (corresponding to 0.5$\,$m head) there is zero baseflow or leakage.  Assume that the flux satisfies

\begin{equation}
\mathrm{flux} = \alpha (P - P_{\mathrm{c}}) \ ,
\end{equation}
where $P_{\mathrm{c}} = 5\,$kPa.  This is implemented by setting

```
    p_or_t_vals = '-Pbig+5000 5000 Pbig+5000'
    fluxes = '-M 0 M'
```

where `Pbig` is chosen large enough so that it exceeds any conceivable porepressure in the simulation, and $M = \alpha / P_{\mathrm{big}}$.  In the case at hand, `Pbig = 1E9`.

As explained in the [sinks page](sinks.md), riverbed conductance, C (measured in kg.Pa$^{-1}$.s$^{-1}$),
\begin{equation}
C = \frac{k_{zz}\rho}{T\mu}L_{\mathrm{seg}}W_{\mathrm{seg}} \ ,
\end{equation}
is often specified for rivers in groundwater models, and $\alpha = C/L_{\mathrm{seg}}$.  In the case at hand, using a river width of $W_{\mathrm{seg}} = 10\,$m and a riverbed thickness of $T = 1\,$m and a vertical permeability of $10^{-13}\,$m$^{2}$, this means $\alpha = 10^{-6}$, and $M = 10^{3}$.

### The entire steady-state input file

This reads:

!listing modules/porous_flow/examples/groundwater/ex02_steady_state.i

### Steady-state results

At steady-state, the baseflow to the river is $11\,$litres(water)/m(river-length)/day, and the saturation is shown in [groundwater_ex02_mesh_steady_state].

!media groundwater_ex02_steady_state.png caption=The saturation at steady-state.  id=groundwater_ex02_mesh_steady_state

### Abstracting groundwater

The only modifications to the steady-state input file needed when modelling the abstraction are the reading of the steady-state solution from the steady-state output result, and the inclusion of the abstraction borehole.

#### Reading the steady-state solution

The steady-state solution is used to initialise the transient simulation.  The mesh is read:

!listing modules/porous_flow/examples/groundwater/ex02_abstraction.i block=Mesh

A SolutionUserObject is employed to read the steady-state solution:

!listing modules/porous_flow/examples/groundwater/ex02_abstraction.i start=[steady_state_solution] end=[baseflow]

and the `pp` Variable is initialized:

!listing modules/porous_flow/examples/groundwater/ex02_abstraction.i block=ICs

#### The abstraction borehole

A [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) is used to model the borehole:

!listing modules/porous_flow/examples/groundwater/ex02_abstraction.i start=[horizontal_borehole] end=[polyline_sink_borehole]

with point file representing a long horizontal borehole with radius 0.2$\,$m at an elevation of -90$\,$m that sits almost directly underneath the river (the river sits at $x=250$, while the borehole is at $x=240$):

!listing modules/porous_flow/examples/groundwater/ex02.bh

The value `bottom_p_or_t = -1E5` means that the pump at the borehole's bottom point is able to suck at -1$\,$atmosphere of pressure.  (In this case, the borehole is horizontal, so its "bottom point" is meaningless, but in general it is the first point in the `point_file`, as explained in the [documentation](PorousFlowPeacemanBorehole.md).)

It is vital that readers understand that a [PorousFlowPeacemanBorehole](PorousFlowPeacemanBorehole.md) uses an approximation of the groundwater solution around a small borehole in order to *accurately represent the small borehole in a coarse mesh*.  The small borehole need not coincide with the finite-element nodes or elements, since the Peaceman formulation automatically takes the mesh into consideration.  In this case, it means that the porepressure at any node around the borehole will not be $-10^{5}\,$Pa, even though `bottom_p_or_t = -1E5`.  This is in stark contrast to a [PorousFlowPolyLineSink](PorousFlowPolyLineSink.md), which can easily force the porepressure to be a fixed value at the borehole if the magnitude of `fluxes` is set high enough.  The difference is quantified in a section below.  Almost always:

- Peaceman boreholes should be used to represent boreholes
- PolyLine sinks should be used to represent surface features such as streams and the interaction with the atmosphere, where the stream or atmosphere is sufficiently "huge" that it does actually fix porepressure at the point of interest.

### Results

The borehole extracts very little groundwater --- only a paltry 7.6$\,$litre/day --- which is not surprising given its small diameter and the "tightness" of the aquifer.  The porepressure in the proximity of the river is only reduced by around 0.2$\,$Pa, as shown in [groundwater_ex02_abstraction_fig].  This reduces the baseflow from 11.09$\,$litre/m/day to 11.07$\,$litre/m/day.

!media groundwater_ex02_abstraction_peaceman.png caption=The porepressure change due to groundwater abstraction.  id=groundwater_ex02_abstraction_fig

Groundwater modelling therefore suggests that the borehole will have negligible impact on the environment, but will also produce virtually no water.

### Using a PolyLineSink

It is a *mistake* to use a [PorousFlowPolyLineSink](PorousFlowPolyLineSink.md) to represent the borehole in this case, such as

!listing modules/porous_flow/examples/groundwater/ex02_abstraction.i start=[polyline_sink_borehole] end=[]

If the `fluxes` parameters were chosen sufficiently large, this would fix the porepressure at finite-element nodes surrounding the borehole to the value of 0$\,$Pa.  Since the finite-element mesh has a resolution of around $20\mathrm{m}\times 20\mathrm{m} \times 10\mathrm{m}$, this could correspond to a colossal borehole with cross-sectional size $20\mathrm{m} \times 10\mathrm{m}$ (and length of 400$\,$m from the width of the model)!

In this case, the water extracted by the borehole is around 640$\,$litre/day, and the river becomes leaky with 4.5$\,$litre/m/day of river water entering the groundwater system (and flowing to the borehole).  The porepressure change is much larger, as shown in [groundwater_ex02_abstraction_error_fig].

!media groundwater_ex02_abstraction.png caption=The porepressure change due to groundwater abstraction when erroneously using a PorousFlowPolyLineSink to represent the borehole.  id=groundwater_ex02_abstraction_error_fig

## Final words

This page has presented some very simple groundwater models that you can use as a framework to build your realistic models.  [PorousFlow](porous_flow/index.md) is highly functional and has been used to solve very complicated groundwater models, but if you require extra functionality not already included, please ask on the MOOSE discussion list.
