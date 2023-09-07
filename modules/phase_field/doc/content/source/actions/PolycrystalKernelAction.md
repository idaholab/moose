# PolycrystalKernelAction

!syntax description /Kernels/PolycrystalKernel/PolycrystalKernelAction

## Overview

This action creates the kernels needed to run a polycrystal simulation. All of the kernels could be added manually in an input file, but the input file would be very long and would require many changes in order to change the number of order parameters being used to represent the grains. This action vastly simplifies the input file syntax.

The required input parameter `op_num` defines how many order parameters will be generated. Each order parameter is named based on the `var_name_base` parameter; e.g. for `op_num` = 3, `var_name_base` = `gr`, three nonlinear variables are created (`gr0`, `gr1`, and `gr2`).

The following objects are created:

+Kernels+

- One per variable, with a total of `op_num`

  - [TimeDerivative](/TimeDerivative.md)
  - [ACGrGrPoly](/ACGrGrPoly.md)
  - [ACInterface](/ACInterface.md)

## Example Input File Syntax

The `PolycrystalKernelAction` is accessed through the `Kernels` block, as shown below.

!listing modules/phase_field/test/tests/grain_growth/test.i block=GlobalParams

!listing modules/phase_field/test/tests/grain_growth/test.i block=Kernels

!syntax parameters /Kernels/PolycrystalKernel/PolycrystalKernelAction

!syntax children /Kernels/PolycrystalKernel/PolycrystalKernelAction
