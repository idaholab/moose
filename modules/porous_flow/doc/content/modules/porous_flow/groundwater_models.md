# Groundwater models

Authors: Andy Wilkins and Ma{\"{e}}lle Grass Ma&#228;lle TODO: get umlauts!

The PorousFlow module may be used to simulate groundwater systems.   A typical example can be found in [!citet](herron).  Large and complex models may be built that include the effect of rainfall patterns, spatially and temporally varying evaporation and transpiration, seasonal river flows, realistic topography and hydrogeology, as well as human factors such as mines, water bores, dams, etc.

It is also possible to include the vadose zone (unsaturated flow), to track the transport of tracers and pollutants through the system (multi-component flow), to utilize high-precision equations of state for the groundwater, to include heat flows, to include the impact of rock deformation (solid mechanics), to include gas flows (multi-phase flows) and geochemical reactions.  This page concentrates mostly on the construction of traditional groundwater models and only touches lightly on these more elaborate options, which are explained in the [other PorousFlow examples, tutorials and tests](porous_flow/index.md).

## Groundwater concepts

Because PorousFlow has been built as a general multi-component, multi-phase simulator, its models and physics are most conveniently and commonly expressed in terms of quantities that are alien to many experienced groundwater modellers, such as porepressure, permeability and porosity.  This section provides translations between more conventional concepts --- hydraulic head, hydraulic conductivity and storativity --- and the quantities usually used in PorousFlow simulations.  In the following sections, PorousFlow models are presented using the conventional groundwater quantities (hydraulic head, etc) as well as the "natural" quantities (porepressure, etc) but for the reasons mentioned below, users are encouraged to use the "natural" quantities in their models.

### Hydraulic head and porepressure

Many experienced groundwater modellers will be used to expressing their models in terms of [hydraulic head](https://en.wikipedia.org/wiki/Hydraulic_head).  For instance, if a borehole is drilled into an [aquifer](https://en.wikipedia.org/wiki/Aquifer) that has a hydraulic head of 5$\,$m, then the groundwater in the borehole will rise 5$\,$m above the water table.  (This is assuming the hydraulic head is measured relative to the local water table, which is the common convention.)  Or, if the hydraulic head of a sandstone aquifer is greater than a siltstone aquifer, groundwater will attempt to flow from the sandstone to the siltstone.  Or, if the hydraulic gradient increases from south to north --- that is, the hydraulic head is small in the southern regions and is large in the northern regions --- then groundwater will tend to flow from north to south.

PorousFlow models are more naturally expressed in terms of *porepressure*, which is the pressure of the groundwater.  This is related to hydraulic head through:
\begin{equation}
\label{eqn.p.h}
P = \rho g (h + d) \ .
\end{equation}
Here:

- $P$ is porepressure, measured in Pa
- $h$ is hydraulic head, with respect to the water table, measured in m
- $d$ is depth below the water table, measured in m
- $\rho$ is the water density (approximately 1000$\,$kg.m$^{-3}$) and $g$ is the acceleration due to gravity (approximately 10$\,$m.s$^{-2}$), so $\rho g \approx 10^{4}\,$Pa.m$^{-1}$.  (Recall 1$\,$Pa = 1$\,$N.m$^{-2}$ = 1$\,$kg.m$^{-1}$.s$^{-2}$.)

Consider a [confined aquifer](https://en.wikipedia.org/wiki/Aquifer) with constant hydraulic head, $h$.  This means that the groundwater in a borehole will rise $h$ metres above the local groundwater level, irrespective of where the borehole is drilled and how deep it is (assuming the well intersects this aquifer and not another one).  [eqn.p.h] shows that porepressure increases with depth!  This may be conceptually challenging for modellers used to hydraulic head.  Remember, porepressure is the actual pressure of the groundwater.  It increases with depth because of the weight of the groundwater sitting above it.

Porepressure is advantageous to use in PorousFlow because users will often like to enhance their models using the advanced inbuilt features of PorousFlow, such as

- realistic equations of state for the water (or brine), in which density and viscosity are functions of porepressure
- vadose-zone physics (unsaturated flow) which is is conventionally expressed in terms of porepressure
- coupling with solid mechanics through the effective stress, which is expressed in terms of porepressure

Below, models are expressed in terms of hydraulic head and porepressure, but in order to facilitate easy extension of their models, users are encouraged to utilize porepressure.

### Hydraulic conductivity and permeability



### Storativity and porosity




