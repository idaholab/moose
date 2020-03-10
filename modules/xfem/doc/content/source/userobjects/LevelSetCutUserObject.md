# LevelSetCutUserObject

!syntax description /UserObjects/LevelSetCutUserObject

## Overview

This userobject, `LevelSetCutUserObject` uses a level set field to cut the finite element mesh. The zero level set contour represents the interface.

## Example Input File Syntax

!listing modules/xfem/test/tests/diffusion_xfem/levelsetcut2d_aux.i block=UserObjects/level_set_cut_uo

!syntax parameters /UserObjects/LevelSetCutUserObject

!syntax inputs /UserObjects/LevelSetCutUserObject

!syntax children /UserObjects/LevelSetCutUserObject
