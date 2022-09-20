# Step 1: Flow Through a Channel

+Complete input file for this step:+ [01_flow_channel.i](thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i)

!media thermal_hydraulics/tutorials/single_phase_flow/step-01.png
       style=width:15%;float:right;margin-left:40px
       caption=Model diagram
       id=fig-model

In the first step of this tutorial, we will setup a flow channel with mass flow rate inlet boundary
condition and pressure outlet boundary condition.

We start by specifying several model parameters at the beginning of the input file

## Simulation Parameters

We define inlet temperature `T_in`, inlet mass flow rate `m_dot_in` and pressure `press` parameters.
These values will be used in the input file.
It is a good practice to setup input files this way, because if a change is needed one can do it in
a single location rather then hunting for all the right occurrences.
This will become very important when input files get large.

```
T_in = 300.         # K
m_dot_in = 1e-4     # kg/s
press = 1e5         # Pa
```

It is a good habit to include the units we are assuming in case our input file will be used by
other people.

!alert note
+Note:+ THM is using SI units: `kg`, `m`, `s`, `K`.


## Fluid Properties

In our model, we will be circulating helium in tubes, so we need to add a top-level `[FluidProperties]` block
into the input file.
In this block, we define all fluids that will be used by the flow components.

To model the helium gas, we will use the ideal gas equation of state.
To do so, we will put the following block inside the `[FluidProperties]` block.

!listing thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i
         block=FluidProperties/he
         link=False

## Closures

Closure relations are provided through objects created in a `[Closures]` block.
Here we create a simple set of closures, which will be used by our flow channel component:

!listing thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i
         block=Closures
         link=False

## Flow Channel

The heart of all input files is the top-level `[Components]` block.
Components represent pieces of a model, including physical pieces such as pipes, turbo-machinery,
etc., and intangible pieces that add physics, like boundary conditions or source terms.
They are like LEGO bricks which when put together they build up the overall simulation.


The first component we introduce is the `FlowChannel1Phase` component which represents a channel
with single-phase flow.

!listing thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i
         block=Components/core_chan
         link=False

The `position` parameter is a location in 3D space where the flow channel starts.
The `orientation` parameter is the directional vector (in 3 dimensions) of the channel relative to
the `position` parameter.
The `length` parameter is the length of the channel.
The `n_elems` parameter describes the number of elements that will be created for the mesh that supports
the channel.
The `A`, `D_h`, and `f` are cross-sectional area, hydraulic diameter, and a wall friction factor, respectively.

Every flow channel defines a subdomain block with its name and two boundaries.
They are named `flow_channel_name:in` and `flow_channel_name:out` and refer to the start and the end
of the flow channel, respectively. This will be useful later for connecting components together and
setting up other objects like postprocessors.

!alert note
+Note:+ The `flow_channel_name:in` and `flow_channel_name:out` are only refering to the geometry of the channel.
The actual flow direction will be determined by the physics and the boundary conditions connected to each end,
i.e. `flow_channel_name:in` can be a flow outlet.


## Boundary Conditions

Every flow channel has to be connected to either a boundary condition or a junction.
If it is not, the code will detect it and report an error.

In this part of the tutorial we will look at connecting boundary conditions -- junctions will be
covered later.
To connect a boundary condition to a flow channel we use the `input` parameter.
The value we assign into it will be the name of the side of the flow channel (see the above note).

### Inlet

For the channel inlet, we prescribe a mass flow rate and temperature boundary condition.
The component expects two required parameters: `m_dot` and `T`.
We use the simulation parameters we defined earlier at the beginning of this step, so the block will look like this:

!listing thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i
         block=Components/inlet
         link=False

### Outlet

For the channel outlet, we prescribe pressure boundary condition and we use our `press` parameter to
specify the pressure via a `p` parameter.

!listing thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i
         block=Components/outlet
         link=False

## Global Parameters

In `[GlobalParams]` block, we prescribe parameters that are global to the whole simulation.

The following parameter should be almost always included in your input file.
It enables the full slope reconstruction in the underlying rDG scheme.

```
rdg_slope_reconstruction = full
```

If initial conditions are the same for the whole simulation, it is convenient to place them in this
block.
This is the case in this tutorial, so we will do so by including the following parameters in the
`[GlobalParams]` block:

```
initial_p = ${press}
initial_vel = 0
initial_T = ${T_in}
```

The `closures` parameter is used to specify the name of a closures object, so
we pass the name we gave to the closures object we created in the `Closures` block:

```
closures = simple_closures
```

Note that the corresponding closures class, [Closures1PhaseSimple.md],
requires users to specify wall friction factor `f` and wall heat transfer
coefficient `Hw` (if heating is present) manually as a constant value.

Since we will be using helium gas in almost all flow components, we can place the following in the
`[GlobalParams]` block as well:

```
fp = he
```

This will make sure that all flow components will pick the helium gas, unless they will be directed
otherwise.

!alert note
+Tip:+ Place the `[GlobalParams]` at the top of the input file for visibility.

## Executioner

This top-level block describes how the simulation will be executed.

!listing thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i
         block=Executioner
         link=False

The `Transient` type says that we will be solving multiple time steps starting from `start_time` and
running until `end_time` with time step size (`dt`) of `0.1`.

The `line_search = basic` and `solve_type = NEWTON` should be always present in this block when
running a THM-based code.
Their exact explanation is beyond the scope of this tutorial.

The `nl_rel_tol`, `nl_abs_tol`, and `nl_max_its` are parameters to decide if the non-linear solver
converged and their meaning is relative and absolute tolerance and maximum number of non-linear
iterations, respectively.


## Outputs

By default, MOOSE will only produce an output to terminal.

For visualizing the obtained solution, we need to enable an output to a file or files.
The most useful output is stored in an ExodusII file format and can be enabled by:

!listing thermal_hydraulics/tutorials/single_phase_flow/01_flow_channel.i
         block=Outputs
         link=False

## Notes

This is a typical input file structure of a THM-based code.
All input files will be more or less similar to this.


!content pagination previous=tutorials/single_phase_flow/problem_description.md
                    next=tutorials/single_phase_flow/step02.md
