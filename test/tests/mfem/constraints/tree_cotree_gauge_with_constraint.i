# Magnetostatic A-formulation, curl(nu curl A) = J, solved with first-kind Nedelec
# elements on a beam split into two subdomains. The magnetic vector potential is
# strongly prescribed in subdomain 2 by an MFEMVectorEssentialConstraint, and a
# tree-cotree gauge removes the gradient null space of the curl-curl operator
# elsewhere.
#
# The gauge must seed its spanning forest with the edges the constraint already
# fixes; a forest grown without them links vertices of subdomain 2 through edges of
# subdomain 1, whose gauged values then over-constrain the potential. Gauging the
# whole mesh therefore has to give exactly the same solution as excluding subdomain
# 2 from the gauge with 'block', which seeds the forest with the same edges.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/beam-hex.mesh
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [HCurlFESpace]
    type = MFEMVectorFESpace
    fec_type = ND
    fec_order = FIRST
  []
  [HDivFESpace]
    type = MFEMVectorFESpace
    fec_type = RT
    fec_order = CONSTANT
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
  # Potential of a uniform field B = (0, 0, 1).
  [prescribed_potential]
    type = ParsedVectorFunction
    expression_x = '-0.5 * y'
    expression_y = ' 0.5 * x'
    expression_z = '0'
  []
[]

[BCs]
  [tangential_a_zero]
    type = MFEMVectorTangentialDirichletBC
    variable = a_field
    vector_coefficient = '0 0 0'
    boundary = 1
  []
[]

[Constraints]
  [prescribed_potential]
    type = MFEMVectorEssentialConstraint
    variable = a_field
    block = 2
    vector_coefficient = prescribed_potential
  []
  [tree_cotree_gauge]
    type = MFEMTreeCotreeGaugeEssentialConstraint
    variable = a_field
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = a_field
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = a_field
    vector_coefficient = source_current
    block = 1
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
  # Magnetic field energy; a convenient deterministic scalar for regression.
  [MagneticEnergy]
    type = MFEMVectorFEInnerProductIntegralPostprocessor
    primal_variable = b_field
    dual_variable = b_field
  []
[]

[Outputs]
  csv = true
[]
