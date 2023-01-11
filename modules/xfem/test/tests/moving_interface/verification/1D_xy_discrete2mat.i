# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #
# XFEM Moving Interface Verification Problem
# Dimensionality:                                   quasi-1D
# Coordinate System:                                      xy
# Material Numbers/Types:discrete homog 2 material, 2 region
# Element Order:                                         1st
# Interface Characteristics: u independent, prescribed level set function
# Description
#   A transient heat transfer problem in Cartesian coordinates designed with
#   the Method of Manufactured Solutions. This problem was developed to verify
#   XFEM performance in the presence of a moving interface separating two
#   discrete material regions for linear element models. Both the temperature
#   solution and level set function are designed to be linear to attempt to
#   minimize error between the exact solution and XFEM results. Thermal
#   conductivity, density, and heat capacity are homogeneous in each material
#   region with a discontinuous jump in thermal flux between the two material
#   regions.
# Results:
#   The temperature at the left boundary is determined by the analytical
#   solution, so temperature at the right boundary (x=1) should exhibit the
#   largest difference between the analytical solution and XFEM results. We
#   present the analytical and XFEM results at the material interface position
#   and right side boundary at various times.
#  Interface:
#     Time    Expected Temperature    XFEM Calculated Temperature
#       20       746.75                  746.7235521
#       40       893.05                  893.0379081
#       60      1040.15                 1040.1527530
#
#  Right Boundary (x=1):
#     Time    Expected Temperature    XFEM Calculated Temperature
#       20       720                     719.9708681
#       40       840                     839.9913293
#       60       960                     960.0100886
#
# IMPORTANT NOTE:
#   When running this input file, add the --allow-test-objects tag!!!
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

[GlobalParams]
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
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
    level_set_var = phi
    heal_always = true
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./phi]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./heat_cond]
    type = MatDiffusion
    variable = u
    diffusivity = 'diffusion_coefficient'
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
    variable = phi
    function = ls_func
  [../]
[]

[Constraints]
  [./xfem_constraint]
    type = XFEMSingleVariableConstraint
    variable = u
    geometric_cut_userobject = 'level_set_cut_uo'
    jump_flux = jump_flux_func
    use_penalty = true
    alpha = 1e5
  [../]
[]

[Functions]
  [./src_func]
    type = ParsedFunction
    expression = 'phi:=(0.75-x-0.001*t);
        i:=(0.75-0.001*t);
        if (phi>=0,
            10*(8-x),
            (7/(1-i))*((i-2)*x + (8-7*i)) )'
  [../]
  [./right_du_func]
    type = ParsedFunction
    expression = 'i:=(0.75-0.001*t);
        (2.0/(1-i))*(-5+5*i+i*t-2*t)'
  [../]
  [./exact_u_func]
    type = ParsedFunction
    expression = 'phi:=(0.75-x-0.001*t);
        i:=(0.75-0.001*t);
        if (phi>=0,
            605 - 5*x + t*(8-x),
            (1/(1-i))*((-5+5*i+i*t-2*t)*x + (605-605*i+8*t-7*t*i)) )'
  [../]
  [./jump_flux_func]
    type = ParsedFunction
    expression = 'i:=(0.75-0.001*t);
        k_1:=(20.0);
        k_2:=(2.0);
        k_1*(5+t) + (k_2/(1-i))*(-5+5*i+i*t-2*t)'
  [../]

  [./ls_func]
    type = ParsedFunction
    expression = '0.75 - x - 0.001*t'
  [../]
[]

[Materials]
  [./mat_time_deriv_prop]
    type = GenericConstantMaterial
    prop_names = 'A_rhoCp B_rhoCp'
    prop_values = '10 7'
  [../]
  [./therm_cond_prop]
    type = GenericConstantMaterial
    prop_names = 'A_diffusion_coefficient B_diffusion_coefficient'
    prop_values = '20.0 2.0'
  [../]

  [./combined_rhoCp]
    type = LevelSetBiMaterialReal
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = phi
    prop_name = rhoCp
  [../]
  [./combined_diffusion_coefficient]
    type = LevelSetBiMaterialReal
    levelset_positive_base = 'A'
    levelset_negative_base = 'B'
    level_set_var = phi
    prop_name = diffusion_coefficient
  [../]
[]

[BCs]
  [./left_u]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left'
    function = exact_u_func
  [../]
  [./right_du]
    type = FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = right_du_func
  [../]
[]

[ICs]
  [./u_ic]
    type = ConstantIC
    value = 600
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  # petsc_options_iname = '-pc_type -pc_hypre_type'
  # petsc_options_value = 'hypre boomeramg'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'

  l_tol = 1.0e-6
  nl_max_its = 15
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-9

  start_time = 0.0
  dt = 20
  end_time = 60.0
  max_xfem_update = 2
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
