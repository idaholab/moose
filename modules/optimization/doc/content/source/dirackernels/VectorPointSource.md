# VectorPointSource

!syntax parameters /DiracKernels/VectorPointSource

## Overview

A `VectorPointSource` reads in multiple point sources from a [Reporter](Reporters/index.md) or [VectorPostprocessor](VectorPostprocessors/index.md).  The point source values and coordinates are updated as the values are changed.

!alert note
It is important for the `VectorPointSource` to never use a [VectorPostprocessor](VectorPostprocessors/index.md) with [!param](/VectorPostprocessors/PointValueSampler/contains_complete_history)` = true`, as this can modify the ordering of the coordinates and points.

## Example Input Syntax

An example of a `VectorPointSource` using a [ConstantReporter](/ConstantReporter.md):

!listing vector_point_source.i block=DiracKernels Reporters

!syntax parameters /DiracKernels/VectorPointSource

!syntax inputs /DiracKernels/VectorPointSource

!syntax children /DiracKernels/VectorPointSource
