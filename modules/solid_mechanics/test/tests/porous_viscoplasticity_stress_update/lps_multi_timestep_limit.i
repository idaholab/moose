# Verify that the generic porous-LPS timestep limit uses the admitted effective rate from all
# simultaneously active creep mechanisms. The included pure-deviatoric fixture makes the admitted
# controller rate equal to the sum of the per-law Norton rates.

!include lps_multi_pure_deviatoric.i

[Postprocessors]
  [material_dt_limit]
    type = ElementAverageMaterialProperty
    mat_prop = material_timestep_limit
    execute_on = TIMESTEP_END
  []
[]

[UserObjects]
  [check_material_dt_limit]
    type = Terminator
    expression = 'material_dt_limit <= 0 |
                  material_dt_limit != material_dt_limit |
                  gauge_0 != gauge_0 | gauge_1 != gauge_1 | gauge_2 != gauge_2 |
                  abs(material_dt_limit *
                      (abs(1e-10 * gauge_0) + abs(1e-20 * gauge_1^3) +
                       abs(3e-28 * gauge_2^4.5)) /
                      1e-2 -
                      1) > 1e-6'
    execute_on = FINAL
    error_level = ERROR
    message = 'The generic porous-LPS multi-law timestep limit did not match the summed admitted Norton rate.'
  []
[]
