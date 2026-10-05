!include momentum-pressure-block-restricted.i

[FVGradientMethods]
  [reconstructed]
    type = FVReconstructedPressureGradient
    base_gradient_method = green-gauss
    gradient_relaxation = 0.5
  []
[]
