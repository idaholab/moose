# Hysteretic spring along the global X axis under a load history, the same bilinear backbone as
# hysteretic.i (stiffness 100 to a force of 5 at 0.05, then 20). The load rises to 10 (deformation
# 0.3 on the backbone), drops to 0 (the Masing branch 10 + 2 f((x - 0.3) / 2) is zero at x = 0.2,
# an elastic unloading of twice the yield force) and goes on to -10, where the branch meets the
# backbone at x = -0.3. Under a load the switch between the backbone and a branch inside the
# Newton iterations needs the backtracking line search; under a prescribed deformation it does not.
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

[Functions]
  [bilinear]
    type = PiecewiseLinear
    axis = x
    x = '-1 -0.05 0 0.05 1'
    y = '-24 -5 0 5 24'
  []
  [load_history]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 10 0 -10'
  []
[]

[Materials]
  [deformation]
    type = ComputeSpringDeformation
    displacements = 'disp_x disp_y disp_z'
  []
  [law]
    type = MasingHystereticSpring
    backbone_functions = 'bilinear bilinear bilinear'
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
    type = UserForcingFunctionNodalKernel
    variable = disp_x
    boundary = free_end
    function = load_history
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
  line_search = 'bt'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  dt = 0.5
  end_time = 3
[]

[Outputs]
  csv = true
[]
