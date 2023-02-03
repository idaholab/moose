flux=10

[GlobalParams]
  porosity = 'porosity'
  splitting = 'porosity'
  locality = 'global'
  average_porosity = 'average_eps'
  average_k_fluid='average_k_fluid'
  average_k_solid='average_k_solid'
  average_kappa='average_k_fluid'  # because of vector matprop, should be kappa
  average_kappa_solid='average_kappa_solid'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 20
  xmin = 0.0
  xmax = 1.0
  ymin = 0.0
  ymax = 1.0
[]

[Variables]
  [Tf]
    type = MooseVariableFVReal
  []
  [Ts]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [k]
    type = MooseVariableFVReal
  []
  [kappa]
    type = MooseVariableFVReal
  []
  [k_s]
    type = MooseVariableFVReal
  []
  [kappa_s]
    type = MooseVariableFVReal
  []
  [porosity]
    type = MooseVariableFVReal
    initial_condition = 0.2
  []
[]

[Functions]
  [k_function]
    type = ParsedFunction
    expression = 0.1*(100*y+1)
  []
  [kappa_function]
    type = ParsedFunction
    expression = 0.2*(200*y+1)
  []
  [kappa_s_function]
    type = ParsedFunction
    expression = 0.4*(200*y+1)
  []
  [k_s_function]
    type = ParsedFunction
    expression = 0.2*(200*y+1)+2*x
  []
[]

[FVKernels]
  [Tf_diffusion]
    type = FVDiffusion
    variable = Tf
    coeff = 1
  []
  [Ts_diffusion]
    type = FVDiffusion
    variable = Ts
    coeff = 1
  []
[]

[FVBCs]
  [left_Ts]
    type = NSFVFunctorHeatFluxBC
    variable = Ts
    boundary = 'left'
    phase = 'solid'
    value = ${flux}
    k = 'k_mat'
    k_s = 'k_s_mat'
    kappa = 'kappa_mat'
    kappa_s = 'kappa_s_mat'
  []
  [right_Ts]
    type = FVDirichletBC
    variable = Ts
    boundary = 'right'
    value = 1000.0
  []
  [left_Tf]
    type = NSFVFunctorHeatFluxBC
    variable = Tf
    boundary = 'left'
    phase = 'fluid'
    value = ${flux}
    k = 'k_mat'
    k_s = 'k_s_mat'
    kappa = 'kappa_mat'
    kappa_s = 'kappa_s_mat'
  []
  [right_Tf]
    type = FVDirichletBC
    variable = Tf
    boundary = 'right'
    value = 1000.0
  []
[]

[AuxKernels]
  [k]
    type = ADFunctorElementalAux
    variable = k
    functor = 'k_mat'
  []
  [k_s]
    type = ADFunctorElementalAux
    variable = k_s
    functor = 'k_s_mat'
  []
  [kappa_s]
    type = ADFunctorElementalAux
    variable = kappa_s
    functor = 'kappa_s_mat'
  []
[]

[Materials]
  [thermal_conductivities_k]
    type = ADGenericFunctorMaterial
    prop_names = 'k_mat'
    prop_values = 'k_function'
  []
  [thermal_conductivities_k_s]
    type = ADGenericFunctorMaterial
    prop_names = 'k_s_mat'
    prop_values = 'k_s_function'
  []
  [thermal_conductivities_kappa]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'kappa_mat'
    prop_values = '0.1 0.2 .03'
  []
  [thermal_conductivities_kappa_s]
    type = ADGenericFunctorMaterial
    prop_names = 'kappa_s_mat'
    prop_values = 'kappa_s_function'
  []
[]

[Postprocessors]
  [average_eps]
    type = ElementAverageValue
    variable = porosity

    # because porosity is constant in time, we evaluate this only once
    execute_on = 'initial'
  []
  [average_k_fluid]
    type = ElementAverageValue
    variable = k
  []
  [average_k_solid]
    type = ElementAverageValue
    variable = k_s
  []
  [average_kappa_solid]
    type = ElementAverageValue
    variable = kappa_s
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  hide = 'porosity average_eps'
[]
