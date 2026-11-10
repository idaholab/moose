# Spring and dashpot in parallel on one element (a Kelvin-Voigt element) through base_name: the
# spring deformation material, the law and the kernels of the dashpot carry the prefix 'dashpot'.
# Stiffness 50 along X, damping 50, a constant load of 5 at the free node from t = 0: with the
# implicit Euler integration and dt = 0.5 the displacement follows u_n+1 = (u_n + dt F / c) /
# (1 + dt k / c), 0.033333, 0.055556, 0.070370, 0.080247, toward F / k = 0.1.
[Mesh]
  [spring]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0'
    element_connectivity = '0 1'
    elem_type = EDGE2
  []
  [fixed_end]
    type = ExtraNodesetGenerator
    input = spring
    new_boundary = fixed_end
    nodes = '0'
  []
  [free_end]
    type = ExtraNodesetGenerator
    input = fixed_end
    new_boundary = free_end
    nodes = '1'
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
[]

[Materials]
  [spring_deformation]
    type = ComputeSpringDeformation
    displacements = 'disp_x disp_y disp_z'
  []
  [spring]
    type = LinearSpring
    stiffness = '50 50 50'
  []
  [dashpot_deformation]
    type = ComputeSpringDeformation
    displacements = 'disp_x disp_y disp_z'
    compute_rates = true
    base_name = dashpot
  []
  [dashpot]
    type = ViscousSpring
    damping = '50 0 0'
    base_name = dashpot
  []
[]

[Kernels]
  [spring_disp_x]
    type = StressDivergenceSpring
    variable = disp_x
    component = 0
    displacements = 'disp_x disp_y disp_z'
  []
  [dashpot_disp_x]
    type = StressDivergenceSpring
    variable = disp_x
    component = 0
    displacements = 'disp_x disp_y disp_z'
    base_name = dashpot
  []
  [spring_disp_y]
    type = StressDivergenceSpring
    variable = disp_y
    component = 1
    displacements = 'disp_x disp_y disp_z'
  []
  [dashpot_disp_y]
    type = StressDivergenceSpring
    variable = disp_y
    component = 1
    displacements = 'disp_x disp_y disp_z'
    base_name = dashpot
  []
  [spring_disp_z]
    type = StressDivergenceSpring
    variable = disp_z
    component = 2
    displacements = 'disp_x disp_y disp_z'
  []
  [dashpot_disp_z]
    type = StressDivergenceSpring
    variable = disp_z
    component = 2
    displacements = 'disp_x disp_y disp_z'
    base_name = dashpot
  []
[]

[BCs]
  [fixed_x]
    type = DirichletBC
    variable = disp_x
    boundary = fixed_end
    value = 0
  []
  [fixed_y]
    type = DirichletBC
    variable = disp_y
    boundary = fixed_end
    value = 0
  []
  [fixed_z]
    type = DirichletBC
    variable = disp_z
    boundary = fixed_end
    value = 0
  []
[]

[NodalKernels]
  [load]
    type = ConstantRate
    variable = disp_x
    boundary = free_end
    rate = 5
  []
[]

[AuxVariables]
  [spring_force]
    order = CONSTANT
    family = MONOMIAL
  []
  [dashpot_force]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [spring_force]
    type = MaterialRealVectorValueAux
    variable = spring_force
    property = spring_force
    component = 0
  []
  [dashpot_force]
    type = MaterialRealVectorValueAux
    variable = dashpot_force
    property = dashpot_spring_force
    component = 0
  []
[]

[Postprocessors]
  [disp_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 1
  []
  [spring_force]
    type = ElementalVariableValue
    variable = spring_force
    elementid = 0
  []
  [dashpot_force]
    type = ElementalVariableValue
    variable = dashpot_force
    elementid = 0
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  dt = 0.5
  end_time = 2
[]

[Outputs]
  csv = true
[]
