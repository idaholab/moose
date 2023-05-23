# MappingReporter

!syntax description /Reporters/MappingReporter

## Overview

This object is responsible for mapping high-dimensional solution fields to low-dimensional
vectors. The mapped vectors are then stored within the reporter data storage. The
[!param](/Reporters/MappingReporter/mapping) parameter can be used to specify the
mapping object from the `VariableMappings` block. The variables which need to be mapped can be defined using
[!param](/Reporters/MappingReporter/variables). There are two distinct ways to use this object:

1. +In conjunction with a [ParallelSolutionStorage.md] object:+ In this case, the first time
   this object is executed, it will build the mapping using the available data in the parallel storage.
   Then, using the generated mapping, it maps the solution fields in the parallel storage into the latent space and
   saves the coordinates into the reporter storage. This gives a straightforward pipeline for the training
   of surrogate models for the coordinates of the solutions fields in the latent space. In this case,
   the user is expected to define the [!param](/Reporters/MappingReporter/parallel_storage) and
   [!param](/Reporters/MappingReporter/sampler) parameters.

2. +Without a [ParallelSolutionStorage.md] object:+ This functionality is designed to load an already
   trained mapping object and map a solution variable in a nonlinear system into the latent space and
   store the coordinates in this reporter.


## Example Input File Syntax

Example for mapping solutions in a [ParallelSolutionStorage.md] object:

!listing test/tests/reporters/mapping/map_main.i block=Reporters

Example for mapping solutions:

!listing test/tests/reporters/mapping/load_main.i block=Reporters

## Syntax

!syntax parameters /Reporters/MappingReporter

!syntax inputs /Reporters/MappingReporter

!syntax children /Reporters/MappingReporter
