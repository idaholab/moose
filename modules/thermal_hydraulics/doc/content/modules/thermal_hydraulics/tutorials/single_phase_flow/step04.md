# Step 4: Primary Loop

+Complete input file for this step:+ [!file text=04_loop.i](thermal_hydraulics/tutorials/single_phase_flow/04_loop.i)

!media thermal_hydraulics/tutorials/single_phase_flow/step-04.png
       style=width:33%;float:right;margin-left:40px
       caption=Model diagram
       id=fig-model

In this step, we will complete the primary loop and set up a simple PID controller for the pump so that
it maintains the prescribed mass flow rate.

## Close the Loop

We add two pipes for the bottom section of the primary loop with a pump in the middle.
A pump is a junction-like component that connects to two flow channels corresponding to its inlet and outlet.

!listing thermal_hydraulics/tutorials/single_phase_flow/04_loop.i
         start=jct7
         end=jct8
         link=False
         max-height=None

The pump component needs 2 more parameters to be specified: reference area `A_ref`, and `head`,
which is the pump head.


Because the system is now closed, we add a simple pressurizer to maintain the system pressure as the fluid heats up. We split the pipe at the top to create a T-junction with a pipe connected to the pressurizer using a [VolumeJunction1Phase.md]. The pressurizer is modeled by prescribing the stagnation pressure with an [InletStagnationPressureTemperature1Phase.md] component.

!listing thermal_hydraulics/tutorials/single_phase_flow/04_loop.i
         start=jct4
         end=jct5
         link=False
         max-height=None

## Control Logic

The [ChainControls](syntax/ChainControls/index.md) system allows users to monitor the simulation
and change its parameters while it is running.

The system consists of 3 layers:

1. +input layer:+ which brings values from the simulation inside the control data system
2. +execution layer:+ which performs the prescribed operations
3. +output layer:+ that feeds the values back into simulation

All control blocks should be included in the top-level `[ChainControls]` block.
Here, each control sets `execute_on = 'INITIAL TIMESTEP_BEGIN'` so that the controlled
parameters are updated before each time step is solved; by default, controls execute at the
end of the time step.

## Setup PID

A PID control requires several values as an input: set point `set_point`, input value `input`,
initial integral `initial_integral`, and three constants `K_p`, `K_i`, and `K_d`, which are the
coefficients for the proportional, integral, and derivative terms, respectively.

For the input value, we set up a postprocessor `m_dot_pump` with type `ADFlowJunctionFlux1Phase`
which will be measuring the outlet mass flow rate from the pump.

!listing thermal_hydraulics/tutorials/single_phase_flow/04_loop.i
         block=Postprocessors/m_dot_pump
         link=False

To bring this value into the control data system, we use a [GetPostprocessorChainControl.md]
block. Setting `name_data_same_as_postprocessor = true` makes the control data available under
the name of the postprocessor, `m_dot_pump`:

!listing thermal_hydraulics/tutorials/single_phase_flow/04_loop.i
         block=ChainControls/get_m_dot_pump
         link=False

A set point will be our desired mass flow rate specified by the global parameter `m_dot_in`.
To bring this value into the control data system, we need to use a
[GetFunctionValueChainControl.md] block like so:

!listing thermal_hydraulics/tutorials/single_phase_flow/04_loop.i
         block=ChainControls/set_point
         link=False

This value will be available in the control data system as `set_point:value` (in general
`control_block_name:value`).

Then, we add the PID control block, [PIDChainControl.md], as follows:

!listing thermal_hydraulics/tutorials/single_phase_flow/04_loop.i
         block=ChainControls/pid
         link=False

The value computed by the PID control is available in the control data system under the name
`pid:value`, where `pid` is the name of the block.

As a last step, we need to feed this value back into the system.
That can be done via a [SetValueChainControl.md] block.

!listing thermal_hydraulics/tutorials/single_phase_flow/04_loop.i
         block=ChainControls/set_pump_head
         link=False

The parameter to control is specified via the `parameter` parameter. For a component parameter,
it has the form `component::<component_name>/<parameter_name>`, where `<component_name>` and
`<parameter_name>` are the component name and the parameter name of that component we want to
modify.


!content pagination previous=tutorials/single_phase_flow/step03.md
                    next=tutorials/single_phase_flow/step05.md
