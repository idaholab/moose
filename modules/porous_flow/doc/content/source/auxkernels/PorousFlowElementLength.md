# PorousFlowElementLength

This `AuxKernel` calculates a notion of element "length" along a given direction.  A plane is constructed through the element's centroid, with normal equal to the [!param](/AuxKernels/PorousFlowElementLength/direction).  The average of the distance of the nodal positions to this plane is the "length" returned.  An example is shown in [ele_l_fig].

!media media/porous_flow/PorousFlowElementLength.png style=width:20%;margin-left:10px caption=Length $L$ (red) returned for a rectangular element (black) for the given direction (green).  id=ele_l_fig

!alert note
Only elemental (`Monomial`) `AuxVariables` can be used with this `AuxKernel`

!syntax parameters /AuxKernels/PorousFlowElementLength

!syntax inputs /AuxKernels/PorousFlowElementLength

!syntax children /AuxKernels/PorousFlowElementLength
