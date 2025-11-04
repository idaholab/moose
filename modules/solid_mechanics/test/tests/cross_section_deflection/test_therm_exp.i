[GlobalParams]
  volumetric_locking_correction = true
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = one_duct.e
  []
[]

[Functions]
  [pressure]
    type = PiecewiseLinear
    x = '0 10'
    y = '0 0.05'
    scale_factor = 1
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[AuxVariables]
  [temp]
    initial_condition = 300
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = FINITE
    block = '1'
    eigenstrain_names = 'thermal_expansion'
  []
[]

[BCs]
  [fix_y]
    type = DirichletBC
    variable = 'disp_y'
    boundary = '16'
    value = 0.0
  []
  [fix_x]
    type = DirichletBC
    variable = 'disp_x'
    boundary = '16'
    value = 0.0
  []
  [fix_z]
    type = DirichletBC
    variable = 'disp_z'
    boundary = '16'
    value = 0.0
  []
  [Pressure]
    [hex1_pressure]
      boundary = '4'
      function = pressure
      factor = 80
    []
  []
[]

[VectorPostprocessors]
  [section_output]
    type = AverageSectionValueSampler
    axis_direction = '0 0 1'
    block = '1'
    variables = 'disp_x disp_y disp_z'
    reference_point = '0 0 0'
  []
[]

[Materials]
  [hex_elas_tens]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1e4
    poissons_ratio = 0.0
  []
  [hex_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1'
  []
  [thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    temperature = temp
    thermal_expansion_coeff = 1e-5
    stress_free_temperature = 0
    eigenstrain_name = 'thermal_expansion'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type '
  petsc_options_value = 'lu       '

  line_search = 'none'

  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-10

  l_max_its = 20
  dt = 0.5
  end_time = 0.5
[]

[Outputs]
  exodus = true
  csv = true
[]
