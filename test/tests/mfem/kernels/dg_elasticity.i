# Isotropic linear elasticity on a two-material beam, discretized with the symmetric interior
# penalty DG method. The displacement (0, 0, -0.2 x) is imposed weakly on boundaries 1 and 2, and
# the L2 norm of the displacement is recorded.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/beam-hex.mesh
  uniform_refine = 1
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [L2FESpace]
    type = MFEMVectorFESpace
    fec_type = L2
    fec_order = FIRST
    basis = GaussLobatto
    range_dim = 3
    ordering = vdim
  []
[]

[Variables]
  [displacement]
    type = MFEMVariable
    fespace = L2FESpace
  []
[]

[Functions]
  [dirichlet_displacement]
    type = ParsedVectorFunction
    expression_x = 0
    expression_y = 0
    expression_z = '-0.2 * x'
  []
[]

[FunctorMaterials]
  [attribute_1]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '50.0 50.0'
    block = 1
  []
  [attribute_2]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '1.0 1.0'
    block = 2
  []
[]

[Kernels]
  [elasticity]
    type = MFEMLinearElasticityKernel
    variable = displacement
    lambda = lambda
    mu = mu
  []
  [dg_elasticity]
    type = MFEMDGElasticityKernel
    variable = displacement
    lambda = lambda
    mu = mu
  []
[]

[BCs]
  [dg_elasticity]
    type = MFEMDGElasticityIntegratedBC
    variable = displacement
    boundary = '1 2'
    lambda = lambda
    mu = mu
  []
  [dirichlet]
    type = MFEMDGElasticityDirichletLFIntegratedBC
    variable = displacement
    boundary = '1 2'
    vector_coefficient = dirichlet_displacement
    lambda = lambda
    mu = mu
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    fespace = L2FESpace
    vector_treatment = by_component
    print_level = 0
  []
  [main]
    type = MFEMCGSolver
    preconditioner = boomeramg
    l_tol = 1e-12
    l_max_its = 500
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [l2_norm]
    type = MFEMVectorL2Error
    variable = displacement
    function = '0 0 0'
  []
[]

[Outputs]
  [csv]
    type = CSV
    file_base = OutputData/DGElasticity/dg_elasticity
  []
[]
