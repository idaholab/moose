# Kokkos p-multigrid GPU performance

Working notes for the GPU performance of the `kokkos-pa` branch: what has been measured, what was
changed and what it bought, what is still open, and which plausible explanations the counters have
already refuted. It is meant to survive a change of author, so it records the evidence behind a
claim rather than the claim alone. Update it in place as work lands.

Deliberately untracked, so that it cannot reach a pull request. `.git/info/exclude` carries the
entry, which is local to this clone and is not part of any commit.

## The objective

**Make `SNESSolve` as fast as possible.** Device residency and kernel throughput are both means to
that end and neither outranks the other; what ranks an item is the time it removes from inside
`SNESSolve`, which includes `PCSetUp`. Performance outside the solve is out of scope by choice: the
23 s of idle GPU in startup, initial-condition projection, equation-system init and output is real and
is recorded below so that nobody rediscovers it, but it is not what this effort optimizes.

That framing puts the algorithm on the table alongside the implementation, and the largest result of
this work came from there rather than from a kernel. **Which smoother is needed depends on the basis,
and the difference is not one of degree.** On a nodal basis at the Gauss-Lobatto points, point Jacobi
costs one extra linear iteration against the entity-block smoother, 11 against 10, and is 1.7x faster
because it deletes the smoother's assembly, factorization and application entirely. On the modal
hierarchic basis it does not converge at all: `DIVERGED_ITS` at ten thousand iterations against the
thirty the entity-block smoother needs.

So the entity-block smoother is required for the modal basis and redundant for the nodal one. An
earlier version of this file recorded "the entity-block smoother stays" as a fixed constraint with no
basis attached, and listed `smoother=point_jacobi` as settled and not to be revisited. The reasoning
behind that closure -- that point Jacobi is not p-robust for a modal basis -- is right, and the run
above confirms it in the strongest terms; what was wrong was leaving it unqualified, which cost a
ranking built around two kernels that a one-line option removes.

**A second closure failed the same way, so the pattern is worth naming.** "The coarse solve belongs on
the host" was recorded as settled on the reasoning that sparse factorization is slower on a GPU. That
reasoning is correct and about factorization; the closure was written about the coarse solve. One
BoomerAMG cycle in place of the factorization was worth 2.52 s, and putting that cycle on the device a
further 0.30 s, both inside a question the document had marked closed. **A closure is only as wide as
the argument under it, so record the argument's scope with it**, and when a closure is reopened, say
which part of the old reasoning survived.

## How to read a measurement here

Four tools, and each answers something the others cannot.

- **MOOSE's performance graph** (`kokkos_pa_perf.sh`, `Outputs/perf_graph=true`) times MOOSE's own
  sections. Read per-section times here. Do **not** read them from `-log_view`'s `%T`, which prints
  `n/a` in the time column for most events, `MatMult` among them, while still reporting a percentage.
- **`-log_view`** (`kokkos_pa_verify.sh <tag> bench`) is authoritative for counts, flops and the GPU
  share of an event's flops in its rightmost column. Its `CpuToGpu` and `GpuToCpu` columns count only
  what PETSc itself copies, and only where that implementation calls `PetscLogCpuToGpu`; here they
  understate real traffic about threefold and are silent for `VECKOKKOS`. Never conclude anything
  about data placement from a zero in those columns.
- **Nsight Systems** (`kokkos_pa_nsys.sh`) decides whether the GPU is doing anything, and how much
  crosses the bus, because it counts traffic regardless of instrumentation. Drive it through the
  `nsight-systems` skill: idle-gap classification, synchronous-copy classification and utilization are
  recipe-owned semantics and hand-written SQL gets them wrong.
- **Nsight Compute** (`kokkos_pa_ncu*.sh`) for anything inside a kernel: occupancy, stalls, sectors
  per request, roofline. Its serialized replay stretches absolute durations, so compare ratios.
- **`-ksp_view`** is the only place the solver's own configuration is legible, and it is not only a
  record of what was asked for: PETSc reconfigures BoomerAMG according to the memory space of the
  matrix it is given, and the relaxation, coarsening and interpolation it prints are what is actually
  running. It also prints each level's operator type, which is how to tell where a matrix lives.

Two standing cautions. Kernel timeline coverage from Nsight Systems is an activity proxy, not SM
utilization; it says a kernel was resident, and the capture below collected no GPU metrics, so nothing
here claims how busy the SMs were while a kernel ran. And profiling inflates the host side: that
capture ran 56.6 s against about 46 s unprofiled, so read host-phase durations as roughly a fifth
high, while kernel durations and byte counts stand.

## The benchmark and where it stands

`pmultigrid.i Mesh/uniform_refine=7 Outputs/exodus=false -vec_type cuda`, one MPI process on one
H200, `METHOD=oprof`. The fine level carries 16.78M degrees of freedom at order 8 and the coarsest
263,169 at order 1, so a fine-level vector is 134 MB.

| state | total | `KSPSolve` |
| --- | --- | --- |
| before this work | 99.6 s | |
| device residency and the team-per-element apply | 48.4 s | 16.44 s |
| sum factorization | 47.0 s | 13.48 s |
| a device vector type for the shell operators | 37.8 s | 6.26 s |
| not uploading the zeroed entity-block entries | 37.6 s | 6.25 s |
| a team per element in the entity-block assembly | 34.1 s | 6.26 s |
| one BoomerAMG cycle on the coarsest level | 31.6 s | 5.76 s |
| a star forest for the ghost reduction | 29.1 s | 4.21 s |
| the same, over LAGRANGE_GLL rather than HIERARCHIC | 25.0 s | 1.40 s |
| zeroing the entity block entries on the device alone | 24.3 s | 1.41 s |
| the same, over LAGRANGE_GLL with a point Jacobi smoother | 20.9 s | 0.86 s |
| BoomerAMG on the device, owning the coarsest level's matrix | 20.5 s | 0.66 s |

The full Kokkos suite passes: 180 passed, 42 skipped, 0 failed, and the p-multigrid directory passes
at `-p4` as well. The branch has been rebased onto
`up/next`, so hashes an earlier version of this document named no longer resolve.

**A nodal basis on the Gauss-Lobatto points is worth more than anything else measured here.**
`Variables/u/family=LAGRANGE_GLL` converges the benchmark in 10 linear iterations against the
hierarchic basis's 30, because the basis is collocated with the Gauss-Lobatto rule of its own order and
the entity blocks the smoother inverts are better conditioned for it. With the apply sum factorized
over it as well, `SNESSolve` is 3.292 s against 6.139 s and `KSPSolve` 1.405 s against 4.221 s, so
1.86x and 3.0x on one basis change. The residual norms of the two bases are not comparable, a norm
being basis dependent, but they span the same order-eight space; iteration counts and times are.

**The matrix-free apply is no longer where the time is, and further kernel work on it is not worth
doing.** That is the most useful single fact about this branch, and the timeline evidence below says
why.

## Where the time is now

Two Nsight Systems captures, `kokkos_pa_nsys.sh` with `--trace=cuda,nvtx`, collector 2026.2.1 in the
container and read with the 2026.3.2 host tools. The second was taken after the shell vector type
change recorded under landed changes, and the pair is the evidence for it.

| | before | after |
| --- | --- | --- |
| total, unprofiled | 46.45 s | 37.77 s |
| `SNESSolve` | 22.443 s | 14.444 s |
| `KSPSolve` | 13.403 s | 6.255 s |
| kernel timeline coverage | 17.9% | 23.0% |
| summed kernel duration | 7.86 s | 7.90 s |
| kernels launched | 5490 | 7530 |
| host to device | 40.96 GB | 14.34 GB |
| device to host | 20.64 GB | 0.40 GB |
| device to device | 36.79 GB | 52.75 GB |
| GPU idle inside `SNESSolve` | 11.70 s | 5.58 s |
| GPU idle outside it | 24.47 s | 23.16 s |

Summed kernel duration barely moved while 2040 more kernels ran, which is the point: PETSc now
performs on the device the vector arithmetic it used to bring to the host, and those extra launches
cost 0.035 s against the round trips they replaced. Device-to-device traffic rose for the same reason.

**The balance has shifted out of the solve.** `SNESSolve` is 14.742 s of the profiled capture while
the GPU sits idle 23.16 s outside it, so setup, initial-condition projection and output are now the
larger half of the run by a wide margin.

### GPU idle, by region and gap size

From the `gpu_gaps` recipe at a 1 ms threshold, split at the `SNESSolve` range:

| region | >= 100 ms | 10-100 ms | 1-10 ms | idle |
| --- | --- | --- | --- | --- |
| outside `SNESSolve`, before | 23.28 s (24) | 1.00 s (26) | 0.19 s (49) | 24.47 s |
| outside `SNESSolve`, after | 21.86 s (24) | 1.09 s (27) | 0.21 s (54) | 23.16 s |
| inside `SNESSolve`, before | 2.98 s (4) | 5.05 s (233) | 3.67 s (856) | 11.70 s |
| inside `SNESSolve`, after | 2.74 s (2) | 2.72 s (85) | 0.13 s (31) | 5.58 s |

The 1-10 ms bucket inside the solve fell from 856 gaps to 31. Those were the per-operation host blocks
of the synchronous copies below.

Outside the solve, next to nothing runs on the GPU: in the before capture 5435 of the 5490 kernels fell
inside `SNESSolve`, so the roughly 23 s of setup, projection and output ran 55 kernels between them.
The largest gaps there, attributed to the innermost enclosing NVTX range:

