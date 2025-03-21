# MassDiffusionEnergyGasMixDGKernel

This DG kernel implements the mass diffusion term in the energy equation of
the [gas mixture flow model](thermal_hydraulics/theory_manual/gas_mix_model/index.md):

!equation
\sum\limits_g \pd{J_g H_g}{x} \,.

!syntax parameters /DGKernels/MassDiffusionEnergyGasMixDGKernel

!syntax inputs /DGKernels/MassDiffusionEnergyGasMixDGKernel

!syntax children /DGKernels/MassDiffusionEnergyGasMixDGKernel
