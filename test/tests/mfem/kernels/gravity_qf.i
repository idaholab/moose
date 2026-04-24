# Linear elasticity beam under gravity, with the vector body-force density routed through a
# vector quadrature function coefficient in place of the source material coefficient.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/beam-tet.mesh
  uniform_refine = 2
  displacement = "displacement"
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
    range_dim = 3
    ordering = "vdim"
  []
[]

[Variables]
  [displacement]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [dirichlet]
    type = MFEMVectorDirichletBC
    variable = displacement
    boundary = '1'
  []
[]

[FunctorMaterials]
  [Rigidium]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '50.0 50.0'
    block = 1
  []
  [Bendium]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '1.0 1.0'
    block = 2
  []
  [RigidiumWeightDensity]
    type = MFEMGenericFunctorVectorMaterial
    prop_names = 'gravitational_force_density'
    prop_values = '{0.0 0.0 -1e-2}'
    block = 1
  []
  [BendiumWeightDensity]
    type = MFEMGenericFunctorVectorMaterial
    prop_names = 'gravitational_force_density'
    prop_values = '{0.0 0.0 -5e-3}'
    block = 2
  []
[]

[QuadratureFunctions]
  [qf_gravity]
    type = MFEMVectorQuadratureFunction
    vector_coefficient = gravitational_force_density
    # the quadrature rule order matches the one used by VectorDomainLFIntegrator
    # for first-order elements (2 * fe_order = 2)
    order = 2
    updates = none
  []
[]

[Kernels]
  [diff]
    type = MFEMLinearElasticityKernel
    variable = displacement
    lambda = lambda
    mu = mu
  []
  [gravity]
    type = MFEMVectorDomainLFKernel
    variable = displacement
    vector_coefficient = qf_gravity
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    fespace = H1FESpace
    l_max_its = 20
    l_tol = 1e-5
    print_level = 2
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_max_its = 100
    l_tol = 1e-10
    l_abs_tol = 0.0
    print_level = 2
  []
[]

[Executioner]
  type = MFEMSteady
  device = "cpu"
[]

[Postprocessors]
  [displacement_l2_norm]
    type = MFEMVectorL2Error
    variable = displacement
    function = '0 0 0'
  []
[]

[Outputs]
  csv = true
  file_base = OutputData/GravityQF
[]
