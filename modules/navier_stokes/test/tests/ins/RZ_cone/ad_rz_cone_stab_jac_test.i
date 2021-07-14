[GlobalParams]
  order = SECOND
  integrate_p_by_parts = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = 0
    xmax = 1.1
    ymin = -1.1
    ymax = 1.1
    elem_type = QUAD9
  []
  [./corner_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '0'
    input = gen
  [../]
[]

[Problem]
  coord_type = RZ
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = NEWTON
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1.1
[]

[Variables]
  [./velocity]
    family = LAGRANGE_VEC
  [../]
  [./p]
    order = FIRST
  [../]
[]


# Need to set a non-zero initial condition because we have a velocity norm in
# the denominator for the tau coefficient of the stabilization term
[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    variable = velocity
  []
[]

[Kernels]
  [./mass]
    type = INSADMass
    variable = p
  [../]
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
  []

  [momentum_time]
    type = INSADMomentumTimeDerivative
    variable = velocity
  []
  [momentum_advection]
    type = INSADMomentumAdvection
    variable = velocity
  []
  [./momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  [../]

  [./momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
  [../]
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  []
[]

[BCs]
  [inlet]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'bottom'
    function_x = 0
    function_y = 1
  [../]
  [wall]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'right'
    function_x = 0
    function_y = 0
  []
  [axis]
    type = ADVectorFunctionDirichletBC
    variable = velocity
    boundary = 'left'
    set_y_comp = false
    function_x = 0
  []
  [outlet]
    type = INSADMomentumNoBCBC
    variable = velocity
    pressure = p
    boundary = 'top'
  []
  # When the NoBCBC is applied on the outlet boundary then there is nothing
  # constraining the pressure. Thus we must pin the pressure somewhere to ensure
  # that the problem is not singular. If the below BC is not applied then
  # -pc_type svd -pc_svd_monitor reveals a singular value
  [p_corner]
    type = DirichletBC
    boundary = pinned_node
    value = 0
    variable = p
  []
[]

[Materials]
  [./const]
    type = ADGenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '1.1 1.1'
  [../]
  [ins_mat]
    type = INSADTauMaterial
    velocity = velocity
    pressure = p
  []
[]
