# Magnetostatic A-formulation with a SECOND order Nedelec space,
#
#   (nu curl A, curl v) + (kappa A, v) = (J, v),
#
# on a beam split into two subdomains. The mass term kappa A is present in
# subdomain 2 only, so the curl-curl operator is singular in subdomain 1, where a
# tree-cotree gauge removes the gradient null space. Above FIRST order this null
# space also contains the gradients of the higher order H1 functions on edges,
# faces and elements, which the gauge removes with a selection of their own
# degrees of freedom.
#
# The potential itself depends on the gauge, so its norm checks that the gauge is
# independent of the partitioning; the magnetic energy does not.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/beam-tet.mesh
  uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = SECOND
  []
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = FIRST
  []
[]

[Variables]
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[AuxVariables]
  [b_field]
    type = MFEMVariable
    fespace = HDivFESpace
  []
[]

[AuxKernels]
  [curl_a]
    type = MFEMCurlAux
    variable = b_field
    source = a_field
    scale_factor = 1.0
    execute_on = TIMESTEP_END
  []
[]

[Functions]
  # Azimuthal current density, divergence-free.
  [source_current]
    type = ParsedVectorFunction
    expression_x = '-y'
    expression_y = ' x'
    expression_z = '0'
  []
[]

[BCs]
  [tangential_a_zero]
    type = MFEMVectorTangentialDirichletBC
    variable = a_field
    vector_coefficient = '0 0 0'
    boundary = '1'
  []
[]

[Constraints]
  [tree_cotree_gauge]
    type = MFEMTreeCotreeGaugeEssentialConstraint
    variable = a_field
    block = 1
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = a_field
    coefficient = 1.0
  []
  [mass]
    type = MFEMVectorFEMassKernel
    variable = a_field
    coefficient = 1.0
    block = 2
  []
  # Confined to subdomain 2: a current in the unregularized subdomain with a normal
  # component on its boundary would not be orthogonal to the gradient null space,
  # leaving the ungauged problem without a solution.
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = a_field
    vector_coefficient = source_current
    block = 2
  []
[]

[Solvers]
  [ams]
    type = MFEMHypreAMS
    fespace = HCurlFESpace
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = ams
    l_tol = 1e-12
    l_max_its = 500
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [MagneticEnergy]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    primal_variable = b_field
    dual_variable = b_field
  []
  [PotentialNormSquared]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    primal_variable = a_field
    dual_variable = a_field
  []
[]

[Outputs]
  csv = true
[]
