# ElementQualityCriterion

!syntax description /Remeshing/Criteria/ElementQualityCriterion

## Description

The measured quantity is the smallest value of the libMesh quality metric selected by
[!param](/Remeshing/Criteria/ElementQualityCriterion/quality_metric) over the active elements. The
criterion fires when that minimum, reduced over all ranks, falls below
[!param](/Remeshing/Criteria/ElementQualityCriterion/threshold). The metric names are the libMesh
quality metrics, the same set [ElementQualityChecker.md] accepts, and each one has its own range and
its own sense of what a good element scores, so the threshold has to be read against the chosen
metric.

The quality is measured on the displaced mesh when
[!param](/Remeshing/RemeshingAction/displacements) names the displacement variables of the problem,
and on the reference mesh otherwise. Measuring the displaced mesh is what lets a deformation drive
remeshing in a problem whose reference mesh never moves.

The metric here decides when a replacement is attempted; the metric of the remesher decides which
elements it then replaces. The two are independent, so a criterion that fires at a quality the
remesher still accepts triggers a replacement in which the remesher finds nothing to do.

## Example Input File Syntax

!listing test/tests/remeshing/true_disp_quality_remesh.i block=Remeshing

!syntax parameters /Remeshing/Criteria/ElementQualityCriterion

!syntax inputs /Remeshing/Criteria/ElementQualityCriterion

!syntax children /Remeshing/Criteria/ElementQualityCriterion
