[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [./pressure]
  [../]
[]

[AuxVariables]
  [./velocity_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./velocity_x]
    type = DarcyVelocity
    variable = velocity_x
    component = x
    execute_on = timestep_end
    darcy_pressure = pressure
  [../]
[]

[Functions]
  [./pressure_ic_func]
    type = ParsedFunction
    value = 2000*x*y*x*y
  [../]
[]

[ICs]
  [./pressure_ic]
    type = FunctionIC
    variable = pressure
    function = pressure_ic_func
  [../]
[]

[Problem]
  type = FEProblem
  coord_type = RZ
  rz_coord_axis = X
  solve = false
[]

[Materials]
  [./pressure]
    type = GenericConstantMaterial
    prop_values = '0.8451e-9 7.98e-4'
    prop_names = 'permeability viscosity'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
