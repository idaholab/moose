# Mohr-Coulomb interface spring of mohr_coulomb.i (tributary area 2, normal stiffness 1000, shear
# stiffness 500, cohesion 2, friction angle 30 degrees) with the closure of 0.01 and the shear
# deformation of 0.05 along axis 2 prescribed, and a load of 3 along axis 3. The spring slides,
# so the shear resultant is on the limit of 15.547: the force along axis 2 is
# sqrt(15.547^2 - 3^2) = 15.2547 and the force along axis 3 is the load. The tangent of the
# sliding state couples the shear to the closure and is not symmetric.
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
    x = '0 1 2'
    y = '0 -0.01 -0.01'
  []
  [shear_2]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 0 0.05'
  []
  [shear_load]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 0 3'
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
[]

[NodalKernels]
  [shear_load]
    type = UserForcingFunctionNodalKernel
    variable = disp_z
    boundary = free_end
    function = shear_load
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
