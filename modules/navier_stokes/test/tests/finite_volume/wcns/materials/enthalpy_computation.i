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
    type = Water97FluidProperties
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

[Postprocessors]
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
[]


[Executioner]
  type = Transient
  end_time = 0.1
  dt = 0.1
[]

[Outputs]
  csv = true
[]

[Problem]
  solve = false
[]
