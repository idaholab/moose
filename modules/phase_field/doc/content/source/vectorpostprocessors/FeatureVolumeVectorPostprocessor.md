# FeatureVolumeVectorPostprocessor

This VectorPostprocessor is designed to pull data from a [FeatureFloodCount](FeatureFloodCount.md) or one of its
derived classes. In particular, the FeatureFloodCount object stores information
about each connected component that it discovers: volumes, whether or not that component intersects a
boundary, and the variable number.

If this VPP is used with a GrainTracker or derived object, the row number represents the unique identifier of each
feature. This value is stable between time steps meaning that row number 5 would always represent the grain with an
ID of 5 (zero-based indexing). This also means that information from inactive grains is output and should be ignored.
Inactive grains are identified with `var_num == -1`.

When using this VPP with a FeatureFloodCount object, there is no implied ordering of the features.

- var_num - The index of which coupled variable represents this feature [0..n).
- feature_volume - The volume of the feature, computed as an integral of the solution over each element representing this feature.
- intersects_bounds - A Boolean indicating whether this feature intersects any boundary.
- centroid_<x, y, z> (optional) - The coordinates of each feature centroid (Currently only supported when no periodic boundaries are used).

## Typical Output

!listing grain_tracker_volume_out_grain_volumes_0000.csv

## Centroid Output

This VectorPosptrocessor can also output centroid information when the simulation is not using periodic boundary conditions. To enable
centroid output, add `output_centroids = true` to the VPP block in your input file.

## Usage

To use this VPP, you simply need to specify the name of a FeatureFloodCount or derived object in
the input file.

!listing grain_tracker_volume.i block=VectorPostprocessors

## Advanced Parameter(s)

The `single_feature_per_element` parameter is a crude attempt at total volume conservation. When you set this Boolean to true, the volume of every element
will be added only to the most dominant feature. In this manner, the loss of an element in one feature results in the addition of an element to another feature.
There is no loss of volume to grain boundaries or anything else using this assumption.

## Class Description

!syntax description /VectorPostprocessors/FeatureVolumeVectorPostprocessor

!syntax parameters /VectorPostprocessors/FeatureVolumeVectorPostprocessor

!syntax inputs /VectorPostprocessors/FeatureVolumeVectorPostprocessor

!syntax children /VectorPostprocessors/FeatureVolumeVectorPostprocessor

!bibtex bibliography
