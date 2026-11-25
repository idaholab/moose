# Spring with six degrees of freedom along the global X axis: a linear translational law with
# the stiffnesses (100, 200, 300) and a linear rotational law with (10, 20, 30) on the same
# element. The loads (10, 4, 3) and the moments (1, 0.5, 0.3) at the free node give the
# displacements (0.1, 0.02, 0.01) and the rotations (0.1, 0.025, 0.01).
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
  [rot_x]
  []
  [rot_y]
  []
  [rot_z]
  []
[]

[Materials]
  [deformation]
    type = ComputeSpringDeformation
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [translational]
    type = LinearSpring
    stiffness = '100 200 300'
  []
  [rotational]
    type = LinearSpring
    mode = rotational
    stiffness = '10 20 30'
  []
[]

[Kernels]
  [spring_disp_x]
    type = StressDivergenceSpring
    variable = disp_x
    component = 0
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [spring_disp_y]
    type = StressDivergenceSpring
    variable = disp_y
    component = 1
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [spring_disp_z]
    type = StressDivergenceSpring
    variable = disp_z
    component = 2
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [spring_rot_x]
    type = StressDivergenceSpring
    variable = rot_x
    component = 3
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [spring_rot_y]
    type = StressDivergenceSpring
    variable = rot_y
    component = 4
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
  [spring_rot_z]
    type = StressDivergenceSpring
    variable = rot_z
    component = 5
    displacements = 'disp_x disp_y disp_z'
    rotations = 'rot_x rot_y rot_z'
  []
[]

[BCs]
  [fixed_disp_x]
    type = DirichletBC
    variable = disp_x
    boundary = fixed_end
    value = 0
  []
  [fixed_disp_y]
    type = DirichletBC
    variable = disp_y
    boundary = fixed_end
    value = 0
  []
  [fixed_disp_z]
    type = DirichletBC
    variable = disp_z
    boundary = fixed_end
    value = 0
  []
  [fixed_rot_x]
    type = DirichletBC
    variable = rot_x
    boundary = fixed_end
    value = 0
  []
  [fixed_rot_y]
    type = DirichletBC
    variable = rot_y
    boundary = fixed_end
    value = 0
  []
  [fixed_rot_z]
    type = DirichletBC
    variable = rot_z
    boundary = fixed_end
    value = 0
  []
[]

[NodalKernels]
  [load_disp_x]
    type = ConstantRate
    variable = disp_x
    boundary = free_end
    rate = 10
  []
  [load_disp_y]
    type = ConstantRate
    variable = disp_y
    boundary = free_end
    rate = 4
  []
  [load_disp_z]
    type = ConstantRate
    variable = disp_z
    boundary = free_end
    rate = 3
  []
  [load_rot_x]
    type = ConstantRate
    variable = rot_x
    boundary = free_end
    rate = 1
  []
  [load_rot_y]
    type = ConstantRate
    variable = rot_y
    boundary = free_end
    rate = 0.5
  []
  [load_rot_z]
    type = ConstantRate
    variable = rot_z
    boundary = free_end
    rate = 0.3
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
  [rot_x]
    type = NodalVariableValue
    variable = rot_x
    nodeid = 1
  []
  [rot_y]
    type = NodalVariableValue
    variable = rot_y
    nodeid = 1
  []
  [rot_z]
    type = NodalVariableValue
    variable = rot_z
    nodeid = 1
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
  dt = 1
  end_time = 1
[]

[Outputs]
  csv = true
[]
