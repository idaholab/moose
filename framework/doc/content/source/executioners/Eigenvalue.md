# Eigenvalue

!syntax description /Executioner/Eigenvalue

## Outputting every eigenvector id=output-all-eigenvectors

A solve converges up to [!param](/Executioner/Eigenvalue/n_eigen_pairs) eigenpairs, but by default only
the one selected by [!param](/Problem/EigenProblem/active_eigen_index) reaches the output. Set
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) to `true` to write one output step per
converged eigenvector instead, so that all of the modes land in a single Exodus file.

Each step loads eigenvector $i$ into the solution, normalizes it, and executes the `LINEAR` and
`TIMESTEP_END` objects before the output is written, so aux variables and post-processors such as
[NodalExtremeValue.md] report on the mode of that step. The `TIMESTEP_END` objects therefore run once per
converged mode, and once more for the restore at the end of the loop, so a post-processor that
accumulates on `TIMESTEP_END`, such as a running sum or a counter, advances on each run instead of
once per solve. MultiApps and transfers are not re-executed per mode; they run once, as they do with
the flag off. After the last mode the executioner reloads the eigenvector selected by
[!param](/Problem/EigenProblem/active_eigen_index), so `FINAL` objects, transfers, and any following
fixed-point iteration see the active eigenvector in the solution. The `FINAL` output step is written
at the time of the active eigenvector's step in the last solve, so it stays on the same time axis as
the mode steps. When a solve yields no converged eigenpair, for example with
[!param](/Problem/EigenProblem/solve) set to `false`, the single output step is written as without
the option. The option also marks the problem as transient, because an output object otherwise
takes its time from the time step number. Any object that checks whether the problem is transient
therefore sees a transient problem.

[!param](/Executioner/Eigenvalue/eigenvector_time) chooses what the output time of step $i$ is: the
time step number (`index`, the default, as when a single eigenvector is written), the real part
$\lambda_i$ of the eigenvalue (`eigenvalue`), or $\sqrt{\lambda_i}$ (`sqrt_eigenvalue`, which errors
on a negative eigenvalue). The `index` time is the one-based index $i+1$ in the first solve and keeps
increasing over the mode steps of every adaptivity cycle; with one adaptivity step and five modes the
times are 1 to 10. The imaginary part of a complex eigenvalue is never used as a time; it remains
available through the [Eigenvalues.md] vector post-processor. Setting
[!param](/Executioner/Eigenvalue/eigenvector_time) without
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) set to `true` is an error, since a single
eigenvector is written at a single output time.

The two eigenvalue time axes do not always give one distinct time per step. Degenerate eigenvalues
give repeated times: Exodus accepts repeated times and keeps every step, but CSV output drops the
repeated row, so use `index` when the post-processor history per mode matters. A
[!param](/Executioner/Eigenvalue/which_eigen_pairs) that does not order the eigenvalues ascending,
such as `largest_magnitude`, gives decreasing or otherwise non-monotonic times.

Each output mode is scaled by the normalization hook, so
[!param](/Executioner/Eigenvalue/normalization) and [!param](/Executioner/Eigenvalue/normal_factor)
apply per mode rather than to the active eigenvector alone. Combining
`normalization` with [EigenvectorBNorm.md] and `normal_factor = 1` mass-normalizes every mode, so that
$\phi_i^{\top} B \phi_i = 1$ in every output step. Without a `normalization` post-processor each mode
keeps the scaling SLEPc returned for it and the magnitudes are not comparable between steps.

!listing test/tests/problems/eigen_problem/output_all_eigenvectors/all_modes.i block=Executioner

!syntax parameters /Executioner/Eigenvalue

!syntax inputs /Executioner/Eigenvalue

!syntax children /Executioner/Eigenvalue
