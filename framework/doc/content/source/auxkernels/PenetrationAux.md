# PenetrationAux

Auxililary Kernel for computing several geometry related quantities between two different bodies in or near contact.

## Gap offset parameters

Gap offset can be provided to offset the gap distance between in or near contact bodies. It can be either `slave_gap_offset` (gap offset from slave side) or `mapped_master_gap_offset` (gap offset from master side but mapped to slave side). However, the offsetted gap is treated as rigid region without deformation and temperature gradient.

!syntax description /AuxKernels/PenetrationAux

!syntax parameters /AuxKernels/PenetrationAux

!syntax inputs /AuxKernels/PenetrationAux

!syntax children /AuxKernels/PenetrationAux
