[Mesh]
  file = cube.e
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./disp_z]
  [../]

  [./temp]
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]

[BCs]
  [./bottom_x]
    type = PresetBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./bottom_y]
    type = PresetBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./bottom_z]
    type = PresetBC
    variable = disp_z
    boundary = 1
    value = 0.0
  [../]

  [./bottom_temp]
    type = PresetBC
    variable = temp
    boundary = 1
    value = 10.0
  [../]
[]

[Materials]
  [./constant]
    type = LinearIsotropicMaterial
    block = 1

    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    temp = temp

    youngs_modulus = 1.0
    poissons_ratio = .3
    thermal_expansion = 1e-5
  [../]

  [./heat1]
    type = HeatConductionMaterial
    block = 1

    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./density]
    type = Density
    block = 1
    density = 1.0
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-14
  l_tol = 1e-3

  l_max_its = 100

  dt = 1.0
  end_time = 1.0
[]

[Outputs]
  file_base = thermo_mech_smp_out
  [./exodus]
    type = Exodus
    execute_on = 'initial timestep_end nonlinear'
    nonlinear_residual_dt_divisor = 100
  [../]
[]
