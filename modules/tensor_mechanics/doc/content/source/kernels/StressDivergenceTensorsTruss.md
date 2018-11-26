# StressDivergenceTensorsTruss

!syntax description /Kernels/StressDivergenceTensorsTruss

## Description

This class computes the axial force in the truss local coordinate system using the axial stress and the cross-section area. This local force is then transformed to the global coordinate system using the orientation vector of the truss. The component of the force in the $i^{th}$ global coordinate direction is returned as the residual.

!syntax parameters /Kernels/StressDivergenceTensorsTruss

!syntax inputs /Kernels/StressDivergenceTensorsTruss

!syntax children /Kernels/StressDivergenceTensorsTruss

!bibtex bibliography
