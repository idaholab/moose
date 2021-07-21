# InterfaceMeshCut3DUserObject

!syntax description /UserObjects/InterfaceMeshCut3DUserObject

## Overview

The `InterfaceMeshCut3DUserObject` is used to cut the 3D mesh with a 2D cutter mesh. The nodes on the cutter mesh move at a velocity that is given by a function or `XFEMMovingInterfaceVelocityBase` derived UserObject.

## Example Input File Syntax

!listing modules/xfem/test/tests/moving_interface/phase_transition_3d.i block=UserObjects/cut_mesh

!syntax parameters /UserObjects/InterfaceMeshCut3DUserObject

!syntax inputs /UserObjects/InterfaceMeshCut3DUserObject

!syntax children /UserObjects/InterfaceMeshCut3DUserObject
