# Handoff: Kokkos AD GPU work on PR #34088 (arco, updated 2026-11-12)

Status: branch at `a94736bf66c` (steps 0-10b, 12-13c), pushed to the fork. Next: step 13e. Updated
2026-11-14.
Git tracks this document on the feature branch. Commit it alone with a `PLAN:` subject, and drop
those commits before the merge. Relative paths outside the MOOSE tree, such as `profiles/`,
`scripts/`, `patches/`, `ptx/`, `logs/`, `runs/`, and `archive/`, are under `moose-stack-build/`
(`/ibmstor/scratch/lindad/projects/moose-stack-build`, untracked).
Pre-trim copy of this document, with all per-step tables: `archive/HANDOFF-pre-trim-2026-11-10.md`.

## Where things stand

Branch `kokkos-reference-configuration` in `/ibmstor/scratch/lindad/projects/moose`. The draft PR
is idaholab/moose#34088. NamjaeChoi left a performance review on it. Answer it with measurements
once the design work below is final. Do not reply yet.

**Next: step 13e**: find the cause of the AD residual gap with ncu. AD `ResidualLoop` takes
1.40x the hand-coded time at hex8-q8 and 1.44x at hex27-q27 (`nsys-v24`). The team sizes match and
AD uses fewer registers (48 vs 54).
1. The main-tree build is the rejected 13c probe. Rebuild `a94736bf66c`:
   `build_and_test.sbatch <moose dir> '<AD regex>' cuda`. Expect 33 pass.
2. Add `handdisp_h27`, `handsdisp_h27`, `ad3disp_h27`, and `ad3sdisp_h27` to
   `test/tests/kokkos/profiling/pr34088/sweep.sbatch`. Copy the `h27` arguments, plus the `disp`
   or `sdisp` arguments. That file is untracked.
3. Run ncu on `ResidualLoop` for `hand_q8|ad3_q8|hand_h27|ad3_h27` into `profiles/ncu-v19`, with
   `NCU_KERNELS='ResidualLoop.*Kokkos(AD)?Diffusion'`. Compare instructions, shared and global
   load/store bytes, stall reasons, and achieved occupancy with `ncu_table.sh` and
   `ncu_metrics.sh`.
4. Hypotheses from code reading:
   - AD stores 4 terms + 1 pad per qp. The hand-coded path stores 3 + 1.
   - `contractADTerms` reads the test function value even when the value term is zero
     (`KokkosADDiffusion` has no value hook). The hand-coded path skips it (`has_value`).
   - The ratio falls with the qp count at 8 test functions (q8 1.40, q27 1.22, q64 1.14). This
     points to a cost per element, not per qp.
5. If the cause is clear and its fix is in the AD residual path (for example, skip absent terms at
   compile time), fix it and measure a full `nsys-v26` sweep.

Exit: a measured cause of the gap, recorded in this document. Then either a commit that closes the
gap at q8 and `h27`, or a measured reason why the gap is inherent.

The step 13d reply draft is done except for these steps. It must change after 13e-13h:
- Rename "hardware constants" (`KokkosADAssembly.h:36` section).
- Replace "cost of AD itself" with the 13e result.
- Lead with q8 and `h27` numbers.
- Remove bare "group".

The draft reply is `moose-stack-build/review-reply-34088.md`. A local git repository in
`moose-stack-build` tracks only that file (`.gitignore` allowlist). Commit the reply after each
turn that edits it, so the user can review the edits with `git diff`. `TODO(user)` marks sections
that wait on step 13d. Post nothing without user approval.

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

- Decision cases (user, 2026-11-14): report and accept or reject changes on hex8-q8 and hex27-q27
  (`*_q8` and `*_h27` variants) only. Use hex8-q27 and hex8-q64 only to check that nothing breaks
  or slows down badly. They do not decide a step. The `h27` sweep has only `hand_h27` and
  `ad3_h27`. Step 13e adds `disp` and `sdisp` `h27` variants.
