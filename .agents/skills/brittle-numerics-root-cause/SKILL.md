---
name: brittle-numerics-root-cause
description: >-
  Use when a numerical calculation or test gives different results under compiler flags,
  optimization level, hardware, thread count, or execution order. Treat this as a symptom that
  some part of the algorithm is ill-conditioned (near-singular solve, an algebraically-truncated
  slowly-converging series, large cancelling terms) — not as flaky-test noise to suppress by
  pinning flags, loosening tolerances, or blindly regolding. Walks through instrumenting for real
  per-iteration data, finding the actual non-robust mechanism, validating a targeted fix against
  that data, and anticipating the blast radius on other tests before regolding.
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

## Anti-patterns (don't reach for these first)

- Pinning or loosening compiler flags to make the test pass again.
- Regolding the test to whatever the new answer happens to be, without understanding why it moved.
- Loosening the test's tolerance until the diff disappears.

Any of these can be a legitimate *final* step, but only after you understand why the value is
unstable — otherwise you're just relocating where the instability bites next time.

## Procedure

1. **Reproduce deterministically first.** Confirm the failure is deterministic given fixed
   inputs/flags (same build + same run always gives the same wrong answer). This rules out actual
   nondeterminism (uninitialized memory, races, order-dependent floating sums) and confirms you're
   chasing numerical conditioning, not a different bug class.
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
   rather than erratic.
8. **Separate "already-known artifact in the gold" from "new bug."** If something ugly turns up
   mid-investigation (e.g. a value that should be physically non-negative but isn't), diff against
   old logs/gold to check whether it predates your change before treating it as something you
   caused.

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
`custom_cmp` for Exodiff — see below, and run-tests), with a comment recording the physical
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

MOOSE's `Exodiff` tester has no `override_columns` equivalent; instead point `custom_cmp` at a
command file passed to `exodiff -f`. Build it, don't hand-write it:

- `exodiff -summary gold/<file>.e` prints every variable with its file-wide peak magnitude,
  already formatted as `NODAL VARIABLES`/`ELEMENT VARIABLES` blocks — use this as the starting
  template (it also tells you each column's peak, which the sizing rule below needs).
- Give every variable-type block `(all)` (`GLOBAL VARIABLES`, `NODAL VARIABLES`, `ELEMENT
  VARIABLES` alike — exodiff's `Parse_Variables` in `ED_SystemInterface.C` parses the `(all)` flag
  and the indented per-variable override list independently of each other and of block type) with
  **no** explicit `relative`/`floor` on the header line. With no tolerance of its own, `(all)`
  falls through to the `-F <abs_zero> -t <rel_err>` the tester already passes on the command line
  from the test spec — so the rest of the file's comparison stays exactly as strict as before, and
  it stays correct if the spec's `abs_zero`/`rel_err` change later. Because the two are independent,
  this holds even for a block containing some affected variables: list only the affected
  variable's override line(s) under `(all)` and leave every unaffected variable out entirely,
  rather than enumerating them by hand just because they share a block with an affected one. `(all)`
  also keeps the file resilient to new output variables being added later.
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
- Look for a sibling `.cmp` file already in the test suite before inventing the format — e.g. a
  neighboring test's `custom_cmp` file that already floors a different near-zero column is the
  precedent to extend (per the Procedure's "look for precedent in sibling code" step above), not a
  new convention to invent.
