# FeatureFloodCount

The FeatureFloodCount object is a utility that inspects solution fields looking for [Connected Components](https://en.wikipedia.org/wiki/Connected_component_(graph_theory)) or "topologically connected regions of a solution field sharing a similar characteristic". Typically, this means a region where the solution value is higher than some threshold. This object is designed to work efficiently on a partitioned unstructured mesh with hundreds to thousands of processors.

!media media/phase_field/unstructured_1.png id=unstructured1 style=width:360px;float:left caption=The identification of a new feature.

!media media/phase_field/unstructured_2.png id=unstructured2 style=width:360px;margin-left:380px caption=Intermediate stage of identification.

!media media/phase_field/unstructured_3.png id=unstructured3 style=width:360px;float:left caption=Identification of the region complete.

!media media/phase_field/unstructured_4.png id=unstructured4 style=width:360px;margin-left:380px caption=Halo extension complete.

<br/>
The algorithm for identifying portions of connected components begins by running a [Flood Fill](https://en.wikipedia.org/wiki/Flood_fill) algorthim on each processor that recursively visits neighboring elements on the unstructured mesh while the connecting criteria is met. \ref{unstructured1} illustrates the identification of a new region on a processor. The dark shaded element represents an element that was identified whose variable value exceeds a given threshold. The lightly shaded elements surronding the dark element represent the current "halo" markings of the region. These halo markings always extend one neighbor beyond the currently shaded region. They are used for both the connected component algorithm and for identifying potential collisions among disjoint regions.


!media media/phase_field/grain_stitching_small_split.svg id=grain_stitch_split style=width:400px;float:left;margin-left:20px caption=Regular grid with 6 features partitioned 3 ways.

!media media/phase_field/grain_stitching_small.svg id=grain_stitch style=width:350px;margin-left:480px caption=Global identification of all features.


<br/>
Several pieces of information are recorded on each processor including all of the marked elements, a minimum ID for a partition independent stable ordering and "overlapping elements" for stitching. \ref{grain_stitch_split} shows a regular mesh partitioned among three processors with several regions of interest. The alpha characters represent a possible local ordering of the features. The subscript represents the processor ID. Portions of the feature data structure is serialized and sent to the rank 0 process where connection information is used to discover the global picture \ref{grain_stitch}.


!syntax description /Postprocessors/FeatureFloodCount

!syntax parameters /Postprocessors/FeatureFloodCount

!syntax inputs /Postprocessors/FeatureFloodCount

!syntax children /Postprocessors/FeatureFloodCount
