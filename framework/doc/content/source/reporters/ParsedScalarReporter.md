# ParsedScalarReporter

!syntax description /Reporters/ParsedScalarReporter

## Overview

`ParsedScalarReporter` operates on scalars contained from a series of scalar reporters using a [parsed function expression](MooseParsedFunction.md) as shown in [vectorMath].  This reporter allows several scalar reporters to be combined into a single scalar.  Variables in the [parsed function expression](MooseParsedFunction.md) are given by the [!param](/Reporters/ParsedScalarReporter/scalar_reporter_symbols).  There is one [!param](/Reporters/ParsedScalarReporter/scalar_reporter_names) for each [!param](/Reporters/ParsedScalarReporter/scalar_reporter_symbols).
This is a scalar version of the [ParsedVectorReporter.md] reporter.

!listing parsed_reporters/vectorMath.i id=vectorMath
block=Reporters/vecs Reporters/scalarOperation

## Optimization use case

This reporter was created to compute the objective function from the misfit reporter vector created by [OptimizationData](source/reporters/OptimizationData.md optional=True).  The scalar reporter created can then be transferred as the objective value into [GeneralOptimization](source/optimizationreporters/GeneralOptimization.md optional=True).

!syntax parameters /Reporters/ParsedScalarReporter

!syntax inputs /Reporters/ParsedScalarReporter

!syntax children /Reporters/ParsedScalarReporter
