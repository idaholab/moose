# Static Condensation Field Split Preconditioner

This class is derived from the [static condensation preconditioner](MooseStaticCondensationPreconditioner.md)
and the class template [FieldSplitPreconditioner.md]. Essentially this class combines the
capabilities of its bases: it enables field split preconditioning of a statically condensed system.
This can be used for things such as multi-field HDG discretizations such as of the Navier-Stokes
equations. In that case static condensation removes the element interior velocity and pressure
degrees of freedom, leaving only the facet velocity and pressure degrees of freedom (dofs). One may
then field split the facet velocity and pressure dofs.

!syntax parameters /Preconditioning/SCFSP

!syntax inputs /Preconditioning/SCFSP

!syntax children /Preconditioning/SCFSP
