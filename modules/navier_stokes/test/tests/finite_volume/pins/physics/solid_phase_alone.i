[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = 1
    nx = 20
    ny = 10
  []
[]

[Physics]
  [NavierStokes]
    [SolidHeatTransfer]
      [solid]
        block = 0

        thermal_conductivity_solid = 'k_solid'

        # Heat source directly in the solid
        external_heat_source = '300'
        external_heat_source_coeff = 2

        # Ambient convection
        fluid_temperature_variable = 300
        ambient_convection_alpha = 10
        ambient_convection_blocks = 0
        ambient_convection_temperature = 310
      []
    []
  []
[]

[AuxVariables]
  [porosity]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 0.5
  []
[]

[FunctorMaterials]
  [constants]
    type = GenericFunctorMaterial
    prop_names = 'k_solid cp_solid rho_solid'
    prop_values = '1 1000 1000'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
