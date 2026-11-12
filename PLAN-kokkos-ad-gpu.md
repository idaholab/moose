# Handoff: Kokkos AD GPU work on PR #34088 (arco, updated 2026-11-12)

Status: branch at `c9d341fd322` (steps 0-10b). Next: step 12. Updated 2026-11-12.
Git tracks this document on the feature branch. Commit it alone with a `PLAN:` subject, and drop
those commits before the merge. Relative paths outside the MOOSE tree, such as `profiles/`,
`scripts/`, `patches/`, `ptx/`, `logs/`, `runs/`, and `archive/`, are under `moose-stack-build/`
(`/ibmstor/scratch/lindad/projects/moose-stack-build`, untracked).
Pre-trim copy of this document, with all per-step tables: `archive/HANDOFF-pre-trim-2026-11-10.md`.

## Where things stand

Branch `kokkos-reference-configuration` in `/ibmstor/scratch/lindad/projects/moose`. The draft PR
is idaholab/moose#34088. NamjaeChoi left a performance review on it. Answer it with measurements
once the design work below is final. Do not reply yet.

**Next: step 12**, finish the reply to NamjaeChoi. First action: read
`moose-stack-build/review-reply-34088.md` and list its `TODO(user)` markers. Then:

1. Ask the user whether to remove the aliases in this PR (`index.md:113`).
2. Draft the p-refinement scratch sizing issue with the `issue-create` skill. File it only after
   approval.
3. Fill the `TODO(user)` markers with the step 8-10b results. Add step 8, 8d and later rows to the
   step tables from `profiles/nsys-v19` (`step_table.sh`, `launch_compare.py`).

Exit: the user approves the reply text. Do not post it without approval.

The draft reply is `moose-stack-build/review-reply-34088.md`. It answers all 10 inline comments
(`gh api repos/idaholab/moose/pulls/34088/comments`) and holds the per-step time and ratio tables.
Its `TODO(user)` markers wait on steps 8d-12.

Kokkos ignores `--compute-device`. This build compiles Kokkos for CUDA, so every Kokkos kernel runs
on the H200 under both `cpu` and `cuda`. The flag only selects specs (`compute_devices`). Test once,
with `build_and_test.sbatch <moose dir> <regex> cuda`. The OpenMP backend needs a non-GPU build.

Working tree:

- `libmesh` and `petsc` submodule pointers differ from `HEAD`. They match the built stack. Keep
  them out of every commit.
- `test/tests/kokkos/profiling/` is untracked and not for upstream. It holds the profiling case.

Loop names (step 8b): profiles up to `nsys-v12` and `ncu-v11` use the old names. Old
`ResidualOnlyLoop` is `ResidualLoop`, old `ResidualLoop` (the AD `[J]` launch) is
`ResidualAndJacobianLoop`, and old `DisplacedResidualLoop` is `DisplacementSeededLoop`.

### Decisions

- Step 10b: land the qp slices if no AD launch is more than 3% slower than `nsys-v16` (user,
  2026-11-12). Met.
- Step 10: use a hand-rolled slice reduction over the group. Do not use a Kokkos
  `ThreadVectorRange` with vector length above 1. That needs a new team and group layout, which is
  too large for this PR (user, 2026-11-12).
- Step 8d: sweep R4 and T1, then land the faster one (user, 2026-11-11). T1 won.
- Pursue the best computer-science design, also when it is a large change, for example a change to
  the hook interface (user, 2026-11-10, step 8d).
- Commit a step when the AD Kokkos tests pass on the H200 and no launch is slower than before the
  step. Report a step that slows any variant to the user before committing it. Accepted with
  regressions open: step 2 (ad3sdisp_q64 J +4.6%), step 3 (ad3sdisp_q27 J +2.1%), step 4
  (ad3sdisp_q8 J +1.2%), step 5 ("Open performance items"), step 7 (AD +0-2.5%, node noise), and
  step 8 (ad3 reference J, fixed by step 8d), step 8d (ad8disp_q8 J +28%, fixed by step 8e; ad3
  reference `ResidualLoop` +8-13%), and step 8e (seeded J and AD `ResidualLoop` +4-9%).
- Step 8d: commit T1 with its regressions, then probe the ad8 displaced J in step 8e (user,
  2026-11-11).
