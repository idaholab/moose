# Test for check Jacobian correctness of bond-based peridynamic heat conduction formulation
[Mesh]
  type = GeneratedMeshPD
  dim = 2
  nx = 4
  horizon_number = 3
[]

[Variables]
  [./temp]
  [../]
[]

[AuxVariables]
  [./bond_status]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 1
  [../]
[]

[Kernels]
  [./HeatConduction]
    type = HeatConductionBPD
    variable = temp
  [../]
[]

[Materials]
  [./thermal_mat]
    type = ThermalConstantHorizonBPD
    temperature = temp
    thermal_conductivity = 1.0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -snes_type'
    petsc_options_value = 'bcgs bjacobi test'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
[]
