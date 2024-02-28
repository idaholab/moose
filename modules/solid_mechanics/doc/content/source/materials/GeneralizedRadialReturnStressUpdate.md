# Generalized Radial Return Stress Update with automatic differentiation

Base class which computes the solution of a generalized radial return mapping algorithm.
The generalization of the isotropic radial return mapping to Hill plasticity allows for
computing the return to the yield surface through a scalar variable when computed
in a proper subspace.

Anisotropic (Hill) creep and plasticity classes such as `HillCreepStressUpdate` and
`HillPlasticityStressUpdate` inherit from the generalized radial return mapping. For
details on the anisotropic plasticity and anisotropic elasto-plasticity algorithms, consult
[!cite](versino2018generalized).

!bibtex bibliography
