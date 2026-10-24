---
name: brittle-numerics-root-cause
description: >-
  Use when a numerical calculation or test gives different results under compiler flags,
  optimization level, hardware, thread count, or execution order. Treat this as a symptom that
  some part of the algorithm is ill-conditioned (near-singular solve, an algebraically-truncated
  slowly-converging series, large cancelling terms) — not as flaky-test noise to suppress by
  pinning flags, loosening tolerances, or blindly regolding. Also covers a transient run whose
  adaptive time-step history differs between machines, because rounding decides whether a
  nonlinear solve fails. Walks through these steps: instrument for real per-iteration data, find
  the ill-conditioned mechanism that amplifies small differences such as rounding, and validate a
  targeted fix against that data. Before regolding, anticipate the blast radius on other tests.
  Shows how to reproduce a failure that occurs only on another machine with a perturbation
  ensemble.
---

# Brittle Numerics Root Cause

## Core idea

A calculation whose result changes under compiler flags, hardware, or optimization level is not
"flaky" the way a race condition is. It's almost always exposing an existing ill-conditioned spot
in the algorithm — a near-singular linear solve, a slowly- (algebraically, not exponentially-)
converging series truncated at an arbitrary cutoff, or a subtraction of two large near-equal
numbers — where the compiler's FP rounding choices tip the balance. Genuinely fixing it means
finding and repairing that ill-conditioned spot, not suppressing where it happens to surface.

The near-singular-solve case also surfaces through solver internals, not just compiler/hardware
choices: PETSc can select a different default preconditioner depending on run configuration (e.g.
ILU in serial vs. block-Jacobi-with-ILU-on-the-blocks once the run spans more than one rank), and
if the linear system is ill-conditioned enough, different PCs that each satisfy the same
convergence tolerance can still land on distinguishably different solutions — because a converged
residual only bounds the solution error by roughly `condition number x solver tolerance`. `-ksp_view`
(or `-snes_view`) on the passing and failing runs confirms whether a PC/solver-type change is what's
different before chasing per-iteration instrumentation.

