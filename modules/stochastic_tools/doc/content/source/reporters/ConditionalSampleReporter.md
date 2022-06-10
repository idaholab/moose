# ConditionalSampleReporter

!syntax description /Reporters/ConditionalSampleReporter

## Description

This object is a type of [StochasticReporter.md], so it can be used in to transfer reporters into using [SamplerReporterTransfer.md]. This object is mainly meant for demonstration for eventual active learning algorithms, but could prove useful. Basically, it enables a inputted function to determine if a multiapp solve is "possible" based on sampler values. The parsed function inputted with the parameter [!param](/Reporters/ConditionalSampleReporter/function) should evaluate to 0 or `false` if multiapp run is not possible/necessary. The variables within the function should be associated with a sampler column, which is defined by the pair of parameters [!param](/Reporters/ConditionalSampleReporter/sampler_vars) and [!param](/Reporters/ConditionalSampleReporter/sampler_var_indices). If the inputted function evaluates to 0 or `false`, the quantity being transferred to this object using [SamplerReporterTransfer.md] with be replaced with [!param](/Reporters/ConditionalSampleReporter/default_value).

## Example Input Syntax

An example usage of this reporter is to ensure that the sampler values are positive. For demonstration, the following makes sure that the sampler value is greater-than or equal to the current simulation time. If it isn't, the multiapp is not run and the transferred quantity is replaced with a value of `1.0`.

!listing conditional_main.i block=MultiApps Transfers Reporters

!syntax parameters /Reporters/ConditionalSampleReporter

!syntax inputs /Reporters/ConditionalSampleReporter

!syntax children /Reporters/ConditionalSampleReporter
