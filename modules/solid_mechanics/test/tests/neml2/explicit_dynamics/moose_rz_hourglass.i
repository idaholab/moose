!include 'moose_rz.i'

[Kernels]
  [hgx]
    type = HourglassCorrectionQuad4
    variable = disp_x
    penalty = 10
    shear_modulus = 1
  []
  [hgy]
    type = HourglassCorrectionQuad4
    variable = disp_y
    penalty = 10
    shear_modulus = 1
  []
[]

[Executioner]
  [Quadrature]
    type = GAUSS
    order = CONSTANT
  []
[]