| gap | enclosing range |
| --- | --- |
| 3.81 s | `FEProblem::projectSolution` |
| 3.47 s | before `MooseApp::run`, no enclosing range, so startup |
| 2.82 s | `Console::outputStep`, with `exodus=false` |
| 2.40 s | `MooseApp::runInputFile` |
| 3.55 s over three gaps | `FEProblem::EquationSystems::Init` |
| 3.37 s over five gaps | `NonlinearSystemBase::nlInitialSetup` |
| 0.65 s | `NonlinearSystemBase::setupKokkosEntityBlockSmoother` |

### The coarse LU is 2.08 s of a fully idle GPU

Before the change, the single large gap inside the solve carried no CUDA activity at all, one
`cudaMalloc` and 22 memory handle calls across 2.08 s, and its CPU samples were
`MatLUFactorNumeric_SeqAIJ` at 84.8% with `MatLUFactorSymbolic_SeqAIJ` at 9.4%. Afterwards the largest
remaining in-solve gap is the same thing: 2.081 s, `MatLUFactorNumeric_SeqAIJ` at 76.8%,
`MatLUFactorSymbolic_SeqAIJ` at 8.6%, `VecSetPreallocationCOO_Seq` at 7.2%. The settled question below,
that the coarse solve belongs on the host, rests on its own argument; this is what that choice costs,
and it is now 14% of the solve where it was 9%.

### Host/device traffic

Attributing the iteration-phase copies to the innermost enclosing range that is not itself one of
PETSc's own `VecCUDACopyTo` or `VecCUDACopyFrom` events, which otherwise swallow every copy:

| range | before H2D | before D2H | after H2D | after D2H |
| --- | --- | --- | --- | --- |
| `MatMult` | 12.67 GB | 6.30 GB | 0.076 GB | ~0 |
| `PCApply` | 10.08 GB | 5.04 GB | 0.047 GB | ~0 |
| `VecAXPY` | | 5.04 GB | | gone |
| `VecSetPreallCOO` | 4.83 GB | | 5.00 GB | |
| `MatMultTranspose` | 2.58 GB | 1.32 GB | 0.064 GB | 0.063 GB |
| `VecPointwiseMult` | | 1.30 GB | | gone |
| every MOOSE Kokkos range together | 0.03 GB | 0.00 GB | 0.027 GB | 0.00 GB |

The iteration phase went from 49.2 GB to 5.28 GB. MOOSE's own sections never moved anything: the
device residency work in this branch had already done its job, and what remained was PETSc moving its
own vectors for the reason recorded under landed changes. `MatMultTranspose`'s 127 MB residual is the
restriction into the coarsest level, which keeps host work vectors on purpose because a host
factorization solves that level.

**`VecSetPreallocationCOO` is now 5.00 GB of the 14.34 GB that still goes up**, so 35% of all remaining
upload traffic and the largest single transfer item left. It is described under known issues.

By memory kind, from the `cuda_memcpy_sync` recipe over the before capture's 1774 synchronous copies:

| | copies | bytes | time | effective |
| --- | --- | --- | --- | --- |
| pageable to device | 733 | 20.17 GB | 1.584 s | 12.7 GB/s |
| device to pageable | 714 | 13.56 GB | 0.703 s | 19.3 GB/s |
| device to pinned | 327 | 7.07 GB | 0.129 s | 54.8 GB/s |

The 327 pinned copies were exactly PETSc's `VecCUDACopyFrom`, so where a transfer is genuinely wanted,
staging it through pinned host memory is worth four times the bandwidth. The largest single one is
better removed than accelerated: **442 ms for one 5.24 GB pageable upload**, which is
`EntityBlocks::zero()` setting `_entries` to zero on the host and then calling `copyToDevice()`.
Filling that array on the device removes both the upload and the host memset behind it. It sits in
`PCSetUp` and nothing so far has touched it.

### Nothing pipelines, and two thirds of the launches serialize by construction

Inside the solve, by total duration:

| API | before | after |
| --- | --- | --- |
| `cudaDeviceSynchronize` | 7298 calls, 5.583 s | 5634 calls, 5.586 s |
| `cudaMemcpy` | 5319 calls, 2.505 s | 3351 calls, 0.566 s |
| `cudaEventSynchronize` | 3570 calls, 2.038 s | 3570 calls, 2.051 s |
| `cudaMemcpyAsync` | 4700 calls, 0.642 s | 3836 calls, 0.642 s |
| `cudaStreamSynchronize` | 8335 calls, 0.405 s | 7079 calls, 0.170 s |
| `cudaMalloc` | 2382 calls, 0.097 s | 1910 calls, 0.036 s |

`cudaDeviceSynchronize` holding its total while shedding 1664 calls is the clearest illustration of why
its duration cannot be read as waste: what it waits on is now kernels rather than transfers.

`cudaMemcpyToSymbolAsync` and `cudaEventSynchronize` each appear exactly 3570 times against 5435
launches, and neither moved, which is the signature of Kokkos's constant-memory launch path. That path
makes the host wait on an event for the previous constant-memory kernel to finish before it stages the
next functor (the `ConstantMemory` specialization in `Kokkos_Cuda_KernelLaunch.hpp`), and Kokkos selects
it whenever `sizeof(DriverType)` reaches `CudaTraits::ConstantMemoryUseThreshold`, which is 512 bytes
(`Kokkos_Cuda_Instance.hpp`). Two thirds of the launches take it. Either shrink the functors below 512
bytes or tag the policies with `Kokkos::Experimental::WorkItemProperty::HintLightWeight`, which the
mechanism table in that header routes to the plain kernel-argument path for anything under
`KernelArgumentLimit`; `sizeof` on the functors decides which of the two is open. Separately, about one
device-wide fence per launch means no two kernels can overlap and the host never runs ahead.

### The kernel budget is not where the effort went

From the after capture. MOOSE's own loops are unchanged, since the arithmetic is the same.

| functor and loop | launches | total | mean |
| --- | --- | --- | --- |
| `QpJacobianOperator` `BlockLoop` | 3 | 3.824 s | 1.27 s |
| `QpJacobianOperator` `BlockApplyLoop` | 393 | 2.036 s | 5.18 ms |
| `QpJacobianOperator` `ApplyTensorLoop` | 420 | 0.867 s | 2.06 ms |
| `ElementEmbedding` `ProlongLoop` | 90 | 0.443 s | 4.92 ms |
| PETSc's own kernels | 3955 | 0.268 s | 68 us |
| `ElementEmbedding` `RestrictLoop` | 90 | 0.250 s | 2.78 ms |
| `EntityBlocks` `FactorLoop` | 3 | 0.047 s | 15.6 ms |

The sum-factorized apply is 0.867 s of a 37.8 s run, and the decision to stop spending on it is
confirmed. The ranking around it is not what earlier rounds assumed. `BlockLoop`, the entity-block
assembly nobody has looked at, is half of all kernel time in three launches, and `BlockApplyLoop` is
more than twice the apply. Both still contract the full element basis.

## Open items, ranked

Retaken after the coarse solve moved to the device: `nsys_postcoarse.nsys-rep`, one process on one
H200, `LAGRANGE_GLL` with a point Jacobi smoother, 11 linear iterations, converged residual
8.564966e-09. The capture runs 28.3 s against 20.5 s unprofiled and `SNESSolve` measures 1.401 s in it
against 1.27 s unprofiled, so read host phases as roughly a tenth high. Kernel names come from
`kernel_summary`; the exact totals from bounded SQL over `CUPTI_ACTIVITY_KIND_KERNEL`; the idle split
from the `gpu_gaps` recipe at a 1 ms threshold scoped with `--nvtx=SNESSolve`.

| item | launches | cost | per |
| --- | --- | --- | --- |
| **GPU idle inside `SNESSolve`** | 40 gaps | **0.301 s** | solve |
| `QpJacobianOperator::ApplyTensorLoop` | 173 | 0.356 s | iteration |
| `ElementEmbedding::ProlongLoop` | 33 | 0.157 s | iteration |
| `QpJacobianOperator::DiagonalLoop` | 3 | 0.158 s | setup |
| `ElementEmbedding::RestrictLoop` | 33 | 0.089 s | iteration |
| everything PETSc, cuBLAS, cuSPARSE and hypre launch | 5251 | 0.096 s | iteration |
| `FESystem` | 3 | 0.068 s | setup |
| `QpJacobianOperator::MatrixLoop` | 1 | 0.022 s | setup |
| kernel total over the whole capture | 5502 | 0.986 s | |

**The kernel figures the earlier table carried were right; what the change removed was host time, not
kernel time.** `ApplyTensorLoop` is 0.356 s in both captures to the millisecond, `ProlongLoop`
0.157 s against 0.159 s, `DiagonalLoop` 0.158 s in both, `RestrictLoop` 0.089 s against 0.088 s. What
moved is hypre: its 0.339 s of host relaxation is replaced by `hypreGPUKernel_IVAXPY` and
`hypreGPUKernel_compute_twiaff_w`, 270 launches totalling **0.75 ms** of GPU time. PETSc's own kernels
fell from 0.158 s to 0.096 s.

**Three things the retake changes about the ranking.**

The largest single item in the solve is now **GPU idle, 0.301 s of a 1.401 s solve**, and its shape has
changed: no gap reaches 100 ms, where the previous capture had two totalling 2.74 s. It is 9 gaps of
10-100 ms for 0.191 s and 31 of 1-10 ms for 0.111 s, largest 49.2 ms. The big idle blocks inside the
solve -- the coarse factorization, then the host BoomerAMG -- are gone, and what is left is diffuse.
That is the signature the Kokkos constant-memory launch serialization below predicts, which makes that
item the first thing to try rather than a curiosity.

