# Eigenvalue

!syntax description /Executioner/Eigenvalue

## Outputting every eigenvector id=output-all-eigenvectors

A solve converges up to [!param](/Executioner/Eigenvalue/n_eigen_pairs) eigenpairs, but by default only
the one selected by [!param](/Problem/EigenProblem/active_eigen_index) reaches the output. Set
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) to `true` to write one output step per
converged eigenvector instead, so that all of the modes land in a single Exodus file.

MultiApps and transfers are not re-executed per mode. They run once per solve, at the same execution
points as without the option. With the option, the executioner gives the output objects the
eigenvector time below as their output time instead of the time step number; the problem stays
non-transient for every other object. When a solve yields no
converged eigenpair, for example with [!param](/Problem/EigenProblem/solve) set to `false`, the
single output step is written as without the option.

### Order of operations per mode id=per-mode-order

After each solve, the executioner performs these steps for every converged eigenvector $i$:

1. Load eigenvector $i$ into the solution.
2. Only when [!param](/Executioner/Eigenvalue/normalization) is set: execute the
   [!param](/Executioner/Eigenvalue/normalization_dependencies) and the normalization
   post-processor by name, and scale the eigenvector (see [#eigenvector-normalization]).
3. Execute the `TIMESTEP_END` objects.
4. Write one output step.

The `LINEAR` objects do not execute per mode. Aux variables and post-processors that execute on
`TIMESTEP_END`, such as [NodalExtremeValue.md], therefore report on the mode of that step. After the
last mode the executioner reloads the eigenvector selected by
[!param](/Problem/EigenProblem/active_eigen_index) and normalizes it with the
[!param](/Executioner/Eigenvalue/normalization) hook as after the solve, which can execute the
`LINEAR` objects. Then it executes the `TIMESTEP_END` objects, without an output step. `FINAL`
objects, transfers, and any following fixed-point iteration thus see the same solution as without
the option. The `TIMESTEP_END` objects run once per converged mode and once more
for the restore, so a post-processor that accumulates on `TIMESTEP_END`, such as a running sum or a
counter, advances on each run instead of once per solve.

### Scaling of each mode id=eigenvector-normalization

With the option, the [!param](/Executioner/Eigenvalue/normalization) post-processor and
[!param](/Executioner/Eigenvalue/normal_factor) scale every output mode, not only the active
eigenvector. After loading mode $i$, the executioner executes the post-processors listed in
[!param](/Executioner/Eigenvalue/normalization_dependencies) and then the normalization
post-processor, by name. It scales the mode and evaluates them again until the normalization
post-processor equals the target: [!param](/Executioner/Eigenvalue/normal_factor), or
$1/|\lambda_i|$ when [!param](/Executioner/Eigenvalue/normal_factor) is not set. Without a
[!param](/Executioner/Eigenvalue/normalization) post-processor, each mode is written as the eigen
solver returns it, and the magnitudes are not comparable between steps.

Nothing else executes before a mode is scaled: neither the `LINEAR` objects nor the aux kernels. A
normalization post-processor or listed dependency that couples an aux variable is therefore an
error, and a dependency on an object that is not listed in
[!param](/Executioner/Eigenvalue/normalization_dependencies) gives a warning. Each listed
post-processor has to execute on `linear`, and
[!param](/Executioner/Eigenvalue/normalization_dependencies) requires
[!param](/Executioner/Eigenvalue/normalization) and
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) set to `true`. To mass-normalize every
output mode, set [!param](/Executioner/Eigenvalue/normalization) to an [EigenvectorBNorm.md]
post-processor and [!param](/Executioner/Eigenvalue/normal_factor) to 1, as the input below does.

### Output time of each mode id=eigenvector-time

[!param](/Executioner/Eigenvalue/eigenvector_time) chooses what the output time of step $i$ is: the
time step number (`index`, the default, as when a single eigenvector is written), the real part
$\lambda_i$ of the eigenvalue (`eigenvalue`), or $\sqrt{\lambda_i}$ (`sqrt_eigenvalue`, which errors
on a negative eigenvalue). For a vibration problem $K\phi = \omega^2 M\phi$, $\sqrt{\lambda_i}$ is the
angular frequency $\omega_i$ in 1/s; it serves as the output time coordinate and is not a physical
time. The imaginary part of a complex eigenvalue is never used as a time; it remains available
through the [Eigenvalues.md] vector post-processor. Setting
[!param](/Executioner/Eigenvalue/eigenvector_time) without
[!param](/Executioner/Eigenvalue/output_all_eigenvectors) set to `true` is an error, since a single
eigenvector is written at a single output time. The `FINAL` output step is written at the time of
the active eigenvector's step in the last solve, so it stays on the same time axis as the mode steps.

The `index` time is the one-based index $i+1$ in the first solve and keeps increasing over the mode
steps of every adaptivity cycle; with one adaptivity step and five modes the times are 1 to 10. With
an eigenvalue time axis, each adaptivity cycle writes its modes at that cycle's eigenvalues, so the
times rise within a cycle and restart at the next one. Exodus starts a new file for each mesh, and CSV
keeps every row.

The two eigenvalue time axes do not always give one distinct time per step. Degenerate eigenvalues
give repeated times: Exodus accepts repeated times and keeps every step, but CSV output drops the
repeated row, so use `index` when the post-processor history per mode matters. A
[!param](/Executioner/Eigenvalue/which_eigen_pairs) that does not order the eigenvalues ascending,
such as `largest_magnitude`, gives decreasing or otherwise non-monotonic times.

!listing test/tests/problems/eigen_problem/output_all_eigenvectors/all_modes.i block=Executioner

!syntax parameters /Executioner/Eigenvalue

!syntax inputs /Executioner/Eigenvalue

!syntax children /Executioner/Eigenvalue
