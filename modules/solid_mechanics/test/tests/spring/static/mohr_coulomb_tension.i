# Bonded Mohr-Coulomb interface spring (unit area, normal and shear stiffness 1000, cohesion 4,
# friction angle 20 degrees, initial normal stress 1, tensile strength 2) under a prescribed
# opening with a shear deformation of (0.05, 0.02). The normal stress is 1 - 1000 times the
# opening. At an opening of 0.001 it is zero: no normal force (the soil carried the initial
# stress), the shear limit is the cohesion, forces (0, 3.7139, 1.4856). At 0.002 it is -1, half
# the tensile strength: the spring carries the tension 1 and the limit has fallen to half the
# cohesion, 2, forces (1, 1.8570, 0.7428). At 0.003 the tension is the tensile strength and the
# limit is zero: forces (2, 0, 0). At 0.004 the interface separates: every force is zero.
# Closing it again through an opening of 0.001 (normal stress zero) to a closure of 0.002 with
# the shear pushed to 0.1 along axis 2 (the slip took up the whole shear while the interface was
# open) depends on the breakage:
# - the bond is back by default: at zero normal stress the cohesion alone, forces (0, 4, 0);
#   closed, the limit is 4 + 3 tan 20 = 5.0919, forces (-2, 5.0919, 0);
# - with permanent_breakage, residual_cohesion 1 and residual_friction_angle 10 (the spec's
#   cli_args): the residual cohesion alone at zero normal stress, forces (0, 1, 0), then the
#   residual limit 1 + 3 tan 10 = 1.5290, forces (-2, 1.5290, 0).
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
  [opening]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0.002 0.004 -0.002'
  []
  [shear_2]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0.05 0.05 0.1'
  []
  [shear_3]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0.02 0.02 0.02'
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
    shear_stiffness = 1000
    cohesion = 4
    friction_angle = 20
    initial_normal_stress = 1
    tensile_strength = 2
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
  [opening]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = free_end
    function = opening
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
  dt = 0.5
  end_time = 3
[]

[Outputs]
  csv = true
[]