- Step 8e: commit Y1 with its regressions and list them as open items (user, 2026-11-12). Not
  noise: a repeat run (`nsys-v14r`) agrees within 0.5%, and the 35 hand-coded launches, which Y1
  does not touch, vary by -0.8% to +1.6%.
- Step 8d: remove the per-thread stack. Reason: the stack writes ~4 GB per Jacobian at ad3_q27,
  and its cost depends on the carveout (step 8a). Rejected: an explicit carveout preference, and a cap
  on blocks per SM. Both depend on the GPU.
- Step 5: displacement derivatives outside the dual, in this PR. Width: optional class constant
  `max_ad_displaced_slots`, default `max_ad_slots + 3` (the position), trait `ad_displaced_slots`.
  Table-driven seeding, no width-based `if constexpr`. Only the chain rule after the hook takes a
  `bool displaced` template parameter.
- Step 7: compile-time `displaced` flag on the dispatch, carried as the constant member
  `Datum::_displaced`, with no template on `Datum` and no hook signature change. Rejected: a
  `noinline` device function for the displaced block, and a `displaced` template on `Datum`.
- Step 8b: rename the AD team loops in this PR. Step 8c: use libMesh#4575 for the PJFNK base-vector
  check in this PR.
- Step 6: do not rebase the strip-mining branch (`kokkos-ad-strip-mining`, `1be282ae343`). Keep it,
  unmerged. It regressed ad3disp_q64 2x, added bank conflicts, and conflicts with
  `dispatchADTeams` and the step 5 instantiations.

### Commits on this branch (newest first)

- `c9d341fd322` Step 10b: AD residual qp sum split in slices over idle group threads.
- `2709e503f9d` Step 9: AD groups aligned to half-warps when the launch has contraction planes.
- `168e488f70c` Step 8e: datum transform reset per AD qp.
- `4736f915264` Step 8d: AD datum out of local memory (T1).
- `be2f2ac9b8f` Step 8c: `set_reproducible_residual(false)` for Kokkos GPU residuals.
- `c256be61c09` Step 8b: AD team loops renamed.
- `dcf41163bd1` Step 8: AD loops compiled once per configuration.
- `7e2a5e19554` Step 7: hand-coded loops compiled once per configuration.
- `9616e1de950` Step 5: displacement derivatives out of the dual.
- `f0197421166` Step 4: two-stage Jacobian contraction.
- `73438adc679` Step 3: several elements or sides per team.
- `7a4b90ca7f9`, `4aaf4ab57fb` Step 2: residual-only launches with zero slots.
- `1b044c68151` Step 1: no displacement slots when no component is seeded.
- `e400804478f`, `4695f627a65` Step 0: device fixes.
- `e607057c751` Current-configuration geometry in plain reals. `09c46a01a6f` AD slots by
  registration index.

## Working notes

(empty)

## Step 10b result (`c9d341fd322`, 2026-11-12)