The grid transfers are the largest kernel pair that is not ruled out: `ProlongLoop` and `RestrictLoop`
together are 0.246 s, 18% of the solve, and neither has ever been looked at.

`ApplyTensorLoop` remains nominally top at 0.356 s and remains off the list, on the four rounds of
counter-evidence below.

1. **Multi-GPU.** One rank on one H200 throughout. The ghost reduction is a device-side star forest
   now, which is the piece that had to exist first. One thing found while moving the coarse solve to
   the device is worth carrying into this: the coarsest level's matrix assembles correctly on two,
   three and four processes, and the p-multigrid directory passes at `-p4`, but nothing here has ever
   measured more than one rank.
2. **`BlockApplyLoop` and `BlockLoop`**, only if the modal basis matters, where the entity-block
   smoother is not optional. Over a nodal basis they do not run.

Not `ApplyTensorLoop`, at 0.356 s with four rounds of counter-evidence behind it.

### The solve is no longer the run

`SNESSolve` is 1.27 s of a 20.5 s benchmark, so **94% of the wall clock is now outside it** -- startup,
initial-condition projection, equation-system init and output, which this document has recorded as
roughly 23 s of idle GPU since the solve was 22 s and it did not matter. It is out of scope by the
stated objective and remains so, but "make the benchmark fast" and "make `SNESSolve` fast" have now
almost entirely diverged, and anyone reading a wall-clock number should know which of the two they are
looking at.

### The default smoother stays `entity_block`

Decided. It is the choice that serves both bases, and robustness across a user's basis choice is worth
more than the 1.7x a nodal solve gets from point Jacobi, which that user can select. Revisit it only if
modal bases stop being something p-multigrid is expected to support at all, at which point the default
could follow the family instead. The `smoother` parameter description and the `PMultigrid.md` section
both used to state the point Jacobi weakness as though it were about polynomial order; both now say it
is about the basis, and give the nodal and modal outcomes separately.

### What the two entity-block loops are

Worth stating because the names do not say it, and because they are the two largest kernels in the
solve. The smoother inverts the block of degrees of freedom each mesh entity carries; on an
order-eight quadrilateral those blocks are an element's 49 interior modes, 7 modes on each edge, and 1
per vertex. The blocks partition the degrees of freedom, each belonging to exactly one owning entity,
so this is non-overlapping additive Schwarz, equivalently block Jacobi with entity blocks.

- **`BlockLoop`** is `QpJacobianOperator::assembleBlocks`, the smoother's **setup**: it forms the block
  matrices from the quadrature-point Jacobian cache. One thread per (element, variable) pair walks
  every pair of the element's degrees of freedom, keeps the pairs whose rows lie in the same block, and
  contracts over the quadrature points, accumulating into the block with an atomic add. An entity's
  block gathers a contribution from every element carrying that entity. Three launches, one per
  smoothed level: p = 8, 4 and 2. The coarsest level is solved rather than smoothed and has no blocks.
- **`BlockApplyLoop`** is `QpJacobianOperator::applyBlocks`, the smoother's **application**: one thread
  per block gathers that block's residual, applies the inverse by Cholesky substitution against the
  factor `EntityBlocks::factor()` built, and scatters the correction. A block of one degree of freedom
  carries no factor and is applied as a diagonal, which is a point smoother on those rows.
  `BlockIdentityLoop` then copies through the rows no block covers, which are the level's fixed rows
  where the operator carries the identity. The 393 launches are 30 V-cycles times three smoothed levels
  times the four preconditioner applications a Chebyshev smoother with `max_it=2` makes pre and post,
  plus the eigenvalue estimation.

### The two no-code experiments, both run

**Halving the smoothing is refuted.** `-mg_levels_ksp_max_it 1` takes the outer iterations from 30 to
53 and `SNESSolve` from 10.813 s to 12.070 s, so the four fine applications a V-cycle saves cost more
in extra outer iterations than they save. With BoomerAMG as well it is 9.169 s against 8.292 s, so the
verdict holds either way.

**The coarse solver result was adopted** and is recorded under landed changes.

### Code changes, by size

3. **`BlockApplyLoop`, 2.036 s.** Its traffic problem is understood and its two obvious fixes are
   refuted below; the only direction left is interleaving the entries across blocks so that a warp's
   threads read consecutive doubles while each keeps its own block, which is recorded there. Read that
   entry before touching this kernel. The measurements that led there: Nsight
   Compute on the fine-level launch, 15.16 ms: **DRAM throughput 73.4%**, 3.53 TB/s, L2 80.6%, compute
   6.2%, so it is bandwidth bound. It reads **52.0 GB** against about 5.5 GB of useful data, the
   5.24 GB of factors plus the residual and correction vectors, so roughly a 9x amplification. Sectors
   per request is 8.03, which is exactly coalesced for doubles, while **only 24.8% of each sector's
   bytes are used** -- the signature of requests with few active lanes rather than of a bad stride. The
   cause is that one thread per block puts blocks of 49, 7 and 1 degrees of freedom in the same warp,
   so the `for i < size` loops diverge almost immediately and later requests are issued by a handful of
   lanes while still fetching whole sectors.

   **The register-pressure reading was wrong.** The two `Real[MAX_BLOCK_DOF]` arrays at
   `MAX_BLOCK_DOF = 64` suggested 1024 bytes of per-thread local storage capping occupancy, as happened
   to the old apply loop. Measured: 42 registers per thread and 59.8% achieved occupancy against a
   62.5% theoretical. Occupancy is not the problem. A team per block still helps, but for coalescing
   rather than for occupancy, and grouping the blocks by size so a warp handles one size class is the
   more direct fix for the divergence. The metric that says whether it worked is bytes per sector,
   24.8% now, not achieved occupancy.
4. **`VecSetPreallocationCOO_Seq`, 0.91 s of host time plus a 5.00 GB upload**, which is 35% of all
   remaining host-to-device traffic. Build the reduction once per `DofSpace` rather than once per
   vector. Described under known issues; also what makes multi-GPU viable.
5. **The Kokkos constant-memory launch serialization.** 3570 of 5435 launches make the host wait on an
   event for the previous kernel, 2.05 s of `cudaEventSynchronize`. That time overlaps real kernel
   execution so it is not 2.05 s of recoverable waste; its cost is that the host cannot run ahead,
   which is some unattributed part of the solve's 5.58 s of idle. Cheap to test: check `sizeof` on the
   functors, then either shrink them below 512 bytes or tag the policies `HintLightWeight`.
6. **3600 small host-to-device copies per iteration phase inside
   `computeKokkosJacobianVectorProduct`**, 0.019 GB in total. Irrelevant as bandwidth, but that many
   synchronization points is worth a look once the above are done.

`ApplyTensorLoop` at 0.867 s is deliberately not on this list; four rounds of counter evidence below
say it is not where the time is. `BlockLoop` has come off it: at 0.525 s and 68.3% compute throughput
it is no longer among the larger items, and the remaining gain there would be a few tenths of a second
against whole seconds elsewhere.

### Baseline for the incoming setup-cost work

A branch reducing setup cost is being merged into this one. Taken with `kokkos_pa_perf.sh`, the before
picture is `setupKokkosEntityBlockSmoother` 4.767 s, `outputStep` 3.297 s, `InitializeKokkos` 1.343 s,
total 45.474 s, against `FEProblem::solve` self of 13.456 s. Re-run that script after the merge and
compare those four.

### Still on the dense contraction

`DiagonalLoop`, `MatrixLoop` and the entity-block loops were left alone, so the smoother and the
coarsest level's assembled matrix contract the full element basis.

3D is not covered. `HEX27` HIERARCHIC is separable too, but `cube_indices` flips a reference
coordinate rather than applying a sign, so a per-direction reversal of the quadrature points has to be
carried through; the 2D path is guarded on the element type being a quadrilateral.

## Landed changes and what each bought

**BoomerAMG ran on the host because the matrix it was handed was a host matrix.** The mechanism is
sharper than the conversion per setup this was first ranked as. `MatConvert_AIJ_HYPRE` reads the
source matrix's memory type and calls `HYPRE_SetMemoryLocation` with it
(`petsc/src/mat/impls/hypre/mhypre.c:603`), so `PCHYPRE` handed libMesh's host AIJ operator built hypre
a host matrix and put hypre's whole solve on the host, however device resident the rest of the cycle
was. The conversion itself is one `MatConvert` per setup and was never the cost.

A `coarse_solver` parameter on `PMultigrid` now names the coarse solver, as `smoother` names the
smoother, and the format the coarsest level's operator is assembled in follows it: `MATHYPRE` for
`boomeramg`, the libMesh AIJ default for `lu`, so a factorization still has a matrix it can read. The
format is settled in `PLevelSpace::init()` before `Matrix::create()`, because that call reads the
matrix's memory type to place its value buffer and then preallocates the coordinate pattern, and
`MatSetType()` discards both. Nothing about the device assembly had to change: `MATHYPRE` carries the
coordinate interface `Matrix::close()` already fills the operator through, and its delegate matrix
holds the same array hypre's own matrix does, so the assembly writes where hypre reads. The one
pairing the formats rule out, `boomeramg` with `-mg_coarse_pc_type` overridden to a factorization, is a
`paramError` naming both.

**Two things had to change alongside it, and one is worth more than a workaround.**

