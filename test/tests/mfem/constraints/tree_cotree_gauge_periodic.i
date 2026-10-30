# Error case: the tree-cotree gauge identifies mesh vertices by their coordinates,
# which a periodic mesh, carrying discontinuous nodes, cannot provide.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/periodic-square.mesh
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
[]

[Variables]
  [a_field]
    type = MFEMVariable
    fespace = HCurlFESpace
  []
[]

[Constraints]
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
[]

[Solvers]
  [main]
    type = MFEMHyprePCG
    l_tol = 1e-10
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]
