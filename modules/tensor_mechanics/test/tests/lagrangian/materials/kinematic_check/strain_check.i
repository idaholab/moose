[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 3
  ny = 3
  nz = 3
  elem_type = HEX8
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  large_kinematics = true
[]

[Modules/TensorMechanics/Master]
  [all]
    new_system = true
    formulation = TOTAL
    strain = FINITE
    add_variables = true
    generate_output = 'cauchy_stress_xx cauchy_stress_yy cauchy_stress_zz cauchy_stress_xy'
  []
[]

[Functions]
  [tdisp]
    type = ParsedFunction
    expression = '0.5 * t'
  []
  [tdisp_quer]
    type = ParsedFunction
    expression = '0.5 * y * t'
  []
[]

[BCs]
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [bottom_x]
    type = DirichletBC
    variable = disp_x
    boundary = bottom
    value = 0
  []
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  []
  [front_z]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0
  []
  [back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [tdisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = tdisp
  []
[]

[Materials]
  [elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 30
    poissons_ratio = 0.4
  []
  [stress]
    type = ComputeLagrangianWrappedStress
  []
  [stress_base]
    type = ComputeLinearElasticStress
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1
  dt = 0.25
[]