The identity the level's operator carries on every fixed row was written with `MatSetValues` into the
assembled matrix, after the coordinate assembly had filled the rest. On a hypre matrix in device
memory that aborts on more than one process: hypre loses its off-diagonal column map, and the next
copy reads from a null pointer. It reproduces in twenty lines of PETSc with no MOOSE in it -- COO
preallocate, `MatSetValuesCOO`, then one `MatSetValues` and a second assembly -- at two ranks and
above, host or device values alike, with AIJ unaffected; the reproducer is `hyprecoo.c` under
`/scratch/$USER/tmp/hyprecoo` and the defect is worth reporting upstream. The identity is now written
by `MatrixIdentityLoop`, a device loop over the level's own rows that sets the diagonal entry of each
fixed row in the same coordinate buffer the element loop fills. That is the better arrangement
regardless: it removes a host loop over the local degrees of freedom, its `MatSetValues` calls and a
second matrix assembly from `PCSetUp`, and leaves the whole of the level's operator assembled on the
device. Only the locally owned rows are written, since a ghosted row's own entries are summed into the
owner and writing the identity twice would double it.

And the two verification checks that read the assembled operator now read a host AIJ copy of it.
`verify=level_matrices` measures symmetry, which PETSc does through the transpose and `MATHYPRE` has no
transpose for, and multiplies the operator by the level's own vectors, which a device hypre matrix
refuses. `verify=entity_blocks` reads entries one at a time, which on a device hypre matrix costs a
device allocation each and, from order five upward, returns values that disagree with the blocks by
0.68 where the same operator read through a copy agrees with them to round-off. Both checks agree to
2.7e-15 and 4.4e-16 through the copy, under either coarse solver, at orders one through five.

**That last read-back failure is unexplained and did not reduce.** The standalone reproducer reads
every entry of the locally owned rows back through `MatGetValues` after the coordinate fill and finds
no error, at one, two and four ranks, with host and with device values alike, so "entry reads of a
device hypre matrix are wrong" is not the mechanism. What is established is narrower: the operator
itself is right, since `verify=level_matrices` measures its action against the matrix-free one through
a copy and agrees to 2.7e-15, and the solve converges in the same 11 iterations to the same residual
as the AIJ path. So the copy reads correctly and the direct read did not, at that sparsity, for a
reason nobody has pinned down. Do not repeat the stronger claim an earlier version of this file
made. **Ask for
`MATSEQAIJ` or `MATMPIAIJ` by name, not the `MATAIJ` alias:** converting a device-resident hypre matrix
to `MATAIJ` gives a device AIJ matrix (`mhypre.c:711`), whose product with host vectors is silently
zero rather than an error, which is how this was first mistaken for a wrong assembly.

**PETSc substitutes a weaker relaxation for a device matrix and leaves the sweep count alone.** For a
device-resident operator BoomerAMG gets l1-scaled Jacobi in place of symmetric SOR/Jacobi, PMIS in
place of Falgout and ext+i in place of classical interpolation, all of which `-ksp_view` reports. The
relaxation is the one that matters: at one sweep the benchmark took 14 outer iterations against 11 and
`SNESSolve` was 1.596 s, so moving hypre to the device on its own was a 2% loss. The sweep count is
therefore set to two wherever that operator is in device memory, through the options database because
hypre's sweep count has no API setter, and only where the option is unset so that it remains
overridable. Measured alternatives at 11 iterations: two sweeps 1.343 s, two AMG cycles
(`-mg_coarse_pc_hypre_boomeramg_max_iter 2`) 1.345 s, Chebyshev relaxation 1.360 s,
`l1scaled-SOR/Jacobi` 2.001 s. Under-relaxing at weight 0.7 made it worse, 16 iterations. Forcing the
host coarsening or interpolation onto a device matrix (`coarsen_type Falgout`, `interp_type classical`)
segfaults, which is presumably why PETSc switches them.

Measured effect, over `LAGRANGE_GLL` with a point Jacobi smoother, medians of three runs: `SNESSolve`
1.57 s to 1.27 s and `KSPSolve` 0.847 s to 0.662 s, so 19% and 22%, at an unchanged 11 linear
iterations. The spread over those three runs is about 0.05 s on `SNESSolve` and 0.005 s on
`KSPSolve`, so read the latter for anything finer than a tenth of a second. The old state is reachable on the same binary for the comparison, which is what these
numbers are: `coarse_solver = lu` assembles the AIJ operator and `-mg_coarse_pc_type hypre` then puts
BoomerAMG over it. `MatConvert` leaves the log. The 11 host-to-device copies of 23.2 MB that
`MatMult` and `PCApply` carried are gone, which was one 2.1 MB coarse vector per V-cycle, and
`KSPSolve` goes from 22 copies and 23.2 MB up to 11 copies and 0.5 kB. The converged residual moves
from 8.233809e-09 to 8.564966e-09, both inside the 1e-8 nonlinear tolerance, which is the same kind of
move the coarse solver produced when it became a multigrid cycle. `coarse_solver = lu` on the same
benchmark is 3.879 s, so the cycle is worth 2.6 s against the factorization it replaced, up from the
2.52 s recorded when that change landed.

**Work vectors were host resident.** `MatCreate()` sets a matrix's `defaultvectype` to the host type
(`petsc/src/mat/utils/gcreate.c:106`) and `MatCreateVecs()` applies it through `VecSetType()`
(`petsc/src/mat/interface/matrix.c:9748`), which consults no options. So `-vec_type` reached the
system's own vectors and nothing else, and every Krylov, multigrid and preconditioner work vector
stayed on the host. That one fact made PETSc run the host implementations of the cycle's vector
arithmetic, made the Kokkos wrapper stage a full 134 MB vector per tag per operator application, and
sent `Vector::close()` into PETSc's host coordinate-format assembly. `applySystemVectorTypeOptions()`
now carries the requested type onto the system's matrices and records it in the options database for
matrices built later.

**The shell operators never received that type.** Recording `-mat_vec_type` in the options database
only reaches a matrix that calls `MatSetFromOptions()`, and `MatCreateShell()` does not: it runs
`MatSetSizes()`, `MatSetType()` and `MatSetUp()` and no options routine
(`petsc/src/mat/impls/shell/shell.c`). So the p-multigrid level operators and transfers kept the host
default, `PCMG` built every level's work vectors from them with `MatCreateVecs()`, and each of the 350
coarse-level applies and every level's smoother arithmetic crossed PCIe. The system's own matrix-free
operator escaped this because it is a `libMesh::PetscMatrixShellMatrix` registered through
`System::add_matrix`, so the matrix loop above covers it. `Moose::PetscSupport::applyMatrixVecTypeOptions()`
now applies the recorded type with `MatSetVecType()`.

**Where that call goes is constrained by two lifecycle points, and the obvious site is between them.**
The shells are created in `PMultigrid::initialSetup()`, but the type only reaches the options database
when `FEProblemBase::solve()` applies the system's PETSc options (`FEProblemBase.C:7244`), which is
later. Applying it at construction is therefore a silent no-op. `PMultigrid::setupSolver()` runs at
`FEProblemBase.C:7256`, after that recording and before any level operator or transfer is handed to
PCMG, so `PLevelSpace::applyShellVecTypeOptions()` is called from there for every level; it covers the
level operator, skipping a level that assembles because a real matrix settled its type at creation,
and the transfer. The two diagnostic shells in `KokkosPMultigrid.K` that feed `MatComputeOperator` are
created in `postJacobianAssembly()`, which is already inside the solve, so those apply the type at
construction.

Measured effect: the iteration phase went from 49.2 GB of host/device traffic to 5.28 GB, device-to-host
over the whole run from 20.64 GB to 0.40 GB, `KSPSolve` from 13.403 s to 6.255 s and the total from
46.45 s to 37.77 s, with the converged residual and the iteration count unchanged and the Kokkos suite
still at 175 passed, 42 skipped, 0 failed.

**What the 8.33 s came off is host CPU work, not transfer time.** Nearly every CPU sample in the solve
lands on the main thread (28572 of 28589 before), which runs continuously, so its sample count is that
thread's wall clock: 0.807 ms per sample before and 0.824 after, and the count fell 28572 to 17887.
Comparing leaf symbols inside `SNESSolve`, these disappeared outright:

| host symbol | samples | time | what it was |
| --- | --- | --- | --- |
| `VecSetValuesCOO_Seq` | 1752 | 1.41 s | host COO assembly, because `Vector::close()` took its `_is_host` branch |
| `__memcpy_avx512_unaligned_erms` | 1428 | 1.15 s | `VecCopy`, 1078 calls, on host-resident level vectors |
| `VecAYPX_Seq` | 1312 | 1.06 s | host vector arithmetic |
| `__memset_avx512_unaligned_erms` | 935 | 0.75 s | `VecSet`, 889 calls |
| `daxpy_k_COOPERLAKE` | 752 | 0.61 s | the BLAS kernel behind `VecAXPY` |
| `VecAXPBYPCZ_Seq` | 642 | 0.52 s | host vector arithmetic |

That is 5.50 s, two thirds of the reduction; the remaining third sits in CUDA driver frames, which is
the host blocking on synchronous copies. `MatLUFactorNumeric_SeqAIJ` (2275 to 2269), `MatSolve_SeqAIJ`
(1349 to 1342) and `VecSetPreallocationCOO_Seq` (1109 to 1104) are untouched, which is the control:
those are the coarse LU and the COO preallocation, and nothing here was supposed to move them.

