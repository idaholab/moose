# FeatureFloodCount

The FeatureFloodCount object is a utility that inspects solution fields looking for [Connected Components](https://en.wikipedia.org/wiki/Connected_component_(graph_theory)) or "topologically connected regions of a solution field sharing a similar characteristic". Typically, this means a region where the solution value is higher than some threshold. This object is designed to work efficiently on a partitioned unstructured mesh with hundreds to thousands of processors.

!row!
!col! small=12 medium=6 large=6
!media media/phase_field/unstructured_1.png id=unstructured1 caption=The identification of a new feature.
!col-end!

!col! small=12 medium=6 large=6
!media media/phase_field/unstructured_2.png id=unstructured2 caption=Intermediate stage of identification.
!col-end!
!row-end!

!row!
!col! small=12 medium=6 large=6
!media media/phase_field/unstructured_3.png id=unstructured3 caption=Identification of the region complete.
!col-end!

!col! small=12 medium=6 large=6
!media media/phase_field/unstructured_4.png id=unstructured4 caption=Halo extension complete.
!col-end!
!row-end!


<br/>
The algorithm for identifying portions of connected components begins by running a [Flood Fill](https://en.wikipedia.org/wiki/Flood_fill) algorthim on each processor that recursively visits neighboring elements on the unstructured mesh while the connecting criteria is met. [unstructured1] illustrates the identification of a new region on a processor. The dark shaded element represents an element that was identified whose variable value exceeds a given threshold. The lightly shaded elements surronding the dark element represent the current "halo" markings of the region. These halo markings always extend one neighbor beyond the currently shaded region. They are used for both the connected component algorithm and for identifying potential collisions among disjoint regions.


!row!
!col! small=12 medium=6 large=6
!media media/phase_field/grain_stitching_small_split.svg id=grain_stitch_split caption=Regular grid with 6 features partitioned 3 ways.
!col-end!

!col! small=12 medium=6 large=6
!media media/phase_field/grain_stitching_small.svg id=grain_stitch caption=Global identification of all features.
!col-end!
!row-end!


<br/>
Several pieces of information are recorded on each processor including all of the marked elements, a minimum ID for a partition independent stable ordering and "overlapping elements" for stitching. [grain_stitch_split] shows a regular mesh partitioned among three processors with several regions of interest. The alpha characters represent a possible local ordering of the features. The subscript represents the processor ID. Portions of the feature data structure is serialized and sent to the rank 0 process where connection information is used to discover the global picture [grain_stitch].


!syntax description /Postprocessors/FeatureFloodCount

!syntax parameters /Postprocessors/FeatureFloodCount

!syntax inputs /Postprocessors/FeatureFloodCount

!syntax children /Postprocessors/FeatureFloodCount
