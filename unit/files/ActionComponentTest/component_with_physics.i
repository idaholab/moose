# A cylinder with a diffusion Physics, used to check the boundary condition, initial condition
# and material property inputs on a component
[Physics]
  [Diffusion]
    [ContinuousGalerkin]
      [diff_u]
        diffusivity_matprop = 'diff'
      []
    []
  []
[]

[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 2
    length = 4
    radius = 1
    n_axial = 2
    n_radial = 2
    block = 'cyl'
    physics = 'diff_u'
    property_names = 'diff'
    property_values = '1'
  []
[]

[Executioner]
  type = Steady
[]
