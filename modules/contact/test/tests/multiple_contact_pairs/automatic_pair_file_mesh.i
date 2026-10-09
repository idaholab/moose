[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = three_hexagons_coarse.e
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
[]

[Contact]
  [contact_pressure]
    formulation = penalty
    model = frictionless
    penalty = 2e+03
    automatic_pairing_boundaries = '102 201 301'
    automatic_pairing_method = CENTROID
    automatic_pairing_distance = 2.75
  []
[]

[Executioner]
  type = Steady
[]
