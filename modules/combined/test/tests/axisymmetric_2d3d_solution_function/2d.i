[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  file = 2d.e
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [temp]
    initial_condition = 400
  []
[]

[AuxVariables]
  [hoop_stress]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Functions]
  [temp_inner_func]
    type = PiecewiseLinear
    xy_data = '0 400
               1 350'
  []
  [temp_outer_func]
    type = PiecewiseLinear
    xy_data = '0 400
               1 400'
  []
  [press_func]
    type = PiecewiseLinear
    xy_data = '0 15
               1 15'
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temp
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    volumetric_locking_correction = true
    add_variables = true
    incremental = true
    strain = FINITE
    eigenstrain_names = thermal_expansion
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress hydrostatic_stress'
    temperature = temp
  []
[]

[AuxKernels]
  [hoop_stress]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = hoop_stress
    scalar_type = HoopStress
    execute_on = timestep_end
  []
[]

[BCs]
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = '1'
    value = 0.0
  []

  [Pressure]
    [internal_pressure]
      boundary = '4'
      factor = 1.e6
      function = press_func
    []
  []

  [t_in]
    type = FunctionDirichletBC
    variable = temp
    boundary = '4'
    function = temp_inner_func
  []

  [t_out]
    type = FunctionDirichletBC
    variable = temp
    boundary = '2'
    function = temp_outer_func
  []
[]

[Constraints]
  [disp_y]
    type = EqualValueBoundaryConstraint
    variable = disp_y
    primary = '65'
    secondary = '3'
    penalty = 1e18
  []
[]

[Materials]
  [thermal1]
    type = HeatConductionMaterial
    block = '1'
    thermal_conductivity = 25.0
    specific_heat = 490.0
    temp = temp
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 193.05e9
    poissons_ratio = 0.3
  []

  [stress]
    type = ComputeFiniteStrainElasticStress
  []

  [thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 13e-6
    stress_free_temperature = 295.00
    temperature = temp
    eigenstrain_name = thermal_expansion
  []

  [density]
    type = Density
    block = '1'
    density = 8000.0
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = ' 201                hypre    boomeramg      4'
  line_search = 'none'
  l_max_its = 25
  nl_max_its = 20
  nl_rel_tol = 1e-9
  l_tol = 1e-2

  start_time = 0.0
  dt = 1
  end_time = 1
  dtmin = 1
[]

[Outputs]
  file_base = 2d_out
  exodus = true
  [console]
    type = Console
    max_rows = 25
  []
[]
