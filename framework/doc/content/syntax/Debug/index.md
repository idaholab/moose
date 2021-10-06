# Debug System

## Overview

The `[Debug]` input file block is designed to contain options to enable debugging tools for a
simulation. For example, the input file snippet below demonstrates how to enable the material
property debugging tool. A complete list of the available options is provided below.

!listing show_material_props_debug.i block=Debug

!syntax parameters /Debug id=debug-params


!! Describe and include an example of how to use the Debug system.

!! The following lines automatically list syntax associated with the system

!syntax list /Debug objects=True actions=False subsystems=False

!syntax list /Debug objects=False actions=False subsystems=True

!syntax list /Debug objects=False actions=True subsystems=False
