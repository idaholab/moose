# Step 3: Upper Loop

+Complete input file for this step:+  [03_upper_loop.i](thermal_hydraulics/tutorials/single_phase_flow/03_upper_loop.i)

!media thermal_hydraulics/tutorials/single_phase_flow/step-03.png
       style=width:33%;float:right;margin-left:40px
       caption=Model diagram
       id=fig-model

In this step, we will add the flow channels corresponding to the upper part of the loop and
the primary side of the heat exchanger.
We will explain how to connect flow channels together using junction components and how to specify
a convective heat transfer using a specified wall temperature.

## Junctions

Junctions are 0-D components that can connect 2 or more flow channels.

In this tutorial we will use so-called volume junctions to connect flow channels to build up the
primary loop.

Let's look at an example of a volume junction:

!listing thermal_hydraulics/tutorials/single_phase_flow/03_upper_loop.i
         block=Components/jct1
         link=False

In a volume junction component, users have to specify its location (via the `position` parameter),
its volume via the `volume` parameter, and list the connected flow channels via the `connections` parameter.

If the channels are parallel and the cross-sectional area is changing, the `type` of the junction
should be [JunctionParallelChannels1Phase](JunctionParallelChannels1Phase.md).

If the cross-sectional area is the same or the channels are not parallel, the `type` should be
[VolumeJunction1Phase](VolumeJunction1Phase.md).

We also need to specify the initial conditions.
Besides the pressure `p` and temperature `T`, we need specify velocity initial condition, which has
3 components corresponding to x-, y- and z-dimension.

```
initial_vel_x = 0
initial_vel_y = 0
initial_vel_z = 0
```

Since all the junctions will start from the same initial conditions, we can specify those
in the `[GlobalParams]` block, like we did earlier for the flow channels.


## Top Part of the Loop

To build the upper part of the loop, we define a global parameter for the pipe diameter.

```
# pipe parameters
pipe_dia = ${units 10. cm -> m}
```

This dimension is shared by all the pipes.
If we needed to change it later, we can do so just in one place.

The following part of the input file defines all the flow channels and junctions that build up the
upper part of the loop

!listing thermal_hydraulics/tutorials/single_phase_flow/03_upper_loop.i
         start=jct1
         end=jct4
         link=False
         max-height=None

In the heat exchanger section, we only build the primary side and connect it to
[HeatTransferFromSpecifiedTemperature1Phase](HeatTransferFromSpecifiedTemperature1Phase.md).
This component requires the `T_wall` parameter -- wall temperature and `Hw` convective wall
heat transfer coefficient (this is a requirement of [Closures1PhaseSimple.md]).
Using a simplified secondary side is a good first step when building a heat exchanger model.


## Postprocessors

The [postprocessor system](syntax/Postprocessors/index.md) comes from the MOOSE framework.
Postprocessors are single `Real` values computed at different locations like blocks, sides, etc.,
or at different mesh entities like nodes or elements.
There can also be postprocessors that are not associated with any mesh entities (like a
postprocessor to output time step size, etc.).

In our model, we will add the two following postprocessors:

1. `core_T_out` for monitoring core outlet temperature

   !listing thermal_hydraulics/tutorials/single_phase_flow/03_upper_loop.i
            block=Postprocessors/core_T_out
            link=False

2. `hx_pri_T_out` for monitoring heat exchanger outlet temperature

   !listing thermal_hydraulics/tutorials/single_phase_flow/03_upper_loop.i
            block=Postprocessors/hx_pri_T_out
            link=False

Both postprocessors are of `SideAverageValue` type which means they are computed on a side.
The side is specified via the `boundary` parameter and both postprocessors operate on the
temperature variable `T`.


## Notes

THM also provides components like pumps and valves, which behave like junctions.
However, they may have some limitations on the number of connected channels.
For example, a pump component might have only a single inlet and a single outlet.

The single-phase flow model does not support mixing of fluids.
This means you cannot bring 2 different fluids into a junction and have their mixture produced at the junction outlet.
The code will detect this problem and report an error, if you do so.

!content pagination previous=tutorials/single_phase_flow/step02.md
                    next=tutorials/single_phase_flow/step04.md
