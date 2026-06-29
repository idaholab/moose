# ComputeCrackTipEnrichmentIncrementalStrain

!syntax description /Materials/ComputeCrackTipEnrichmentIncrementalStrain

# Description

With XFEM, the displacement field contains both the standard and optionally, the near-tip enrichment solution. This Material calculates the strains for near-tip enrichment from both the standard and enrichment solutions. This model is incremental, which allows for the use of nonlinear material models, but is applicable only to small-strain problems. This is a drop-in replacement for the standard strain calculators.

!syntax parameters /Materials/ComputeCrackTipEnrichmentIncrementalStrain

!syntax inputs /Materials/ComputeCrackTipEnrichmentIncrementalStrain

!syntax children /Materials/ComputeCrackTipEnrichmentIncrementalStrain
