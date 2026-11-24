
[Mesh]
  [rod_rz]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.0
    xmax = 0.05
    ymin = 0.0
    ymax = 0.01
    nx = 100
    ny = 20
  []
   coord_type = rz
   rz_coord_axis = y
[]
#[Problem]
#    type = FEProblem
#    coord_type = rz
#    rz_coord_axis = y
#[]
[Materials]
  [copper]
    type = GenericConstantMaterial
    prop_names = 'mu sigma'
    prop_values = '1.25663706e-6 5.8e7'
    block = 0
  []
  [copper_thermal]
    type = GenericConstantMaterial
    prop_names  = 'rho_cp k_thermal'
    prop_values = '3.45e6 400'
    block = 0
  []
[]

[Functions]
  [Jz]
    type = ParsedFunction
    #expression = '1e6 * (x^2 + y^2 <= 0.01^2)'  # Jz = 1e6 inside r <= 0.01
    expression = '1.25663706 * (x <= 0.01)' # expression = '1.25663706e-6 * 1e6 * (x <= 0.01)'
  []
  [Q_joule]
    type = ParsedFunction
    #expression = '(1e6^2 / 5.8e7) * (x <= 0.01)'
    expression = '(1.72413793e8) * (x <= 0.01)'   # ~1.72e8 W/m^3
  []
[]

[Variables]
  [Az]
    family = LAGRANGE
    order = FIRST
  []
  [T]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxVariables]
  # dAz/dr stored as an ELEMENTAL aux variable (required by VariableGradientComponent)
  [dAz_dr]
    family = MONOMIAL
    order = CONSTANT
  []
  # B0 = Bphi = -dAz/dr
  [B0]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [Az_ic]
    type = ConstantIC
    variable = Az
    value = 0.0
  []
  [T_ic]
    type = ConstantIC
    variable = T
    value = 300.0
  []
[]

[Kernels]
  [A_diff]
    type = Diffusion
    variable = Az
  [ ]
  [A_source]
    type = BodyForce
    variable = Az
    function = Jz # multiply Jz by mu0 in the function definition
  []
  [A_time]
    type = CoefTimeDerivative
    variable = Az
    Coefficient = 72.98       # sigma for copper
  []
  [T_time]
    type = CoefTimeDerivative
    variable = T
    Coefficient = 3.45e6
  []
  [T_diff]
    type = CoefDiffusion
    variable = T
    coef = 400
  []
  [T_source]
    type = BodyForce
    variable = T
    function = Q_joule
  []
[]

[AuxKernels]
  # dAz/dr = dAz/dx (since x is the radial coordinate r in this rz mesh)
  [dAz_dr_aux]
    type = VariableGradientComponent
    variable = dAz_dr
    component = x
    gradient_variable = Az
  []

  # B0 = - dAz/dr
  [B0_aux]
    type = ParsedAux
    variable = B0
    coupled_variables = 'dAz_dr'
    expression = '-dAz_dr'
  []
[]

[BCs]
  [Az_outer]
    type = DirichletBC
    variable = Az
    value = 0.0
    boundary = 'right'
  []
  [T_outer]
    type = DirichletBC
    variable = T
    value = 300.0
    boundary = 'right'
  []
[]


[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  start_time = 0.0
  end_time = 108.0
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  nl_max_its = 50
  line_search = 'bt'
  petsc_options_iname = '-pc_type'
  petsc_options_value  = 'lu'
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1e-4
    optimal_iterations = 6
    growth_factor = 2.0
    cutback_factor = 0.5
    #dtmax = 5.0
  []
[]

#[Executioner]
#  type = Transient
#  solve_type = 'NEWTON'
#  start_time = 0.0
#  end_time = 108.0
#  dt = 0.01
#  nl_rel_tol = 1e-8
#  nl_abs_tol = 1e-10
#  nl_max_its = 200
#[]

[VectorPostprocessors]
  [az_line]
     type = LineValueSampler
     variable =Az
     start_point = '0.0 0.0 0.0'
     end_point = '0.05 0.0 0.0'
     num_points = 201
     sort_by = x
   []
   [b0_line]
    type = LineValueSampler
    variable = B0
    start_point = '0.0 0.0 0.0'
    end_point = '0.05 0.0 0.0'
    num_points = 201
    sort_by = x
  []
  [az_T_line]
    type = LineValueSampler
    variable = T
    start_point = '0.0 0.0 0.0'
    end_point = '0.05 0.0 0.0'
    num_points = 201
    sort_by = x
  []
[]

[Postprocessors]
  [T_centre]
    type = PointValue
    variable = T
    point = '0.0 0.0 0.0'
  []
  [Q_check]
    type = FunctionValuePostprocessor
    function = Q_joule
    point = '0.005 0.0 0.0'
  []
[]

[Outputs]
  exodus = true
  [csv_out]
    type = CSV
    file_base = 'thermal_long1/copper_stablizer_thermal'
  []
[]
