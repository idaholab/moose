# OptimizationReporter

!syntax description /OptimizationReporter/OptimizationReporter

## Overview

The `OptimizationReporter` sets up a reporter used for communicating data between the optimization executioner and the transfers to the sub-app forward and adjoint solves.  Only one `OptimizationReporter` is allowed per main-app.

## Example Input File Syntax

There two required parameters in the `OptimizationReporter` specified in [optRep] used to define the design parameters.  The first, [!param](/OptimizationReporter/OptimizationReporter/parameter_names), defines a vector of names for each group of design parameters.  The second, [!param](/OptimizationReporter/OptimizationReporter/num_values), defines a vector containing the number of design parameters in each group named in `parameter_names`.  Optional parameters bounds can be placed on each parameter group using [!param](/OptimizationReporter/OptimizationReporter/lower_bounds) and [!param](/OptimizationReporter/OptimizationReporter/upper_bounds).

The `OptimizationReporter` contains reporters holding measurement data.  This measurement data can optionally be defined in the `OptimizationReporter` through the input file, as shown in [optRep], or from a csv file.  Alternatively, the measurement data can be transferred from a subapp reporter into the `OptimizationReporter` measurement reporters.

!listing test/tests/optimizationreporter/point_loads/main.i
         block=OptimizationReporter
         id=optRep

!syntax parameters /OptimizationReporter/OptimizationReporter

!syntax inputs /OptimizationReporter/OptimizationReporter

!syntax list /OptimizationReporter objects=True actions=False subsystems=False groups=OptimizationApp

!syntax list /OptimizationReporter objects=False actions=True subsystems=False
