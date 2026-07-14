# FVSideSetHeatTransferKernel

!syntax description /FVInterfaceKernels/FVSideSetHeatTransferKernel

## Description

`FVSideSetHeatTransferKernel` models heat transfer across an internal sideset for the finite
volume method using an interface conductance. The interface heat flux is

!equation
q'' = G \left(T_1 - T_2\right)

where \(G\) is the interface conductance, \(T_1\) is the temperature on `subdomain1`, and
\(T_2\) is the temperature on `subdomain2`.

The `conductance` functor is evaluated on the `variable1` / `subdomain1` side of the interface.

This object is useful for representing thermal contact, gap conductance, or an unresolved thin
layer between two finite volume thermal domains. For a thin layer of conductivity \(k\) and
thickness \(\delta\), the equivalent interface conductance is

!equation
G = \frac{k}{\delta}

Equivalently, if a thermal resistance per unit area \(R_\mathrm{th}\) is known,

!equation
G = \frac{1}{R_\mathrm{th}}

The kernel returns the interface flux density. The `FVInterfaceKernel` base class multiplies this
flux by the face area and applies equal-and-opposite residual contributions to the two sides of the
interface.

## Example input syntax

!listing modules/heat_transfer/test/tests/fviks/fv_sideset_heat_transfer/fv_sideset_heat_transfer.i
         block=FVInterfaceKernels/interface_heat_transfer

!syntax parameters /FVInterfaceKernels/FVSideSetHeatTransferKernel

!syntax inputs /FVInterfaceKernels/FVSideSetHeatTransferKernel

!syntax children /FVInterfaceKernels/FVSideSetHeatTransferKernel
