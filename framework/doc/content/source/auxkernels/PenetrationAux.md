# PenetrationAux

Auxililary Kernel for computing several geometry related quantities between two different bodies in or near contact.

## Gap offset parameters

Gap offset can be provided to offset the gap distance between in or near contact bodies. It can be either `secondary_gap_offset` (gap offset from secondary side) or `mapped_primary_gap_offset` (gap offset from primary side but mapped to secondary side). Use of these gap offset parameters treats the surfaces as if they were virtually extended (positive offset value) or narrowed (negative offset value) by the specified amount, so that the surfaces are treated as if they are closer or further away than they actually are. There is no deformation or resistance to heat transfer within the material in this gap offset region.

!syntax description /AuxKernels/PenetrationAux

!syntax parameters /AuxKernels/PenetrationAux

!syntax inputs /AuxKernels/PenetrationAux

!syntax children /AuxKernels/PenetrationAux
