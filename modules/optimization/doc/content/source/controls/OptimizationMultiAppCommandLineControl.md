# OptimizationMultiAppCommandLineControl.md

!syntax description /Controls/OptimizationMultiAppCommandLineControl

## Overview

This control uses the command line control mechanism to change parameters on the subapp.  This allows us to modify parameters during optimization that are not controllable.  This control is an exact copy of MultiAppCommandLineControl from the stochastic tools module with OptimizeFullSolveMultiApp added as a friend.

## Example Input File Syntax

This example controls a function's `val` on the forward multiapp shown by:
!listing test/tests/optimizationreporter/constant_heat_source/main.i block=Controls/toforward

!alert warning title=MultiApps Setup
This control requires the MultiApps to use `reset_app = true` as shown here in the above input file:

!listing test/tests/optimizationreporter/constant_heat_source/main.i block=MultiApps/forward


!syntax parameters /Controls/OptimizationMultiAppCommandLineControl

!syntax inputs /Controls/OptimizationMultiAppCommandLineControl

!syntax children /Controls/OptimizationMultiAppCommandLineControl
