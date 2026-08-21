# FunctorNodalCorrector

!syntax description /Correctors/FunctorNodalCorrector

## Design

The `FunctorNodalCorrector` is derived from the `DomainUserObject`, so it can change primary variable values
for degrees of freedom located on elements (subdomain-restricted if desired) or on nodes (boundary-restricted if desired).

!alert note
Except for constant monomials, Lagrange variable, and other simple FE families,
simply setting variable DOFs using targetted evaluations may not give the expected result
of a variable following closely the functor values over the domain of interest.

!syntax parameters /Correctors/FunctorNodalCorrector

!syntax inputs /Correctors/FunctorNodalCorrector

!syntax children /Correctors/FunctorNodalCorrector

!bibtex bibliography
