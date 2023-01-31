#
# Initial single block thermal input
# https://mooseframework.inl.gov/modules/heat_conduction/tutorials/introduction/therm_step01.html
#

[Mesh]
  [generated]
    type = CartesianMeshGenerator
    dim = 2
    ix = '5 5'
    iy = '10'
    dx = '1 0.5'
    dy = '1'
    subdomain_id = '0 1'
  []
  [rename_bdr]
    type = RenameBlockGenerator
    input = generated
    old_block = '0 1'
    new_block = 'a b'
  []
[]

[Variables]
  [v]
  []
[]

[Kernels]
  [heat_conduction]
    type = Diffusion
    variable = v
  []
  [time]
    type = TimeDerivative
    variable = v
  []
  [source]
    type = HeatSource
    value = 100
    variable = v
    block = 'b'
  []
[]

[Functions]
  [my_mat_prop]
    type = PiecewiseLinear
    x = '0.0 50.0 100.0'
    y = '50.0 50.0 0.01'
  []
[]

[Materials]
  [thermal_left]
    type = HeatConductionMaterial
    thermal_conductivity = 45.0
    block = 0
  []

  [thermal_right]
    type = HeatConductionMaterial
    thermal_conductivity = 0.1
    block = 1
  []

  # [parsed_mat]
  #   type = ParsedMaterial
  #   function = '0.1 * v'
  #   args = 'v'
  # []

  # [my_mat]
  #   type = GenericFunctionMaterial
  #   prop_names = 'thermal_conductivity'
  #   prop_values = my_mat_prop
  # []

[]

[BCs]
  [left_BC]
    type = DirichletBC
    boundary = left
    variable = v
    value = 10
  []
  [right_BC]
    type = NeumannBC
    boundary = right
    variable = v
    value = 100
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  end_time = 5
  dt = 1
[]

[Outputs]
  exodus = true
  [exo]
    type = Exodus
  []
[]