Two independent corroborations of the same story. `-log_view` shows identical call counts with the
placement changed, `VecAXPY` going from 40% of its flops on the GPU to 100%, `VecAYPX` from 83%,
`VecPointwiseMult` from 84% and the grid transfers `MatMultAdd` and `MatMultTranspose` from 76%. And
device-to-device traffic rose from 36.79 GB to 52.75 GB while device memset rose from 49.18 GB to
58.97 GB, which is the `VecCopy` and `VecSet` work arriving on the device, for 0.028 s and 0.014 s
against the 1.90 s of host memcpy and memset it replaced.

The lesson for the next change of this kind is that a host-resident work vector costs far more than the
bus traffic it generates. The traffic is what a profile makes obvious; the host arithmetic, the host
memcpy behind `VecCopy` and the host COO path behind `Vector::close()` are what actually took the time.

**`EntityBlocks::zero()` uploaded the entries it had just zeroed.** It assigned zero and then called
`copyToDevice()`. `Array::create()` allocates both sides and assigning a scalar fills whichever sides
an array holds, so the device entries were already zero and the copy put the host zeros on top of
them: one 5.24 GB pageable transfer, the largest single memory copy in the run. Host-to-device traffic
over the run fell from 14.34 GB to 8.88 GB, the largest single copy from 434 ms to 18 ms and
`SNESSolve` from 14.444 s to 14.032 s, which is the transfer time and nothing else. The host-side
`std::fill_n` of the same array remains and is dead work, since the host copy is never read before
`copyToHost()` overwrites it, but `__memset_avx512_unaligned_erms` totals 0.202 s across the whole
capture, so it is not worth a device-only fill on the `Array` API. Measured and dropped rather than
left open.

**The Gauss-Lobatto nodal families ran the dense contraction.** The tensor path was gated on
`HIERARCHIC`, so `LAGRANGE_GLL` paid 1.22e+10 flops per application against the hierarchic basis's
2.39e+09 over the same element. libMesh supplies the shape-function-to-tensor-index map for those
families in `fe_lagrange_gll_tensor_index()`, orientation dependence included, and lays the grid out
first-coordinate-fastest, which is the convention `QBase::tensor_product_quad()` and so the existing
hierarchic path already use; a nodal basis carries no sign where the hierarchic one signs an odd edge
mode. `MatMult` fell from 2.32e+12 flops to 4.63e+11, `KSPSolve` from 2.464 s to 1.405 s and
`SNESSolve` from 4.596 s to 3.292 s, with the iteration count and converged residual unchanged to the
digit. The family acceptance itself was cherry-picked from idaholab/moose#33770; only the tensor
mapping is new here.

**The entity block entries were zeroed on both sides when only the device is read.**
`EntityBlocks::zero()` assigned through `Array::operator=`, which fills whichever sides an array holds,
and nothing reads the host copy before `reduce()` or `copyEntriesToHost()` overwrites it. `Array` gained
`fillDevice()`, the device half on its own, which fits a class that already offers a host and a device
variant of everything else. `SNESSolve` fell from 6.139 s to 5.492 s over HIERARCHIC and 3.292 s to
2.631 s over LAGRANGE_GLL, with `KSPSolve` unchanged since `zero()` runs in `PCSetUp`.

The saving was 2.8x the estimate, and the reason generalizes: the estimate counted CPU samples inside
`std::fill_n`, while most of the cost was faulting in 5.24 GB of host pages that are never read. Peak
resident memory fell from 9.03 GB to 3.70 GB, which is the direct evidence. **A leaf-symbol profile
undercounts first touch**, so anything that writes a large host buffer is worth more than its samples
suggest.

**The ghost reduction paid a setup proportional to the vector length, per vector.**
`Vector::close()` reduced contributions to rows this rank does not own through PETSc's
coordinate-format interface. That interface builds a star forest for the communication and, in its
preallocation, a map from each local row to the entries landing on it so repeated indices can be
summed. The map is what makes the preallocation O(local size) rather than O(contributions), and PETSc
keeps it on the vector, so an operator application handed whichever of its caller's work vectors is
free paid it once per vector: 64 vectors, each a 134 MB allocation, a 16.8M-iteration host prefix sum
and a 134 MB upload. There is exactly one contribution per ghost degree of freedom here, since the
buffer is indexed by ghost index, so that map has nothing to coalesce. `close()` now builds the forest
alone, once per DOF layout, and reduces with `PetscSFReduceWithMemTypeBegin()`, which takes a device
pointer; because it works on arrays rather than on the vector it runs before the array is handed back.
`SNESSolve` fell from 8.291 s to 6.111 s, `KSPSolve` from 5.765 s to 4.214 s, host-to-device traffic by
7.10 GB and peak resident memory from 15.4 GB to 9.0 GB.

Two things worth keeping. The forest is held through a shared handle, not directly, because `Vector` is
a value type copied on the host and bytewise into device memory: a raw `PetscSF` freed by `destroy()`
was freed by every copy's destructor and reused dangling, which segfaulted inside `PetscSFSetUp`.
`libMesh::WrappedPetsc` is move-only and would have caught that at compile time. And the estimate for
this change was 0.9 s against 2.18 s delivered, because it counted only the preallocation and not
`VecSetValuesCOO`, which launched a kernel over the whole local size 742 times.

**The coarsest level was factorized when one multigrid cycle would do.** Coarsening the polynomial
degree leaves the mesh alone, so the coarsest level of a p-hierarchy still carries a degree of freedom
per mesh vertex, 263,169 of them here, and it was factorized once per Jacobian and solved exactly on
every cycle. A single BoomerAMG cycle is equally a fixed linear operator, which is what the outer
Krylov method requires, and is cheaper: `SNESSolve` fell from 10.813 s to 8.291 s and the benchmark
from 35.21 s to 31.62 s at an unchanged 30 linear iterations, with `MatLUFactorSymbolic`,
`MatLUFactorNumeric` and the thirty `MatSolve` calls gone from the log. About 2.07 s of that is the
factorization in `PCSetUp` and 0.45 s the coarse solves. The converged residual moves from
1.951325e-09 to 2.289006e-09, inside the 1e-8 nonlinear tolerance, and the suite was unchanged at 175
passed, 42 skipped, 0 failed: the tests in that directory check convergence reasons and verification
messages rather than iteration counts, and the one gold file pins an `INITIAL` residual, which no
solver choice affects. hypre ran on the host at the time; it no longer does, under the entry above.

**The entity-block assembly re-read the linearization thousands of times per element.**
`assembleBlocks()` ran a thread per (element, variable) pair with the quadrature-point loop innermost,
inside the loops over the element's degrees of freedom, so `_cache.getTensor()` was re-issued for every
pair sharing a block, about 2600 per element at order eight. Nsight Compute measured the fine level's
launch reading **7.40 TB from DRAM** against a 2.72 GB linearization, at 4.4% compute throughput, 99.4%
L1 throughput and an executed IPC of 0.11: the kernel was not computing, it was re-reading. A team per
element now stages that element's tensors in team scratch, 11.5 kB at order eight, and distributes the
pairs across the team. The fine launch reads 8.37 GB and takes 458 ms rather than 3.64 s, with compute
throughput at 68.3%, IPC at 1.60 and occupancy at 61.5% of a 62.5% theoretical; over the three smoothed
levels the kernel falls from 3.825 s to 0.525 s and `SNESSolve` from 14.03 s to 10.88 s. It is now
compute bound rather than bandwidth bound, which is why no second round was attempted: staging the
basis values per quadrature point, or factorizing the assembly, would chase a few tenths of a second.

**`Vector::close()` re-preallocated on every assembly.** `VecSetPreallocationCOO()` describes the
contribution indices alone, PETSc keeps its result on the vector, and its cost is proportional to the
vector's local size rather than to the number of contributions. It is now set once per vector, tracked
by PETSc object id. 742 calls and 13% of the run became 64 calls and 4%. A single-slot cache is not
enough: an operator application is handed whichever of its caller's work vectors is free and cycles
among several.

**The apply loop ran one thread per element.** It accumulated into a per-thread array of
`MAX_CACHED_DOF` entries, so an order-eight element was walked in three batches with the trial gather
and the tensor contraction repeated in each. That cost 72 registers and a 1024 byte stack frame per
thread, capping occupancy at 43.75%, and left compute throughput at 11.7% with DRAM at 38.7%. A team
per element now gathers once into scratch, shares the quadrature-point contraction, and gives each
test function one thread. Kernel time fell 4.7x and DRAM throughput fell to 4.2%.

**`VecCopy` refuses a cupm source with a Kokkos destination.** Converting the ghosted direction vector
to `VECKOKKOS` unconditionally aborted under `-vec_type cuda` with `Incompatible offload mask
PETSC_OFFLOAD_KOKKOS`. The conversion is now skipped when the vector is already CUDA/HIP typed, which
is already device resident.

**Sum factorization of the apply.** The matrix-free apply contracts one reference coordinate at a time
against one-dimensional tables. libMesh's HIERARCHIC family on a quadrilateral is an exact tensor
product (`src/fe/fe_hierarchic_shape_2D.C`, the `QUAD9` case of `fe_hierarchic_2D_shape`), the sign it
puts on an odd edge mode is the only orientation dependence, and `QBase::tensor_product_quad` lays the
rule out with the first coordinate's index innermost, so the factorization holds exactly. `MatMult`
flops fell from 6.16e+12 to 1.22e+12 and `KSPSolve` from 16.44 s to 13.48 s. **Sizing the team to its
range is kept**: `Kokkos::AUTO` gave 128 threads to phases indexed 0 to 80, and the size now comes
from the loop's own index space, measured once by `ApplyTensorSizeLoop`.