Qp slices for the AD residual (reply to NamjaeChoi's inner `parallel_reduce`). Groups are
hand-rolled sub-teams, so a Kokkos `ThreadVectorRange` reduce does not map onto them. With
`ADScratchLayout::residual_slices > 1`, one thread per (i, slice) sums a strided qp subset. The
partials go through the qp scratch after the Jacobian. One slice keeps the serial loop. Slices at
8 test functions: q8 1, q27 3 (R) or 4 (R&J), q64 8. AD tests 34/34 (job 13755). Sweep
`profiles/nsys-v19` vs `nsys-v16`: every AD launch within +2.9%.

- AD `ResidualLoop`: ad3_q64 2.086 -> 0.912 ms (now 0.73x hand-coded 1.241), ad3_q27 0.587 ->
  0.451, ad3disp/ad3sdisp_q64 -20%, ad8 displaced -4-5%. Largest increase: ad3sdisp_q8 +2.9%.
- R&J: ad3_q8 -13%, ad3_q64 -8% (reference R&J registers 96 -> 127, yet faster). Seeded J
  +1-2.5%. Hand-coded launches unchanged apart from 5 us `DirichletBC` quantization.
- Displaced `ResidualLoop` registers (base/v1/v2/v3): `KokkosADDiffusion` 91/93/104/94,
  `KokkosADMatDiffusion` 104/96/96/96 (`ptx/10b-*`). v2 put the one-slice store branch inside the
  slice loop. v3 splits the one-slice and sliced paths.
- Probes in `patches/step10/` (v1 always two barriers, `nsys-v17`; v2 one loop for both, `nsys-v18`;
  v3 is the commit).

## Step 9 result (`2709e503f9d`, 2026-11-12)

`ADTeamGroup` takes the `ADScratchLayout`. With contraction planes (`max_jdofs > 0`), a group of
more than 16 threads rounds up to a multiple of 16. Sweep `profiles/nsys-v16` vs `nsys-v14`, ncu
`profiles/ncu-v13` (before) and `ncu-v14` (after, first patch with the same Jacobian path). AD tests
34/34 (job 13694).

- Cause (`ncu-v13`): all 47.2M excess wavefronts were 28 stage 2 `LDS.64` loads. Two groups of 27
  shared a half-warp (16 doubles per wavefront). After: 0.00 load conflicts.
- ad3_q27 J 3.47 -> 2.67 ms, ad3sdisp_q27 seeded J 23.2 -> 17.9, ad3disp_q27 J 3.64 -> 3.22. No
  other launch changes beyond 5 us `DirichletBC` quantization.
- Dead ends: padding the group stride to 8 mod 16 (fixes stage 2, adds stage 1 store conflicts in a
  half-warp model). Aligning residual-only groups too (q27 `ResidualLoop` +8-15%: the 8 active
  residual threads per group span more warps, `nsys-v15`).
- Stage 1 stores still conflict (`st_conflicts` ~0.39M ad3_q27, 1.1M ad3sdisp_q27). Not measured
  as a cost.

## Step 8e result (`168e488f70c`, 2026-11-12)

`Datum::resetTransform()` at the start of `evaluateADTerms`. Sweep `profiles/nsys-v14` (Y1 patch,
same code as the commit apart from comments), against `nsys-t1`. AD tests 34/34 (job 13602).

- Cause: occupancy. ncu (`profiles/ncu-v12/ad8disp_q8`, at T1): 248 registers, 0 spills, block
  limit 2 by registers and 3 by shared memory (40.3 KB per 72-thread block). The S1b `_xyz(i) += u`
  select in `reinitTransform` put the whole datum, kept live by the qp cache, in registers. The
  `F_inv` and geometry gathers do not affect R&J. Without the `F_inv` gather, the reference seeded
  loop gets 112/616. Probes X1-X6 and Y1-Y2 in `patches/step8e/`, output `ptx/8e*/`.
- ad8disp_q8 J 3.49 -> 1.94 ms (166 registers). Faster: ad8_q8 J -12% (127 -> 162 registers),
  ad3sdisp_q64 seeded -11%, ad8sdisp_q8 seeded -7%, ad3 and ad3disp R&J -4-5%.
- Slower: see "Open performance items". The user accepted them (2026-11-12).
- Dead end: Y2, a reset only when `displaced()`. The ad8 reference R&J still has 163 registers.

## Step 8d sweep result (2026-11-11)

T1 is faster than R4 in almost every launch. Compare with `python3 -I scripts/launch_compare.py
profiles/nsys-v12/launches.txt profiles/nsys-t1/launches.txt 1.03` (per launch, old loop names
mapped). `step_table.sh profiles/nsys-t1` gives the per-variant sums.

- Exit targets met: ad3_q8 J 1.74 -> 1.48 ms, ad3_q27 J 5.03 -> 3.66 ms (targets 1.63, 4.60).
- Large gains: the displaced `ResidualLoop` and hand-coded displaced loops 3-4x faster.
  ad3disp_q27/q64 J 6.60/9.41 -> 3.74/6.28. Seeded J 10-15% faster. The material
  `ElementCompute` 0.41 -> 0.13 ms (REG/STACK 172/1816 -> 108/88).
- Slower than v12, accepted by the user (2026-11-11), the ad8 displaced J to be probed in step 8e:
  - ad8disp_q8 `ResidualAndJacobianLoop` 2.72 -> 3.49 ms (+28%, reg 132 -> 248). R4: 2.53.
  - ad3 reference `ResidualLoop` +8-13% (reg 48 -> 54). R4 has it too, so it comes from the
    shared R4 part (N or the forced inlining), not from S1.
  - ad8 reference `ResidualLoop` +5% (56 regs, unchanged). `DirichletBC` launches +3-11% at 5 us
    (noise).
- R4 alone: the material `ElementCompute` slowed 1.8x (0.41 -> 0.74 ms) with fewer registers and
  less stack (134/808). Likely the step 8a carveout effect (unverified). R4 is rejected.
- Builds: R4 and T1 pass the AD tests (34/34, jobs 13448, 13504). g++ rejected the first T1
  (`Real3 _rz_radial_direction = 0`, two conversions). The patch now drops the initializer.

## Step 8d findings (probes, no commit)

Probe table, patch descriptions, and REG/STACK per probe: `archive/step8d-findings.md`. Patches in
`patches/step8d/`, output in `ptx/8d-*`.

- Cause of the reference frame: `ADDatumBase(datum)` copied the ~550 B `AssemblyDatum`, which
  stayed in local memory. Fix M builds the AD datum in place.
- Do not forward `libMesh::invalid_uint` by reference into device code. Pass
  `static_cast<unsigned int>(libMesh::invalid_uint)`.
- Dead ends: the runtime dual index (A, B), maps held by value (F), forced inlining with the copy
  (L), a direct qp loop (R1: outlines `evaluateADTerms`), one `assembleADTerms` instantiation per
  configuration (R3: equals N), the RZ `constexprIndex` (K: adds code to every geometry call), and
  composition (`ADDatumBase` holding `Base &`: changes every accessor and hook for one indirection).
- NVVM IR dump: cicc's front end consumes every `-X...` option, so LLVM options do not reach the
  optimizer.

## User decisions on the draft reply (2026-11-10)

- Fix the 27-qp contraction-plane bank conflict in this PR (step 9, done).
- Measure an inner `parallel_reduce` over qps before replying (step 10).
- Open an issue for p-refinement scratch sizing: one high-order element sizes the scratch of every
  team (`Assembly::getMaxQpsPerElem()`). Reference the issue in the reply (step 12).
- Aliases: NamjaeChoi asked to remove them (`index.md:113`). The user has not decided whether to
  remove them in this PR (ask in step 12).

## Step results that the code does not record

- Step 8a (no commit): the ad3_q27 J regression of step 8 (4.60 -> 5.06 ms) came from L1 size. Fewer
  registers (128 -> 91) let a sixth block fit per SM. The driver then picked the 228 KB
  shared-memory carveout instead of 164 KB, so L1 per SM fell from ~92 KB to ~28 KB. A 164 KB
  carveout hint (`scripts/launch_shim.c`) restores 4.62 ms. Padding to 5 blocks at 228 KB is slower
  (5.53), so the lever is L1 size, not blocks per SM. L2 read misses do not change. The stack
  writes do: 87382 blocks x 81 threads x 584 B = 4.13 GB, which matches the DRAM writes (4.15 GB).
  Kokkos sets the carveout only for a `DesiredOccupancy` policy. Not probed: ad3_q8 J (+6.5%).
  Profiles: `profiles/probe-8a*`.
- Step 8 (`dcf41163bd1`): reference AD residual 0.49 -> 0.12 ms (ad3_q8), 2.23 -> 0.51 (q27), now
  within 1.2x of the hand-coded one. `DisplacementSeededLoop` also takes both instantiations,
  because a reference-configuration object selects it when it reads an AD property that depends on
  seeded displacements (`ADSlotSetup::assign`). So its doc comment ("an object posed on the current
  configuration") is wrong. `ADMaterial` is unchanged: its loops build a plain `Datum` from the
  runtime `_configuration`.
- Step 7 (`7e2a5e19554`): reference hand-coded loops about 3x faster. The AD datum must read
  `Datum::displaced()`. With `_configuration.displaced()` nvcc could not fold the two flags
  (122/648 -> 168/1296). The `reference_*` copies in `reinitTransform` cost nothing where nothing
  reads them. Dead end: an unrolled component loop adds registers and stack.
- Step 5 (`9616e1de950`): the true slots and the scratch do not change, only the dual of the
  displaced instantiation. `ADDatumBase` takes an optional `ADChainRecord<slots> *`, and the
  pointer is the mode flag. Never record an address inside the functor: on CUDA it can force a
  thread-local copy of the whole functor. The default `max_ad_displaced_slots` can be too small
  where the old width was enough, e.g. an unseeded gradient accessor. `unit/src/KokkosADSlotTest.K`
  calls `_u.slot()`, which no accessor has: the unit app is broken and not in the test regex.
- Steps 0-4: the device functor comes from the explicit `ResidualObject` copy constructor
  (`KokkosResidualObject.K`). A device-read member must be listed there. A missing `_ad_layout` once
  left `qp_chunk = 0`, and every AD test hung. `DispatcherBase::teamSizeMax()` builds its policy
  with `AUTO`, because the OpenMP backend aborts on a team size above its thread pool.

### Open performance items

- ad3sdisp_q8 Jacobian (12.96 ms at step 5): 6 groups of 8 threads per team. Shared memory (37.7 KB
  per team) and registers both cap it at 6 teams per SM (`profiles/ncu-v10`). 16 lanes idle.
- Step 5 regressions (v9 -> v10): `KokkosADQuadraticMaterial` `ElementCompute` +15% in every ad8
  variant (lead, unverified: the narrow path of `ADMaterialPropertyValue::operator=` goes through
  `store()` and holds a datum reference). ad8disp_q8 J +9% and AD `ResidualLoop` +1-7%, causes
  unknown.
- ad3disp_q27 R&J is 20% slower than ad3_q27 (3.20 vs 2.66 ms, `profiles/ncu-v15`). DRAM reads
  double (0.88 -> 1.74 GB): the precomputed qp values and gradients of 3 displacements are 12 reals
  per qp (0.68 GB), and the L2 hit rate falls 55 -> 34%. Registers 96 -> 120 cut blocks/SM 6 -> 5.
  Unmeasured lever: form `grad u` from the 8 nodal DOFs per element instead of the qp arrays.
- AD materials compute derivatives in residual-only evaluations. A plain material can read the
  stored `_ad_derivatives` (`KokkosADDerivativeSlotMaterial`, test `ad_material/values`), so a skip
  must know whether an active consumer reads them. Separate change.
- The reference hand-coded loops are 8-20 registers above devel (`KokkosDiffusion` `ResidualLoop`
  72/240, devel 64/240). The precompute hooks of `122f328ca17` are a likely cause. Unmeasured.
- Step 8e regressions (`nsys-t1` -> `nsys-v14`, reproduced): ad3sdisp_q8 seeded J +9.2% (10.36 ->
  11.32 ms, 252 -> 167 registers), ad3sdisp_q27 seeded J +5.0%, ad3_q64 `ResidualLoop` +6.2% (54 ->
  48 registers), ad8sdisp_q8 and ad8disp_q8 `ResidualLoop` +5.1% and +4.1% (96 -> 104 registers).
  Unverified hypothesis: shared memory already caps the seeded loops, so fewer registers add no
  blocks, and the new instruction schedule costs the difference. Check with ncu on ad3sdisp_q8.

### Design directions (after steps 9-10, not started)

Registers and shared scratch both grow with the slot count, so trading one for the other moves
the binding limit. Reducing the live data per thread can relieve both. Step 8e is an example.

- Slim the datum: `AssemblyDatum` holds ~550 B (geometry cache, `reference_*` copies, `F_inv`).
  Check the PTX for what stays live, since the compiler drops what it can prove dead.
- Fewer full-width duals live at once: `evaluateADTerms` holds four (`f[term_size]`). Step 5
  (`9616e1de950`) added the array on purpose: the displaced chain rule reads all four terms after
  the hooks. Before step 5, each term went to scratch as soon as it was computed. A reference
  instantiation could store each term at once again, but the gradient hook returns three terms
  together, so the gain is limited to the value term. Unmeasured.
- Slots split across threads: step 6 strip-mining (`1be282ae343`). Rejected because ad3disp_q64
  ran 2x slower, shared-store bank conflicts appeared, and the branch conflicts with
  `dispatchADTeams` and the step 5 instantiations. A redesign on the current code is untested.

## Performance plan: remaining steps

Measure each step alone against the step before, with a full sweep on arco-c04.

8d. Done (`4736f915264`, sweep `profiles/nsys-t1`). See "Step 8d sweep result".
8e. Done (`168e488f70c`, sweep `profiles/nsys-v14`). See "Step 8e result".
9. Done (`2709e503f9d`, sweep `profiles/nsys-v16`). See "Step 9 result".
10. Done (measurement probes v1, v2, no commit).
10b. Done (`c9d341fd322`, sweep `profiles/nsys-v19`). See "Step 10b result".
11. Done (trim of this document, 2026-11-10).
12. Finish the reply. Draft the p-refinement issue with the `issue-create` skill (file it only
    after approval). Ask about alias removal. Fill the `TODO(user)` markers with the step 8-10
    results. The reply has the step tables up to step 7. Add step 8 and 8d rows from a new sweep.

Out of scope: each matrix entry costs a CSR search (`Matrix::find`) and an atomic, in hand-coded
and AD kernels alike. A per-pattern map from element entries to value indices would help both.
Open a separate issue for it.

## Review findings for the reply to NamjaeChoi

- Comment 1 (scratch size limits occupancy): not the limiter up to 15 slots with one element per
  team. Registers were. With several elements per team (step 3), scratch caps the group count of
  the wide kernels. After step 5, scratch limits the seeded displaced Jacobians as much as
  registers, or more (`profiles/ncu-v10`: 6 blocks/SM by registers and 5-6 by shared memory).
- Comment 2 (bank conflicts of `[qp][entry]`): zero excess wavefronts. `qpSize` is always odd, and
  an odd stride in doubles is conflict-free. The C planes conflicted at q27 only. Step 9 fixed it.
- Comment 3 (short qp loops, `AUTO` team size): real. At q8 the `AUTO` team of 128 threads idled
  120 of them during the hook evaluation. Step 3 fixed it: ad3 q8 residual 2.04 -> 0.48 ms.
- Main finding: register pressure from dense duals, with spills in the displaced path. Also,
  residual-only launches computed full duals (step 2), and auxiliary displacements carried 12 dead
  slots (step 1).
- State which hand-coded baseline each ratio uses: before step 7 or after it.
- Non-performance comments still open: alias removal, the `Configuration` name, the `Coupleable.h`
  question, and the `KokkosADGeometry.h` placement (step 5 deletes the header).

## Known failures not caused by this work

Details: `moose-stack-build/PLAN-kokkos-known-failures.md`.

- `kokkos/problems/eigen_problem.test` and `eigen_problem.petsc_gpu`: libMesh was built without
  SLEPc detection.
- `kokkos/materials/parsed.parsed_material`: a CPU-only test that fails on CPU in a GPU build.

## Profiling setup

- Case: `test/tests/kokkos/profiling/pr34088/`. `ad_assembly.i` is a 64^3 HEX8 diffusion problem
  with 2 Newton steps of 5 linear iterations each. Options: `n`, `order`/`elem`, `qorder` (SECOND =
  8, FOURTH = 27, SIXTH = 64 qps), the kernel type, and `use_displaced_mesh`.
- Variants: `adN` is the kernel's `max_ad_slots` (3 for `KokkosADDiffusion`, 8 for
  `KokkosADMatDiffusion`). `hand*` is `KokkosDiffusion`. `disp` runs on the displaced mesh with
  auxiliary displacements. `sdisp` merges `seeded_displacements.i`, which makes them nonlinear.
  `qM` is the qps per element.
- Sweep, from the case directory: `sbatch -w arco-c04 --output=<log dir>/%x-%j.log sweep.sbatch
  <out dir> nsys|ncu`. Filter with `VARIANTS_FILTER`, `NCU_KERNELS`, `NCU_COUNT`. Latest full sweep:
  `profiles/nsys-v19` (step 10b). Latest ncu: `ncu-v15`. Next: `nsys-v20`, `ncu-v16`.
  Build `launches.txt` with `launch_table.sh <dir>/*.nsys-rep`.
- Tables in `scripts/`: `step_table.sh <dir>` (per-step summary), `launch_table.sh <nsys-rep>...`
  (every launch, tagged `[R]`/`[J]`), `kernel_table.sh`, `ncu_table.sh`, `ncu_shared_stores.py`,
  and `ncu_metrics.sh <metric,...> <ncu-rep>...`.
- `scripts/occupancy_probe.sbatch <out> <variant regex> <label>[:VAR=val,...]...` runs the sweep
  under `scripts/launch_shim.c` (LD_PRELOAD). The shim matches a launch by dynamic shared memory
  and block size, then pads the shared memory or sets the carveout hint.
- Pass `NCU_KERNELS` through the environment: `sbatch --export=ALL,NCU_KERNELS=a,b` splits at the
  comma. Reference AD Jacobian: `ResidualAndJacobianLoop.*KokkosADDiffusion`. ncu needs
  `--kernel-name-base demangled`. It aborts in CUDA teardown after writing the report (harmless).
- Static register counts: `cuobjdump -res-usage <lib>.so | c++filt`. `ptxas -v` does not work on
  the libraries. SASS `STL`/`LDL` counts include stack arrays. For spills, use ncu
  `derived__local_spilling_requests`.
- `regs_variants.sbatch` covers only the framework library. Probe test-app kernels, such as
  `KokkosADQuadraticMaterial`, with `object_variants.sbatch`. On a test-app source it needs
  `runs/branch-exp/test/build/header_symlinks` (created by hand, one link per `test/include` header).
- `scripts/patch_build_test.sbatch <moose dir> <reverse patch|-> <patch|-> <regex> [devices]` applies
  a patch and runs `build_and_test.sbatch`. `scripts/launch_compare.py <before> <after> [min ratio]`
  compares two `launch_table.sh` outputs per launch and maps the old loop names.
- Probes: `scripts/regs_variants.sbatch <worktree> <kernel regex> base|<patch>...` rebuilds the
  whole library per patch. `scripts/object_variants.sbatch <wt> <src .K> <regex> <out root>
  base|<patch>...` builds one object per patch through `scripts/kokkos_object_ptx.sbatch` (PTX with
  `.loc` lines, SASS, res usage). PTX entries are `.weak .entry`. Scratch worktree:
  `runs/branch-exp`, at `2709e503f9d`, clean. It has empty `libmesh`/`petsc`, so it serves object
  probes only. Build the app in the main tree. Step 8d probe trees: `/tmp/pe-x` (login node, one git branch
  per probe). It can disappear. The patches in `patches/step8d/` are the record. Write probe patches in a copy (`git archive
  be2f2ac9b8f framework/include/kokkos` into `/tmp/pe`, then `git init`), never in the worktree
  while a job uses it. Queue jobs on the worktree with `--dependency=singleton
  --job-name=object-variants`. `~/.claude/skills/gpu-local-memory/scripts/ptx_local_sites.py <ptx> <entry regex>` (skill `gpu-local-memory`) lists the
  runtime-indexed local sites of one entry. Check `.func` and `call.uni` in the PTX as well.
- `scripts/hang_stack.sh <exe> <input dir> <delay s> -- <args>` prints the thread stacks of a run
  that is still running after the delay.

## Build environment

See the auto-memory `arco-hpc-moose-build`. In short:

- No conda. The stack is a CUDA build on arco. Modules: `cuda/13.3.1 openmpi/5.0.10_gcc14.3.1`.
- `sbatch -A employee --wckey=neams -p general --gres=gpu:N`. Do not pin builds to a node, and keep
  the 120G default of `build_and_test.sbatch`: 60G at 24 nvcc jobs is OOM-killed.
- Linking needs `LIBRARY_PATH=/ibmstor/scratch/lindad/projects/moose-stack-build/linkdir`.
- `scripts/build_and_test.sbatch <moose dir> <test regex> cuda` rebuilds the opt test app and runs
  the tests. `BUILD_METHOD=dbg` and `REPEAT=n` select dbg and repeat. The login shell exports
  `METHOD=dbg`, so scripts must not read `METHOD`. Write logs to `moose-stack-build/logs/`.
- AD regex: `'kokkos.*(ad_|current_configuration|function_dirichlet)'`. Expected: 33 pass with
  `cuda`. Whole `kokkos` dir: 189 pass, only the known failures. Filter logs for ` error`,
  `FAILED`, and `Ran `.
