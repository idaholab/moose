# ExternalProblem

Intermediate class that should be used as an extension point (inheritance) for externally
coupled codes (MOOSE-Wrapped Apps). The external coupling interface should override the
"solve" method along with a method for testing convergence of the last solve.

More information on using the `ExternalProblem` for wrapping applications may be found
in the [application development documentation](modules/doc/content/application_development/moose_wrapped_apps.md optional=True).

## Examples

- External Petsc Application


The [ExternalPetscProblem](modules/external_petsc_solver/doc/content/source/ExternalPETScProblem.md optional=True)
inherits from the `ExternalProblem` to run a pure PETSc solver. It is also able to sync the PETSc solutions
to MOOSE variables.

- Cardinal


[Cardinal](https://github.com/neams-th-coe/cardinal) is an open source coupling of MOOSE with the Monte Carlo
code [OpenMC](https://github.com/openmc-dev/openmc) and the spectral element CFD code
[NekRS](https://github.com/Nek5000/nekRS), which are both
external solvers and not MOOSE-based applications.
The `ExternalProblem` is used to order the solves and custom transfers were implemented to
pass fields between each application.


!bibtex bibliography
