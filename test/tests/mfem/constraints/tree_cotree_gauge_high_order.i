# Error case: the tree-cotree gauge fixes lowest-order edge degrees of freedom
# only, so a second order H(curl) space would keep the gradient modes carried by
# its higher-order degrees of freedom and stay singular.

[Mesh]
  type = MFEMMesh
  file = ../mesh/embedded_concentric_torus.e
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
[]

[Variables]
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[Functions]
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
    boundary = 'Exterior'
  []
[]

[Constraints]
  [tree_cotree_gauge]
    type = MFEMTreeCotreeGaugeEssentialConstraint
    variable = a_field
    boundary = 'Exterior'
  []
[]

[Kernels]
  [curlcurl]
    type = MFEMCurlCurlKernel
    variable = a_field
    coefficient = 1.0
  []
  [source]
    type = MFEMVectorFEDomainLFKernel
    variable = a_field
    vector_coefficient = source_current
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
    l_tol = 1e-10
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]
