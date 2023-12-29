[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  use_displaced_mesh = false
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]
[]

[Functions]
  [./top_displacement]
    type = ParsedFunction
    expression = t
  [../]
[]

[BCs]
  [./bottom_x]
    type = DirichletBC
    variable = 'disp_x'
    boundary = bottom
    value = 0
  [../]
  [./bottom_y]
    type = DirichletBC
    variable = 'disp_y'
    boundary = bottom
    value = 0
  [../]
  [./bottom_z]
    type = DirichletBC
    variable = 'disp_z'
    boundary = bottom
    value = 0
  [../]

  [./top_x]
    type = DirichletBC
    variable = 'disp_x'
    boundary = top
    value = 0
  [../]
  [./top_y]
    type = FunctionDirichletBC
    variable = 'disp_y'
    boundary = top
    function = top_displacement
  [../]
  [./top_z]
    type = DirichletBC
    variable = 'disp_z'
    boundary = top
    value = 0
  [../]
[]

[Kernels]
  [./x]
    type = ADStressDivergenceTensors
    variable = disp_x
    component = 0
  [../]
  [./y]
    type = ADStressDivergenceTensors
    variable = disp_y
    component = 1
  [../]
  [./z]
    type = ADStressDivergenceTensors
    variable = disp_z
    component = 2
  [../]
[]

[Materials]
  [./rubber_elasticity]
    type = ComputeIsotropicElasticityTensor
    # lambda = 1.2e7
    # shear_modulus = 1.2e7
    youngs_modulus = 1
    poissons_ratio = 0.45 # the closer this gets to 0.5 the worse the problem becomes
  [../]
[]

[Materials]
  [./strain]
    type = ADComputeGreenLagrangeStrain
  [../]
  [./stress]
    type = ADComputeLinearElasticStress
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.05
  dtmin = 0.05
  nl_abs_tol = 1e-10
  num_steps = 500
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  exodus = true
  print_linear_residuals = false
[]
