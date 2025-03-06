[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    dy = '0.5 0.5'
    ix = '5 5'
    iy = '5 5'
    subdomain_id = '0 1
                    0 1'
  []
  [add_sideset0]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    new_boundary = middle01
    primary_block = 0
    paired_block = 1
  []
  [add_sideset1]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_sideset0
    new_boundary = middle10
    primary_block = 1
    paired_block = 0
  []
[]

[Variables]
  [u_fe]
    block = 0
  []
  [u_fv]
    type = MooseVariableFVReal
    block = 1
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
    boundary = middle01
    variable = u_fe
    T_infinity_functor = u_fv
    heat_transfer_coefficient_functor = 1.0
  []
[]

[FVKernels]
  [u_fv_diff]
    type = FVDiffusion
    variable = u_fv
    coeff = 1.0
  []
[]

[FVBCs]
  [u_fv_right]
    type = FVDirichletBC
    boundary = right
    variable = u_fv
    value = 1.0
  []
  [u_fv_middle]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = middle10
    variable = u_fv
    T_bulk = u_fv
    T_solid = u_fe
    heat_transfer_coefficient = 1.0
    is_solid = false
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
