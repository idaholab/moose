# Step 2: Conjugate Heat Transfer

+Complete input file for this step:+ [02_core.i](thermal_hydraulics/tutorials/single_phase_flow/02_core.i)

!media thermal_hydraulics/tutorials/single_phase_flow/step-02.png
       style=width:15%;float:right;margin-left:40px
       caption=Model diagram
       id=fig-model

In this step, we will add a heating block to our flow channel, set up heat source, and connect the
solid block to the flow channel via conjugate heat transfer.


## Parameters of the Heated Channel

We include the core parameters as named parameters to the top of the input file as follows:

```
core_length = 1.    # m
core_n_elems = 10
core_dia = ${units 2. cm -> m}
core_pitch = ${units 8.7 cm -> m}
```


For total power used for heating the block, we prescribe a parameter called `tot_power`.

```
tot_power = 100       # W
```

## Heat Structure Materials

To set up a heat conduction, we will need to define a solid material used in the block with
heat conduction.
To do that, we put the following block into a top-level `[HeatStructureMaterials]` block:

!listing thermal_hydraulics/tutorials/single_phase_flow/02_core.i
         block=HeatStructureMaterials/steel
         link=False

where `rho`, `k`, and `cp` are density, thermal conductivity, and specific heat, respectively.
The name `steel` is arbitrary and is used to refer to this material from other parts of the input file.

## Heat Structure

A heat structure is a 2D or 3D component that is used for modeling heat conduction.
In our setup, the heating block is a rod, so we use `HeatStructureCylindrical` for the model.
The component takes the `position` parameter, which is the location in 3D space.
The `orientation` parameter is the axial directional vector, `length` is the axial length, and
`n_elems` is the number of elements in the axial direction.

!alert note
The number of axial elements must match the number of elements in the flow channel.

In radial direction we define one block called `block` and assign our previously defined `steel`
material to it. The number of radial element in this block will be `3`.

!listing thermal_hydraulics/tutorials/single_phase_flow/02_core.i
         block=Components/core_hs
         link=False

## Heat Source

Our heating will be given by the specified total power parameter. For this, we need to include
`TotalPower` component and link it with another component -- `HeatSourceFromTotalPower`.

!listing thermal_hydraulics/tutorials/single_phase_flow/02_core.i
         block=Components/total_power
         link=False

!listing thermal_hydraulics/tutorials/single_phase_flow/02_core.i
         block=Components/core_heating
         link=False

`HeatSourceFromTotalPower` needs to know which heat structure and which part it acts on, which is
done via the `hs` and `regions` parameters.  The link to the `TotalPower` component is created via
`power` parameter which takes the name of the `TotalPower` component.

## Heat Transfer

To exchange heat between a flow channel and heat structure, we use the `HeatTransferFromHeatStructure1Phase`
component.  We need to specify the `flow_channel` parameter which takes the name of the connected
flow channel, `hs` parameter which is the name of the heat structure component, `hs_side` parameter
which is the side of the heat structure and can be either `inner` or `outer`.

Lastly, we need to specify `P_hf`, which is the heated perimeter, and because we are using `simple`
closure we need to supply `Hw`, which is a convective wall heat transfer coefficient.

!listing thermal_hydraulics/tutorials/single_phase_flow/02_core.i
         block=Components/core_ht
         link=False

This concludes the step of coupling a flow channel with a heat structure.

!content pagination previous=tutorials/single_phase_flow/step01.md
                    next=tutorials/single_phase_flow/step03.md
