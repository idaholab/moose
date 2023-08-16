# PolycrystalRandomIC

!syntax description /ICs/PolycrystalRandomIC

## Overview

This initial condition (IC) sets the variable values to initialize a grain structure from a pseudo random structure. There are two options for how the IC functions:

-  `Random_type = continuous`: The variable value is randomly generated between 0 and 1 at every node.
-  `Random_type = discrete`: One of the order parameter variables representing the polycrystal is randomly selected to equal 1 at the node, and all others equal 0.

## Example Input File Syntax

We never recommend using this IC directly, but rather creating the full set of ICs for all of the variables using [PolycrystalRandomICAction](PolycrystalRandomICAction.md).

!syntax parameters /ICs/PolycrystalRandomIC

!syntax inputs /ICs/PolycrystalRandomIC

!syntax children /ICs/PolycrystalRandomIC
