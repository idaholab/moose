# MassDiffusionEnergyGasMixDGKernel

This DG kernel implements the mass diffusion term in the energy equation of
the [gas mixture flow model](thermal_hydraulics/theory_manual/gas_mix_model/index.md):

!equation
\sum\limits_k \pd{J_k H_k}{x} \,.

!syntax parameters /DGKernels/MassDiffusionEnergyGasMixDGKernel

!syntax inputs /DGKernels/MassDiffusionEnergyGasMixDGKernel

!syntax children /DGKernels/MassDiffusionEnergyGasMixDGKernel
