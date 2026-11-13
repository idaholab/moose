# DistributedLoadDSGShell

!syntax description /Kernels/DistributedLoadDSGShell

## Description

This kernel applies a load per unit area of the mid-surface of a DSG shell element, given by a function of time and position. The load acts along the axis of the displacement variable the kernel is applied to, for a self weight or a wind load, or along the element normal when `project_load_to_normal` is set, for a pressure. A positive value loads the shell in the positive direction of the chosen axis or of the normal, the convention of [BodyForce](/BodyForce.md). The element normal comes from the rotation matrix of [ComputeDSGShellStrain](/ComputeDSGShellStrain.md), so the kernel works on every member of the family.

### Choosing the parameters

- `function` is the load per unit area of the mid-surface, a constant or a function of time and position. A positive value acts against the axis of the variable, or against the normal: this is the sign convention of [ADDistributedLoadShell](/ADDistributedLoadShell.md) and of the [Pressure](/Pressure.md) boundary condition, so inputs move between the two shell families unchanged.
- With `project_load_to_normal = false`, the default, the load acts along the global axis of the variable of the kernel: one kernel on `disp_z` with a positive function is a self weight on a horizontal plate.
- With `project_load_to_normal = true` the load is a pressure against the element normal, and each kernel applies the component along its own axis; three kernels with the same function, one per displacement variable, are then needed. The normal is the one of the element frame, which follows the node ordering of the element (right-hand rule), so a positive pressure on a closed surface whose normals point inward is an internal pressure; the tank case of the [verification page](/dsg_shell_vandv.md) shows this.

## Example Input File syntax

!listing modules/solid_mechanics/test/tests/shell_dsg/static/scordelis_lo_roof.i block=Kernels/self_weight

!syntax parameters /Kernels/DistributedLoadDSGShell

!syntax inputs /Kernels/DistributedLoadDSGShell

!syntax children /Kernels/DistributedLoadDSGShell
