# Self-stabilized Non-ordinary State-based Generalized Plane Strain UserObject

## Description

The `GeneralizedPlaneStrainUserObjectHNOSPD` UserObject is used to provide the residual and Jacobian for the scalar out-of-plane strain field variable in a generalized plane strain formulation using the horizon-stabilized peridynamic correspondence model. The out-of-plane stress component is retrieved from the `Compute***Stress` Material class. Integration of this component over the whole simulation domain is accomplished by summation of the product of material point area and the component value. The residual is the difference between stress integral and the applied force in the our-of-plane direction. The Jacobian is the derivative of residual with respect to the out-of-plane strain.

!syntax parameters /UserObjects/GeneralizedPlaneStrainUserObjectHNOSPD

!syntax inputs /UserObjects/GeneralizedPlaneStrainUserObjectHNOSPD

!syntax children /UserObjects/GeneralizedPlaneStrainUserObjectHNOSPD