- Vocabulary (user, 2026-11-14): never write a bare "group". Use Kokkos terms (team, thread,
  vector lane), libMesh/MOOSE terms (element, side, qp, test function), or "warp". Say "CUDA
  block" or "hardware thread" only for the profiled hardware, and always say whether a thread is
  a Kokkos thread or a hardware thread. The set of Kokkos threads of a team that assemble one
  element or side is an "element-group" in prose. Step 13i renames `TeamGroup` in the code. The
  proposed name is `ElemThreads`, because a libMesh side is also an `Elem`. Confirm the name with
  the user first.
- Team size (user, 2026-11-14): a fixed `target_team_size = 128` is not technically sound. Derive
  the team size from measurement or from an occupancy model (step 13f).
- Reply (user, 2026-11-14): do not call the AD residual gap "the cost of AD" until ncu shows the
  cause (step 13e). Do not call 128 or `half_warp` hardware constants. Steps 13e-13h come before
  the reply is posted.
- Step 13b4 (user, 2026-11-14): commit the `resetTransform` fix with the q8 reference J regression
  open. Carveout hints are acceptable if they are a general CS solution, not tuned to one GPU
  architecture. This revises the step 8d rejection.
- Step 13 (user, 2026-11-13): do steps 13a-13d. The reply must say the `PLAN:` commits are
  squashed before merge, and must give a technical reason for or against `ThreadVectorRange`, not
  a cost reason. The agent may push the branch to the fork at any time.
- Step 12 (user, 2026-11-12): remove the kernel aliases in this PR (step 12b). Rename
  `Moose::Kokkos::Configuration` to `Deformation`, and `ConfigurationLoop` to `DeformationLoop`
  (step 12c, user, 2026-11-13). Rejected: `MeshConfiguration`, because the mesh never moves. The reply says some reference
  Jacobian gap is expected for AD, and names no follow-up work.
- Step 10b: land the qp slices if no AD launch is more than 3% slower than `nsys-v16` (user,
  2026-11-12). Met.
- Step 10: use a hand-rolled slice reduction over the element-group. Do not use a Kokkos
  `ThreadVectorRange` with vector length above 1. That needs a new team and element-group layout, which is
  too large for this PR (user, 2026-11-12).
- Step 8d: sweep R4 and T1, then land the faster one (user, 2026-11-11). T1 won.
- Pursue the best computer-science design, also when it is a large change, for example a change to
  the hook interface (user, 2026-11-10, step 8d).
- Commit a step when the AD Kokkos tests pass on the H200 and no launch is slower than before the
  step. Report a step that slows any variant to the user before committing it. Accepted with
  regressions open: step 2 (ad3sdisp_q64 J +4.6%), step 3 (ad3sdisp_q27 J +2.1%), step 4
  (ad3sdisp_q8 J +1.2%), step 5 ("Open performance items"), step 7 (AD +0-2.5%, node noise), and
  step 8 (ad3 reference J, fixed by step 8d), step 8d (ad8disp_q8 J +28%, fixed by step 8e; ad3
  reference `ResidualLoop` +8-13%), step 8e (seeded J and AD `ResidualLoop` +4-9%), step
  13b3 (`KokkosCoefDiffusion` `JacobianLoop` +3-15%, fixed by 13b4 except q8), and step 13b4 (q8
  reference `JacobianLoop` +10-23%, user, 2026-11-14).
- Step 8d: commit T1 with its regressions, then probe the ad8 displaced J in step 8e (user,
  2026-11-11).
- Step 8e: commit Y1 with its regressions and list them as open items (user, 2026-11-12). Not
  noise: a repeat run (`nsys-v14r`) agrees within 0.5%, and the 35 hand-coded launches, which Y1
  does not touch, vary by -0.8% to +1.6%.
- Step 8d: remove the per-thread stack. Reason: the stack writes ~4 GB per Jacobian at ad3_q27,
  and its cost depends on the carveout (step 8a). Rejected then: an explicit carveout preference, and
  a cap on blocks per SM, because both depend on the GPU. The step 13b4 decision revises this.
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

