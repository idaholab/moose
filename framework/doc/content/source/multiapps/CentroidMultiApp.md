# CentroidMultiApp

!syntax description /MultiApps/CentroidMultiApp

## Description

Creates a sub-app at the centroid of every element in the parent app, which can be leveraged for
doing multiscale solves. This object requires no special parameters, but this is
[block restrictable](BlockRestrictable.md) so that the sub-applications can be restricted to only
be generated on specified subdomains.

## Example Input Syntax

The following code snippet demonstrates the creation of a CentroidMultiApp object.

!listing centroid_multiapp/centroid_multiapp.i block=MultiApps

!syntax parameters /MultiApps/CentroidMultiApp

!syntax inputs /MultiApps/CentroidMultiApp

!syntax children /MultiApps/CentroidMultiApp
