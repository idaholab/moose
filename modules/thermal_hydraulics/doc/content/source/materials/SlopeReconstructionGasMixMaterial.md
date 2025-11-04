# SlopeReconstructionGasMixMaterial

This object computes reconstructed solution values for the
[gas mixture flow model](modules/thermal_hydraulics/theory_manual/gas_mix_model/index.md).
First, slopes are computed for the primitive variables $\mathbf{W}$,
limited with a slope limiter, if specified, and then used to compute the extrapolated values
at the cell interfaces:

!equation
\mathbf{W}_{i+1/2} = \mathbf{W}_i + (x_{i+1/2} - x_i)\Delta\mathbf{W}_i \,.

Then, the solution values at the interfaces are computed from these values:

!equation
\mathbf{U}_{i+1/2} = \mathbf{U}(\mathbf{W}_{i+1/2}) \,.

!syntax parameters /Materials/SlopeReconstructionGasMixMaterial

!syntax inputs /Materials/SlopeReconstructionGasMixMaterial

!syntax children /Materials/SlopeReconstructionGasMixMaterial
