# CrackTipEnrichmentStressDivergenceTensors

The displacement field consists of the summation of the standard displacements and the near-tip enrichment (when near-tip enrichment is enabled). The `CrackTipEnrichmentStressDivergenceTensors` kernel computes the contribution of the enrichment variables to the residual and Jacobian. This is only applicable for small-strain calculations.

Both a non-AD version (`CrackTipEnrichmentStressDivergenceTensors`) and an AD version
(`ADCrackTipEnrichmentStressDivergenceTensors`) are provided by the same underlying template
class `CrackTipEnrichmentStressDivergenceTensorsTempl<bool is_ad>`. The non-AD version is the
default and computes the Jacobian analytically using `_Jacobian_mult`. The AD version computes
the Jacobian automatically via MOOSE's automatic differentiation system and does not require
`_Jacobian_mult`.

When using `XFEMAction`, the non-AD version is selected by default. Set `use_AD = true` in
the action to use the AD version instead.

## Example Input Syntax

!syntax description /Kernels/CrackTipEnrichmentStressDivergenceTensors

!syntax parameters /Kernels/CrackTipEnrichmentStressDivergenceTensors

!syntax inputs /Kernels/CrackTipEnrichmentStressDivergenceTensors

!syntax children /Kernels/CrackTipEnrichmentStressDivergenceTensors

## AD Version

!syntax description /Kernels/ADCrackTipEnrichmentStressDivergenceTensors

!syntax parameters /Kernels/ADCrackTipEnrichmentStressDivergenceTensors

!syntax inputs /Kernels/ADCrackTipEnrichmentStressDivergenceTensors

!syntax children /Kernels/ADCrackTipEnrichmentStressDivergenceTensors
