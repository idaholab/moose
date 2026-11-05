# AD residuals of Nedelec (H(curl)) and Raviart-Thomas (H(div)) variables on a displaced mesh with
# nonlinear second order displacements. The Piola maps of their shape function values, curls and
# divergences depend on the displacements, so the Jacobian blocks with respect to disp_x and disp_y
# are only exact when the AD shape data carries the derivatives of the element map. The input
# follows jxw_grad_test_dep_on_displacements/not-handling-jxw.i.

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
  [q]
    family = RAVIART_THOMAS
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
  # value and curl of the Nedelec variable and its test functions
  [e_wave]
    type = ADVectorFEWave
    variable = e
    x_forcing_func = 1
    y_forcing_func = 2
    use_displaced_mesh = true
  []
  # divergence of the Raviart-Thomas variable and its test functions
  [q_div]
    type = ADVectorDivDiv
    variable = q
    use_displaced_mesh = true
  []
  # value of the Raviart-Thomas variable and its test functions, with a forcing term
  [q_reaction]
    type = ADVectorFEWave
    variable = q
    x_forcing_func = -1
    y_forcing_func = 3
    use_displaced_mesh = true
  []
[]

[BCs]
  # face values of both variables and their test functions
  [e_robin]
    type = ADVectorRobinBC
    variable = e
    boundary = 'left right top bottom'
    use_displaced_mesh = true
  []
  [q_robin]
    type = ADVectorRobinBC
    variable = q
    boundary = 'left right top bottom'
    use_displaced_mesh = true
  []
  # BCs cannot be preset due to the Jacobian test. The position dependent displacements keep the
  # vertices of every edge apart in both coordinates: libMesh orients the Nedelec and
  # Raviart-Thomas shape functions by comparing vertex coordinates, so vertices with equal x
  # would let the finite difference perturbation flip an orientation and the Jacobian test fail.
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
  [e]
    type = VectorConstantIC
    variable = e
    x_value = 0.3
    y_value = 0.7
  []
  [q]
    type = VectorConstantIC
    variable = q
    x_value = 0.5
    y_value = -0.4
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
