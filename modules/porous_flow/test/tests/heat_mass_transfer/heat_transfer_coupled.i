# Models fluid advecting down a single fracture
# The fluid carries heat which convects into the matrix

[Mesh]
  type = FileMesh
  file = '1_frac_in_2D_lower_dim.e'
[]

[Variables]
  [./t]
    block = 1
  [../]
  [./t_matrix]
  [../]
[]


[BCs]
  [./t_top]
    type = PresetBC
    value = 0
    variable = t
    boundary = 1
  [../]
  [./t_bottom]
    type = PresetBC
    value = 1
    variable = t
    boundary = 2
  [../]
[]

[Kernels]
  [./T_fracture_dot]
    type = TimeDerivative
    variable = t
    block = 1
  [../]
  [./T_fracture_diffusion]
    type = AnisotropicDiffusion
    variable = t
    block = 1
    tensor_coeff = '1 0 0  0 1 0  0 0 1'
  [../]

  [./T_matrix_dot]
    type = TimeDerivative
    variable = t_matrix
    block = '2 3'
  [../]
  [./T_matrix_diffusion]
    type = AnisotropicDiffusion
    variable = t_matrix
    block = '2 3'
    tensor_coeff = '1e-3 0 0  0 1e-3 0  0 0 1e-3'
  [../]


  [./heat_out_of_fracture]
    type = PorousFlowHeatMassTransfer
    block = 1
    variable = t
    v = t_matrix
    transfer_coefficient = 1e-4
  [../]


  [./fracture_into_matrix]
    type = PorousFlowHeatMassTransfer
    block = 1
    variable = t_matrix
    v = t
    transfer_coefficient = 1e-4
  [../]
[]

[Preconditioning]
  [./basic]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = 'gmres      asm      lu           NONZERO                   2             '
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  end_time = 100
  dt = 10

# controls for nonlinear iterations
  nl_max_its = 15
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-9
[]


[Outputs]
  exodus = true
[]
