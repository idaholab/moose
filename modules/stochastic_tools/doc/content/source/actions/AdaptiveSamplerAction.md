# AdaptiveSamplerAction

!syntax description /Samplers/AdaptiveSamplerAction

## Overview

This action automatically adds the necessary objects to terminate the simulation if an adaptive sampler is used.
This is done by creating a [Terminator](Terminator.md) and an [AdaptiveSamplingCompletedPostprocessor](AdaptiveSamplingCompletedPostprocessor.md).
The simulation is terminated by the former whenever the latter indicates that the adaptive sampler has completed its sampling.
Currently, the two adaptive samplers are [AdaptiveImportanceSampler](AdaptiveImportanceSampler.md) and [ParallelSubsetSimulation](ParallelSubsetSimulation.md).


!syntax parameters /Samplers/AdaptiveSamplerAction
