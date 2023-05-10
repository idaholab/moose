# HSBoundaryExternalAppConvection

This component is the same as [HSBoundaryAmbientConvection.md] but uses
temperatures and heat transfer coefficients transferred from an external
application into auxiliary variables.

If this component is used with a cylindrical heat structure, the post-processor
*name*`_integral` is added, which gives the heat rate found by integrating this
heat flux over the boundary.

!syntax parameters /Components/HSBoundaryExternalAppConvection

!syntax inputs /Components/HSBoundaryExternalAppConvection

!syntax children /Components/HSBoundaryExternalAppConvection

!bibtex bibliography