- `a94736bf66c` Step 13b4: datum transform cache reset after each precompute qp.
- `d30dadacd08` Step 13b3: `Real33::identity` indexed at compile time, unread `Datum` members out.
- `ccbfc3288a5` Step 13b2: precompute loops on teams of element-groups.
- `e4c7b785f70` Step 13b: displacement Jacobian packs several elements per team (`TeamGroup`).
- `53584d78efd` Step 12c: `Configuration` renamed to `Deformation`.
- `98852e75c2f` Step 12b: Kokkos kernel and integrated BC aliases removed.
- `c9d341fd322` Step 10b: AD residual qp sum split in slices over idle Kokkos threads of the element-group.
- `2709e503f9d` Step 9: AD element-groups aligned to half-warps when there are contraction planes.
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

## Steps 12-13c results (summary)

Full results: `archive/step12-13c-results.md`. Per-launch numbers: `profiles/nsys-v24/launches.txt`.

- 13c (no commit, `nsys-v25`): qps over `ThreadVectorRange` lanes, rejected (every AD launch
  1.04-3.6x slower). Causes: CUDA caps a vector at 32 lanes, and NVVM outlines the vector closure as
  a `.func`, which moves the AD datum to a 680 B stack. Do not retry without a team layout that
  keeps several elements per team at 64 qps and inlines the vector loop. Patches: `patches/step13c/`.
- 13b4 (`a94736bf66c`, `nsys-v24`): `resetTransform` per precompute qp. Hand reference J 83 -> 63
  registers. Open: q8 reference J +10-23% (L1 carveout, see "Open performance items"). Dead ends:
  resetting only `_cached_qp`, or a `bool` validity flag. Writing constants to the cache is what
  ends the carry.
- 13b3 (`d30dadacd08`): `Real33::identity` indexed at compile time. Do not restore the runtime
  index: it put the outlined `DisplacementSeededLoop` datum in local memory.
- 13b2 (`ccbfc3288a5`, `nsys-v22`): every hand-coded precompute loop runs on element-groups through
  `assemblePrecomputedTerms`, and each qp is evaluated once. `LocalParallelInterface` was removed.
  Device code cannot bind `libMesh::invalid_uint` to a reference: copy it into a local first.
  Refresh `runs/branch-exp/framework/build/header_symlinks` after a header rename.
- 13b (`e4c7b785f70`, `nsys-v21`): the hand-coded displacement Jacobian packs several elements per
  team. Stage 1 stores four directions per qp, and stage 2 runs over (i, j).
- 13a (`nsys-up1` upstream `ffc574921d7`, `nsys-v20`): upstream AD/hand R 14-22x, J 90-191x.
  Upstream setup: worktree `runs/upstream-next`. Run `LIBMESH_DIR=<moose>/libmesh/installed
  ./configure --with-kokkos` there first. Build with `LIBMESH_DIR=...
  WASP_DIR=<moose>/framework/contrib/wasp/install build_and_test.sbatch`. Upstream has no
  displaced mesh support and no `KokkosADQuadraticMaterial`. Sweep it with
  `EXTRA_ARGS="Materials/inactive=k Mesh/use_displaced_mesh=false"`. Delete stale `.sqlite`
  exports before `nsys stats` on a rerun directory.
- 12 (no commit): filed idaholab/moose#34456 (p-refinement scratch sizing). `profiles/ncu-v16`
  holds seeded J occupancy (ad8sdisp_q8 4/4 blocks/SM by registers/shared, ad3sdisp_q8 6/6,
  ad3sdisp_q64 6/5). The unit app is broken (`unit/src/KokkosADSlotTest.K` calls `_u.slot()`).

## Steps 8d-10b results (summary)

Full results, probe lists, and per-launch numbers: `archive/step8d-10b-results.md`. Per-step
times and ratios: the reply tables (`review-reply-34088.md`).

- 10b (`c9d341fd322`, `nsys-v19`): residual qp sum in slices (`ADScratchLayout::residual_slices`).
  Every AD launch within +2.9% of `nsys-v16`. Probes in `patches/step10/`.
