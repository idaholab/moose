# When a constitutive update should be batched, and when it should be fused

Measurements on the cost of a NEML2 constitutive update relative to the cost of storing its output,
taken while deciding whether NEML2 should serve as MOOSE-Kokkos's material property system. The
design discussion lives in idaholab/moose#33859; this file holds only the numbers and what they do
and do not support.

This is a sibling of `elasticity_cantilever/PERFORMANCE_NOTES.md` and follows its rules: no number
without its build identity, measured and inferred kept apart, and refuted claims kept rather than
deleted. It is deliberately not tracked, like that file.

## Build identity

MOOSE `9e13bce73b0` and later on branch `kokkos-pa`. Two environments, and the difference matters:

- **CPU measurements**: apptainer `moose-dev-ubuntu24-gcc-mpich-x86_64_v3:2026.08.23`, libMesh from
  `libmesh/installed`, `METHOD=opt`, NEML2 2.1.2, libtorch 2.13.0. Single rank, single thread.
- **GPU measurements**: apptainer `moose-dev-ubuntu24-cuda-gcc-openmpi-x86_64_v3:2026.08.23`, CUDA
  13.3, 8x NVIDIA L4 (23 GB, compute 8.9), PETSc with CUDA and Kokkos 4.7.4, libMesh rebuilt for
  OpenMPI. `METHOD=opt`.

The CPU figures reproduced across both containers within 2 to 4 percent despite the change of MPI,
PETSc and compiler, which is the main reason to trust them.

## The measurement: constitutive cost per quadrature point

`NEML2ModelExecutor::NEML2 solve` from the perf graph, divided by quadrature points and by calls.
Taken at 46,656 points, which is inside the regime where the per-point cost has stopped changing.

| NEML2 model | local implicit unknowns | us/qp (CPU) | transient memory |
| ----------- | ----------------------- | ----------- | ---------------- |
| elasticity, closed form | 0 | **0.189** | 1.1 KB/qp |
| radial_return, Perzyna | 1 scalar | **7.28** | 14.4 KB/qp |
| perfect plasticity | - | **10.37** | 25.4 KB/qp |
| isoharden, Voce | 8 | **11.07** | 10.7 KB/qp |

Storing the output costs 672 B/qp for NEML2's symmetric stress and tangent written once and read
once, or 1440 B/qp for the dense representation. At a measured 14.35 GB/s of single-core read plus
write bandwidth that is 0.047 and 0.100 us/qp.

**The discriminator is binary: does the model require a local implicit solve?** 0.189 us/qp without,
7.3 to 11.1 with. A factor of 39 to 59 with nothing in the gap. So:

- **Closed-form models.** Storing the output costs a quarter to a half of computing it. Fusing the
  update into the element kernel is worth real time. The elasticity cantilever lives here.
- **Anything with a local solve.** Storing the output costs under 1.5 percent of computing it.
  Batching is the right design and fusing buys nothing.

There is no intermediate regime to design for.

## Batch size, and the fixed cost

Per-point cost against batch size, `isoharden`, CPU:

| quadrature points | us/qp |
| ----------------- | ----- |
| 1,728 | 15.77 |
| 13,824 | 11.00 |
| 46,656 | 11.08 |
| 110,592 | 11.07 |

Fixed overhead amortises by roughly 13,000 points and is flat thereafter across an eight-fold range.
**NEML2 wants batches of order 10^4 points.** Element granularity is 8 points per hexahedron, three
to four orders of magnitude below that, which is why the two designs cannot be interleaved at element
level and why any interoperation is a per-assembly barrier.

## What is not the cost

Two variants at 46,656 points, `isoharden`, CPU:

| configuration | us/qp |
| ------------- | ----- |
| default, JIT on | 11.073 |
| `Settings/disable_jit=true` | 13.447 |
| `production=true` on every model | 11.061 |

JIT accounts for 21 percent. The `production` option, which is **off by default** and leaves
training-oriented graph and tensor version tracking enabled, accounts for nothing measurable. So the
7 to 11 us/qp is genuine solver work, not framework plumbing: NEML2 is not inefficient, those models
are expensive. The pure-elastic floor of 0.189 us/qp is the framework's own per-point cost and it is
small.

## Refuted

**"NEML2's GPU cost is launch-latency bound."** Withdrawn. The claim came from a trace with no NVTX
ranges, taken before libMesh was configured `--with-nvtx`, in which per-call time looked independent
of batch size. With ranges the cost is small synchronised device-to-host transfers in
`NEML2ToMOOSEMaterialProperty`, which slices one batch entry per quadrature point:

| measurement | value |
| ----------- | ----- |
| Device-to-host memcpy operations | 484,032, averaging 80 bytes, 38.7 MB total |
| Host-to-device memcpy operations | 232, averaging 33 KB |
| `cudaLaunchKernel` calls | 2,070 |
| Kernels, total execution | 2,361, summed 44 ms |
| Kernel timeline coverage | 0.42 percent, upper bound |
| `cudaMemcpyAsync` inside `NonlinearSystemBase::Kernels` | 207,360 |
| `cudaMemcpyAsync` inside `NEML2ModelExecutor::NEML2 solve` | **727** |

The GPU is idle for essentially the whole trace. Setting `output_device=cpu`, so the output lands on
the host in one bulk transfer per property instead of one per point, took a `refinement`-equivalent
run from 147.1 s to 69.5 s, a factor of 2.1, while the model solve time barely moved, 1.854 to
1.756 s.

