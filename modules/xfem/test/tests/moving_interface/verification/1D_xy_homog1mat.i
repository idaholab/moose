# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
# XFEM Moving Interface Verification Problem
# Dimensionality:                                   quasi-1D
# Coordinate System:                                      xy
# Material Numbers/Types:   homogeneous 1 material, 2 region
# Element Order:                                         1st
# Interface Characteristics: u independent, prescribed linear level set function
# Description:
#   A simple transient heat transfer problem in Cartesian coordinates designed
#   with the Method of Manufactured Solutions. This problem was developed to
#   verify XFEM performance in the presence of a moving interface for linear
#   element models that can be exactly evaluated by FEM/Moose. Both the
#   temperature solution and level set function are designed to be linear to
#   attempt to minimize error between the Moose/exact solution and XFEM results.
#   Thermal conductivity is a single, constant value at all points in the system.
# Results:
#   The temperature at the left boundary (x=0) exhibits the largest difference
#   between the FEM/Moose solution and XFEM results. We present the XFEM results
#   at this location with 10 digits of precision:
#     Time    Expected Temperature    XFEM Calculated Temperature
#      0.2                  440         440
#      0.4                  480         480.0000064
#      0.6                  520         520.0000323
#      0.8                  560         560.0000896
#      1.0                  600         600.0001870
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 1
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 0.5
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
  [./xfem_constraint]
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
    expression = '10*(-200*x+200)'
  [../]
  [./ls_func]
    type = ParsedFunction
    expression = '1-(x-0.04)-0.2*t'
  [../]
  [./neumann_func]
    type = ParsedFunction
    expression = '1.5*200*t'
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
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 400
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
