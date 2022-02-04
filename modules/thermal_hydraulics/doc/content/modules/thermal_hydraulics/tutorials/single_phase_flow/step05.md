# Step 5: Secondary Side

+Complete input file for this step:+  [05_secondary_side.i](thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i)

!media thermal_hydraulics/tutorials/single_phase_flow/step-05.png
       style=width:35%;float:right;margin-left:40px
       caption=Model diagram
       id=fig-model


In this step, we will add the secondary side of the heat exchanger and set up the inlet mass flow
rate boundary condition as a function of time.

## Heat Exchanger

We will define the following heat exchanger parameters:

```
# heat exchanger parameters
hx_dia_inner = ${units 10. cm -> m}
hx_wall_thickness = ${units 5. mm -> m}
hx_dia_outer = ${units 50. cm -> m}
hx_radius_wall = ${fparse hx_dia_inner / 2. + hx_wall_thickness}
hx_length = 1       # m
hx_n_elems = 10

m_dot_sec_in = 1    # kg/s
```

We also define second fluid that we will be using on the secondary side:

!listing thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i
         block=FluidProperties/water
         link=False


## Components

To define the heat exchanger block, we will use the syntax for grouping components together.

The general syntax is:

```
[group]
  [component1]
  []
  [component2]
  []
[]
```

Then, individual components can be referred to as `group/component1` and `group/component2`.

!alert note
+Note:+ It is possible to define parameters within the group.
Good candidates would be `hx_length` and `hx_n_elems`.

We will take advantage of this feature and set our heat exchanger as follows:

!listing thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i
         block=Components/hx
         link=False
         max-height=None

Then, we connect the inlet boundary condition to the secondary side flow channel:

!listing thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i
         block=Components/inlet_sec
         link=False

And then, the outlet boundary condition for the same channel:

!listing thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i
         block=Components/outlet_sec
         link=False

This is the same as what we did in step 1 of this tutorial.


## Inlet Mass Flow Rate

To set up the inlet boundary condition as a function of time, we first need to define a time-dependent
function in the top-level `[Functions]` block:

!listing thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i
         block=Functions/m_dot_sec_fn
         link=False

In the `[ControlLogic]` block, we bring the function value in using the `GetFunctionValueControl`
block:

!listing thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i
         block=ControlLogic/m_dot_sec_inlet_ctrl
         link=False

And then we feed this value back into the system:

!listing thermal_hydraulics/tutorials/single_phase_flow/05_secondary_side.i
         block=ControlLogic/set_m_dot_sec_ctrl
         link=False


### Alternative Solution

An alternative solution to this is to use a convenience block called `TimeFunctionComponentControl`
which combines these two `ControlLogic` blocks into one.
It takes three parameters `component`, `parameter`, and `function`.

The equivalent syntax would look like this:

```
[set_m_dot_sec_ctrl]
  type = TimeFunctionComponentControl
  component = inlet_sec
  parameter = m_dot
  function = m_dot_sec_fn
[]
```

!alert note
+Note:+ This should be your preferred setup when you want to use time-dependent component parameters.


!content pagination previous=tutorials/single_phase_flow/step04.md
                    next=tutorials/single_phase_flow/step06.md
