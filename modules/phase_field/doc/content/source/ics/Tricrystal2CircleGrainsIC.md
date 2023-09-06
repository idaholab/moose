# Tricrystal2CircleGrainsIC

!syntax description /ICs/Tricrystal2CircleGrainsIC

## Overview

This initial condition (ICs) sets the variable values to represent a grain structure with the two grains represented by circles in 2D and spheres in 3D embedded in a third grain. The locations and radii of the circles are defined in terms of the dimensions of the domain, where $d_x$, $d_y$, and $d_z$ are the domain dimensions in the x-, y-, and z-directions, and the coordinate of the bottom left corner of the domain ($bl_x$, $bl_y$, $bl_z$).

- `op_index` = 0: Matrix grain
- `op_index` = 1: Left grain with radius $= d_x/5$ and center location ($bl_x+d_x/4, bl_y+d_y/2, bl_z+d_z/2$)
- `op_index` = 2: Right grain with radius $= d_x/5$ and center location ($bl_x+3d_x/4, bl_y+d_y/2, bl_z+d_z/2$)

Note that the circle grains are created with sharp interfaces. Similar geometry can be created with the [SpecifiedSmoothCircleIC](/SpecifiedSmoothCircleIC), but the locations, radii, and whether a diffuse interface is used are all determined by the user.

## Example Input File Syntax

We never recommend using this IC directly, but rather creating the full set of ICs for all of the variables using [Tricrystal2CircleGrainsICAction](Tricrystal2CircleGrainsICAction.md).

!syntax parameters /ICs/Tricrystal2CircleGrainsIC

!syntax inputs /ICs/Tricrystal2CircleGrainsIC

!syntax children /ICs/Tricrystal2CircleGrainsIC
