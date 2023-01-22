[Functions]
  [src_func]
    type = ParsedFunction
    expression = "1000*sin(f*t)"
    symbol_names = 'f'
    symbol_values = '20'
  []
[]

[Mesh]
  [msh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    xmin = -0.5
    xmax = 0.5
    ny = 100
    ymin = -0.5
    ymax = 0.5
  []
  [source_domain]
    type = ParsedSubdomainMeshGenerator
    input = msh
    combinatorial_geometry = '(x<0.1 & x>-0.1) & (y<0.1 & y>-0.1)'
    block_id=1
  []
[]

[Variables]
  [T]
    initial_condition = 300
  []
[]

[Kernels]
  [diffusion]
    type = MatDiffusion
    variable = T
    diffusivity = diff_coeff
  []
  [source]
    type = BodyForce
    variable = T
    function = src_func
    block = 1
  []
  [time_deriv]
    type = TimeDerivative
    variable = T
  []
[]

[Materials]
  [diff_coeff]
    type = ParsedMaterial
    property_name = diff_coeff
    coupled_variables = 'T'
    constant_names = 'C'
    constant_expressions = 0.02
    expression = 'C * pow(300/T, 2)'
  []
[]

[BCs]
  [neumann_all]
    type = NeumannBC
    variable = T
    boundary = 'left right top bottom'
    value = 0
  []
[]

[Executioner]
  type = Transient
  num_steps = 100
  dt = 0.01
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  nl_rel_tol = 1e-6
  l_abs_tol = 1e-6
  timestep_tolerance = 1e-6
[]

[Postprocessors]
  [max]
    type = NodalExtremeValue
    variable = T
  []
  [min]
    type = NodalExtremeValue
    variable = T
    value_type = min
  []
  [time_max]
    type = TimeExtremeValue
    postprocessor = max
  []
  [time_min]
    type = TimeExtremeValue
    postprocessor = min
    value_type = min
  []
[]
