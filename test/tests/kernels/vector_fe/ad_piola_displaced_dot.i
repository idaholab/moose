# Time derivative of a coupled Nedelec variable on a displaced mesh, through adCoupledVectorDot().
# On the displaced mesh the coupled variable e enters the residual of f only through its time
# derivative, so the time derivative itself must request the AD vector shape data of e. Otherwise
# its Piola map carries no displacement derivatives and the Jacobian block of f with respect to
# the displacements is incomplete. The geometry follows ad_piola_displaced.i.

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Variables]
  [disp_x]
    order = SECOND
  []
  [disp_y]
    order = SECOND
  []
  [e]
    family = NEDELEC_ONE
    order = FIRST
  []
  # f is a different FE type than e, whose AD vector shape data is therefore requested only by
  # the time derivative of e
  [f]
    family = LAGRANGE_VEC
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
  # e and f on the undisplaced mesh, so that these do not request the AD shape data of e
  [e_wave]
    type = ADVectorFEWave
    variable = e
    x_forcing_func = 1
    y_forcing_func = 2
  []
  [f_diffusion]
    type = ADVectorDiffusion
    variable = f
  []
  [f_dot_e]
    type = ADVectorCoupledTimeDerivative
    variable = f
    v = e
    use_displaced_mesh = true
  []
[]

[BCs]
  # f needs boundary conditions: with diffusion alone its Jacobian block is singular
  [f]
    type = ADVectorFunctionDirichletBC
    variable = f
    boundary = 'left right top bottom'
    preset = false
  []
  # BCs cannot be preset due to the Jacobian test
  [disp_x_left]
    type = FunctionDirichletBC
    preset = false
    function = '0.05 * y'
    boundary = 'left'
    variable = disp_x
  []
  [disp_x_right]
    type = FunctionDirichletBC
    preset = false
    function = '0.1 + 0.03 * y * y'
    boundary = 'right'
    variable = disp_x
  []
  [disp_y_bottom]
    type = FunctionDirichletBC
    preset = false
    function = '0.04 * x'
    boundary = 'bottom'
    variable = disp_y
  []
  [disp_y_top]
    type = FunctionDirichletBC
    preset = false
    function = '0.1 + 0.02 * x * x'
    boundary = 'top'
    variable = disp_y
  []
[]

[ICs]
  # smooth displacements, which unlike RandomIC do not depend on the mesh distribution
  [disp_x]
    type = FunctionIC
    variable = disp_x
    function = '0.05 * x * y + 0.02 * y * y'
  []
  [disp_y]
    type = FunctionIC
    variable = disp_y
    function = '0.04 * x * x + 0.03 * x * y'
  []
  [e]
    type = VectorConstantIC
    variable = e
    x_value = 0.3
    y_value = 0.7
  []
  [f]
    type = VectorConstantIC
    variable = f
    x_value = -0.2
    y_value = 0.4
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 2
  dt = 0.1
[]
