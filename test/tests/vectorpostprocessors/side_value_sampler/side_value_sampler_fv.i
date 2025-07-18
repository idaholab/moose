[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.5 0.5'
    dy = '1'
    ix = '5 5'
    iy = '10'
    subdomain_id = '1 1'
  []
  # Limited to 1 side to avoid inconsistencies in parallel
  [internal_sideset]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y<0.51 & y>0.49 & x<0.11'
    new_sideset_name = 'center'
    input = 'mesh'
  []
  # this keeps numbering continuous so tests dont fail on different ids in CSV
  allow_renumbering = false
[]

[Problem]
  nl_sys_names = 'u_sys'
  linear_sys_names = 'v_sys'
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    solver_sys = u_sys
  []
  [v]
    type = MooseLinearVariableFVReal
    solver_sys = v_sys
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = 1.0
  []
[]

[LinearFVKernels]
  [diff]
    type = LinearFVDiffusion
    variable = v
  []
[]

[FVBCs]
  [all]
    type = FVFunctorDirichletBC
    variable = u
    boundary = 'top bottom left right'
    functor = linear_x
  []
[]

[LinearFVBCs]
  [all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = v
    boundary = 'top bottom left right'
    functor = linear_x
  []
[]

[Functions]
  [linear_x]
    type = ParsedFunction
    expression = '5*x'
  []
[]

[VectorPostprocessors]
  [side_sample]
    type = SideValueSampler
    variable = 'u v'
    boundary = top
    sort_by = x
  []
  [internal_sample]
    type = SideValueSampler
    variable = 'u v'
    boundary = center
    sort_by = 'id'
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  system_names = 'u_sys v_sys'
  l_tol = 1e-8
[]

[Outputs]
  execute_on = 'timestep_end'
  [vpp_csv]
    type = CSV
  []
[]
