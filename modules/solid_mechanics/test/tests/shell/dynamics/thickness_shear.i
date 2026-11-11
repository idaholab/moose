L = 1
h = 0.1
E = 2e11
nu = 0.25
rho = 7850
moment_rate = 2e9
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmax = ${L}
    ymax = ${L}
  []
[]
[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [rot_x]
  []
  [rot_y]
  []
[]
[Functions]
  [moment]
    type = ParsedFunction
    expression = '${moment_rate}*t'
  []
  [exact]
    type = ParsedFunction
    symbol_names = 'w'
    symbol_values = '${fparse sqrt(12*E/(2*(1+nu))/(rho*h*h))}'
    expression = 't - sin(w*t)/w'
  []
[]
[BCs]
  [fix_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0
  []
  [fix_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'left right'
    value = 0
  []
  [fix_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'left right'
    value = 0
  []
  [fix_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = 'left right'
    value = 0
  []
[]
[NodalKernels]
  [moment]
    type = UserForcingFunctorNodalKernel
    variable = rot_x
    boundary = 'left right'
    functor = moment
  []
[]
[Kernels]
  [i0]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 0
    variable = disp_x
    thickness = ${h}
  []
  [s0]
    type = ADStressDivergenceShell
    component = 0
    variable = disp_x
    through_thickness_order = SECOND
  []
  [i1]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 1
    variable = disp_y
    thickness = ${h}
  []
  [s1]
    type = ADStressDivergenceShell
    component = 1
    variable = disp_y
    through_thickness_order = SECOND
  []
  [i2]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 2
    variable = disp_z
    thickness = ${h}
  []
  [s2]
    type = ADStressDivergenceShell
    component = 2
    variable = disp_z
    through_thickness_order = SECOND
  []
  [i3]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 3
    variable = rot_x
    thickness = ${h}
  []
  [s3]
    type = ADStressDivergenceShell
    component = 3
    variable = rot_x
    through_thickness_order = SECOND
  []
  [i4]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 4
    variable = rot_y
    thickness = ${h}
  []
  [s4]
    type = ADStressDivergenceShell
    component = 4
    variable = rot_y
    through_thickness_order = SECOND
  []
[]
[Materials]
  [elasticity]
    type = ADComputeIsotropicElasticityTensorShell
    youngs_modulus = ${E}
    poissons_ratio = ${nu}
    through_thickness_order = SECOND
  []
  [strain]
    type = ADComputeIncrementalShellStrain
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    thickness = ${h}
    through_thickness_order = SECOND
  []
  [stress]
    type = ADComputeShellStress
    through_thickness_order = SECOND
  []
  [density]
    type = GenericConstantMaterial
    prop_names = 'density'
    prop_values = '${rho}'
  []
[]
[Postprocessors]
  [rot_x]
    type = PointValue
    point = '0 0 0'
    variable = rot_x
  []
  [exact]
    type = FunctionValuePostprocessor
    function = exact
  []
  [difference]
    type = ParsedPostprocessor
    expression = 'abs(rot_x - exact)'
    pp_names = 'rot_x exact'
    outputs = none
  []
  [max_difference]
    type = TimeExtremeValue
    postprocessor = difference
    value_type = abs_max
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 5e-7
  num_steps = 240
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
  l_tol = 1e-12
  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]
[Outputs]
  csv = true
[]
