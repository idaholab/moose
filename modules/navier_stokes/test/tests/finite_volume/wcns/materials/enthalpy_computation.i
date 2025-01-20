[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
  []
[]

[AuxVariables]
  [pressure]
    type = INSFVPressureVariable
  []
  [T_fluid]
    type = INSFVEnergyVariable
  []
[]

[FVICs]
  [p]
    type = FVFunctionIC
    variable = 'pressure'
    function = '1e5 + 1e4 * x + 5e3 * y'
  []
  [T]
    type = FVFunctionIC
    variable = T_fluid
    function = '300 + 20 * x + 100 * y'
  []
[]

[FluidProperties]
  [fp]
    type = LeadBismuthFluidProperties
  []
[]

[FunctorMaterials]
  [fluid_props_to_mat_props]
    type = GeneralFunctorFluidProps
    fp = fp
    pressure = 'pressure'
    T_fluid = 'T_fluid'
    speed = '1'

    # For porous flow
    characteristic_length = 2
    porosity = 1
  []
  [compute_cp]
    type = INSFVEnthalpyFunctorMaterial
    # Use these for non constant cp
    # fp = fp
    # pressure = 'pressure'
    temperature = 'T_fluid'
    cp = 'cp'
    rho = 'rho'
  []
[]

T_mo = 398

[Postprocessors]
  [min_T]
    type = ElementExtremeFunctorValue
    value_type = 'min'
    functor = 'T_fluid'
  []
  [max_T]
    type = ElementExtremeFunctorValue
    functor = 'T_fluid'
  []
  [min_h]
    type = ElementExtremeFunctorValue
    value_type = 'min'
    functor = 'h'
  []
  [max_h]
    type = ElementExtremeFunctorValue
    value_type = 'max'
    functor = 'h'
  []
  [min_rho_h]
    type = ElementExtremeFunctorValue
    value_type = 'min'
    functor = 'rho_h'
  []
  [max_rho_h]
    type = ElementExtremeFunctorValue
    value_type = 'max'
    functor = 'rho_h'
  []
  [expected_min_h]
    type = ParsedPostprocessor
    expression = '164.8 * (min_T - T_mo) - 1.97e-2 * (min_T * min_T - T_mo * T_mo) +
         (1.25e-5 / 3) * (min_T * min_T * min_T - T_mo * T_mo * T_mo) + 4.56e+5 * (1. / min_T - 1. / T_mo)'
    pp_names = 'min_T'
    constant_names = 'T_mo'
    constant_expressions = '${T_mo}'
  []
  [expected_max_h]
    type = ParsedPostprocessor
    expression = '164.8 * (max_T - T_mo) - 1.97e-2 * (max_T * max_T - T_mo * T_mo) +
         (1.25e-5 / 3) * (max_T * max_T * max_T - T_mo * T_mo * T_mo) + 4.56e+5 * (1. / max_T - 1. / T_mo)'
    pp_names = 'max_T'
    constant_names = 'T_mo'
    constant_expressions = '${T_mo}'
  []
[]


[Executioner]
  type = Transient
  end_time = 0.1
  dt = 0.1
[]

[Outputs]
  csv = true
  hide = 'min_T max_T'
[]

[Problem]
  solve = false
[]
