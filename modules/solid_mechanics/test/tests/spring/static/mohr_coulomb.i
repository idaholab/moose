# Mohr-Coulomb interface spring along the global X axis (the normal) with a tributary area of 2,
# normal stiffness 1000, shear stiffness 500, cohesion 2 and friction angle 30 degrees, under a
# prescribed deformation of its second node in three stages of one time unit each:
# closure of 0.01 (normal stress 10, normal force -20), then a shear deformation of (0.05, 0.02)
# along axes 2 and 3 at constant closure, then an opening back through zero. The shear limit at
# the closure is 2 + 10 tan 30 = 7.7735 per unit area, 15.547 in force, reached at a shear
# deformation of 0.015547; at (0.05, 0.02) the resultant slides along the direction of the
# elastic trial, (0.9285, 0.3714), giving the forces (14.435, 5.7740). A quarter of the way
# through the opening (t = 2.25) the closure is 0.005, the normal stress 5, the limit 4.8868 per
# unit area and the forces (-10, 9.0745, 3.6298); at the half way point the closure is zero, the
# limit is the cohesion alone, 4, forces (0, 3.7139, 1.4856); beyond it the interface is open and
# every force is zero.
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
  [closure]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 -0.01 -0.01 0.01'
  []
  [shear_2]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 0.05 0.05'
  []
  [shear_3]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 0.02 0.02'
  []
[]

[Materials]
  [deformation]
    type = ComputeSpringDeformation
    displacements = 'disp_x disp_y disp_z'
  []
  [law]
    type = MohrCoulombSpring
    normal_stiffness = 1000
    shear_stiffness = 500
    cohesion = 2
    friction_angle = 30
    tributary_area = 2
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
    boundary = fixed_end
    value = 0
  []
  [fixed_z]
    type = DirichletBC
    variable = disp_z
    boundary = fixed_end
    value = 0
  []
  [closure]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = free_end
    function = closure
  []
  [shear_2]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = free_end
    function = shear_2
  []
  [shear_3]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = free_end
    function = shear_3
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
  dt = 0.25
  end_time = 3
[]

[Outputs]
  csv = true
[]