A transient run adds a mechanism that is not about the accuracy of any single solve. With an
adaptive time stepper, rounding can decide whether a nonlinear solve converges or exhausts
`nl_max_its`. A failed solve triggers a cutback, which changes every later time step. Two runs
whose solves fail at different steps therefore follow different time-step sequences for the rest
of the run, and every output on those sequences differs. One flipped solve early in a long
simulation can produce a percent-level gold diff at the end. See
[Failure-path divergence](#failure-path-divergence).

## Anti-patterns (don't reach for these first)

- Pinning or loosening compiler flags to make the test pass again.
- Regolding the test to whatever the new answer happens to be, without understanding why it moved.
- Loosening the test's tolerance until the diff disappears.

Any of these can be a legitimate *final* step, but only after you understand why the value is
unstable — otherwise you're just relocating where the instability bites next time.

Successive regolds that each match a different set of CI jobs are the signature of this pattern.
Each gold records one machine's rounding path, and the next regold only moves which machines fail.

## Procedure

1. **Reproduce deterministically first.** Confirm the failure is deterministic given fixed
   inputs/flags (same build + same run always gives the same wrong answer). This rules out actual
   nondeterminism (uninitialized memory, races, order-dependent floating sums) and confirms you're
   chasing numerical conditioning, not a different bug class. If the failure occurs only on a
   machine you cannot run on, use a [perturbation ensemble](#perturbation-ensembles) to reproduce
   its sensitivity locally.
2. **Instrument, don't guess.** Add temporary per-iteration/per-mode debug prints of the internal
   quantities actually accumulating error (partial sums, determinants, convergence-criterion
   values), or step through with a debugger/conditional breakpoints when the quantity of interest
   is easier to inspect live than to log — either is "instrumenting," pick whichever gets you real
   numbers fastest. If logging, write one small reusable parser for the log format rather than a
   fresh one-off regex per question — you'll ask several follow-up questions of the same data.
3. **Find the mechanism from the data, not from intuition.** Let the instrumented values tell you
   the shape of the problem instead of assuming it. For example: measure the decay rate of a
   truncated series' terms empirically rather than assuming it's exponential; check whether a
   "near-singular" determinant is really near zero or just the result of large-magnitude
   cancellation. The specific check depends on the algorithm — the point is to derive it from what
   the numbers actually do, not from what's typical. For a linear or nonlinear solve where
   different builds or PCs converge to different answers, check whether the discrepancy is within
   roughly `condition number x solver tolerance` — if so, the fix is tightening a tolerance,
   prioritized by problem type. For a nonlinear problem, the Executioner's `nl_rel_tol`/
   `nl_abs_tol` is the tolerance that ultimately governs the final answer's robustness: because
   Newton re-linearizes and self-corrects every iteration, a sufficiently tight nonlinear tolerance
   drives the accepted answer to that residual regardless of which PC/linear-solver path got there,
   as long as the linear solves converge well enough for Newton to keep making progress. `l_tol`/
   `l_abs_tol` mainly governs whether Newton converges at all, since a too-loose linear tolerance
   can stall it before it reaches a tight `nl_rel_tol`. Tightening `l_tol` directly, or replacing
   the linear solve with a direct factorization (e.g. `-pc_type lu -pc_factor_mat_solver_type
   mumps`), are equally effective, fast ways to remove that risk — the direct factorization just
   avoids having to search for a tolerance value that's tight enough. If the divergence persists
   under a tight
   `nl_rel_tol` and a direct linear factorization, that's a sign the matrix is severely enough
   conditioned that roundoff itself is the exposed mechanism — the main procedure's case. Only
   fall back to the tolerance-widening path below if the near-singularity itself is physically
   expected and tightening the solve isn't warranted.

   Tightening has a floor. A relative tolerance is reachable only if the residual it implies is
   above the round-off floor of the converged solve. Per-variable checks such as
   `ReferenceResidualConvergence` make this limit easy to exceed. A variable with a tiny reference
   residual turns `nl_rel_tol` into an absolute target far below the round-off floor. The
   signature has three parts:

   - The residual drops normally and then stays at a fixed level.
   - The solve keeps iterating until `nl_max_its`.
   - The time stepper then cuts dt to `dtmin`.

   The fix is an `nl_abs_tol` above that level, not a tighter relative tolerance.
4. **Look for precedent in sibling code.** If a related algorithm/branch in the same codebase
   already handles this class of problem (e.g., a fallback for non-convergence), extend that
   pattern rather than inventing a new one.
5. **Validate the fix against independent data — and treat "brute force" skeptically too.**
   Cross-check a proposed closed-form correction (tail sum, limit, etc.) against a much
   less-truncated run of the same algorithm (e.g., rerun once, offline, with an absurdly large
   iteration cap). That run is not automatically ground truth: it's still inside the same
   slow-converging/cancellation-prone regime, just further along it, so it can carry its own
   residual error. Treat it as a second, independently-derived estimate to check trend and
   order-of-magnitude agreement against — not an authoritative reference — and corroborate with
   another angle (e.g., does the correction's sign and shape match physical expectation?) before
   trusting either number alone.
6. **Test whether "more correct" complexity actually pays for itself.** An enhancement (e.g. an
   early-exit heuristic) can look principled but not help the actual pathological case if it's
   gated on a coupled quantity that converges slower than the one you're optimizing for. Measure it
   on the failing case specifically before keeping it.
7. **Anticipate blast radius before regolding.** A fix to shared numerical code changes results
   for every consumer of that code, not just the test that was failing. Before mass-regolding: (a)
   enumerate every test exercising the changed code path, (b) sample across parameter regimes that
   stress the fix differently (e.g. a much smaller iteration cap, a much faster-converging case) to
   confirm it degrades gracefully rather than blowing up outside the regime it was tuned against,
   (c) only then regold in bulk, checking the diffs are smooth/deterministic/physically sensible
   rather than erratic. For a validation test, also recompute the metric that the test's
   requirement rests on before you regold. An example is the RMSPE against experiment that the
   documentation's comparison script produces. Large changes in compared columns can leave that
   metric unchanged, and small changes can move it. Only the metric shows whether the requirement
   still holds.
8. **Separate "already-known artifact in the gold" from "new bug."** If something ugly turns up
   mid-investigation (e.g. a value that should be physically non-negative but isn't), diff against
   old logs/gold to check whether it predates your change before treating it as something you
   caused.

## Perturbation ensembles

Rerunning cannot reproduce a failure that occurs on one CI machine and not on yours, but you can
reproduce its sensitivity. Run several copies of the same input; each copy is a member of the
ensemble. In each member, perturb one parameter that feeds the solve by a relative amount of
hundreds to thousands of machine epsilon. In double precision (epsilon about `2.2e-16`), scale a
source magnitude or a material fraction by `1 + k * 1e-13`, with `k = 0..7`. A different compiler,
CPU, or reduction order accumulates a difference of that size over a solve. Run at least four
members. Run more when the first members agree, because a small ensemble can agree by chance.

- **As a reproducer.** If members disagree in the quantity that the test compares, the test is
  sensitive to rounding. One member usually reproduces the remote failure. A diff of the members'
  solver logs finds the first solve where they differ. The remote log alone cannot show that solve.
- **As the acceptance test for a fix.** A fix passes when every member produces the same output to
  round-off. Fewer failed solves is not the criterion, and a closer match to one member is not the
  criterion either. If a change reduces failures but the members still disagree, the instability
  is still there in a different place.
- **For sizing tolerances afterward.** The spread across the members of a fixed run is the
  measured noise floor. Any remaining tolerance must be larger than that floor.

Make sure that the perturbation actually reaches the solve: the members must differ in their
residual history. While you try candidate fixes, keep the ensemble short. For example, run only the
first segment of a transient that contains the sensitive solves. Confirm the chosen fix on the full
run.

## Failure-path divergence

In a transient with an adaptive time stepper, a nonlinear solve can sometimes converge and
sometimes exhaust `nl_max_its`. Such a solve makes the whole time-step history depend on rounding.
These signatures separate this case from an ill-conditioned linear system:

- The linear solves converge to round-off on every iteration, so the matrix is not the problem.
- The nonlinear residual cycles: the same short sequence of values repeats until `nl_max_its`.
- With a variational inequality solver (`vinewtonrsls`), `-snes_vi_monitor` shows the active set
  alternating between the same configurations in each cycle. The reduced-space method switches
  constraints on and off discontinuously. A last-digit difference can therefore decide on which
  side of a switch a Newton step lands.

This case is still ill-conditioned, but the conditioning that matters belongs to a different map.
The Jacobian's condition number bounds how much a perturbation of the linear system's inputs
perturbs that solve's output, and even an optimal condition number leaves a small nonzero output
perturbation. The active-set decision then compares that output against a bound, which is a step
function. The map from simulation input to final output therefore has an effectively infinite
local condition number at the switching surface: an arbitrarily small input change near the switch
produces an O(1) output change. A single decision rarely lands close enough to the switch to flip,
because the chance is roughly proportional to the size of the perturbation. A cycling solve
revisits the same switch on every cycle, though, and a long transient makes many such decisions, so
the chance of at least one flip grows until a machine-dependent history is likely.

Tighter tolerances and direct factorizations do not help here. A change to the bound value or to
`nl_max_its` does not help either. Each cycling solve already reaches the same impasse. Remove
what drives the cycling instead, and then judge the result with a perturbation ensemble. The
change that works is different for each case, so try more than one:

- Damp the Newton step with a line search (e.g. `l2`). Damping stops the full step from
  overshooting into the alternate active set.
- Remove the extrapolated initial guess of a `Predictor` (`Executioner/Predictor/scale=0`). Use
  this change when that guess starts solves on the far side of a constraint switch.

## When targeted tolerance widening is legitimate, not an anti-pattern

Not every diff that varies with build/flags/hardware/thread count is an algorithmic conditioning
bug. A quantity computed as the residual of two much larger, comparable-magnitude terms can be
genuinely near a physical zero for reasons unrelated to any conditioning problem in the production
code — e.g. a test deliberately suppresses the process that quantity measures, a physics residual
that's genuinely near zero at a converged/equilibrium state, or it's a release/growth quantity
still ramping up from zero in the first few steps. There, ordinary
floating-point rounding noise — from any source: compiler codegen, FMA/vectorization, thread or
MPI reduction order — dominates the *relative* error simply because the true value is tiny, while
every other reported quantity in the file stays tight. That's a legitimate case for widening only
that column's tolerance (e.g. MOOSE's `override_columns`/`override_rel_err` for CSVDiff, or
`custom_cmp` for Exodiff — see below), with a comment recording the physical
reason and the measured noise floor — not for a blanket `rel_err` bump on the whole test. Confirm
before doing this that (a) the widened floor stays far enough below any physically meaningful
violation that the test would still catch one — even when the near-zero column is itself what the
test is validating (e.g. a mass-balance residual asserted to be ~0) — and (b) the column is
near-zero for a documented physical/test-setup reason rather than because an upstream calculation
is ill-conditioned — otherwise you're back in this skill's main case.

A related but distinct sub-case is a solver-telemetry column — a linear/nonlinear iteration count
(and any cumulative postprocessor built on one), or a Picard iteration count — rather than a
physical quantity. A non-smooth solve (e.g. frictional/complementarity contact) can take one more
or fewer Newton or linear iterations on a different platform/build without the converged solution
changing at all. Before reaching for a tolerance, though, ask whether the column belongs in the
comparison at all: unless the test's own purpose is to validate solver performance (e.g. a
dedicated iteration-count regression test), the telemetry postprocessor usually shouldn't be
gold-compared in the first place, and dropping it from the comparison — rather than widening a
tolerance to tolerate a quantity the test was never meant to pin down — is the simpler, more
robust fix. Reserve the rest of this sub-case for tests that do intend to track solver performance.
There, this isn't the "near-zero" case above — the count itself is O(10-100), not tiny — so a
`floor` doesn't fit; widen that column's *relative* tolerance instead. Before doing so, confirm
the actual solution variables (nodal variables, and any other global postprocessors) show zero or
negligible diff on the same comparison — that's what tells you the extra iteration changed only the
solver's path, not the answer.

A blanket file-wide `rel_err` bump introduced alongside an unrelated fix is a red flag worth
git-archaeology (`git log`/`git show` on the test spec) even when it isn't currently failing: it
often means this exact situation was mishandled by loosening everything instead of the one or two
affected columns.

### Exodiff: per-variable floor/relative error via `custom_cmp`

MOOSE's `Exodiff` tester has no `override_columns` equivalent; a per-variable tolerance goes in the
`custom_cmp` command file instead. `.agents/skills/write-regression-tests/references/tolerances.md`
holds that file's mechanics: generating it from `exodiff -summary`, and the `(all)` block header
that leaves every unaffected variable at the spec's own tolerances. Which override to write, and how
big to make it, is the part this investigation decides:

- Only add an explicit `floor` (near-zero-residual case) or `relative` (solver-telemetry case, and
  only when the test's purpose is to track solver performance — otherwise omit the telemetry
  postprocessor from the compared variables instead) on the specific affected variable line(s),
  with a comment stating the physical reason. For a floor, keep `-summary`'s auto-generated
  trailing `# min: ... max: ...` comment on that line (and on every other line, for consistency)
  rather than dropping it — it's tool-sourced evidence backing the floor, not hand-typed, so it
  corroborates the hand-written physical-reason comment instead of just asserting it. For a
  solver-telemetry `relative` override, look for an existing precedent value in a sibling `.cmp`
  before picking a number.
- Size that floor as roughly `1e-8 × that variable's own peak magnitude` (from the retained
  `-summary` comment) — a principled, reproducible "many orders of magnitude below the values that
  matter" rule, rather than hand-tuning to the exact noise observed on one build. Then sanity-check
  it against the actually-measured noise from step 2's instrumentation: it should clear the worst
  observed noise by at least ~10x, and dumping the raw variable's full time/node distribution (e.g.
  via `scipy.io.netcdf_file`, since Exodus is classic netCDF) to confirm there's no *real*,
  physically-meaningful value of that same variable sitting just below the chosen floor.
