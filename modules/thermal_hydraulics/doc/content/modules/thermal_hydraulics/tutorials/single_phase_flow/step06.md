# Step 6: Custom Closures

+Complete input file for this step:+ [06_custom_closures.i](thermal_hydraulics/tutorials/single_phase_flow/06_custom_closures.i)

## Closure System

The +closure system+ is another powerful system provided by THM.
It allows users to either use pre-defined closure sets or entirely define their own ones.

Flow channels have the `closures` parameter, which corresponds to the name of
a [Closures](syntax/Closures/index.md) object.
This parameter can be specified globally in `[GlobalParams]` block or on per-component basis.
Note that not all components require the closure parameter (for example heat structures have no
concept of closure correlations).

+Custom closure correlations+ are provided via [MOOSE material system](/syntax/Materials/).
Users can use any pre-built material objects that come with MOOSE and provide the required value.
The most notable and useful material is [ADParsedMaterial](ParsedMaterial.md) which allows users to
provide the closure formula on the input file level.

!alert note
+Tip:+ THM provides convenient materials for computing Reynolds and Prandtl number, named
[ADReynoldsNumberMaterial](ADReynoldsNumberMaterial.md) and
[ADPrandtlNumberMaterial](ADPrandtlNumberMaterial.md), respectively.

The last bit to know is which material properties we need to supply.
In single-phase flow, they are Darcy wall friction factor `f_D` and convective wall heat transfer
coefficient `Hw`.


## Add Custom Closure Correlations

Before we add our custom materials it is advisable to define two named parameters.
One parameter will be for the wall friction and the other for the wall heat transfer correlation.
They will hold the names of components where we will be applying each closure.

!alert note
+Note:+ Heat transfer is optional, so defining a wall heat transfer coefficient on blocks that
do not have wall heat transfer linked to them makes no sense.

This will avoid problems of defining closure correlations on components that do not support it.
For example, we would not want to define flow closures on heat structure blocks, etc.

In our tutorial we will define the following two subsets of blocks:

```
flow_blocks = 'core_chan up_pipe top_pipe hx/pri hx/sec down_pipe bottom_b bottom_a'
ht_blocks = 'core_chan hx/pri hx/sec'
```

The `flow_blocks` parameter will hold the names of components where we prescribe the wall friction
closure.
The `ht_blocks` parameter will contain the names of components that have the wall heat transfer
associated with them.

To use the custom closure set, we create a closures object of the class [Closures1PhaseNone.md],
which does not create any of its own Materials:

!listing thermal_hydraulics/tutorials/single_phase_flow/06_custom_closures.i
         block=Closures
         link=False

Then the name we gave this closures object (`no_closures`) is passed to the `closures` parameter.
Again, this can be done on a global level or per-component basis.

## Materials

In our model, we will use the Churchill's formula for wall friction closure and Dittus-Boelter
formula for convective wall heat transfer closure

The Churchill's formula can be set up by using the [ADWallFrictionChurchillMaterial](ADWallFrictionChurchillMaterial.md)

!listing thermal_hydraulics/tutorials/single_phase_flow/06_custom_closures.i
         block=Materials/f_mat
         link=False

The Dittus-Boelter formula can be set up by using [ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial](ADWallHeatTransferCoefficient3EqnDittusBoelterMaterial.md).

!listing thermal_hydraulics/tutorials/single_phase_flow/06_custom_closures.i
         block=Materials/Hw_mat
         link=False

Since these are materials, we need to put these two blocks into the top-level block `[Materials]`.

!content pagination previous=tutorials/single_phase_flow/step05.md
                    next=tutorials/single_phase_flow/finish.md
