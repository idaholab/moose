# InterfaceMeshCut2DUserObject

!syntax description /UserObjects/InterfaceMeshCut2DUserObject

## Overview

The `InterfaceMeshCut3DUserObject` is used to cut the 3D mesh with a 2D cutter mesh. The element type of the 2D cutter mesh must be `TRI3`. The nodes on the cutter mesh move at a velocity that is given by a function or `XFEMMovingInterfaceVelocity` UserObject.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition_2d.i block=UserObjects/cut_mesh

!syntax parameters /UserObjects/InterfaceMeshCut2DUserObject

!syntax inputs /UserObjects/InterfaceMeshCut2DUserObject

!syntax children /UserObjects/InterfaceMeshCut2DUserObject
