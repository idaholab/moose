[Problem]
  solve = false
[]

[Mesh]
  type = ImageMesh
  cells_per_pixel = 1
  dim = 2
  file = moose_logo_small.png
[]

[Variables]
  [original]
    family = MONOMIAL
    order = CONSTANT
  []
  [scaled]
    family = MONOMIAL
    order = CONSTANT
  []
  [shifted]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [image]
    type = ImageFunction
    file = moose_logo_small.png
  []
  [image_scale]
    type = ImageFunction
    file = moose_logo_small.png
    scale = 0.00392156862
  []
  [image_shift]
    type = ImageFunction
    file = moose_logo_small.png
    shift = -127.5
  []
[]

[ICs]
  [original_IC]
    type = FunctionIC
    function = image
    variable = original
  []
  [scaled_IC]
    type = FunctionIC
    function = image_scale
    variable = scaled
  []
  [shifted_IC]
    type = FunctionIC
    function = image_shift
    variable = shifted
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
