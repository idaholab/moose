# Eigenvalue

This [/Executioner.md] solves a standard/generalized linear or nonlinear eigenvalue problem.

## Outputting every eigenvector id=output-all-eigenvectors

A solve converges up to [!param](/Executioner/Eigenvalue/n_eigen_pairs) eigenpairs, but by default only
the one selected by [!param](/Problem/EigenProblem/active_eigen_index) reaches the output. Set
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) to `true` to write one output step per
converged eigenvector instead, so that all of the modes land in a single Exodus file.

Each step loads eigenvector $i$ into the solution, normalizes it, and executes the `LINEAR` and
`TIMESTEP_END` objects before the output is written, so aux variables and post-processors such as
[NodalExtremeValue.md] report the mode of that step. The `TIMESTEP_END` objects therefore run once per
converged mode, and once more for the restore at the end of the loop, so a post-processor that
accumulates on `TIMESTEP_END`, such as a running sum or a counter, advances once per mode instead of
once per step. MultiApps and transfers are not re-executed per mode; they run once, as they do with
the flag off. After the last mode the executioner reloads the eigenvector selected by
[!param](/Problem/EigenProblem/active_eigen_index), so `FINAL` objects, transfers, and any following
fixed-point iteration see the active eigenvector in the solution. The `FINAL` output step is written
at the eigenvector time of that active mode, so it stays on the same time axis as the mode steps.

[!param](/Executioner/Eigenvalue/eigenvector_time) chooses what the output time of step $i$ is: the
one-based index $i+1$ (`index`, the default, which keeps the time equal to the time step number as on
the single-eigenvector path), the real part $\lambda_i$ of the eigenvalue (`eigenvalue`), or
$\sqrt{\lambda_i}$ (`sqrt_eigenvalue`, which errors on a negative eigenvalue). The imaginary part of a
complex eigenvalue is never used as a time; it remains available through the [Eigenvalues.md]
vector post-processor. Setting `eigenvector_time` without `output_all_eigenvectors = true` is an
error, since a single eigenvector is written at a single output time.

The two eigenvalue time axes do not always give one distinct time per step. Degenerate eigenvalues
give repeated times: Exodus accepts repeated times and keeps every step, but CSV output drops the
repeated row, so use `index` when the post-processor history per mode matters. A
[!param](/Executioner/Eigenvalue/which_eigen_pairs) that does not order the eigenvalues ascending,
such as `largest_magnitude`, gives decreasing or otherwise non-monotonic times.

Each output mode is scaled by the existing normalization hook, so
[!param](/Executioner/Eigenvalue/normalization) and [!param](/Executioner/Eigenvalue/normal_factor)
apply per mode rather than to the active eigenvector alone. Combining
`normalization` with [EigenvectorBNorm.md] and `normal_factor = 1` mass-normalizes every mode, so that
$\phi_i^{\top} B \phi_i = 1$ in every output step. Without a `normalization` post-processor each mode
keeps the scaling SLEPc returned for it and the magnitudes are not comparable between steps.

!listing test/tests/problems/eigen_problem/output_all_eigenvectors/all_modes.i block=Executioner

!syntax parameters /Executioner/Eigenvalue

!syntax inputs /Executioner/Eigenvalue

!syntax children /Executioner/Eigenvalue
