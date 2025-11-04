[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    dy = '0.5 0.5'
    ix = '5 5'
    iy = '5 5'
    subdomain_id = '0 1
                    2 3'
  []
  [add_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    new_boundary = 'middle middle'
    primary_block = '0 2'
    paired_block = '1 3'
  []
[]

[Variables]
  [u_fe]
    block = '0 2 3'
  []
[]

[AuxVariables]
  [u_fv]
    type = MooseVariableFVReal
    block = '0 3'
    initial_condition = 1
  []
[]

[Kernels]
  [u_fe_diff]
    type = ADDiffusion
    variable = u_fe
  []
[]

[BCs]
  [u_fe_left]
    type = ADDirichletBC
    boundary = left
    variable = u_fe
    value = 0
  []
  [u_fe_middle]
    type = ADConvectiveHeatFluxBC
    boundary = middle
    variable = u_fe
    T_infinity_functor = u_fv
    heat_transfer_coefficient_functor = 1.0
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  kernel_coverage_check = false
[]
