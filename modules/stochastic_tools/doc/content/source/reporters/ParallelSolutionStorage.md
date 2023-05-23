# ParallelSolutionStorage

!syntax description /Reporters/ParallelSolutionStorage

## Overview

This object serves as a central storage for serialized solution fields in a stochastic
simulation. The solution fields are indexed by variable name (alphabetically) and global sample index.
The solutions are stored in a parallel fashion, distributed based on which process
deposits the serialized solution field.
For time-dependent simulations this object holds multiple solution fields for each
global sample index. External objects can deposit data in this container using the following function:

!listing modules/stochastic_tools/include/reporters/ParallelSolutionStorage.h
 start=addEntry
 end=DenseVector
 include-end=true

## Example Input File Syntax

!listing test/tests/reporters/parallel_storage/parallel_storage_main.i block=Reporters

## Syntax

!syntax parameters /Reporters/ParallelSolutionStorage

!syntax inputs /Reporters/ParallelSolutionStorage

!syntax children /Reporters/ParallelSolutionStorage
