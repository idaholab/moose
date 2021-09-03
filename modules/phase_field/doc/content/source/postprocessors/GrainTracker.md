# GrainTracker

The Grain Tracker is a utility that may be used in phase-field simulations to reduce the number of
order parameters needed to model a large polycrystal system. The GrainTracker utilizes the
[FeatureFloodCount](/FeatureFloodCount.md) object for indentifying and extracting individual grains
from a solution field. Once the FeatureFloodCount object has identified all grains, the GrainTracker
does two things:

- Match up grains from the current timestep with grains from the previous timestep.
- Remap grains that are "close" to coming into contact.

## Grain Tracking

The ability to track features over time is of interest for many simulation types. Here we present an
algorithm for tracking arbitrary features on an unstructured mesh over time. The tracking stage is
responsible for maintaining consistent and unique identification for an arbitrary number of moving
and interacting features over time. The tracking stage is the only stage in the algorithm which
requires stateful data between time steps. This is important from an implementation perspective as it
can have an affect on the ability of a simulation to checkpoint, terminate, and successfully
restart. Restart capabilities are useful for handling hardware faults or spreading out a long running
simulation over several execution windows common in high performance computing environments.

During the first invocation of the feature tracking stage there is no previous feature data to
compare against so no tracking is performed. Instead a set of IDs must be assigned to each identified
feature. These IDs may be supplied externally if desired. In fact there are no restrictions on the
IDs if supplied externally. The IDs need not be contiguous nor must they be unique. However, if
separate features are assigned duplicate IDs and those features come into contact during a
simulation, the data will be coalesced, which may or may not result in a correct simulation.  If an
external assignment is not desired, the feature tracking algorithm will assign a set of contiguous
and unique IDs to each individual feature. This is accomplished by first sorting the identified
features by the min element ID stored in each feature's data structure and assigning a number based
on the sorted position. This strategy ensures a stable sorting insusceptible to different mesh
partitionings.

On subsequent invocations, the feature information from the previous time step is compared against
all of the features from the current time step and organized such that the best matches for all
features is determined correctly. The comparison criterion is to globally minimize the centroid
distances of all features simultaneously. The centroid is calculated by averaging the element
centroids making up each feature. As we iterate over the new list of features, we select the feature
in the previous list that is closest by centroid distance. This pairing is saved into a "best match"
data structure while the remaining features are being processed.

It's possible for features to compete for the same "best match" feature on the previous time
step. This indicates that a feature has been absorbed or has otherwise disappeared on the current
step and that its corresponding feature from the previous step is incorrectly identifying an
unrelated feature as the best match. This case is handled by marking the feature with the greater
centroid distance mismatch as inactive.

When all pairs have been compared, all of the features in the best match data structure are marked
("matched") and the IDs from the previous time step are saved into the corresponding matches in the
current time step. Unmatched features from both the new list and previous lists are then handled.
The features in the previous list that are unmatched are marked as inactive. The unmatched features
in the current list are "new", meaning that they haven't been previously identified. The former case
occurs when there are exactly zero features in the current list, meaning any feature in the previous
list will remain unmatched. The latter case can occur when a feature splits or when a new feature is
created.


## Grain Remapping

!row!
!col! small=12 medium=4 large=4
!media media/phase_field/remap_red_conflict.svg
       id=remap_red_conflict
       caption=Red feature bounding boxes intersecting (fast check).
!col-end!

!col! small=12 medium=4 large=4
!media media/phase_field/remap_red_halo.svg
       id=remap_red_halo
       caption=Red feature halos intersecting (complete check).
!col-end!
!row-end!

Grain remapping is implemented using a recursive backtracking algorithm capable of performing several
variable swaps to transform the improperly colored grain graph into a proper one. This backtracking
algorithm runs only on the root process which is the only processor that contains the complete global
grain graph.  When a pair of grains are located that are in close proximity [remap_red_conflict] and
[remap_red_halo], one of them is arbitrarily chosen and designated as the "target" grain indicating
that we seek to remap its defining variable values to a different solution variable. Depending on the
number of neighbors a graph has and the variables representing each of those neighbors, it may or may
not be possible to create a valid graph by remapping only the target grain. In this case a
depth-limited, depth-first search is performed seeking a series of neighbor swaps to leave the graph
in a valid state.

To begin, an array of lists of size $m$ is built and populated, where $m$ is the number of variables
(colors) in use. For each variable the nearest grain represented by that variable (as determined by
the bounding box distance) is located and its distance is stored in the list at the corresponding
array position along with the grain ID itself. In cases where the nearest bounding boxes for a given
variable overlap the target grain, we maintain a negative count representing the total number of
overlaps and the ID of each grain which overlaps. Otherwise we store the closest bounding box edge to
bounding box edge distance for the given variable. We don't bother to calculate or store any
information for grains with matching variable indices, or for grains that live on a reserved order
parameter since those variables are ineligible for remapping. If there are any empty order parameters
(an order parameter representing zero grains), a distance of infinity ($\infty$) is entered into the
corresponding position prioritizing those variables for remapping. This ``color distance'' array is
then sorted in reverse order putting the grains furthest away near the front and leaving those with
several overlaps near the back.

