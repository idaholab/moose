# Introduction

!media thermal_hydraulics/tutorials/single_phase_flow/step-05.png
       style=width:33%;float:right;margin-left:40px
       caption=Diagram of the system
       id=fig-model

In this tutorial, we will build a simple model of system with a primary loop and secondary side.
In the primary loop, we will be circulating helium using a pump.
We will be adding heat in the core section and removing it in the heat exchanger section.
The secondary side will be running liquid water.

We will gradually build the model step by step introducing the following basic concepts:

- Fluid property system
- Components
- Postprocessors
- Control logic
- Functions
- Closure system

Each step will be a complete working model that will be simulating a subset of the final model.

## Problem Description

Figure 1 shows the diagram of the modeled system.
The tables below list its physical parameters.

!table id=primary-loop-parameters caption=Primary loop parameters
| Parameter | Value (and unit) |
| :- | :- |
| Pressure | 10 kPa |
| Temperature | 300 K |
| Mass flow rate | 0.1 g/s |
| Fluid | helium |


!table id=core-parameters caption=Core parameters
| Parameter | Value and unit |
| :- | :- |
| Shape | Heating rod inside a square flow channel |
| Core length | 1 m |
| Side width | 8.7 cm |
| Rod diameter | 2. cm |


!table id=hx-parameters caption=Heat exchanger parameters
| Parameter | Value and unit |
| :- | :- |
| Inner diameter | 10 cm |
| Wall thickness | 5 mm |
| Outer diameter | 50 cm |
| Length | 1 m |


!table id=secondary-loop-parameters caption=Secondary loop parameters
| Parameter | Value and unit |
| :- | :- |
| Mass flow rate | 1 kg/s |
| Fluid | Liquid water |



!content pagination previous=tutorials/single_phase_flow/index.md
                    next=tutorials/single_phase_flow/step01.md
