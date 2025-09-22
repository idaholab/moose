AD = ''
ADBC = ''

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 20
    ny = 4
  []
[]

[Variables]
  [u]
    family = MONOMIAL
  []
[]

[Kernels]
  [convection]
    type = ${AD}ConservativeAdvection
    variable = u
    velocity_material = '${AD}velocity'
  []
  [diffusion]
    type = ${AD}MatDiffusion
    variable = u
    diffusivity = 1
  []
[]

[DGKernels]
  [convection]
    type = ADDGAdvection
    variable = u
    velocity = 'ADvelocity'
  []
  [diffusion]
    type = ${AD}DGDiffusion
    variable = u
    sigma = 6
    epsilon = -1
    diff = 1
  []
[]

[Functions]
  [v_inlet]
    type = ParsedVectorFunction
    expression_x = '1'
  []
[]

[BCs]
  [u_walls]
    type = DGFunctionDiffusionDirichletBC
    boundary = 'bottom top'
    variable = u
    sigma = 6
    epsilon = -1
    function = '0'
    diff = 1
  []
  [u_in]
    type = ${ADBC}ConservativeAdvectionBC
    boundary = 'left'
    variable = u
    velocity_function = v_inlet
    primal_dirichlet_value = 1
  []
  [u_out]
    type = ${ADBC}ConservativeAdvectionBC
    boundary = 'right'
    variable = u
    velocity_mat_prop = '${ADBC}velocity'
  []
[]

[Materials]
  [vel]
    type = VectorFromComponentVariablesMaterial
    vector_prop_name = 'velocity'
    u = 1
    v = 0
  []
  [advel]
    type = ADVectorFromComponentVariablesMaterial
    vector_prop_name = 'ADvelocity'
    u = 1
    v = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
