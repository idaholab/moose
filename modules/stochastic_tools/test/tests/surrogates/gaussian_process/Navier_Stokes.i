param1 = 1.0
param2 = 1.0
param3 = 1.0
param4 = 1.0
rho1 = 1.0
mu1 = 1.0
param1_mod = ${fparse exp(param1)}
param2_mod = ${fparse exp(param2)}
param3_mod = ${fparse exp(param3)}
param4_mod = ${fparse exp(param4)}
rho1_mod = ${fparse exp(rho1)}
mu1_mod = ${fparse exp(mu1)}
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1.0
    ymin = 0
    ymax = 1.0
    nx = 40
    ny = 40
    # elem_type = QUAD9
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
  second_order = true
[]
[AuxVariables]
  [vel_x]
    order = SECOND
  []
  [vel_y]
    order = SECOND
  []
[]
[AuxKernels]
  [vel_x]
    type = VectorVariableComponentAux
    variable = vel_x
    vector_variable = velocity
    component = 'x'
  []
  [vel_y]
    type = VectorVariableComponentAux
    variable = vel_y
    vector_variable = velocity
    component = 'y'
  []
[]
[Variables]
  [./velocity]
    order = SECOND
    family = LAGRANGE_VEC
  [../]
  [./p]
  [../]
[]
[Functions]
    [res_vel]
        type = ParsedFunction
        expression = 'sqrt(a^2 + b^2)'
        symbol_names = 'a b'
        symbol_values = 'vel_x vel_y'
    []
[]
[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]
  [./momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  [../]
  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]
  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = true
  [../]
[]
[BCs]
  [./lid]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'top'
    values = '${param1_mod} 0.0 0.0'
  [../]
  [./lid1]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'bottom'
    values = '${param2_mod} 0.0 0.0'
  [../]
  [./lid2]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'left'
    values = '0.0 ${param3_mod} 0.0'
  [../]
  [./lid3]
    type = VectorDirichletBC
    variable = velocity
    boundary = 'right'
    values = '0.0 ${param4_mod} 0.0'
  [../]
  [./pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0.0
  [../]
[]
[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho1_mod} ${mu1_mod}'
  [../]
  [ins_mat]
    type = INSADMaterial
    velocity = velocity
    pressure = p
  []
[]
[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]
[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  nl_rel_tol = 1e-12
  nl_max_its = 100
  l_max_its = 20
  automatic_scaling = true
[]
[Postprocessors]
  [./vel_x]
    type = PointValue
    point = '0.5 0.5 0.0'
    variable = vel_x
  [../]
  [./vel_y]
    type = PointValue
    point = '0.5 0.5 0.0'
    variable = vel_y
  [../]
  [resultant_velocity]
    type = FunctionValuePostprocessor
    function = 'res_vel'
  []  
[]
[Outputs]
  exodus = false
  perf_graph = false
  csv = true
[]