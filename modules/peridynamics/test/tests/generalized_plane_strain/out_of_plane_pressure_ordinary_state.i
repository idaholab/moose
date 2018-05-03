# Tests for application of out-of-plane pressure in generalized plane strain.

[GlobalParams]
  displacements = 'disp_x disp_y'
  scalar_out_of_plane_strain = scalar_strain_zz
[]

[Mesh]
  type = GeneratedMeshPD
  dim = 2
  nx = 4
  horizon_number = 3
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  [./scalar_strain_zz]
    order = FIRST
    family = SCALAR
  [../]
[]

[AuxVariables]
  [./stress_zz]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]

[]

[Modules]
  [./Peridynamics]
    [./Mechanics]
      formulation = OrdinaryState
    [../]
    [./GeneralizedPlaneStrain]
      [./gps]
        formulation = OrdinaryState
        out_of_plane_stress_variable = stress_zz
        out_of_plane_pressure = pressure_function
        factor = 1e5
      [../]
    [../]
  [../]
[]

[AuxKernels]
  [./stress_zz]
    type = NodalStressStrainPD
    rank_two_tensor = stress
    variable = stress_zz

    poissons_ratio = 0.3
    youngs_modulus = 1e6
    quantity_type = Component
    index_i = 2
    index_j = 2
  [../]
[]

[Postprocessors]
  [./react_z]
    type = NodalVariableIntegralPD
    variable = stress_zz
  [../]
[]

[Functions]
  [./pressure_function]
    type = PiecewiseLinear
    x = '0  1'
    y = '0  1'
  [../]
[]

[BCs]
  [./leftx]
    type = PresetBC
    boundary = 0
    variable = disp_x
    value = 0.0
  [../]
  [./bottomy]
    type = PresetBC
    boundary = 2
    variable = disp_y
    value = 0.0
  [../]
[]

[Materials]
  [./elastic_tensor]
    type = SmallStrainConstantHorizonOSPD
    poissons_ratio = 0.3
    youngs_modulus = 1e6
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

  solve_type = PJFNK
  line_search = none

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  file_base = out_of_plane_pressure_ordinary_state
[]
