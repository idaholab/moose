# Aquifer thermal energy storage

The material in this page is based on [!citet](sheldon2021).

## Introduction

Aquifer Thermal Energy Storage (ATES) systems use resident groundwater in a subsurface aquifer to store heat energy [!citep](fleuchaus2018).  The basic premise of ATES is:

- Water is produced from an aquifer;
- The thermal energy from some external source (e.g. excess renewable energy or industrial waste heat) is transferred to the water;
- The hot water is re-injected into the aquifer, where it is stored until it is needed;
- When needed, the hot water is produced, and the energy extracted.

This process can be reversed to enable cooling. The duration of an ATES cycle can range from hours to months, depending on the intended use of the energy; for example, storing excess solar energy during the day and extracting it for use at night (daily cycle); or, the very common case of storing excess heat energy in the warmer months and extracting it for use in the colder months (annual cycle). As such, ATES systems can be used to address energy storage challenges that arise from the intermittent nature of renewable energy and other sources.

There are many ATES systems in operation currently.  Their viability depends crucially on the recovery efficiency, $R$, which is the ratio of heat energy extracted to heat energy injected, because it is impossible to extract all of the injected heat from an ATES system due to heat losses caused by conduction and convection.  $R$ is

\begin{equation}
R = \frac{\overline{h}_{\mathrm{p}} - h_{\mathrm{amb}}}{h_{\mathrm{i}} - h_{\mathrm{amb}}} 
      \approx \frac{\overline{T}_{\mathrm{p}} - T_{\mathrm{amb}}}{T_{\mathrm{i}} - T_{\mathrm{amb}}} \ ,
\end{equation}

where $h$ denotes enthalpy and $T$ temperature.  The subscript "p" indicates "produced" (retrieved from the aquifer), "amb" indicates "ambient", and "i" indicates "injected".  For instance, if the ambient aquifer temperature is $T_{\mathrm{amb}} = 20^{\circ}$C, the injection temperature is $T_{i} = 150^{\circ}$C and $R=0.9$, then the average produced temperature is $\overline{T}_{\mathrm{p}} \approx 137^{\circ}$C.

$R$ has been measured at ATES sites, and estimated using numerical modelling.  Numerical models can provide accurate estimates of $R$ as a function of operational parameters, such as the injection temperature and rate, and aquifer parameters such as the permeability, thickness and depth.  Such estimates are useful for rapid screening of potential ATES sites, indicating which sites might or might not be viable.

The purpose of this page is to describe a MOOSE model of an ATES system, with the goal of predicting $R$, and present a few results.  For significantly more background discussion, discussion of the modelling approach and results, the reader is referred to [!citet](sheldon2021).

## Model setup

The model simulates an ATES system comprising a single injection-production well penetrating a horizontal aquifer 20 m thick. Five injection-production cycles are simulated, with each cycle comprising 91 days each of injection, storage, production and rest. The injection temperature is 90$^{\circ}\mathrm{C}$, and the injected fluid mass is 10$^8\,$kg. 

The single-well system has radial symmetry, hence it may be simulated using "RZ" coordinates:

!listing modules/porous_flow/examples/ates/ates.i block=Problem

which means gravity acts along what is usually thought of as the "y" direction:

!listing modules/porous_flow/examples/ates/ates.i block=GlobalParams

The geometry is shown in [ates_geometry_fig].

!media porous_flow/ATES_geometry.png caption=Geometry on a radial slice of the ATES model.  id=ates_geometry_fig

In this example, the mesh is created by using a combination of MOOSE MeshGenerators, however, for accurate prediction of $R$, readers are encouraged to use a more sophisticated approach involving greater refinement of the mesh close to the well screen, such as the one outlined in [!citet](sheldon2021).

!listing modules/porous_flow/examples/ates/ates.i block=Mesh

The core physics is handled by the [PorousFlowFullySaturated](PorousFlowFullySaturated.md) Action, and [Kuzmin-Turek stabilization](kt.md) is used to minimise numerical diffusion of the temperature front which impacts computation of $R$:

!listing modules/porous_flow/examples/ates/ates.i block=PorousFlowFullySaturated

The porepressure and temperature are fixed at the extremities of the model:

!listing modules/porous_flow/examples/ates/ates.i start=[outer_boundary_porepressure] end=[inject_heat]

The injection and production of heat from the borehole are the most complicated aspect of this input file.  The boundary conditions corresponding to injection are fixed temperature and constant rate of fluid flux:

!listing modules/porous_flow/examples/ates/ates.i start=[inject_heat] end=[produce_heat]

These boundary conditions are not always active.  They are controlled using

!listing modules/porous_flow/examples/ates/ates.i start=[inject_on] end=[produce_on]

with the `conditional_function` being:

!listing modules/porous_flow/examples/ates/ates.i start=[inject] end=[produce]

The boundary conditions corresponding to production are a withdrawal of fluid mass using a [PorousFlowSink](sinks.md) along with its corresponding withdrawal of heat energy (with the `use_enthalpy` flag set to true):

!listing modules/porous_flow/examples/ates/ates.i start=[produce_heat] end=[Controls]

These are controlled using similar `Controls` as the injection phase.  Notice the `save_in` for the heat withdrawal.  This records the rate of heat leaving each node.  To find the total rate of heat loss (with units J.day$^{-1}$), a [NodalSum](NodalSum.md) is used:

!listing modules/porous_flow/examples/ates/ates.i start=[heat_out_fromBC] end=[heat_out_per_timestep]

This may be multiplied by the time-step size to find the total heat withdrawn (with units J) in each time-step

!listing modules/porous_flow/examples/ates/ates.i start=[heat_out_in_timestep] end=[produced_T_time_integrated]

which in turn can be used to calculate $R$ for each cycle.

## Results

[ates_evolution] shows the evolution of temperature for the first cycle in this example.  Notice the buoyancy-driven convection, which causes hot water to rise to the top of the aquifer: this is the prime reason why not all the heat energy can be extracted.  In subsequent cycles, the aquifer temperature starts slightly hotter than ambient, so recovery efficiency gradually increases with cycle, as shown in [tab:re].

!media porous_flow/ates_evolution.mp4 caption=Evolution of the temperature during injection of 90$^{\circ}$C water into an aquifer of thickness 20m (indicated by the white horizontal lines) for a annual cycle length id=ates_evolution

!table id=tab:re caption=Recovery efficiency
| Cycle 1 | Cycle 2 | Cycle 3 | Cycle 4 | Cycle 5 |
| --- | --- | --- | --- | --- |
| 0.65 | 0.70 | 0.72 | 0.76 | 0.78 |
