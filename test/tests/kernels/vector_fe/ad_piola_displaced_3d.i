# 3D counterpart of ad_piola_displaced.i: AD residuals of a Nedelec (H(curl)) variable on a
# displaced TET10 mesh with nonlinear second order displacements, on elements and boundary faces.
# The Piola maps of its shape function values and curls depend on the displacements, so the
# Jacobian blocks with respect to the displacements are only exact when the AD shape data carries
# the derivatives of the element map. HEX27 exceeds the AD derivative storage of
# MOOSE_AD_MAX_DOFS_PER_ELEM = 64 with three second order displacements, so tetrahedra are used.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 1
  ny = 1
  nz = 1
  elem_type = TET10
[]

[Variables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [disp_z]
    order = SECOND
  []
  [e]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [disp_x]
    type = Diffusion
    variable = disp_x
  []
  [disp_y]
    type = Diffusion
    variable = disp_y
  []
  [disp_z]
    type = Diffusion
    variable = disp_z
  []
  # value and curl of the Nedelec variable and its test functions
  [e_wave]
    type = ADVectorFEWave
    variable = e
    x_forcing_func = 1
    y_forcing_func = 2
    z_forcing_func = -1
    use_displaced_mesh = true
  []
[]

[BCs]
  # face values of the variable and its test functions
  [e_robin]
    type = ADVectorRobinBC
    variable = e
    boundary = 'left right top bottom front back'
    use_displaced_mesh = true
  []
  # BCs cannot be preset due to the Jacobian test. See ad_piola_displaced.i for why the
  # displacements depend on position.
  [disp_x_left]
    type = FunctionDirichletBC
    preset = false
    function = '0.05 * y + 0.021 * z'
    boundary = 'left'
    variable = disp_x
  []
  [disp_x_right]
    type = FunctionDirichletBC
    preset = false
    function = '0.1 + 0.031 * y * y + 0.017 * z'
    boundary = 'right'
    variable = disp_x
  []
  [disp_y_bottom]
    type = FunctionDirichletBC
    preset = false
    function = '0.043 * x + 0.029 * z'
    boundary = 'bottom'
    variable = disp_y
  []
  [disp_y_top]
    type = FunctionDirichletBC
    preset = false
    function = '0.1 + 0.023 * x * x + 0.013 * z'
    boundary = 'top'
    variable = disp_y
  []
  [disp_z_back]
    type = FunctionDirichletBC
    preset = false
    function = '0.033 * x + 0.019 * y'
    boundary = 'back'
    variable = disp_z
  []
  [disp_z_front]
    type = FunctionDirichletBC
    preset = false
    function = '0.1 + 0.027 * x * y + 0.011 * x + 0.007 * y'
    boundary = 'front'
    variable = disp_z
  []
[]

[ICs]
  [disp_x]
    type = RandomIC
    variable = disp_x
    min = 0.01
    max = 0.09
  []
  [disp_y]
    type = RandomIC
    variable = disp_y
    min = 0.01
    max = 0.09
  []
  [disp_z]
    type = RandomIC
    variable = disp_z
    min = 0.01
    max = 0.09
  []
  [e]
    type = VectorConstantIC
    variable = e
    x_value = 0.3
    y_value = 0.7
    z_value = 0.2
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
