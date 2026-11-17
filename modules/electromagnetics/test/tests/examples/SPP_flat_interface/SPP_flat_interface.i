# Test for EMRobinBC in port and absorbing modes with SPP wave


[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    nx = 80
    ny = 160
    dim = 2
    xmin = 0
    xmax = 20
    ymin = -20
    ymax = 20
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 -20 0'
    top_right = '20 0 0'
    input = gen
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    block_id = 2
    bottom_left = '0 0 0'
    top_right = '20 20 0'
    input = block1
  []
  [interface]
    input = block2
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'primary_interface'
  []
[]

[Variables]
  [E_real_lower]
    order = FIRST
    family = LAGRANGE
  []
  [E_real_upper]
    order = FIRST
    family = LAGRANGE
  []
  [E_imag_lower]
    order = FIRST
    family = LAGRANGE
  []
  [E_imag_upper]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [inc_y]
    type = ParsedFunction
    expression = 'ky_p:=0.444288293816;ky_m:=2*0.444288293816;if(y>=0, exp(-ky_p*y), exp(ky_m*y))'
  []
  [top_x]
    type = ParsedFunction
    expression = 'kx:=sqrt(2)*0.444288293816;cos(kx*x)'
  []
  [upper_IC_func]
    type = ParsedFunction
    expression = 'kx:=sqrt(2)*0.444288293816;ky_p:=0.444288293816;if(y>=0, exp(-ky_p*y)*cos(kx*x), 0)'
  []
  [lower_IC_func]
    type = ParsedFunction
    expression = 'kx:=sqrt(2)*0.444288293816;ky_m:=2*0.444288293816;if(y<0, exp(ky_m*y)*cos(kx*x), 0)'
  []
[]

[Kernels]
  [diffusion_real_lower]
    type = Diffusion
    variable = E_real_lower
    block =1
  []
  [coeffField_real_lower]
    type = MatReaction
    reaction_rate = kSquared_lower
    variable = E_real_lower
    block =1
  []
  [diffusion_real_upper]
    type = Diffusion
    variable = E_real_upper
    block =2
  []
  [coeffField_real_upper]
    type = MatReaction
    reaction_rate = kSquared_upper
    variable = E_real_upper
    block =2
  []
  [diffusion_imag_lower]
    type = Diffusion
    variable = E_imag_lower
    block =1
  []
  [coeffField_imag_lower]
    type = MatReaction
    reaction_rate = kSquared_lower
    variable = E_imag_lower
    block =1
  []
  [diffusion_imag_upper]
    type = Diffusion
    variable = E_imag_upper
    block =2
  []
  [coeffField_imag_upper]
    type = MatReaction
    reaction_rate = kSquared_upper
    variable = E_imag_upper
    block =2
  []
[]

[InterfaceKernels]
  [interface_real]
    type = InterfaceDiffusion
    variable = E_real_lower
    neighbor_var = E_real_upper
    boundary = primary_interface
    D = 'k_lower'
    D_neighbor = 'k_upper'
  []
  [interface_imag]
    type = InterfaceDiffusion
    variable = E_imag_lower
    neighbor_var = E_imag_upper
    boundary = primary_interface
    D = 'k_lower'
    D_neighbor = 'k_upper'
  []
[]

[BCs]
   [left_real_upper]
    type = EMRobinBC
    component = real
    profile_func_real = inc_y
    variable = E_real_upper
    mode= port
    sign = positive
    coeff_real = -0.628318530718
    coeff_imag = 0.0
    boundary = left
    field_real = E_real_upper
    field_imaginary = E_imag_upper
  []
  [right_real_upper]
    type = EMRobinBC
    component = real
    variable = E_real_upper
    mode= absorbing
    sign = negative
    coeff_real = 0.628318530718
    coeff_imag = 0.0
    boundary = right
    field_real = E_real_upper
    field_imaginary = E_imag_upper
  []
  [left_real_lower]
    type = EMRobinBC
    component = real
    profile_func_real = inc_y
    variable = E_real_lower
    mode= port
    sign = positive
    coeff_real = -0.628318530718
    coeff_imag = 0.0
    boundary = left
    field_real = E_real_lower
    field_imaginary = E_imag_lower
  []
  [right_real_lower]
    type = EMRobinBC
    component = real
    variable = E_real_lower
    mode= absorbing
    sign = negative
    coeff_real = 0.628318530718
    coeff_imag = 0.0
    boundary = right
    field_real = E_real_lower
    field_imaginary = E_imag_lower
  []
  [left_imag_upper]
    type = EMRobinBC
    component = imaginary
    profile_func_real = inc_y
    variable = E_imag_upper
    mode= port
    sign = positive
    coeff_real = -0.628318530718
    coeff_imag = 0.0
    boundary = left
    field_real = E_real_upper
    field_imaginary = E_imag_upper
  []
  [right_imag_upper]
    type = EMRobinBC
    component = imaginary
    variable = E_imag_upper
    mode= absorbing
    sign = negative
    coeff_real = 0.628318530718
    coeff_imag = 0.0
    boundary = right
    field_real = E_real_upper
    field_imaginary = E_imag_upper
  []
  [left_imag_lower]
    type = EMRobinBC
    component = imaginary
    profile_func_real = inc_y
    variable = E_imag_lower
    mode= port
    sign = positive
    coeff_real = -0.628318530718
    coeff_imag = 0.0
    boundary = left
    field_real = E_real_lower
    field_imaginary = E_imag_lower
  []
  [right_imag_lower]
    type = EMRobinBC
    component = imaginary
    variable = E_imag_lower
    mode= absorbing
    sign = negative
    coeff_real = 0.628318530718
    coeff_imag = 0.0
    boundary = right
    field_real = E_real_lower
    field_imaginary = E_imag_lower
  []
  [middle_real]
    type = MatchedValueBC
    variable = E_real_upper
    boundary = 'primary_interface'
    v = E_real_lower
  []
  [middle_imag]
    type = MatchedValueBC
    variable = E_imag_upper
    boundary = 'primary_interface'
    v = E_imag_lower
  []
[]

[Materials]
  [kSquared_lower]
    type = ParsedMaterial
    property_name = kSquared_lower
    expression = '-2*0.444288293816^2'
    block = 1
    outputs = 'exodus'
  []
  [kSquared_upper]
    type = ParsedMaterial
    property_name = kSquared_upper
    expression = '0.444288293816^2'
    block = 2
    outputs = 'exodus'
  []
  [k_lower]
    type = ParsedMaterial
    property_name = k_lower
    expression = '1'
    block = 1
    outputs = 'exodus'
  []
  [k_upper]
    type = ParsedMaterial
    property_name = k_upper
    expression = '-2'
    block = 2
    outputs = 'exodus'
  []
[]

[VectorPostprocessors]
  [Ex_out]
    type = LineValueSampler
    start_point = '0 5 0'
    end_point = '20 5 0'
    num_points = 641
    sort_by = id
    variable = 'E_real_lower E_real_upper E_imag_lower E_imag_upper'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_max_its = 30
  l_max_its = 30
  l_tol = 1e-4
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = true
[]
