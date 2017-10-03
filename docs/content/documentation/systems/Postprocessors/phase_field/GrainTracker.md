# GrainTracker

The Grain Tracker is a utility that may be used in phase-field simulations to reduce the number of order parameters needed to model a large polycrystal system. The GrainTracker utilizes the [FeatureFloodCount](/FeatureFloodCount.md) object for indentifying and extracting individual grains from a solution field. Once the FeatureFloodCount object has identified all grains, the GrainTracker does two things:

* Match up grains from the current timestep with grains from the previous timestep.
* Remap grains that are "close" to coming into contact.

!syntax description /Postprocessors/GrainTracker

!syntax parameters /Postprocessors/GrainTracker

!syntax inputs /Postprocessors/GrainTracker

!syntax children /Postprocessors/GrainTracker
