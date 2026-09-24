# Confirms that the non-Kokkos bridge refuses to read a batch the Kokkos gatherers numbered, rather
# than returning each quadrature point another point's stress.
#
# This is multiblock.i with the non-Kokkos bridge's property consumed, which is what causes it to be
# computed. Across several subdomains the two numberings disagree, so the executor rejects the
# element-keyed batch index the non-Kokkos bridge asks for.

!include multiblock.i

[AuxVariables]
  [host_s]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [host_s]
    type = MaterialSymmetricRankTwoTensorAux
    variable = host_s
    property = 'neml2_stress'
    component = 5
  []
[]
