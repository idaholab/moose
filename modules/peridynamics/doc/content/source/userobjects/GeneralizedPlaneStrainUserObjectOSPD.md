# Ordinary State-based Peridynamic Generalized Plane Strain UserObject

## Description

The `GeneralizedPlaneStrainUserObjectOSPD` UserObject is used to provide the residual and Jacobian for the scalar out-of-plane strain field variable in a generalized plane strain formulation using the ordinary state-based peridynamic model. The out-of-plane stress component is calculated using the `NodalStressStrainPD` AuxKernel class. Integration of this component over the whole simulation domain is accomplished by summation of product of material point area and the component value. The residual is the difference between stress integral and the applied force in the our-of-plane direction. The Jacobian is the derivative of residual with respect to the out-of-plane strain.

!syntax parameters /UserObjects/GeneralizedPlaneStrainUserObjectOSPD

!syntax inputs /UserObjects/GeneralizedPlaneStrainUserObjectOSPD

!syntax children /UserObjects/GeneralizedPlaneStrainUserObjectOSPD
