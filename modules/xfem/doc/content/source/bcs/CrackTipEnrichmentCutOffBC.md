# CrackTipEnrichmentCutOffBC

MOOSE permits variables to be restricted to specific blocks, but currently does not provide a straightforward way to add variables to just the enriched region near the crack tip. An inefficient workaround is to add new variables associated with the enrichment shape functions for the whole mesh and then to simply fix DOFs to zero for those nodes with basis function supports that are far away from any crack tip.

## Example Input Syntax

!syntax description /BCs/CrackTipEnrichmentCutOffBC

!syntax parameters /BCs/CrackTipEnrichmentCutOffBC

!syntax inputs /BCs/CrackTipEnrichmentCutOffBC

!syntax children /BCs/CrackTipEnrichmentCutOffBC
