# Reactor Module

## Overview

This `reactor` module aims to add advanced meshing capabilities to MOOSE so that users can create complex-geometry meshes that are related to reactors without turning to external meshing software.

This module consists of a series of new mesh generators to enable meshing of reactor cores including both hexagonal and Cartesian geometries. The functionalities of these mesh generators cover:

- Creating unit hexagon meshes as basic components for the core (other polygon meshes can also be created);
- Stitching the unit hexagon meshes and adding appropriate peripheral regions to form hexagon assembly regions;
- Creating unit hexagon meshes with adaptive external boundaries that are stitch-able with assembly meshes;
- Stitching the assembly meshes and adaptive-boundary unit hexagon meshes to form a core mesh;
- Modifying unit hexagon meshes to create azimuthal section blocks to enable static simulation of rotational control drum;
- Adding reporting IDs (extra element integers) to mesh elements to identify pins, assemblies regions;
- Adding a peripheral region to a given input mesh to produce a mesh with a circular external boundary.

Aside from the mesh generators, this `reactor` module also includes a Function object to enable dynamic simulation of rotational control drums in a reactor core mesh.

## Tutorial

This module is demonstrated in detail in the [MOOSE Reactor Module Meshing Tutorial](https://mooseframework.inl.gov/getting_started/examples_and_tutorials/tutorial04_meshing/index.html).

## Objects and Syntax

!syntax complete groups=ReactorApp level=3
