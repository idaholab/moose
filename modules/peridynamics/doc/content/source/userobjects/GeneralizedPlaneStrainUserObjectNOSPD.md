# Self-stabilized Non-ordinary State-based Generalized Plane Strain UserObject

## Description

UserObject `GeneralizedPlaneStrainUserObjectNOSPD` is used to provide the residual and Jacobian for the scalar out-of-plane strain field variable in a generalized plane strain formulation using the Self-stabilized Non-Ordinary State-based Peridynamic theory. The out-of-plane stress component is retrieved from the `Compute***Stress` Material class. Integral of this component over the whole simulation domain is accomplished by summation of product of material point area and the component value. Residual is the difference between stress integral and the applied force in the our-of-plane direction. Jacobian is the derivative of residual with respect to the out-of-plane strain.

!syntax parameters /UserObjects/GeneralizedPlaneStrainUserObjectNOSPD

!syntax inputs /UserObjects/GeneralizedPlaneStrainUserObjectNOSPD

!syntax children /UserObjects/GeneralizedPlaneStrainUserObjectNOSPD
