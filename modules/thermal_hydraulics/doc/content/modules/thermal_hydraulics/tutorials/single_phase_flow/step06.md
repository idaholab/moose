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


In single-phase flow, the Darcy wall friction factor `f_D` and the convective wall heat transfer
coefficient `Hw` need to be supplied.


## Add Custom Closure Correlations


!alert note
+Note:+ Heat transfer is optional, so defining a wall heat transfer coefficient on blocks that
do not have wall heat transfer linked to them makes no sense.

In our tutorial we will define the custom closures on the primary side of the heat exchanger.

To use the custom closure set, we set the `closures` parameter in the `hx/pri` component
to an empty list, which will overwrite the `closures` parameter set in the `GlobalParams` block:


!listing thermal_hydraulics/tutorials/single_phase_flow/06_custom_closures.i
         block=hx/pri
         link=False

Then we are responsible for creating any closure material properties using `Materials`
objects directly.

## Materials

For our custom closure set, we choose the following expression for the friction factor:

!equation
f_D = a + b Re^c

where $f_D$ is the Darcy friction factor, $Re$ is the Reynolds number, and $a$, $b$, and $c$ are constant coefficients with $a = 1$, $b = 0.1$, and $c = -0.5$.

The first step is to define the Reynolds number using an [ADReynoldsNumberMaterial.md], and then the expression for the friction factor is implemented using an [ADParsedMaterial](ParsedMaterial.md). Note that these materials are defined only on the block where the custom closures are used. They are defined by the other closure set on the other components.

!listing thermal_hydraulics/tutorials/single_phase_flow/06_custom_closures.i
         start=Re_mat
         end=Pr_mat
         link=False

For the heat transfer, the following expression for Nusselt number is used:

!equation
Nu = 0.03 Re^{0.9} Pr^{0.5}.

To accomplish this, we define the Prandtl number using an [ADPrandtlNumberMaterial.md] block. The Nusselt number is defined using an [ADParsedMaterial](ParsedMaterial.md), and the heat transfer coefficient is set using an [ADConvectiveHeatTransferCoefficientMaterial.md] block. Again, the block is restricted to the `hx/pri` block to avoid conflicting with the other closure set.

!listing thermal_hydraulics/tutorials/single_phase_flow/06_custom_closures.i
         start=Pr_mat
         end=SolidProperties
         link=False

!content pagination previous=tutorials/single_phase_flow/step05.md
                    next=tutorials/single_phase_flow/finish.md
