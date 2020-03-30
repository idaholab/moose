# Peridynamic Ghost Element UserObject

## Description

The `GhostElemPD` UserObject is used to ghost the element information which is required for calculation of properties on current processor from other processors. This is due to the parallel computing and the nonlocality of the interaction in peridynamics theory.

!syntax parameters /UserObjects/GhostElemPD

!syntax inputs /UserObjects/GhostElemPD

!syntax children /UserObjects/GhostElemPD
