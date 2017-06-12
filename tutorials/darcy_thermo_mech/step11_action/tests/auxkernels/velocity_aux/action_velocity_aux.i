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

[DarcyVelocity]
  darcy_pressure = pressure
  execute_on = TIMESTEP_END
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
  hide = 'velocity_y velocity_z'
[]
