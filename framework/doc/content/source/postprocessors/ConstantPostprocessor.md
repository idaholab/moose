# ConstantPostprocessor

!syntax description /Postprocessors/ConstantPostprocessor

## Overview

The ConstantPostprocessor is used to hold a constant value. This postprocessor
may be useful as a general controllable postprocessor for the [Controls](syntax/Controls/index.md) system.

## Example Input File Syntax

In this example, the value of the ConstantPostprocessor `recv` is controlled by
a [RealFunctionControl](source/controls/RealFunctionControl.md).

!listing test/tests/postprocessors/constant/receiver.i
  start=Controls
  end=Outputs

!syntax parameters /Postprocessors/ConstantPostprocessor

!syntax inputs /Postprocessors/ConstantPostprocessor

!syntax children /Postprocessors/ConstantPostprocessor
