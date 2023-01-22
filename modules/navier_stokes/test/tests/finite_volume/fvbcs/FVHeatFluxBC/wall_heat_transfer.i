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
  [k]
    type = ParsedFunction
    expression = 0.1*(100*y+1)
  []
  [kappa]
    type = ParsedFunction
    expression = 0.2*(200*y+1)
  []
  [kappa_s]
    type = ParsedFunction
    expression = 0.4*(200*y+1)
  []
  [k_s]
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
    type = NSFVHeatFluxBC
    variable = Ts
    boundary = 'left'
    phase = 'solid'
    value = ${flux}
  []
  [right_Ts]
    type = FVDirichletBC
    variable = Ts
    boundary = 'right'
    value = 1000.0
  []
  [left_Tf]
    type = NSFVHeatFluxBC
    variable = Tf
    boundary = 'left'
    phase = 'fluid'
    value = ${flux}
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
    type = ADMaterialRealAux
    variable = k
    property = 'k'
  []
  [k_s]
    type = ADMaterialRealAux
    variable = k_s
    property = 'k_s'
  []
  [kappa_s]
    type = ADMaterialRealAux
    variable = kappa_s
    property = 'kappa_s'
  []
[]

[Materials]
  [thermal_conductivities_k]
    type = ADGenericFunctionMaterial
    prop_names = 'k'
    prop_values = 'k'
  []
  [thermal_conductivities_k_s]
    type = ADGenericFunctionMaterial
    prop_names = 'k_s'
    prop_values = 'k_s'
  []
  [thermal_conductivities_kappa]
    type = ADGenericConstantVectorMaterial
    prop_names = 'kappa'
    prop_values = '0.1 0.2 .03'
  []
  [thermal_conductivities_kappa_s]
    type = ADGenericFunctionMaterial
    prop_names = 'kappa_s'
    prop_values = 'kappa_s'
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
