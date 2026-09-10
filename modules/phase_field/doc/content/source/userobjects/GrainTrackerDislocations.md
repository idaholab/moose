# GrainTrackerDislocations

!syntax description /Postprocessors/GrainTrackerDislocations

`GrainTrackerDislocations` is an extension of the [GrainTracker](/GrainTracker.md) that assigns the initial dislocation densities and tracks the formation time for each grain. Three input parameters are added: `dislocation_density_reader` provides the name of the [DislocationDensityFileReader](/DislocationDensityFileReader.md) object.  There are two optional parameters, `add_default_density_grains` and `default_density`.  The former is a boolean that specifies what the GrainTrackerDislocations should do in the case that more grains exist that need to be given a dislocation density from the file than are provided in the file.  If true, grains will be given the dislocation density provided to the `default_density` parameter.  If false, the simulation will issue an error and end.

!syntax parameters /Postprocessors/GrainTrackerDislocations

!syntax inputs /Postprocessors/GrainTrackerDislocations

!syntax children /Postprocessors/GrainTrackerDislocations

!bibtex bibliography
