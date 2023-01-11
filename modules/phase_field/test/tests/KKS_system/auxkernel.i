#
# This test checks if the two phase and lagrange multiplier solutions can be replicated
# with a two order parameter approach, where the second order parameter eta2 is an
# auxiliary variable that is set as eta2 := 1 - eta1
# The solution is reproduced, but convergence is suboptimal, as important Jacobian
# terms for eta1 (that should come indirectly from eta2) are missing.
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmax = 5
[]

[AuxVariables]
  [Fglobal]
    order = CONSTANT
    family = MONOMIAL
  []

  # order parameter 2
  [eta2]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []
[]

#
# With this approach the derivative w.r.t. eta1 is lost in all terms depending on
# eta2 a potential fix would be to make eta2 a material property with derivatives.
# This would require a major rewrite of the phase field kernels, though.
#
[AuxKernels]
  [eta2]
    type = ParsedAux
    variable = eta2
    expression = '1-eta1'
    coupled_variables = eta1
  []
[]

[Variables]
  # concentration
  [c]
    order = FIRST
    family = LAGRANGE
    [InitialCondition]
      type = FunctionIC
      function = x/5
    []
  []

  # order parameter 1
  [eta1]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []

  # phase concentration 1
  [c1]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.9
  []

  # phase concentration 2
  [c2]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.1
  []
[]

[Materials]
  # simple toy free energies
  [f1] # = fd
    type = DerivativeParsedMaterial
    property_name = F1
    coupled_variables = 'c1'
    expression = '(0.9-c1)^2'
  []
  [f2] # = fm
    type = DerivativeParsedMaterial
    property_name = F2
    coupled_variables = 'c2'
    expression = '(0.1-c2)^2'
  []

  # Switching functions for each phase
  [h1_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta1
    function_name = h1
  []
  [h2_eta]
    type = DerivativeParsedMaterial
    material_property_names = 'h1(eta1)'
    expression = '1-h1'
    property_name = h2
    coupled_variables = eta1
  []

  # Coefficients for diffusion equation
  [Dh1]
    type = DerivativeParsedMaterial
    material_property_names = 'D h1(eta1)'
    expression = D*h1
    property_name = Dh1
    coupled_variables = eta1
  []
  [Dh2]
    type = DerivativeParsedMaterial
    material_property_names = 'D h2(eta1)'
    expression = 'D*h2'
    property_name = Dh2
    coupled_variables = eta1
  []

  # Barrier functions for each phase
  [g1]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta1
    function_name = g1
  []
  [g2]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta2
    function_name = g2
  []

  # constant properties
  [constants]
    type = GenericConstantMaterial
    prop_names  = 'D   L   kappa'
    prop_values = '0.7 0.7 0.2'
  []
[]

[Kernels]
  #Kernels for diffusion equation
  [diff_time]
    type = TimeDerivative
    variable = c
  []
  [diff_c1]
    type = MatDiffusion
    variable = c
    diffusivity = Dh1
    v = c1
    args = eta1
  []
  [diff_c2]
    type = MatDiffusion
    variable = c
    diffusivity = Dh2
    v = c2
    args = eta1
  []

  # Kernels for Allen-Cahn equation for eta1
  [deta1dt]
    type = TimeDerivative
    variable = eta1
  []
  [ACBulkF1]
    type = KKSMultiACBulkF
    variable = eta1
    Fj_names = 'F1 F2'
    hj_names = 'h1 h2'
    gi_name = g1
    eta_i = eta1
    wi = 0.2
    coupled_variables = 'c1 c2 eta2'
  []
  [ACBulkC1]
    type = KKSMultiACBulkC
    variable = eta1
    Fj_names = 'F1 F2'
    hj_names = 'h1 h2'
    cj_names = 'c1 c2'
    eta_i = eta1
    coupled_variables = 'eta2'
  []
  [ACInterface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa
  []

  # Phase concentration constraints
  [chempot12]
    type = KKSPhaseChemicalPotential
    variable = c1
    cb = c2
    fa_name = F1
    fb_name = F2
  []
  [phaseconcentration]
    type = KKSMultiPhaseConcentration
    variable = c2
    cj = 'c1 c2'
    hj_names = 'h1 h2'
    etas = 'eta1 eta2'
    c = c
  []
[]

[AuxKernels]
  [Fglobal_total]
    type = KKSMultiFreeEnergy
    Fj_names = 'F1 F2 '
    hj_names = 'h1 h2 '
    gj_names = 'g1 g2 '
    variable = Fglobal
    w = 0.2
    interfacial_vars = 'eta1  eta2 '
    kappa_names      = 'kappa kappa'
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu      '
  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-10
  nl_abs_tol = 1.0e-11

  end_time = 350
  dt = 10
[]

[Preconditioning]
  [full]
    type = SMP
    full = true
  []
[]

[VectorPostprocessors]
  [c]
    type = LineValueSampler
    variable = c
    start_point = '0 0 0'
    end_point = '5 0 0'
    num_points = 21
    sort_by = x
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
