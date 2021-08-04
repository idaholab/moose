# InterfaceMeshCut2DUserObject

!syntax description /UserObjects/InterfaceMeshCut2DUserObject

## Overview

The `InterfaceMeshCut2DUserObject` is used to cut the 2D mesh with a 1D cutter mesh. The element type of the 1D cutter mesh must be `EDGE2`. The nodes on the cutter mesh move at a velocity that is given by a function or `XFEMMovingInterfaceVelocity` UserObject.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition_2d.i block=UserObjects/cut_mesh

!syntax parameters /UserObjects/InterfaceMeshCut2DUserObject

!syntax inputs /UserObjects/InterfaceMeshCut2DUserObject

!syntax children /UserObjects/InterfaceMeshCut2DUserObject
