# MultiAppPositions

!syntax description /Positions/MultiAppPositions

## Overview

`MultiApps` have a position that is used to translate their frame of reference compared to
the main app frame of reference. This position can be used to align the meshes for the transfers or
for outputting the `MultiApps` [Exodus results](Exodus.md) at the translated positions.

!alert warning
This should **not** be used to generate new `MultiApps`. The `MultiApps` have already
been created when the positions are gathered.

## Example MultiApp File Syntax

In this example, the `MultiAppPositions` is obtaining the positions of three `MultiApps`.

!listing tests/positions/multiapp_positions.i block=Positions MultiApps

!syntax parameters /Positions/MultiAppPositions

!syntax inputs /Positions/MultiAppPositions

!syntax children /Positions/MultiAppPositions