- 9 (`2709e503f9d`, `nsys-v16`, ncu `v13`/`v14`): half-warp-aligned element-groups with contraction planes.
  Dead ends: padding the element-group stride to 8 mod 16 (adds stage 1 store conflicts). Aligning
  residual-only element-groups too (q27 `ResidualLoop` +8-15%). Stage 1 store conflicts remain, unmeasured.
- 8e (`168e488f70c`, `nsys-v14`): `Datum::resetTransform()` per AD qp. Dead end: Y2, a reset only
  when `displaced()` (ad8 reference R&J keeps 163 registers). Probes in `patches/step8e/`.
- 8d (`4736f915264`, `nsys-t1`): T1 won against R4. R4 slowed the material `ElementCompute` 1.8x.
- 8d probes (`patches/step8d/`, `archive/step8d-findings.md`): `ADDatumBase(datum)` copied the
  ~550 B `AssemblyDatum` into local memory. Fix M builds it in place. Do not forward
  `libMesh::invalid_uint` by reference into device code. Dead ends: runtime dual index (A, B), maps
  by value (F), forced inlining with the copy (L), direct qp loop (R1), one `assembleADTerms` per
  configuration (R3), RZ `constexprIndex` (K), composition (`ADDatumBase` holding `Base &`). cicc
  consumes every `-X...` option, so LLVM options do not reach the optimizer.

## User decisions on the draft reply (2026-11-10)

- Fix the 27-qp contraction-plane bank conflict in this PR (step 9, done).
- Measure an inner `parallel_reduce` over qps before replying (step 10).
- p-refinement scratch sizing: filed as #34456, cited in the reply.
- Aliases: remove them in this PR (user, 2026-11-12, step 12b). Also `KernelCurl` and the BC
  aliases `IntegratedBCValue`, `VectorIntegratedBCValue` (user, 2026-11-12). Done.

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

- Step 13b4 q8 reference J (+10-23%): the driver carveout choice at 8 blocks/SM shrinks L1. A
  general fix is acceptable (user, 2026-11-14), for example a carveout derived from the launch's
  shared memory per block and a target block count, not one tuned to the H200. Unprobed.

