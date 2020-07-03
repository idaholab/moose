# Bond Status Convergence Postprocessor

## Description

The `BondStatusConvergedPostprocessorPD` Postprocessor computes the number of peridynamic bonds that have changed status during a given Picard iteration. The bond status is held constant during the nonlinear iterations, and after they have converged, bonds are allowed to be considered failed if they meet certain criteria. Every time new bonds fail, the solution should be repeated to check if additional bonds would then fail. This Postprocessor is designed to be used in conjunction with the `PicardSolve` executioner as a criterion for convergence of those Picard iterations that repeatedly find an updated nonlinear solution after additional bonds fail.

Using the results of this Postprocessor for a convergence criterion, the solution is  considered as converged if both the nonlinear solution tolerance has been reached and the number of bonds with updated status at the end of the time step falls within a given tolerance or the maximum allowed Picard iterations have been reached.

!syntax parameters /Postprocessors/BondStatusConvergedPostprocessorPD

!syntax inputs /Postprocessors/BondStatusConvergedPostprocessorPD

!syntax children /Postprocessors/BondStatusConvergedPostprocessorPD
