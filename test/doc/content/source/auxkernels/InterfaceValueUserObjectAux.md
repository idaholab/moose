# InterfaceValueUserObjectAux

## Description

InterfaceValueUserObjectAux is an AuxKernel used to collect average value collected on the interface via [InterfaceValueUO_QP](/InterfaceValueUO_QP.md) and save them into an aux variable at each quadrature point.

where value_m is the value on the master side of the interface (e.g. where the boundary is defined) and value_s is the value on the slave side of the interface.

## Example Input File Syntax

listing test/tests/userobjects/interface_user_object/interface_user_object_QP.i block=AuxKernels/interface_avg_qp_aux
