# Viscous spring (dashpot) along the global X axis under a constant load of 5 at its second node,
# damping coefficients (50, 0, 0): the node moves at the velocity 0.1 from the first step of the
# implicit Euler integration, so its displacement is 0.1 t.
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
  [deformation]
    type = ComputeSpringDeformation
    displacements = 'disp_x disp_y disp_z'
    compute_rates = true
  []
  [law]
    type = ViscousSpring
    damping = '50 0 0'
  []
[]

[Kernels]
  [spring_x]
    type = StressDivergenceSpring
    variable = disp_x
    component = 0
    displacements = 'disp_x disp_y disp_z'
  []
  [spring_y]
    type = StressDivergenceSpring
    variable = disp_y
    component = 1
    displacements = 'disp_x disp_y disp_z'
  []
  [spring_z]
    type = StressDivergenceSpring
    variable = disp_z
    component = 2
    displacements = 'disp_x disp_y disp_z'
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
    boundary = 'fixed_end free_end'
    value = 0
  []
  [fixed_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'fixed_end free_end'
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
  [force_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [force_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [force_3]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [force_1]
    type = MaterialRealVectorValueAux
    variable = force_1
    property = spring_force
    component = 0
  []
  [force_2]
    type = MaterialRealVectorValueAux
    variable = force_2
    property = spring_force
    component = 1
  []
  [force_3]
    type = MaterialRealVectorValueAux
    variable = force_3
    property = spring_force
    component = 2
  []
[]

[Postprocessors]
  [disp_x]
    type = NodalVariableValue
    variable = disp_x
    nodeid = 1
  []
  [disp_y]
    type = NodalVariableValue
    variable = disp_y
    nodeid = 1
  []
  [disp_z]
    type = NodalVariableValue
    variable = disp_z
    nodeid = 1
  []
  [force_1]
    type = ElementalVariableValue
    variable = force_1
    elementid = 0
  []
  [force_2]
    type = ElementalVariableValue
    variable = force_2
    elementid = 0
  []
  [force_3]
    type = ElementalVariableValue
    variable = force_3
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
