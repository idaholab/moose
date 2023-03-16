[Mesh]
  inactive = 'refine'
  # U-shaped domains to have internal boundaries in
  # a variety of directions
  [cmg]
     type = CartesianMeshGenerator
     dim = 2
     dx = '1 1 1'
     dy = '3 1'
     ix = '4 5 3'
     iy = '12 4'
     subdomain_id = '1 2 1
                     1 1 1'
  []
  [internal_boundary_dir1]
     type = SideSetsBetweenSubdomainsGenerator
     input = cmg
     primary_block = 1
     paired_block = 2
     new_boundary = 'inside_1'
  []
  [internal_boundary_dir2]
     type = SideSetsBetweenSubdomainsGenerator
     input = internal_boundary_dir1
     primary_block = 2
     paired_block = 1
     new_boundary = 'inside_2'
  []
  [refine]
    type = RefineBlockGenerator
    input = internal_boundary_dir2
    block = '1 2'
    refinement = '2 1'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    block = 1
  []
[]

[AuxVariables]
  [v1]
    type = MooseVariableFVReal
    block = 1
    [InitialCondition]
      type = FunctionIC
      function = 'x + y'
    []
  []
  [v2]
    type = MooseVariableFVReal
    block = 2
    [InitialCondition]
      type = FunctionIC
      function = '2*x*x - y'
    []
  []
[]

[Functions]
  [f1]
    type = ParsedFunction
    expression = 'exp(x - y)'
  []
[]

[Materials]
  [m1]
    type = ADGenericFunctorMaterial
    prop_names = 'm1'
    prop_values = 'f1'
  []
  [m2]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'm2'
    subdomain_to_prop_value = '1 12
                               2 4'
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
    coeff = '1'
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = 3
    value = 0
  []

  [right]
    type = FVDirichletBC
    variable = u
    boundary = 1
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Postprocessors]
  # Mesh external boundaries integration
  [ext_u]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'left top right'
    functor = u
    restrict_to_functors_domain = true
  []
  [ext_v1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'left right'
    functor = v1
  []
  [ext_v2]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'top'
    functor = v2
    restrict_to_functors_domain = true
  []
  [ext_f1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'left top right'
    functor = f1
    prefactor = f1
  []
  [ext_m1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'left top right'
    functor = m1
    restrict_to_functors_domain = true
  []
  [ext_m2]
    type = ADSideIntegralFunctorPostprocessor
    boundary = 'left top right'
    functor = m2
    restrict_to_functors_domain = true
  []

  # Internal to the mesh, but a side to the variables
  # With orientation of normal 1->2
  [int_s1_u]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_1
    functor = u
  []
  [int_s1_v1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_1
    functor = v1
  []
  [int_s1_f1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_1
    functor = f1
  []
  [int_s1_m1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_1
    functor = m1
  []
  [int_s1_m2]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_1
    functor = m2
  []
  # With orientation of normal 2->1
  [int_s2_v2]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_2
    functor = v2
  []
  [int_s2_f1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_2
    functor = f1
  []
  [int_s2_m1]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_2
    functor = m1
  []
  [int_s2_m2]
    type = ADSideIntegralFunctorPostprocessor
    boundary = inside_2
    functor = m2
  []
[]

[Outputs]
  csv = true
  exodus = true
[]

[Problem]
  kernel_coverage_check = false
[]
