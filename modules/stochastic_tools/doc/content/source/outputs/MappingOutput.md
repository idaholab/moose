# MappingOutput

!syntax description /Outputs/MappingOutput

## Overview

[Mapping objects](VariableMappings/index.md), once trained, can be output to a binary file for later
use by using this object. All data within the model that was declared using
`RestartableModelInterface::declareModelData()` is automatically stored in the generated file.

## Example Input File Syntax

!listing test/tests/reporters/mapping/map_main.i block=Outputs

## Syntax

!syntax parameters /Outputs/MappingOutput

!syntax inputs /Outputs/MappingOutput

!syntax children /Outputs/MappingOutput
