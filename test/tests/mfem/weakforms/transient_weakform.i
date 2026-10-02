# Include mfem/kernels/heattransfer.i
!include ../kernels/heattransfer.i

# Naming every kernel and boundary condition explicitly must reproduce the default weak form,
# which uses all of them.
[WeakForms]
  [HeatTransfer]
    type = MFEMTimeDependentWeakForm
    kernels = 'diff dT_dt'
    bcs = 'bottom top_convective'
  []
[]

[Postprocessors]
  [temperature_norm]
    type = MFEML2Error
    variable = temperature
    function = 0
  []
[]

[Outputs]
  [HeatTransferCSV]
    type = CSV
    file_base = OutputData/HeatTransferWeakForm
  []
[]
