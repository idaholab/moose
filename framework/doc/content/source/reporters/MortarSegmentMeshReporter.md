# MortarSegmentMeshReporter

## Overview

`MortarSegmentMeshReporter` reports element count and volume statistics for every mortar interface
in the problem. It covers three mesh components at each interface:

- **Secondary lower-dimensional mesh** — the lower-D projection of the secondary boundary
- **Primary lower-dimensional mesh** — the lower-D projection of the primary boundary
- **Mortar segment mesh (MSM)** — the integration mesh formed at the interface.
  See [AutomaticMortarGeneration](source/constraints/AutomaticMortarGeneration.md)
  for details on how it is constructed in 2D and 3D.

For each component the following four quantities are reported:

| Reported value | Description |
| --- | --- |
| `*_n_elems` | Number of elements |
| `*_max_volume` | Maximum element volume |
| `*_min_volume` | Minimum element volume |
| `*_median_volume` | Median element volume |

where `*` is `secondary_lower`, `primary_lower`, or `msm` respectively. Each output is a vector
with one entry per primary-secondary subdomain pair, appended in the order the interfaces are
iterated internally.

By default the reporter operates on the undisplaced mesh. Set
[!param](/Reporters/MortarSegmentMeshReporter/on_displaced) to `true` to instead collect
statistics from the displaced mortar interfaces.

## Example Input File Syntax

The following snippet shows the reporter collecting statistics at the `INITIAL` timestep, with
JSON output.

!listing mortar/mortar-mesh/mortar_stats_reporter.i block=Reporters

A 3D example using tetrahedral elements is also available:

!listing mortar/mortar-mesh/mortar_stats_reporter_3d.i block=Reporters

!syntax parameters /Reporters/MortarSegmentMeshReporter

!syntax inputs /Reporters/MortarSegmentMeshReporter

!syntax children /Reporters/MortarSegmentMeshReporter