Everything derived from those pre-NVTX GPU numbers is void: the fitted fixed and marginal terms, the
claimed GPU-versus-CPU crossover near 13,800 points, and the argument that larger GPUs need
proportionally larger batches. They characterised a bridge defect, not an architecture.

**"Memory capacity is the strongest argument against NEML2 owning property storage."** Weakened to
the point of not being worth making. At 10 KB per quadrature point an L4 holds about 2M points and an
H200 about 13M.

**"The L4's 1/64-rate FP64 biases these results."** Wrong premise. The workload is not
arithmetic-bound, so FP64 throughput is close to irrelevant. The L4's real differences from a
datacentre card are bandwidth, roughly 16x, and capacity, roughly 6x.

## Open, and deliberately unmeasured

No GPU figure here survives the NVTX correction, so the regime question on GPU is **unmeasured**. It
needs a rerun with ranges enabled, and on hardware whose FP64 and bandwidth are not an inference
card's. Reported percentages of wall time were avoided throughout, because these inputs use MOOSE's
default linear solver, which took 41 s of 76 s in one case; any such percentage would describe the
solver choice rather than the material system.

## Storing and aliasing a broadcast tangent

MOOSE `c09915bc129`, apptainer `moose-dev-ubuntu24-cuda-gcc-openmpi-x86_64_v3:2026.08.23`, libMesh from
`libmesh/installed`, `METHOD=opt`, one NVIDIA L4 of 23034 MiB. Input `neml2_tangent_scaling.i` in this
directory, linear elastic NEML2 model whose tangent does not vary along the batch, so NEML2 returns it as
a broadcast. Peak device memory sampled from `nvidia-smi --query-compute-apps` for the run's own PID.

`n` is elements per side; every element is a hexahedron with eight quadrature points. One copy of the
tangent is `qp * 36 * 8` bytes.

| n | quadrature points | one copy | subdomain, aliased | per qp, aliased | per qp, compacted |
| - | ----------------- | -------- | ------------------ | --------------- | ----------------- |
| 40 | 512,000 | 140.6 MiB | 1290 MiB | 1432 MiB | 2426 MiB |
| 60 | 1,728,000 | 474.6 MiB | 2396 MiB | 2874 MiB | 6206 MiB |

All three configurations report the same tangent entry, the Lame parameter 0.5769231, so none of them is
cheap by virtue of storing nothing.

**Storing the property per subdomain saves exactly one copy.** 142 MiB measured against 140.6 predicted,
and 478 against 474.6: within one percent at both sizes. That is Kokkos storage, allocated through
`createDevice()`.

**Aliasing the broadcast instead of compacting it saves seven copies, not one.** 994 MiB and 3332 MiB,
which is 7.07 and 7.02 times one copy, the same factor at both mesh sizes and unchanged when the run is
given a second time step.

The libtorch caching allocator's own statistics, read through
`c10::cuda::CUDACachingAllocator::getDeviceStats`, say where those copies are. Six are simultaneously
live, not retained-but-freed: `allocated_bytes.current` rises by one copy on each of the first six calls,
554.6, 1029.2, 1503.8, 1978.4, 2453.0, 2927.7 MiB at n = 60, and is flat thereafter. Aliasing instead
holds none of them, staying at 80 MiB for the whole run. Reserved memory exceeds live by roughly one
further copy, 3842 against 3402 MiB peak, which is ordinary pool headroom.

The six are three material instances holding two each. MOOSE creates block, face and neighbor copies of
every material, and the probe shows all three compacting: `kokkos_jacobian`, `kokkos_jacobian_face` and
`kokkos_jacobian_neighbor` allocate on calls one to three and again on calls four to six without
releasing the first. Each instance's first copy is pinned by its cached `Moose::Kokkos::Dispatcher`, whose
`_functor_device` member is copy-constructed once and holds the material's `neml2::Tensor` by value.
Nothing consumes the face or neighbor property here, so two thirds of the compaction is waste even before
the pinning.

An earlier draft of this section attributed the factor to the allocator retaining freed blocks. That was
wrong: the blocks are live. The distinction matters because it is not something an allocator setting would
recover.

Both causes were then fixed, in `KokkosNEML2ToMOOSEMaterialProperty`. The face and neighbor copies no
longer run at all, because a NEML2 batch entry is an element quadrature point while their datum carries a
facial quadrature point offset, so they were not merely wasteful but writing values that indexed nothing.
The compacted copy is now owned as a `Moose::Kokkos::Array` that is allocated once and rewritten in place,
wrapped so that the strided gather writes straight into it, which removes both the intermediate allocation
and the pinning. Measured at n = 60 with the compaction forced:

| | peak device | libtorch live bytes |
| - | ----------- | ------------------- |
| before | 6206 MiB | grows to 2928 MiB, six copies |
| after | 3350 MiB | flat at 80 MiB, no copies |

3350 MiB is the aliased figure of 2874 plus one owned buffer of 476, which is the one copy a strided
output genuinely requires. The aliased path is unchanged at 2396 and 2874 MiB, and all three
configurations still report the Lame parameter.

The pinning is not a correctness problem. `Dispatcher`'s copy constructor rebuilds `_functor_device` from
the live `_functor_host` reference rather than from its own snapshot, so the functor Kokkos launches
carries current state; this was checked because the bridge re-aliases a device pointer that does move
between solves, which was confirmed by printing it.

Together the two account for a factor of **2.6** in peak device memory at n = 60, 6206 MiB against
2396 MiB. Note that this is the regime the CPU crossover argues against on other grounds: a tangent that
does not vary along the batch means a closed-form model, where fusing the update into the element kernel
beats batching it through NEML2 regardless of how the result is stored.
