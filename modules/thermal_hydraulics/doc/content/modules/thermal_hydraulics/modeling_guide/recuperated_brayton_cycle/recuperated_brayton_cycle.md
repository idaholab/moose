# Recuperated Brayton Cycle

## Introduction

As shown in the
[Brayton Cycle modelingguide](modules/thermal_hydraulics/modeling_guide/brayton_cycle/brayton_cycle.md),
and the given input files
[closed_brayton_cycle.i](thermal_hydraulics/test/tests/problems/brayton_cycle/closed_brayton_cycle.i)
and
[open_brayton_cycle.i](thermal_hydraulics/test/tests/problems/brayton_cycle/open_brayton_cycle.i),
a Brayton Cycle Power Conversion Unit (PCU) consists of a motor, compressor,
turbine, and generator all coupled by a single shaft.

Detailed descriptions of the compressor and turbine components used in this
example can be found in the
[Brayton Cycle modeling guide](modules/thermal_hydraulics/modeling_guide/brayton_cycle/brayton_cycle.md).
In the aforementioned example, a simplified startup transient with a simplified
heat source was conducted which demonstrated the Thermal Hydraulics module’s
capability to produce torque, power, mass flow rate, and pressure ratios for all
PCU components based on the shaft speed.

In this example a different heat source and recuperator were added to the same
open Brayton Cycle PCU to demonstrate a more nuanced model along with a more
realistic piping structure. Also, a Proportional-Integral-Derivative (PID)
controller was added to the motor to conduct a more realistic startup transient
of the system.

A heat rate of 105.75 kW is applied as a uniform volumetric source to a
10-m-long steel tube with an outer radius of 15 cm. Gas which has been
compressed in the compressor, and preheated in the recuperator, passes over the
exterior of the heat structure and removes heat via forced convection.

The recuperator is represented by an annular cylindrical heat structure of the
same material and width as the main heat source but only 5 m in length and
without internal heat generation. Instead, hot exhaust gas leaving the turbine
transfers heat to the inside of the recuperator, and cooler gas from the
compressor outlet removes heat from the outer surface. This preheats the gas
entering the main heat source and improves thermal efficiency of the cycle.  

## Transient Description

Initially the shaft system is at rest, and the working fluid and heat structures
are the ambient temperature and pressure. At t = 0 s the motor PID activates and
ramps the PCU shaft to approximately 87,000 RPM over the course of 1600 s. When
t = 1000 s, heat generation in the main heat structure is activated and linearly
increases from 0 kW to a maximum power of 105.75 kW by t = 8600 s.

As the working fluid begins to heat, torque supplied to the shaft from the
turbine increases. The motor PID control logic slowly decreases the torque
supplied by the motor to 0 N·m once the turbine torque is greater than the
torque supplied by the motor. This allows for a graceful transition of PCU
control from the motor PID to the working fluid and heat source, and prevents
drastically over-speeding the system above the rated speed of 96,000 RPM.

## Input File description

The recuperated Brayton Cycle example is executed with the input file
[recuperated_brayton_cycle.i](thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i).
Shown below in [brayton_cycle_diagram] is a diagram of the modeled system.

!media thermal_hydraulics/modeling_guide/recuperated_brayton_cycle/brayton_cycle_diagram.png
       id=brayton_cycle_diagram
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Diagram of the recuperated Brayton Cycle.

All 90° pipe bends are modeled using [VolumeJunction1Phase.md] as shown in the example below:

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Components/junction2_cold_leg
       link=False

The main heat source is modeled as a heat structure with internal heat generation and forced convection heat transfer to Pipe 4.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Components/reactor
       link=False

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
        block=Components/total_power
        link=False

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
        block=Components/heat_generation
        link=False

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
        block=Components/heat_transfer
        link=False

The Recuperator is modeled as a heat structure without internal heat generation and heat transfer to both cold_leg and hot_leg.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
        block=Components/recuperator
        link=False

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
        block=Components/heat_transfer_cold_leg
        link=False

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
        block=Components/heat_transfer_hot_leg
        link=False

## Control Logic

