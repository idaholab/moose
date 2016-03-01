# UserObject for Generalized Plane Strain Model Based on Form I of Horizon-Stabilized Peridynamic Correspondence Formulation

## Description

The `GeneralizedPlaneStrainUserObjectNOSPD` UserObject is used to provide the residual and Jacobian for the scalar out-of-plane strain field variable in a generalized plane strain formulation using Form I of the horizon-stabilized peridynamic correspondence formulation. The out-of-plane stress component is retrieved from the `Compute***Stress` Material class. Integration of this component over the whole simulation domain is accomplished by summation of the product of material point area and the component value. The residual is the difference between stress integral and the applied force in the out-of-plane direction. The Jacobian is the derivative of residual with respect to the out-of-plane strain.

!syntax parameters /UserObjects/GeneralizedPlaneStrainUserObjectNOSPD

!syntax inputs /UserObjects/GeneralizedPlaneStrainUserObjectNOSPD

!syntax children /UserObjects/GeneralizedPlaneStrainUserObjectNOSPD
