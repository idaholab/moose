# This test solves a 2D steady state heat equation
# The error is found by comparing to the analytical solution

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
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

[Functions]
  [./analytical_sol]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]
[]

[Kernels]
  [./heat_conduction]
    type = HeatConductionBPD
    variable = temp
  [../]
  [./heat_source]
    type = HeatSourceBPD
    variable = temp
    power_density = -4
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = temp
    boundary = 1003
    function = analytical_sol
  [../]
  [./bottom]
    type = FunctionDirichletBC
    variable = temp
    boundary = 1000
    function = analytical_sol
  [../]
  [./right]
    type = FunctionDirichletBC
    variable = temp
    boundary = 1001
    function = analytical_sol
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = temp
    boundary = 1002
    function = analytical_sol
  [../]
[]

[Materials]
  [./thermal_mat]
    type = ThermalConstantHorizonMaterialBPD
    temperature = temp
    thermal_conductivity = 1
  [../]
[]

[Postprocessors]
  [./nodal_error]
    type = NodalL2Error
    function = 'analytical_sol'
    variable = temp
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

  start_time = 0.0
  end_time = 1.0
[]

[Outputs]
  exodus = true
  file_base = 2D_steady_state_BPD
[]