One of the main changes from the
[Brayton Cycle modeling guide](modules/thermal_hydraulics/modeling_guide/brayton_cycle/brayton_cycle.md)
is the addition of a PID controller to the motor for the start up transient.
This PID control operates a 3-Phase electric motor which applies torque to the
shaft to reach a desired speed set by the user, in this case 87,000 RPM. A speed
lower than the rated turbine speed of 96,000 RPM was chosen to prevent
significant over-speeding of the turbine during the start up transient.

As the shaft increases in speed, the compressor begins to compress the inlet
atmospheric air and produces a coolant flow through the system. Once the heat
source begins heating the coolant, the gas begins conducting work on the system
via the turbine. However, until a fully developed and fully heated flow is
established, the working fluid still requires an assist from the motor to
maintain shaft speed and adequate coolant mass flow rate. At this point, a
direct shutoff of the motor would cause the system to stall. Conversely, if the
motor were to supply a constant torque the shaft would over speed due to the
increasing torque supplied by the turbine from the heating coolant.

To prevent stalling or significant over-speeding of the shaft while the coolant
is reaching a fully developed and fully heated flow, a shutdown function is
applied to the motor. A linearly decreasing torque is applied to the motor
starting from the time the turbine first provides more torque and reaches 0 N·m
after 35,000 s. This function uses `ControlLogic` to determine when the torque
supplied by the turbine is greater than the motor, and initiates a graceful ramp
down of the motor. The proceeding sections discuss in detail how this control
logic is implemented in the input file.



### PID Control

Shown below is the `ControlLogic` block containing the PID block utilizing
[PIDControl.md]. Here, the $K_p$, $K_i$, and $K_d$ variables are the
proportional, integral, and derivative gains respectively. As with any PID
control, gains must be determined for a specific system and are not universal.
In this specific PID, the output signal is used to govern the torque supplied by
the motor to the system shaft based upon the current shaft speed.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=ControlLogic/initial_motor_PID
       link=False

The PID attempts to reach a desired shaft speed, in RPM, which is determined by
the `set_point` block using [GetFunctionValueControl.md]. The rated speed of the
turbine is set at the beginning of the input file as a global parameter
`speed_rated_rpm`. To prevent over-speeding of the turbine and compressor, the
set point of the PID is adjusted to be lower than the rated turbine speed.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=ControlLogic/set_point
       link=False

A [ParsedFunctionControl.md] is applied to determine whether to send the PID
output to the motor or shutdown the motor because the turbine is supplying
enough torque.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=ControlLogic/logic
       link=False

If `logic` determines to send the PID signal to the motor, then [SetComponentRealValueControl.md] `motor_PID` is used to apply the signal directly to the `motor` component torque.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=ControlLogic/motor_PID
       link=False

## Motor Shutdown

Should the turbine provide more torque than the motor, the logic activates a
[MooseParsedFunction.md], `motor_torque_fn_shutdown`, to ramp down the motor
allowing for a graceful transition of the system from the motor to the turbine.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Functions/motor_torque_fn_shutdown
       link=False

The shutdown function utilizes `AuxVariables` values `PID_trip_status` and
`time_trip`. These values are dependent upon a number of factors including the
motor PID, startup rate of the heat source, heat source total power, etc. The
user has no way of knowing these values while writing the input file, so they
must be determined during input file execution. The following steps outline one
method to determine, store, and apply initially unknown variables within an
input file.


### AuxVariables

First, `AuxVariables` are used to create two variables, `time_trip` and
`PID_trip_status`. `time_trip` is used to store the time at which turbine torque
is greater than the motor torque. `PID_trip_status` is used to change a trip
status from 0 to 1.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=AuxVariables/time_trip
       link=False

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=AuxVariables/PID_trip_status
       link=False

### Functions

Next, four `Functions` are created which are used to output the desired values
for the previously mentioned `AuxVariables`. These functions are
`PID_tripped_status_fn`, `PID_tripped_constant_value`, `time_fn`, and
`is_tripped_fn`.

`time_fn` records the current simulation time.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Functions/time_fn
       link=False

`is_tripped_fn` determines if the turbine torque is greater than the motor torque using the `Postprocessors` values `turbine_torque` and `motor_torque`.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Functions/is_tripped_fn
       link=False

`PID_tripped_constant_value` creates a constant function with a value of 1. This
is later used to change the PID trip status from 0 to 1. 0 indicates the motor
is supplying more torque than the turbine, 1 indicates the turbine is supplying
more torque than the motor.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Functions/PID_tripped_constant_value
       link=False

