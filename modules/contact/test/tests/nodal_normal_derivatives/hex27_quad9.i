!include ../3d-mortar-contact/frictionless-mortar-3d-action.i

[UserObjects]
  [verify_nodal_normal_derivatives]
    type = NodalNormalDerivativesTest
    weighted_gap_uo = lm_weightedgap_object_mortar
    disp_x = disp_x
    disp_y = disp_y
    disp_z = disp_z
    max_derivative_dependencies = 199
  []
[]
