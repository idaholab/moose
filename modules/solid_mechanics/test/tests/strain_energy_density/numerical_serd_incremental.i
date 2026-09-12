!include power_law_serd_path.i

# The incremental trapezoidal calculation approaches the analytical result as the time step is
# refined. power_law_serd_path.i uses 100 equal increments over the prescribed power-law path.
[Materials]
  [serd]
    type = StrainEnergyRateDensity
    use_incremental_serd = true
  []
[]
