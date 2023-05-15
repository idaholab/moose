# VariableMapping System

## Overview

The mapping system is dedicated to holding objects which map high-dimensional solution vectors
to lower-dimensional spaces (latent spaces). These objects do not get executed on their own, but only used
in other objects which need mapping functionality.

## Creating a VariableMapping

A mapping object can be created by inheriting from `VariableMappingBase` and overriding the methods in the base class.
These methods describe the mapping from high to low dimensional spaces and the corresponding inverse mapping procedures.

## Using a Mapping

### The VariableMappings block

In an input file, one can create Mapping Objects by specifying them in the `[VariableMappings]` block.

### Mapping from high- to low-dimensional spaces

High-dimensional data can be mapped to lower-dimensional spaces using [MappingReporter.md].
This can either map the fields in a [ParallelSolutionStorage.md] or map multiple solution variables in a given
nonlinear system. The results are stored in a standard vector format in the reporter data structure.

### Inverse mapping from low- to high-dimensional spaces

Low-dimensional data can be mapped to higher-dimensional spaces using [InverseMapping.md].
This can either utilize surrogate models to determine the low-dimensional vectors given
a specific set of model parameters or take a custom low-dimensional vector and use inverse mapping
to populate `AuxVariable`s with the reconstructed approximate fields.

### MappingInterface

By inheriting from `MappingInterface`, classes can easily fetch mapping objects
from the object warehouse using the helper functions. Good examples are the [MappingReporter.md] and
[InverseMapping.md].

## Example Input File Syntax

!listing test/tests/variablemappings/pod_mapping/pod_mapping_main.i block=VariableMappings

!syntax list /VariableMappings objects=True actions=False subsystems=False

!syntax list /VariableMappings objects=False actions=True subsystems=False

!syntax list /VariableMappings objects=False actions=False subsystems=True
