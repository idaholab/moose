# Reduced-integration (single quadrature point) variant of moose_rz.i with
# per-element hourglass stabilization; the NEML2 twin is neml2_rz_hourglass.i.
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
