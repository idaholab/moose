[GlobalParams]
  order = SECOND
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = cyl2D.e
[]

[MeshModifiers]
  [./bottom]
    type = BoundingBoxNodeSet
    new_boundary = 10
    bottom_left = '-1 -1 0'
    top_right = '4 0.01 0'
  [../]
  [./left]
    type = BoundingBoxNodeSet
    new_boundary = 11
    bottom_left = '-1 -1 0'
    top_right = '0.01 4 0'
  [../]
[]

[Functions]
  [./temp]
    type = PiecewiseLinear
    x = '0   1'
    y = '0  25'
    # y = '0 250'
  [../]
[]

[Variables]
  [./temp]
    initial_condition = 0
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        add_variables = true
        strain = SMALL
        eigenstrain_names = thermal_expansion
      [../]
    [../]
  [../]
[]

[AuxVariables]
  [./gap_conductance]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]


[Kernels]
  [./heat_conduction]
    type = HeatConduction
    variable = temp
  [../]
[]


[AuxKernels]
  [./gap_cond]
    type = MaterialRealAux
    property = gap_conductance
    variable = gap_conductance
    boundary = 2
  [../]
[]

[Materials]
  [./heat1]
    type = HeatConductionMaterial
    block = '1'
    specific_heat = 1.0
    thermal_conductivity = 10.0
  [../]
  [./heat2]
    type = HeatConductionMaterial
    block = '2'
    specific_heat = 1.0
    thermal_conductivity = 1.0
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.33
  [../]
  [./thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    eigenstrain_name = thermal_expansion
    thermal_expansion_coeff = 0.01
    stress_free_temperature = 0
    temperature = temp
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    master = 3
    slave = 2
    gap_conductivity = 0.005
    quadrature = true
    gap_geometry_type = PLATE
  [../]
[]

[BCs]
  [./mid]
    type = FunctionPresetBC
    boundary = 1
    variable = temp
    function = temp
  [../]
  [./temp_far_right]
    type = PresetBC
    boundary = 4
    variable = temp
    value = 0
  [../]
  [./bottom]
    type = PresetBC
    boundary = 10
    variable = disp_y
    value = 0
  [../]
  [./left]
    type = PresetBC
    boundary = 11
    variable = disp_x
    value = 0
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
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

  dt = 0.1
  dtmin = 0.01
  end_time = 1

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-7

  # [./Quadrature]
  #   order = fifth
  #   side_order = seventh
  # [../]
[]

[Outputs]
  exodus = true
  [./Console]
    type = Console
  [../]
[]

[Postprocessors]
  [./temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
  [../]

  [./temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
  [../]

  [./flux_left]
    type = SideFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
  [../]

  [./flux_right]
    type = SideFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
  [../]
[]
