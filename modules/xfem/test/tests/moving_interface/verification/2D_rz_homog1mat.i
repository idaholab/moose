# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
# XFEM Moving Interface Verification Problem
# Dimensionality:                                         2D
# Coordinate System:                                      rz
# Material Numbers/Types:   homogeneous 1 material, 2 region
# Element Order:                                         1st
# Interface Characteristics: u independent, prescribed level set function
# Description:
#   Transient 2D heat transfer problem in cylindrical coordinates designed with
#   the Method of Manufactured Solutions. This problem was developed to verify
#   XFEM performance on linear elements in the presence of a moving interface
#   sweeping across the x-y coordinates of a system with homogeneous material
#   properties. This problem can be exactly evaluated by FEM/Moose without the
#   moving interface. Both the temperature and level set function are designed
#   to be linear to attempt to minimize error between the Moose/exact solution
#   and XFEM results.
# Results:
#   The temperature at the bottom left boundary (x=1, y=1) exhibits the largest
#   difference between the FEM/Moose solution and XFEM results. We present the
#   XFEM results at this location with 10 digits of precision:
#     Time    Expected Temperature    XFEM Calculated Temperature
#      0.2                  440         440
#      0.4                  480         479.9998745
#      0.6                  520         519.9995067
#      0.8                  560         559.9989409
#      1.0                  600         599.9987054
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Problem]
  coord_type = RZ
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmin = 1.0
  xmax = 2.0
  ymin = 1.0
  ymax = 2.0
  elem_type = QUAD4
[]

[XFEM]
  qrule = moment_fitting
  output_cut_plane = true
[]

[UserObjects]
  [./level_set_cut_uo]
    type = LevelSetCutUserObject
    level_set_var = ls
    heal_always = true
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./ls]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./heat_cond]
    type = MatDiffusion
    variable = u
    diffusivity = diffusion_coefficient
  [../]
  [./vol_heat_src]
    type = BodyForce
    variable = u
    function = src_func
  [../]
  [./mat_time_deriv]
    type = TestMatTimeDerivative
    variable = u
    mat_prop_value = rhoCp
  [../]
[]

[AuxKernels]
  [./ls_function]
    type = FunctionAux
    variable = ls
    function = ls_func
  [../]
[]

[Constraints]
  [./xfem_constraints]
    type = XFEMSingleVariableConstraint
    variable = u
    geometric_cut_userobject = 'level_set_cut_uo'
    use_penalty = true
    alpha = 1e5
  [../]
[]

[Functions]
  [./src_func]
    type = ParsedFunction
    expression = '10*(-100*x-100*y+400) + 100*1.5*t/x'
  [../]
  [./neumann_func]
    type = ParsedFunction
    expression = '1.5*100*t'
  [../]
  [./dirichlet_right_func]
    type = ParsedFunction
    expression = '(-100*y+200)*t+400'
  [../]
  [./dirichlet_top_func]
    type = ParsedFunction
    expression = '(-100*x+200)*t+400'
  [../]
  [./ls_func]
    type = ParsedFunction
    expression = '-0.5*(x+y) + 2.04 - 0.2*t'
  [../]
[]

[Materials]
  [./mat_time_deriv_prop]
    type = GenericConstantMaterial
    prop_names = 'rhoCp'
    prop_values = 10
  [../]
  [./therm_cond_prop]
    type = GenericConstantMaterial
    prop_names = 'diffusion_coefficient'
    prop_values = 1.5
  [../]
[]

[BCs]
  [./left_du]
    type = FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = neumann_func
  [../]
  [./right_u]
    type = FunctionDirichletBC
    variable = u
    boundary = 'right'
    function = dirichlet_right_func
  [../]
  [./bottom_du]
    type = FunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = neumann_func
  [../]
  [./top_u]
    type = FunctionDirichletBC
    variable = u
    boundary = 'top'
    function = dirichlet_top_func
  [../]
[]

[ICs]
  [./u_ic]
    type = ConstantIC
    value = 400
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  line_search = 'none'

  l_tol = 1.0e-6
  nl_max_its = 15
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-9

  start_time = 0.0
  dt = 0.2
  end_time = 1.0
  max_xfem_update = 1
[]

[Outputs]
  interval = 1
  execute_on = 'initial timestep_end'
  exodus = true
  [./console]
    type = Console
    output_linear = true
  [../]
[]