`PID_tripped_status_fn` stores the value from the `Control` `PID_trip_status` (which will be discussed next) into a separate variable of the same name.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Functions/PID_tripped_status_fn
       link=False

### Controls

Multiple variables and functions have been discussed which are now used by
`Controls` and `AuxScalarKernels` to actually record the desired values. Two
`Controls` use the previously mentioned functions to activate two
`AuxScalarKernels` which then apply the desired values to the previously
mentioned `AuxVariables`.

`PID_trip_status` is a [ConditionalFunctionEnableControl.md] within the
`Controls` block. When the function `is_tripped_fn` becomes true,
`PID_trip_status` activates the `AuxScalarKernel` `set_PID_tripped`.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Controls/PID_trip_status
       link=False

`time_PID` is also a [ConditionalFunctionEnableControl.md] which activates when
`PID_trip_status` activates. This control then disables the `AuxScalarKernel`
`set_time_PID` which was recording the time up until the point of the PID trip.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=Controls/time_PID
       link=False

### AuxScalarKernels

Finally, the `AuxScalarKernels` apply all of the values and conditions discussed up to this point.

`time_trip_aux` takes the function `time_fn` and applies it to the `AuxVariable`
`time_trip`. When the `time_PID` `Control` activates, it disables this
`AuxScalarVariable` which results in `time_trip` remaining constant at the time
the PID trip occurred. This value can now be applied to
`motor_torque_fn_shutdown`.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=AuxScalarKernels/time_trip_aux
       link=False

`PID_trip_status_aux`, once activated, takes the function
`PID_tripped_constant_value` and applies it to the `AuxVariable`
`PID_trip_status`. This causes a simple switch from 0 to 1 meaning the turbine
went from supplying less torque than the motor to supplying more torque than the
motor.

!listing thermal_hydraulics/test/tests/problems/brayton_cycle/recuperated_brayton_cycle.i
       block=AuxScalarKernels/PID_trip_status_aux
       link=False

## Results

[PID_control] shows the first 2000 seconds of the transient where the PID
controlled motor increased shaft speed from 0 to approximately 85,000 RPM. This
is followed by [whole_transient_speed] which displays the shaft speed over the
course of the entire transient.

!media thermal_hydraulics/modeling_guide/recuperated_brayton_cycle/PID_startup_recuperated.png
       id=PID_control
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=PID start-up of shaft system.

!media thermal_hydraulics/modeling_guide/recuperated_brayton_cycle/shaft_speed_vs_time_recuperated.png
       id=whole_transient_speed
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Shaft speed during transient.

As shown in [whole_transient_speed], shaft speed is quickly ramped up by the PID
and then linearly increases as the working fluid is heated and begins working on
the turbine to produce shaft torque. At approximately 20,000 seconds the motor
and turbine provide the same amount of torque which initiates the motor shutdown
function. A comparison of the motor and turbine torques is shown below in
[torque_comparison].

!media thermal_hydraulics/modeling_guide/recuperated_brayton_cycle/torque_comparison_recuperated.png
       id=torque_comparison
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Comparison of the motor and turbine torques.

[pressure_ratios] visualizes the pressure ratios of both the compressor and
turbine during the transient. A slight difference is seen between the compressor
and turbine steady state pressure ratios due to frictional losses across the
piping system and 90° pipe bends, modeled by [VolumeJunction1Phase.md].

!media thermal_hydraulics/modeling_guide/recuperated_brayton_cycle/pressure_ratio_comparison_recuperated.png
       id=pressure_ratios
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Compressor and turbine pressure ratios.

Finally, coolant temperatures across key points of the system are displayed in
[transient_temperatures]. The reactor inlet and outlet temperatures were omitted
as there was no difference between the `cold_leg` outlet and turbine inlet
respectively.

Also, the compressor inlet was not plotted as it remained constant at ambient
temperature of 300 K. The compressor outlet temperature sees an immediate
increase corresponding to the startup of the PID system while the rest of the
temperature responses are dependent upon the heat source power output.

!media thermal_hydraulics/modeling_guide/recuperated_brayton_cycle/temperature_comparison_recuperated.png
       id=transient_temperatures
       style=width:90%;display:block;margin-left:auto;margin-right:auto;
       caption=Coolant temperatures across key components.