**Staging the one-dimensional tables in scratch was reverted**, on the memory hierarchy rather than on
the measurement. Shared memory is per block and L1 is per SM across blocks, and the tables are
read-only and identical for every element of a type, so staging replaces one cached copy serving every
resident team with a private copy per team in the same physical SRAM: twelve to twenty-one redundant
copies of 1296 bytes, spending a resource that bounds occupancy. Staging is for data reused within a
block and not shared across blocks, or for a pattern that would thrash L1, and this is neither. A
table broadcast to every block belongs in L1 or constant memory. It also cost a hand-maintained flat
index at five sites in place of the two-argument accessor, which is what produced a shadowing bug
while it was in.

Alongside these, libMesh gained `LOG_SCOPE_NO_NVTX` for the performance log events that fire once per
mesh entity or more often. The same capture went from 4,257,245 NVTX ranges to 28,987, which is what
makes an enclosing-range attribution query usable; those events are still timed in the performance
log. Shell applications now also report their operation count, so `MatMult` shows its flops at 100% on
the GPU rather than 2.71e+09 at 44%.

## Settled questions

- **`cudaDeviceSynchronize` time cannot be added to the bill, but the fences are still why nothing
  overlaps.** A synchronization's duration includes waiting on a kernel that is already running, so
  counting it alongside kernel time double-counts the same GPU work, and an earlier round was wrong to
  rank it as the largest remaining item. What the timeline adds is that the solve issues about one
  device-wide fence per launch, which is why no two kernels overlap; the 11.70 s of measured idle
  inside `SNESSolve` is the honest measure of the loss. Total kernel time in the run is 7.86 s, not
  the 20.8 s an earlier round reported.
- **The coarse solve does not belong on the host, and "a direct solve there" was the wrong frame.**
  The coarsest level is 263,169 DOFs, which is not small: coarsening the polynomial degree leaves the
  mesh alone. The original reasoning -- a direct solve on the coarsest level is standard multigrid
  practice and sparse factorization on a GPU is generally slower -- is sound about factorization and
  led to the wrong conclusion, because factorization was never the only fixed linear operator
  available. One BoomerAMG cycle was worth 2.52 s at the same iteration count, and putting that cycle
  on the device is worth a further 0.30 s; the factorization is now 3.88 s against 1.27 s for the
  cycle. Both are reachable through `coarse_solver`. What does still hold is why iterating the level
  with a Krylov method fails: the preconditioner has to be a fixed linear operator, and neither a
  relative tolerance nor a fixed iteration count gives one.
- **There is no host `MatMult` to move to the device.** All 510 are device-side matrix-free applies:
  160 fine level, 350 at p=4 and p=2, and the coarsest level never multiplies at all. A GPU matrix
  format would buy nothing.
- **The fine level's 160 applications per solve are not a defect.** 10 are the one-time Chebyshev
  eigenvalue estimation, 120 are the 30 V-cycles at exactly 4 per cycle, 29 are the outer GMRES
  iterations and 1 is the line search. `ksp_view` confirms Chebyshev smoothers with `max_it=2` pre and
  post. The V-cycle costs 4 fine applications per outer iteration on top of the 1 the Krylov method
  needs; `-mg_levels_ksp_max_it 1` trades smoothing quality against that, and has now
  been run: it takes the outer iterations from 30 to 53 and `SNESSolve` from 10.813 s to 12.070 s, so
  it is a loss.
- **The entity-block smoother is not up for replacement _on a modal basis_.** Point Jacobi is not robust in the
  polynomial degree, particularly for a modal basis, which is why the benchmark input pins
  `smoother = entity_block` and notes that a diagonal smoother cannot damp what an order-eight basis
  carries. That the smoother is 5.91 s of a 14.742 s solve is an argument for making `BlockLoop` and
  `BlockApplyLoop` faster, not for trading the smoother away.
- **`GPU %F` in `-log_view` said nothing about placement** while the shells logged no flops. It now
  does.

## Readings the counters refuted

Recorded so that nobody repeats them. Nsight Compute on `ApplyTensorLoop`, L1/TEX at 78-86%
throughout, was the starting point for the first four.

- **Not the loop's scratch.** Shared memory and L1 are one unit on Hopper, so staging six scratch
  arrays across four phases looks like the replacement for the old table traffic. Of 422,532,572 LSU
  wavefronts, 66,538,017 are shared: 16%, against 84% global. Shared byte efficiency is 81.9%.
- **Not divergent reads of the quadrature-point cache.** `QpJacobianTensor` is 128 bytes as an array of
  structs, addressed one thread per quadrature point, which ought to scatter a warp's lanes. Global
  loads average **5.73 sectors per request**, where fully coalesced is 8 and maximally divergent is 32.
  Sector *utilization* is only 27.1%, but that is not divergence.
- **Not the one-dimensional tables.** They are read in all four phases, roughly 46 kB of requests per
  element against a 1.3 kB footprint, so staging them in team scratch should have cut the request count
  sharply. It moved `KSPSolve` by 0.1 s.
- **Not wasted threads.** `Kokkos::AUTO` gave 128 threads to phases with a range of 81. Sizing the team
  to the range took the block to 81, registers from 64 to 56 and occupancy from 45.1% to 51.6%, and the
  kernel's Nsight duration from 2.076 ms to 1.793 ms. `KSPSolve` still moved only 0.1 s, which is
  itself evidence for the overhead finding: a 14% faster kernel that does not show up end to end is a
  kernel that was not the cost.
