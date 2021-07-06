# StressDivergenceTruss

!syntax description /Kernels/StressDivergenceTruss

## Description

This class computes the stiffness and jacobian in the truss local coordinate system using material stiffness and the cross-section area.
This local computation is then transformed to the global coordinate system using the orientation vector of the truss. The component of the force in the $i^{th}$ global coordinate direction is returned as the residual.

!syntax parameters /Kernels/StressDivergenceTruss

!syntax inputs /Kernels/StressDivergenceTruss

!syntax children /Kernels/StressDivergenceTruss

!bibtex bibliography
