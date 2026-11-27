h = 0.5
E = 1
nu = 0.3
rho = 1
tilt = 45
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmax = 1
    ymax = 1
  []
  [tilt]
    type = TransformGenerator
    input = gen
    transform = ROTATE
    vector_value = '0 ${tilt} 0'
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
[BCs]
  [fix_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [fix_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = left
    value = 0
  []
  [fix_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = left
    value = 0
  []
  [fix_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = left
    value = 0
  []
  [fix_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = left
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
    eta = 0.1
    alpha = -0.1
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
    eta = 0.1
    alpha = -0.1
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
    eta = 0.1
    alpha = -0.1
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
    eta = 0.1
    alpha = -0.1
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
    eta = 0.1
    alpha = -0.1
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
[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 1
  [TimeIntegrator]
    type = NewmarkBeta
    beta = 0.25
    gamma = 0.5
  []
[]