- **Not a team per block in `BlockApplyLoop`, although the coalescing diagnosis was right.** Two
  variants were built, measured and reverted. Both staged the block's factor in team scratch with a
  coalesced cooperative read, which did exactly what the sector measurement predicted: DRAM bytes read
  for the fine launch fell from 52.01 GB to 5.53 GB and bytes per sector rose from 24.84% to 83.15%.
  Both were still slower end to end. With the substitution left serial on one lane, `KSPSolve` went
  from 6.26 s to 7.54 s and the kernel from 15.13 ms to 20.15 ms; with each row's dot product spread
  over the team, 8.58 s, worse again, because a 49-row block then pays about a hundred barriers.

  The reason is that the factor's scratch caps a multiprocessor at **8 teams** (`Block Limit Shared
  Mem` of 8), so a team per block trades roughly 1200 concurrent substitution chains for 8. The
  substitution is a dependency chain down the rows, and no arrangement within a team recovers that
  concurrency: a serial inner product leaves one active lane per team, and a parallel one pays a
  barrier per row. Thread per block with a quarter of each sector wasted beats eight well-coalesced
  chains. Its 73.4% DRAM throughput is therefore a consequence of running many concurrent inefficient
  streams rather than evidence that traffic is the limit -- the kernel is latency bound on the
  substitution, and reading it as bandwidth bound is what made a team look indicated.

  What remains untried is the one restructuring that fixes coalescing **without** giving up
  concurrency: interleaving the entries across blocks, so that entry (i, j) of every block in a size
  class is contiguous and the thirty-two threads of a warp reading their own blocks' (i, j) touch
  consecutive doubles. That keeps a thread per block, needs no scratch and no barriers, and is what
  batched dense linear algebra does for exactly this shape of problem. It is a larger change, since
  `_entries` and `_entry_offset` are consumed by the assembly, the factorization, the apply, the
  parallel reduction and the verification, and it requires grouping blocks by size.
- **Not register pressure in `BlockApplyLoop`.** Two `Real[MAX_BLOCK_DOF]` arrays with
  `MAX_BLOCK_DOF = 64` read as 1024 bytes of per-thread local storage, which is what capped the old
  apply loop's occupancy at 43.75%, so the same diagnosis looked obvious here. Nsight Compute reports
  42 registers per thread and 59.8% achieved occupancy against 62.5% theoretical. The kernel is DRAM
  bound at 73.4% and its loss is sector utilization at 24.8% from warp divergence over unequal block
  sizes.
- **Not `VECKOKKOS` instead of `VECCUDA`.** `-vec_type kokkos -mat_vec_type kokkos` looks like it
  removes the COO preallocation traffic: `logview_kokkosboth.txt` reports `VecSetPreallCOO` with no
  host-to-device copies where the CUDA run of the same tree reports tens of gigabytes. That is a
  logging artifact. `VecSetPreallocationCOO_SeqKokkos` calls `VecSetPreallocationCOO_Seq` and then
  `Vec_Kokkos::SetUpCOO` (`petsc/src/vec/vec/impls/seq/kokkos/veckokkosimpl.hpp`), which mirrors the
  same `m + 1` `jmap1` array to the device; the Kokkos path simply never calls `PetscLogCpuToGpu`.
  Nsight Systems counts the traffic either way.

A register-blocked contraction, which is what the matrix-free FEM libraries do to keep partial
contractions off shared memory, is therefore not indicated either.

## One bug class, three instances

**A PETSc object that MOOSE attaches to a p-multigrid level's own `libMesh::System` is invisible to the
problem-level option plumbing.** `applyVectorTypeOptions()` walks the problem's solver and auxiliary
systems, and every `PLevelSpace` owns a private `libMesh::System`, so nothing it holds is reached. Three
instances so far, and a fourth should be expected rather than discovered:

- The level operators and transfers, created by `MatCreateShell()`, which performs no
  `MatSetFromOptions()` either, so neither route reached them. `PCMG` built every level's work vectors
  from them and got host vectors. Fixed by `PLevelSpace::applyShellVecTypeOptions()`.
- The coarsest level's assembled operator, `_sys.add_matrix("level operator")`, which had no device
  type and kept BoomerAMG on the host. Fixed by the format following `coarse_solver`: `MATHYPRE` sets
  its own vector type to `VECCUDA` where hypre is CUDA enabled, so typing the matrix settles both the
  matrix and the work vectors PCMG builds from it, and does so at creation rather than through the
  options database. That makes it the one instance of this class whose fix does not run into the
  ordering trap below.
- The level work vector `_x` and the constrained-rows vector, both `add_vector` on the level's system.

**The ordering is the trap, not the typing.** The type only reaches the options database when
`FEProblemBase::solve()` applies the system's PETSc options, which is after `initialSetup()` builds
these objects and before `PMultigrid::setupSolver()` hands them to PETSc. Anything that types them at
construction is a silent no-op; `setupSolver()` is the one window that works. That cost a GPU run once
already.

## Known issues

- **The `petsc` submodule is at exactly what the build links**, `4146d835194`, the `v3.25.4` tag, with
  a clean tree and the gitlink agreeing. An earlier version of this file called it "far ahead and
  reference-only" and told the reader not to trust it, which was true of an earlier checkout. For a
  question about this version's source it is now the right thing to read, and the `MatGetFactor` check
  cited in the BoomerAMG item is verified after all. Two caveats stand: configuration-dependent
  behaviour has to come from the installed headers under `/opt/petsc/include`, which is where the hypre
  device build was confirmed (`PETSC_HAVE_HYPRE_DEVICE`, `HYPRE_USING_CUDA`,
  `HYPRE_USING_DEVICE_MEMORY`), and patching the submodule changes nothing without a full PETSc
  rebuild.
- **`MatSetValues` on a device-resident `MATHYPRE` matrix preallocated with
  `MatSetPreallocationCOO` aborts on more than one process.** hypre loses the matrix's off-diagonal
  column map and the next copy reads from a null pointer,
  `hypre_Memcpy warning: copy N bytes from (nil)`, followed by `cudaErrorIllegalAddress`. Reproduced
  standalone in `minimal.c` under `/scratch/$USER/tmp/hyprecoo`, which needs only `petscmat.h` and
  `-lpetsc`: preallocate a tridiagonal pattern with `MatSetPreallocationCOO`, fill the diagonal block
  with `MatSetValues`, assemble, multiply.

  | `minimal.c` arguments | np = 1 | np = 2 |
  | --- | --- | --- |
  | `-prealloc coo` | passes | **aborts** |
  | `-prealloc coo -hostbound` | passes | passes |
  | `-prealloc coo -mat_type aij` | passes | passes |
  | `-prealloc coo -uncoupled` | passes | passes |

  `-uncoupled` drops the entries coupling neighbouring ranks from the declared pattern, so no rank has
  an off-diagonal block. **The shortest statement of the bug is that a coordinate preallocation
  naming any inter-rank coupling is enough to break a later `MatSetValues`.** The block has to exist
  and does not have to be written: the first row never writes a coupling column and still aborts.
  `-prealloc hypre -write_offdiag` passes, which is what rules out `MatSetValues` on a device
  `MATHYPRE` matrix being broken generally; that run needs the off-diagonal writes because
  `MatHYPRESetPreallocation` materializes no off-diagonal block until off-diagonal values arrive,
  which the norm shows directly -- 2 for the block-diagonal operator against sqrt(2) for the coupled
  one. A coordinate preallocation materializes that block from the pattern regardless.

  Four things pin it down. The coordinate interface is not involved beyond the preallocation: no
  `MatSetValuesCOO` runs at all, and this is the order `petsc/src/mat/tutorials/ex18.c` itself uses, so
  it is sanctioned rather than novel. Binding the matrix to the host makes it pass, so it is the device
  path. Leaving the ranks uncoupled so that no rank has an off-diagonal block makes it pass, which
  together with the size of the null-source copy -- the rank's off-diagonal column count times
  `sizeof(HYPRE_BigInt)` in every case measured -- identifies `hypre_ParCSRMatrixColMapOffd`. And the
  last pair of rows is the one that names the mechanism rather than guessing at it: the same writes
  over the same sparsity pass under `MatHYPRESetPreallocation` and abort under
  `MatSetPreallocationCOO`, and what the latter does that the former does not is set `MAT_SORTED_FULL`,
  which stops `MatAssemblyEnd_HYPRE` recreating the `hypre_AuxParCSRMatrix` that
  `HYPRE_IJMatrixAssemble` destroys, and call `MatHYPRE_AttachCOOMat`, which aliases the delegate
  matrix's arrays into the ParCSR, takes ownership away from hypre and never re-establishes the alias.

  **Confirmed on PETSc `main`, not inferred.** A minimal PETSc was built from `origin/main`
  (`e7dfb97c677`, 3.25.5 development) with CUDA and hypre alone under
  `/scratch/$USER/tmp/petsc-main`, by `kokkos_pa_petscmain.sh`, and every row of the table above
  reproduces against it. That build is independent of the container's in the ways that might have
  mattered: hypre 3.2.0 rather than 3.1.0, no `--enable-gpu-aware-mpi`, and no
  `PETSC_HAVE_HYPRE_MIXEDINT`, so none of those options is the cause. Reading the source for whether a
  fix had landed was how this started and is worth less than the twenty minutes the build took.

  **Root-caused to hypre, with a one-line fix verified.** The null read is
  `hypre_CSRMatrixMergeColMapOffd` <- `hypre_CSRMatrixSplitDevice_core` <-
  `hypre_IJMatrixAssembleParCSRDevice`, from a backtrace planted in `hypre_Memcpy`. The device
  assemble passes `hypre_ParCSRMatrixDeviceColMapOffd(par_matrix)` into the split without first
  mirroring the host column map to the device, and the merge copies from it device-to-device. A
  ParCSR built on the host carries `col_map_offd` on the host only, which is the state PETSc's
  coordinate preallocation leaves, so that pointer is null while `num_cols_offd` is nonzero. The
  `MatGetValues` path in the same hypre file already calls `hypre_ParCSRMatrixCopyColMapOffdToDevice`
  before reading the same field; the assemble path does not. Adding that one call ahead of the split
  fixes every configuration in the table above at one, two and four ranks, with the expected norms,
  and hypre's own `ij` driver under BoomerAMG is unchanged on host and device at one and two ranks.
  The call is idempotent, being a no-op when the device copy already exists. Submitted upstream as
  [hypre-space/hypre#1625](https://github.com/hypre-space/hypre/pull/1625), from the branch
  `fix-ij-device-assemble-colmap-offd` in
  `/scratch/$USER/tmp/petsc-main/arch-hypre-cuda/externalpackages/git.hypre`, whose `fork` remote is
  `lindsayad/hypre`. That file is unchanged between `v3.2.0` and `master`, so the verification above
  transfers to the branch the PR targets. **The `AI-assisted` label the project asks for could not be
  set from outside the organization; the disclosure is in the PR body instead and the label still
  wants adding by hand.**

  **So this is hypre's bug, not PETSc's**, which three rounds of reading PETSc's `mhypre.c` did not
  establish and one planted backtrace did. `MAT_SORTED_FULL`, the `MatHYPRE_AttachCOOMat` aliasing and
  the destroyed aux matrix were all wrong guesses; none of them is involved. Instrument earlier.

  MOOSE no longer takes this path, for the reason under landed changes.
  Two claims an earlier version of this entry made and should not: that the coordinate fill was part
  of the trigger, and that mixing `MatSetValues` with `MatSetValuesCOO` was the unsupported part.
- **`MatSetValues` into an off-diagonal column after `MatSetPreallocationCOO` fails for MPIAIJ**, with
  `Column too large: col 9 max 0`, because the coordinate preallocation leaves the off-diagonal block's
  column layout unset; `MatConvert_HYPRE_AIJ` patches that up by hand for its own destination matrix.
  Separate from the entry above, which is about an entry in the diagonal block that MPIAIJ accepts, and
  not something this branch needs. Fixed on the branch
  `lindsayad/2026-09-23/coo-prealloc-leaves-hash-mode` in the clone at `/scratch/$USER/petsc`,
  unpushed: `MatSetPreallocationCOO_MPIAIJ` does not leave hash-table assembly mode, which
  `MatMPIAIJSetPreallocation_MPIAIJ` does, so a later `MatSetValues` runs `MatSetValues_MPI_Hash`
  against a reduced off-diagonal block.

  **`MatSetPreallocationCOO_SeqAIJ` is fixed too, for consistency rather than for a known failure,
  which is a deliberate choice and not an oversight.** Two orders were tried against a fix-free PETSc
  and the sequential path misbehaves in neither, and there is a reason rather than just an absence of
  evidence: the failure is the off-diagonal block's reduced column index space, a sequential matrix has
  no off-diagonal block, and `MatSetValues_Seq_Hash` writes a hash table rather than the CSR. The
  argument for changing it anyway is that its sibling `MatSeqAIJSetPreallocation_SeqAIJ` leaves hash
  mode, so a reader comparing the two would otherwise have to work out why one does and one does not.
  The commit message says which half rests on a reproducer and which on consistency, so a reviewer can
  weigh them separately. PETSc's own
  coordinate tests pass with it, `ex123` 120 cases and `ex254` 78, on a GPU node, and the branch adds
  the regression case to `ex254`.

  **The exposure is narrower than it first looks, and the reason is worth keeping.** A stale hash mode
  heals itself: `MatSetValuesCOO` assembles internally, and `MatAssemblyEnd_MPI_Hash` restores the
  regular operations and clears the flag. So the fault only appears when `MatSetValues` is the first
  thing to run after the coordinate preallocation, which is `ex18.c`'s `FillMatrixCPU` order. Anyone
  who fills through the coordinate interface first -- which is the point of preallocating that way,
  and what MOOSE does -- never sees it, which is why MOOSE's AIJ path worked in parallel all along.
  Two wrong guesses were spent before that: typing the matrix `MATHYPRE` first, and giving explicit
  local sizes, neither of which matters. Recorded because it looks like an undocumented restriction and
  because confusing the two makes the hypre report unreadable.
- **`MatConvert` to the `MATAIJ` alias does not give a host matrix.** From a device-resident hypre
  source it resolves the alias to `MATSEQAIJCUSPARSE` or `MATMPIAIJCUSPARSE`
  (`petsc/src/mat/impls/hypre/mhypre.c:711`). Multiplying that by host vectors produces zeros with no
  error, which reads exactly like a wrong assembly. Name `MATSEQAIJ` or `MATMPIAIJ`.
- **The libmesh submodule is resolvable.** It points at `61ef686c13` on the libMesh branch
  `orientation-dev`, which is reachable from `origin`, `git@github.com:libMesh/libmesh`, the same place
  the `.gitmodules` URL resolves to, so a fresh clone can check it out. That commit carries both
  `LOG_SCOPE_NO_NVTX` (`include/base/libmesh_logging.h`) and the `fe_hierarchic_quad_tensor_indices`
  accessor sum factorization needs (`include/fe/fe.h`, `src/fe/fe_hierarchic_shape_2D.C`). An earlier
  version of this document named `9cde942a9d` on an unpushed `kokkos-pa-nvtx` branch; that is stale.
- **`VecSetPreallocationCOO` costs O(local size) per vector however few contributions it describes**,
  and `VecSetValuesCOO_SeqCUPM` then launches over the whole local size rather than over `coo_n`
  (`petsc/src/vec/vec/impls/seq/bvec2.c` and
  `petsc/src/vec/vec/impls/seq/cupm/vecseqcupm_impl.hpp`). MOOSE no longer uses that interface, for the
  reason under landed changes, so this is recorded only so that nobody reaches for it again on the
  assumption that its cost follows the contribution count.
- **The default vector type converges to a different residual** than `-vec_type cuda`: `2.877143e-09`
  against `1.951328e-09`. Larger than a reduction-ordering difference, predates all of this work, fails
  nothing, and is unexplained.
- **A converged residual moves in its last digits between runs**, because elements reduce into a shared
  row through atomic addition in whatever order the thread mapping produces. Observed `...325`,
  `...326`, `...328`. Do not treat the final digit as a regression signal. Sum factorization moves it
  too, for the same reason at one remove, because it sums the quadrature points and the modes in a
  different order: `1.951326e-09` against `1.951328e-09`. The p-multigrid verification checks measure
  the two contractions against one another and agree to within their `1e-10` relative tolerance.
- **The `VecCopy` gap is unfixed on PETSc `main`** as of 991 commits past the pinned 3.25.4, and is
  worth reporting upstream. The COO routines' cost being proportional to vector length is by design
  there, so that fix belonged here.
- **`run_tests` on this NFS mount intermittently reports `FAILED (TIME FILE MISSING)`** on a random
  test. A filesystem timing race in the harness's own bookkeeping; re-run.
- **The container is behind what the tree asks for.** The build warns that
  `moose-dev-ubuntu24-cuda-gcc-openmpi-x86_64_graniterapids` is at `2026.08.19` where `2026.09.14` is
  required. Everything here was built and run against `2026.08.19` regardless, as the earlier work was.

## Reproducing

The sweeps are scripted on `/scratch/$USER/tmp`, which is shared across the nodes:

| script | what it does |
| --- | --- |
| `kokkos_pa_build.sh [libmesh\|framework\|test\|moose\|all] [moose_j] [libmesh_j]` | builds in the container on the login node; libmesh for the `oprof` method only, MOOSE at a low job count because the Kokkos units go through nvcc |
| `kokkos_pa_gpu.sh <script> [minutes] [args]` | runs one of the scripts below on a GPU node, in the container, with `--nv` |
| `kokkos_pa_verify.sh <tag> [pmg\|suite\|bench\|all]` | the p-multigrid tests, the whole Kokkos suite, and the benchmark under `-log_view` |
| `kokkos_pa_timing.sh` | the unrefined-mesh run that bounds mesh-independent setup, and repeated benchmark runs for variance |
| `kokkos_pa_perf.sh` | the benchmark under MOOSE's own performance graph, which is what to read for per-section times |
| `kokkos_pa_nsys.sh [tag]` | the benchmark under Nsight Systems with CUDA and NVTX tracing, which is what to read for GPU idle time and host/device traffic |
| `kokkos_pa_ncu.sh [kernel] [skip] [count]` | Nsight Compute speed of light, occupancy, launch, compute and memory workload sections on one kernel; the launch window matters for a kernel whose expensive launch is not the first |
| `kokkos_pa_ncu_blocks.sh` | the above for `BlockLoop` and `BlockApplyLoop` in one allocation |
| `kokkos_pa_ncu_blocks2.sh` | sectors per request for those two kernels |
| `kokkos_pa_ncu_shared.sh [kernel]` | the same kernel's L1 traffic split into its shared-memory and global parts |
| `kokkos_pa_ncu_sectors.sh [kernel] [skip] [count]` | sectors per global load request and sector utilization |
| `kokkos_pa_hypre.sh [views\|pmg\|bench\|repeat]` | the coarse-solver cases: `-ksp_view` and the matrix-format verification checks under each `coarse_solver`, the conflicting pairing, and the benchmark under each of the AIJ-converted, hypre-owned and factorized coarse operators; `repeat` takes the before/after pair several times for variance |
| `kokkos_pa_hypreopts.sh` | the BoomerAMG device relaxation sweep: sweep count, relaxation type, weight, cycle count, coarsening and interpolation, reporting iterations and `SNESSolve` for each |
| `kokkos_pa_hyprepar.sh` | each `coarse_solver` at one through four processes |
| `kokkos_pa_suite.sh <tag> [pmg\|suite\|parallel\|all]` | the p-multigrid tests, the same at `-p4`, and the whole Kokkos suite, each captured whole so a failure is reported rather than trimmed |
| `kokkos_pa_hypremin.sh` | builds and runs `hyprecoo/minimal.c`, the standalone PETSc reproducer of the `MATHYPRE` defect under known issues, over its preallocation, memory-space, off-diagonal and matrix-type controls |
| `kokkos_pa_petscmain.sh` | configures and builds a minimal PETSc from `origin/main` with CUDA and hypre alone, into `/scratch/$USER/tmp/petsc-main`, for checking an upstream defect against `main` rather than reading its source |
| `kokkos_pa_petscmain_run.sh` | runs the reproducer against that build and reports its hypre configuration alongside the results |

The login profile already exports `MOOSE_JOBS` and `LIBMESH_JOBS`, so a script must not reuse those
names for its own job counts. The build links the container's `/opt/petsc`, not the `petsc` submodule,
so that submodule is reference source only and patching it changes nothing without a full PETSc
rebuild.

Build on the login node, one container invocation:

```
APPTAINER_CACHEDIR=/scratch/$USER/.apptainer \
/apps/local/apptainer/1.5.3/bin/apptainer exec -B /scratch/$USER \
  oras://mooseharbor.hpc.inl.gov/moose-arco/moose-dev-ubuntu24-cuda-gcc-openmpi-x86_64_graniterapids:2026.08.19 \
  bash -lc 'cd .../framework && env -u LIBMESH_DIR METHOD=oprof make -j4'
