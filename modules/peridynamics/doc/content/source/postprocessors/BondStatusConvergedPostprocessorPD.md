# Bond Status Convergence Postprocessor

## Description

The `BondStatusConvergedPostprocessorPD` Postprocessor in combination with PicardSolve executioner provides a bond-status-based convergence criterion in addition to the MOOSE default primary-variable-based convergence criterion for nonlinear steps to check bond breakage convergence for peridynamic fracture problems. Using this for convergence criterion, a nonlinear iteration is considered as converged if the number of updated bond status at the timestep end falls within a given tolerance or reach the maximum allowed Picard iterations.

!syntax parameters /Postprocessors/BondStatusConvergedPostprocessorPD

!syntax inputs /Postprocessors/BondStatusConvergedPostprocessorPD

!syntax children /Postprocessors/BondStatusConvergedPostprocessorPD