- ad3sdisp_q8 Jacobian (12.96 ms at step 5): 6 element-groups of 8 Kokkos threads per team. Shared memory (37.7 KB
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

Steps 0-13c: done. Steps 13d-13i added 2026-11-14 after the user's review of the reply answers. Commits in "Commits on this branch". Step 12b also passed `heat_transfer` Kokkos
12/12. Step 13 split into 13a-13d after the user's review of the draft (2026-11-13).
13a. Done (sweeps `nsys-up1`, `nsys-v20`). See "Step 13a result".
13b. Done (`e4c7b785f70`, `nsys-v21`). See "Step 13b result".
13b2. Done (`ccbfc3288a5`, `nsys-v22`). See "Step 13b2 result".
13b3. Done (`d30dadacd08`, `nsys-v23`). See "Step 13b3 result".
13b4. Done (`a94736bf66c`, `nsys-v24`). See "Step 13b4 result".
13c. Done, rejected (`nsys-v25`). See "Step 13c result".
13d. Reply draft without `TODO(user)` (reply repo `3f43ced`). Revise after 13e-13i, then get user
    approval, then post. Posting notes: post each section as a reply to its review thread. Root
    comment IDs: `gh api repos/idaholab/moose/pulls/34088/comments --jq '.[] |
    select(.in_reply_to_id==null) | "\(.id) \(.path)"'`. The `:588` follow-up replies to
    4296312543. The headings use old line numbers: `:36` is the thread at `KokkosADAssembly.h:38`,
    `:71` is at `:120`, and `:588` is at `KokkosResidualObject.h:550`. End every post with the
    Claude Code trailer.
13e. ncu cause of the AD residual gap at q8 and `h27`. See "Next".
13f. Team size from evidence. Sweep the Kokkos team size 64/128/256 (an environment override of
    `TeamGroup::target_team_size` in a probe build) on the q8 and `h27` variants. Compare with
    Kokkos `team_size_recommended`. It returns an occupancy-optimal size that ignores
    element-groups, so it cannot be used as is. Design: choose the element-groups per team from an
    occupancy model. Inputs: the kernel registers per thread (`cudaFuncGetAttributes`, which
    Kokkos `team_size_max` already queries), scratch bytes per element-group, and the SM limits.
    Exit: a committed team-size rule with no fixed 128, and a sweep where no q8 or `h27` launch is
    slower than `nsys-v26`, or a regression reported to the user.
13g. Replace `TeamGroup::half_warp` with a backend trait: reals per shared-memory wavefront (bank
    count x bank width / `sizeof(Real)`), 16 on NVIDIA. Also derive from it the odd-stride padding
    of `qpSize` (`+ 1`, `| 1u`). Kokkos has no bank query. HIP `WarpSize` is 64 on CDNA and 32 on
    RDNA. Exit: no hard-coded 16 or bank assumption outside the trait, the AD tests pass, and the
    results are unchanged on the H200.
13h. One launch for the hand-coded displacement Jacobian. `dispatchDisplacementJacobian`
    (`KokkosResidualObject.K:373`) launches once per displacement component. Each launch maps the
    qp geometry again and evaluates the hooks again. `T(phi_j, h)` is linear, so one launch can
    evaluate the `3 x 4` direction terms per qp, with a component dimension in
    `DisplacementScratchLayout`. Exit: `handsdisp_q8` and `handsdisp_h27` J faster than
    `nsys-v26`, and the `kokkos` dir 189 pass.
13i. Rename `TeamGroup`, `ADTeamGroup`, `groupSize`, `group_for`, `groupedTeamLaunch`,
    `align_groups`, and the comments (13 files, about 70 uses) after the user confirms the name
    (proposed `ElemThreads`). Exit: `git grep -i -w group` in `framework/*/kokkos` gives no bare
    "group", and the `kokkos` dir 189 pass.

Out of scope: each matrix entry costs a CSR search (`Matrix::find`) and an atomic, in hand-coded
and AD kernels alike. A per-pattern map from element entries to value indices would help both.
Open a separate issue for it.

## Hand-coded loop findings (code reading, not measured)

- Fixed by step 13b2: the hand-coded precompute loops (`ResidualObject::computeResidualInternal`,
  `precomputeResidualBody`, `precomputeJacobianBody`) run one flat `RangePolicy` over
  (local thread, element) (`Thread::resize`, local thread fastest). Each of the
  `num_local_residual_threads` (default 2) threads of an element evaluates every qp. The Jacobian
  (`num_local_jacobian_threads`, default 4) evaluates every qp again per column j, because `Datum`
  caches the geometry of one qp only. With displacements, each evaluation runs
  `Datum::reinitTransform` (12 global reads and an F inverse). AD evaluates each qp once into
  scratch. This is the likely reason AD residuals beat hand-coded ones.
- `assembleDisplacementJacobian` ran one element per team of 128 threads (flaw of review comment
  3, in new code of this PR). Step 13b fixed it.
- `ThreadVectorRange` mapping for the AD element-group: measured in step 13c and rejected. One element per
  team with team size = qps instead hits the 32 resident blocks per SM cap.
- Upstream `up/next` (`ffc574921d7`) has hand-coded and sparse AD Kokkos kernels
  (`KokkosADDiffusion`, `KokkosADReal.h`), Kokkos materials without AD, and no displacement
  support. `origin` is the user's fork. Its `next` is from 2023.
- Default MOOSE quadrature is twice the FE order (q8 for FIRST HEX8, q27 for SECOND HEX27). The q27
  and q64 sweep variants keep 8 HEX8 test functions.

## Review findings for the reply to NamjaeChoi

- Comment 1 (scratch size limits occupancy): not the limiter up to 15 slots with one element per
  team. Registers were. With several elements per team (step 3), scratch caps the element-group count of
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
- Non-performance comments: aliases (step 12b), the `Configuration` name (step 12c, now `Deformation`), the `Coupleable.h`
  question, and the `KokkosADGeometry.h` placement (step 5 deletes the header).

## Known failures not caused by this work

Details: `moose-stack-build/PLAN-kokkos-known-failures.md`.

- `kokkos/problems/eigen_problem.test` and `eigen_problem.petsc_gpu`: libMesh was built without
  SLEPc detection.
- `kokkos/materials/parsed.parsed_material`: a CPU-only test that fails on CPU in a GPU build.
- `modules/thermal_hydraulics` executable: the link fails on undefined SLEPc `EPS*` symbols in
  `libnavier_stokes-opt.so`. The cause is the same missing SLEPc. Its Kokkos test cannot run.

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
  `profiles/nsys-v24` (step 13b4, the branch head). `nsys-v25` is the rejected 13c probe.
  The `VARIANTS_FILTER` for decision cases is `'(hand|ad3|ad8)(disp|sdisp)?_(q8|h27)'`.
  Upstream: `nsys-up1`. Latest ncu: `ncu-v18`. Next: `nsys-v26`, `ncu-v19`.
  Build `launches.txt` with `launch_table.sh <dir>/*.nsys-rep`.
- Tables in `scripts/`: `step_table.sh <dir>` (per-step summary), `ratio_table.sh <adN>
  <label>=<dir>...` (AD/hand ratio per sweep, `FIELD=residual|jacobian`), `launch_table.sh <nsys-rep>...`
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
  `runs/branch-exp`, at `a94736bf66c`, clean. It has empty `libmesh`/`petsc`, so it serves object
  probes only. Build the app in the main tree. Step 8d probe trees: `/tmp/pe-x` (login node, one git branch
  per probe). It can disappear. The patches in `patches/step8d/` are the record. Write probe patches in a copy (`git archive
  be2f2ac9b8f framework/include/kokkos` into `/tmp/pe`, then `git init`), never in the worktree
  while a job uses it. Queue jobs on the worktree with `--dependency=singleton
  --job-name=object-variants`. `~/.claude/skills/gpu-local-memory/scripts/ptx_local_sites.py [--kind entry|func] <ptx> <regex>`
  (skill `gpu-local-memory`) lists the runtime-indexed local sites of one entry or outlined `.func`.
- Register peaks: `~/.claude/skills/gpu-register-pressure/scripts/live_defs.py <cubin> <regex>`
  (skill `gpu-register-pressure`) lists the registers live at the peak by defining source line.
- `scripts/hang_stack.sh <exe> <input dir> <delay s> -- <args>` prints the thread stacks of a run
  that is still running after the delay.

## Build environment

See the auto-memory `arco-hpc-moose-build`. In short:

- No conda. The stack is a CUDA build on arco. Modules: `cuda/13.3.1 openmpi/5.0.10_gcc14.3.1`.
- `sbatch -A employee --wckey=neams -p general --gres=gpu:N`. Do not pin builds to a node, and keep
  the 120G default of `build_and_test.sbatch`: 60G at 24 nvcc jobs is OOM-killed.
- Linking needs `LIBRARY_PATH=/ibmstor/scratch/lindad/projects/moose-stack-build/linkdir`.
- `scripts/build_and_test.sbatch <moose dir> <test regex> cuda` rebuilds the opt test app and runs
  the tests. `APP_DIR=modules/<name>` selects a module app. On a build failure it prints the last
  lines of the full make log (`MAKE_LOG`). Run module jobs one at a time: they share libraries.
  `BUILD_METHOD=dbg` and `REPEAT=n` select dbg and repeat. The login shell exports
  `METHOD=dbg`, so scripts must not read `METHOD`. Write logs to `moose-stack-build/logs/`.
- AD regex: `'kokkos.*(ad_|current_configuration|function_dirichlet)'`. Expected: 33 pass with
  `cuda`. Whole `kokkos` dir: 189 pass, only the known failures. Filter logs for ` error`,
  `FAILED`, and `Ran `.