```

Run and profile on a GPU node; the login node has none. Put scripts on `/scratch`, which is shared,
because `/tmp` is node-local and `srun` lands elsewhere:

```
srun --wckey moose -N 1 --ntasks-per-node=1 -G 1 --time=0-00:40 \
  <apptainer exec ... --nv> bash /scratch/$USER/tmp/<script>.sh
```

### Driving the profilers

For Nsight Systems, use the `nsight-systems` skill. Its bootstrap on this machine fails unless
`NSYS_PATH` names the target-tree binary rather than the `nsys` on `PATH`, which is recorded in the
memory file `nsys-skill-bootstrap-nsys-path.md`; the nsight-systems 2026.3.2 bundled interpreter has
the report dependencies and needs no `_posixshmem` shim. Its bounded SQL times out at 10 s, so
restrict a memcpy-to-range join rather than scanning `NVTX_EVENTS` whole.

Two queries worth keeping. To attribute GPU idle or a transfer to a MOOSE section, join the activity
row's host-side timestamp to the innermost enclosing `NVTX_EVENTS` range, and exclude PETSc's own
`VecCUDACopyTo` and `VecCUDACopyFrom` ranges from the candidate set or they swallow every copy. To
separate matrix-free applications by level, join each `MatMult` range to what nests inside it: the
fine level's contain a `NonlinearSystemBase::computeKokkosJacobianVectorProduct` range, and duration
clusters then separate p=4 from p=2.

For kernel internals use Nsight Compute at `/opt/nvidia/nsight-compute/2026.2.1/ncu` with a few
`--section` arguments and `--launch-skip`/`--launch-count` rather than `--set full`, writing CSV to a
file and filtering it.