!row!
!col! small=12 medium=4 large=4
!media media/phase_field/remap_red_yellow.svg
       id=remap_red_yellow
       caption=Distance check against "B" Features.
!col-end!

!col! small=12 medium=4 large=4
!media media/phase_field/remap_red_green.svg
       id=remap_red_green
       caption=Distance check against "C" Features.
!col-end!


!col! small=12 medium=4 large=4
!media media/phase_field/remap_red_blue.svg
       id=remap_red_blue
       caption=Distance check against "D" Features.
!col-end!
!row-end!

!row!
!col! small=12 medium=12 large=12
!table id=large_red style=border:4px solid black;width:350px; caption=Large Red Distances
| Variable | Distance |
|----------|----------|
| A        | $\varnothing$|
| B        | -2.0     |
| C        | -1.0     |
| D        | -3.0     |
!col-end!
!row-end!

A case with all negative distances is illustrated in [large_red]. In this example, the target grain
is chosen as the large grain labeled $A$, centered on the right side of the image. All of the other
colors have at least one bounding box that overlaps the large $A$ grain: 2 for $B$, 1 for $C$, and 3
for $D$.  The empty list ($\varnothing$) is used for the variable represented by the target grain to
ensure that the same variable is never considered as a possible remapping option.

!row!
!col! small=12 medium=4 large=4
!media media/phase_field/remap_green_red.svg
       id=remap_green_red
       caption=Distance check against "A" Features.
!col-end!

!col! small=12 medium=4 large=4
!media media/phase_field/remap_green_yellow.svg
       id=remap_green_yellow
       caption=Distance check against "B" Features.
!col-end!

!col! small=12 medium=4 large=4
!media media/phase_field/remap_green_blue.svg
       id=remap_green_blue
       caption=Distance check against "D" Features.
!col-end!
!row-end!

!row!
!col! small=12 medium=12 large=12
!table id=small_blue style=border:4px solid black;width:350px; caption=Small light blue distances
| Variable | Distance |
|----------|----------|
| A        | 52.6     |
| B        | 4.2      |
| C        | $\varnothing$|
| D        | -1.0     |
!col-end!
!row-end!

We iterate over the array of distances looking for available variables suitable for remapping the
target grain. If a positive value is encountered, the grain can be immediately remapped and the
algorithm returns "success". If however a negative value is encountered, we must first perform a
fine-level check on each of the corresponding grain halos to see if these grains actually overlap. If
they do not, we can immediately remap the target grain and return "success". If we encounter a case
where there is only a single truly overlapping grain (bounding boxes and halos intersect), the
algorithm tentatively marks the target grain with the other grain's variable effectively simulating a
remapping operation. It then recurses on the other neighboring grain making it the new target. If the
algorithm is able to find a successful remap in the recursive call, the returned "success" value
indicates to the caller that the tentative mark can be removed. The "success" value can then be
propagated on up the call stack. If all items in the "color distance" array are exhausted without
finding a successful swap or set of swaps, the algorithm returns "fail". If we are in a recursive
call, the tentative mark is removed and the next value in the array is inspected. We find that
limiting the depth-first search to a relatively small depth (2 or 3) works reasonably well to fail
out of impossibly tightly colored graphs faster. This also helps avoid the huge runtime penalty and
exponential growth rate possible with an unlimited backtracking algorithm. Note: Tentative markings
are indicated by turning on the `DIRTY` status flag in the feature's data structure. The `DIRTY`
status uses an independent bit so it can exist simultaneously with another status.

!row!
!col! small=12 medium=4 large=4
!media media/phase_field/remap_to_red_pre.svg
       id=remap_to_red_pre
       caption=Ready to remap.
!col-end!

!col! small=12 medium=4 large=4
!media media/phase_field/remap_to_red.svg
       id=remap_to_red
       caption=Remap to "A".
!col-end!

!col! small=12 medium=4 large=4
!media media/phase_field/remap_to_green.svg
       id=remap_to_green
       caption=Remap to "C".
!col-end!
!row-end!

## 3D Halo Images

!row!
!col! small=12 medium=6 large=6
!media media/phase_field/3D_halos_start.png
       id=3D_start
       caption=6000 grains in 3D
!col-end!

!col! small=12 medium=6 large=6
!media media/phase_field/3D_halos_end.png
       id=3D_end
       caption=grain structure after several simulation steps.
!col-end!
!row-end!

## Description and Syntax

!syntax description /Postprocessors/GrainTracker

!syntax parameters /Postprocessors/GrainTracker

!syntax inputs /Postprocessors/GrainTracker

!syntax children /Postprocessors/GrainTracker
