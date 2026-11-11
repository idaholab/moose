h = 0.5
skew = 0
tilt = 0
eta = 0
alpha = 0
E = 1
nu = 0
rho = 1
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
    xmax = 1
    ymax = 1
  []
  [skew]
    type = ParsedNodeTransformGenerator
    input = gen
    x_function = 'x + ${skew}*y'
  []
  [tilt]
    type = TransformGenerator
    input = skew
    transform = ROTATE
    vector_value = '0 ${tilt} 0'
  []
  [n0]
    type = ExtraNodesetGenerator
    input = tilt
    nodes = '0'
    new_boundary = n0
  []
  [rest]
    type = ExtraNodesetGenerator
    input = n0
    nodes = '1 2 3'
    new_boundary = rest
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
[AuxVariables]
  [F_x]
  []
  [F_y]
  []
  [F_z]
  []
  [M_x]
  []
  [M_y]
  []
[]
[Functions]
  [ramp]
    type = ParsedFunction
    expression = '0.25*t*t'
  []
  [ramp2]
    type = ParsedFunction
    expression = '0.5*t*t'
  []
[]
[BCs]
  [dx_n0]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'n0'
    function = ramp
  []
  [dx_rest]
    type = DirichletBC
    variable = disp_x
    boundary = 'rest'
    value = 0
  []
  [dy]
    type = DirichletBC
    variable = disp_y
    boundary = 'n0 rest'
    value = 0
  []
  [dz]
    type = DirichletBC
    variable = disp_z
    boundary = 'n0 rest'
    value = 0
  []
  [rx_n0]
    type = FunctionDirichletBC
    variable = rot_x
    boundary = 'n0'
    function = ramp
  []
  [rx_rest]
    type = DirichletBC
    variable = rot_x
    boundary = 'rest'
    value = 0
  []
  [ry_n0]
    type = FunctionDirichletBC
    variable = rot_y
    boundary = 'n0'
    function = ramp2
  []
  [ry_rest]
    type = DirichletBC
    variable = rot_y
    boundary = 'rest'
    value = 0
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
    eta = ${eta}
    alpha = ${alpha}
    save_in = F_x
  []
  [i1]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 1
    variable = disp_y
    thickness = ${h}
    eta = ${eta}
    alpha = ${alpha}
    save_in = F_y
  []
  [i2]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 2
    variable = disp_z
    thickness = ${h}
    eta = ${eta}
    alpha = ${alpha}
    save_in = F_z
  []
  [i3]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 3
    variable = rot_x
    thickness = ${h}
    eta = ${eta}
    alpha = ${alpha}
    save_in = M_x
  []
  [i4]
    type = ADInertialForceShell
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y'
    component = 4
    variable = rot_y
    thickness = ${h}
    eta = ${eta}
    alpha = ${alpha}
    save_in = M_y
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
[VectorPostprocessors]
  [nodes]
    type = NodalValueSampler
    variable = 'F_x F_y M_x M_y'
    boundary = 'n0 rest'
    sort_by = id
  []
[]
[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1
  num_steps = 1
  nl_abs_tol = 1e-12
  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]
[Outputs]
  csv = true
[]
