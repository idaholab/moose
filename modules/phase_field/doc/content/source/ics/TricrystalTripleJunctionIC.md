# TricrystalTripleJunctionIC

!syntax description /ICs/TricrystalTripleJunctionIC

## Overview

This initial condition (ICs) sets the variable values to represent a grain structure with three grains coming together at a triple junction. The initial triple junction angles are set by the user.

Note that the grains are created with sharp interfaces.

## Example Input File Syntax

!listing modules/phase_field/test/tests/initial_conditions/TricrystalTripleJunctionIC.i block=GlobalParams

!listing modules/phase_field/test/tests/initial_conditions/TricrystalTripleJunctionIC.i block=ICs

!syntax parameters /ICs/TricrystalTripleJunctionIC

!syntax inputs /ICs/TricrystalTripleJunctionIC

!syntax children /ICs/TricrystalTripleJunctionIC
