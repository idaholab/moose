!include 'neml2_rz.i'

[UserObjects]
  [hourglass]
    type = NEML2HourglassCorrection
    assembly = 'assembly'
    fe = 'fe'
    executor = 'neml2'
    displacements = 'disp_x disp_y'
    penalty = 10
    shear_modulus = 1
    residual = 'NONTIME'
  []
[]

[Executioner]
  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
[]
