# Generic independent fixed-porosity recovery fixture owned by solid_mechanics.
# Both test populations are already on explicit porosity floors at the checkpoint. The forced
# continuation deliberately under-iterates the coupled (p,q,f0,f1) Newton so the generic
# fixed-porosity p-q recovery must finish the local solve without BISON gas physics.

!include prescribed_two_population_active_floor.i

[Postprocessors]
  [accepted_dt]
    type = TimestepSize
    execute_on = TIMESTEP_END
  []
[]
