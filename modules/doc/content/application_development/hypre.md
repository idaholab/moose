# Hypre / BoomerAMG

## Overview

Hypre is a set of solvers/preconditioners from Lawrence Livermore National Laboratory.  The [main Hypre website can be found here](https://computation.llnl.gov/projects/hypre-scalable-linear-solvers-multigrid-methods).  For MOOSE we mainly use Hypre's algebraic multigrid (AMG) package: BoomerAMG.

AMG is a scalable, efficient algorithm for solution of PDEs that are fairly elliptic.  Many different sets of PDEs fall into that category including heat conduction, solid mechanics, porous flow, species diffusion, etc.

## AMG Algorithm

I hope to fill this out with some details about how AMG works - but I don't have time right now.

## Options

BoomerAMG has an incredible number of options, many of which can have a large impact on solve speed and convergence rate.  The defaults, as set by PETSc, are ok for small two-dimensional problems.  However, if solving in 3D or on over 32 processors you should take some time to familiarize yourself with these options.  It can be daunting, so if you start to get too deep always turn to moose-users for help!

We specify options for Hypre using PETSc command-line option syntax.  On the command-line these take the form of `-option value`.  However, we also supply a way of setting these in the input file.  In both the `Executioner` block and `Preconditioner` blocks you can set `petsc_options_iname` and `petsc_options_value`.  These two hold the parameter names and values, respectively, that you would like to set.

### Turning on BoomerAMG

To turn on Hypre-BoomerAMG preconditioning you would use this in your input file:

```
[Executioner]
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
[]
```

This is equivalent to setting `-pc_type hypre -pc_hypre_type boomeramg` on the command-line.

Notice that it takes two options to turn on BoomerAMG: one to select Hypre - and one to select BoomerAMG from the Hypre package.  Hypre technically contains many solvers and preconditioners, but many of them overlap with what PETSc already provides.

### Strong Threshold

By far, the most important option is `-pc_hypre_boomeramg_stong_threshold`.  This option controls the primary coarsening mechanism: removal of entries from the matrix by simply deciding they're unimportant.  What you're setting here is a _threshold_: the (scaled) value (between 0 and 1) the entry in the matrix must be over to be _kept_.  Everything below the threshold will be discarded.  This means that setting this to a _higher_ amount (between 0 and 1) will discard more of the matrix.  Discarding more entries is generally good for iteration speed (i.e. how fast each trip through BoomerAMG is) but can dramatically impact the quality of the preconditioner so going too far will lead to overall worse performance by requiring a larger number of linear iterations.

By default this is set to 0.25.  That generally works fine in 2D... but is nowhere close for 3D.

If MOOSE detects that you're using Hypre BoomerAMG and running in 3D it will automatically assign `-pc_hypre_boomeramg_strong_threshold` to be `0.7`.  This was chosen by reading a lot of literature and doing some small-scale optimization tests by the MOOSE team.  HOWEVER: 0.7 is NOT a golden number.  Depending on your problem you may need more coarsening (0.8) or less (0.6 or 0.5 to help convergence).  Be warned though: I highly recommend that you never set this below 0.5 for any 3D problem.  The problem will explode with a huge amount of time and memory taken up by the preconditioner.

### Going Deeper

If you're reading this far, then you've probably run into a real problem.  Either you're not getting the speed/scalability you want, or you're not getting convergence.  I'll try to put these in order of importance (in my opinion) and give you some guidance for each one.

In general, speeding up BoomerAMG or improving scalability typically comes from doing _more_ coarsening.  As a reminder: the first thing to do is make sure you have `-pc_hypre_boomeramg_strong_threshold` set appropriately for your problem (see above).  Even if you have it set to `0.25` (for 2D) or `0.7` (for 3D) you might try increasing it some to try to find that sweet spot between effeciency and effectiveness.

### Timing

Before venturing futher, you will definitely want to turn on the performance log ("perf log").  You do that by putting `print_perf_log = true` in the `[Outputs]` block in your input file.  At the end of the solve it will print out a table showing times.

For preconditioning what you want to pay attention to is the `Total Time With Sub` column.  The total time during the nonlinear solve is in the `solve()` row.  Your objective should be to reduce that.  `solve()` is mainly a combination of three things: `compute_residual()`, `compute_jacobian()` and the preconditioner (with a little going to the linear/nonlinear solver in PETSc).

The first thing to do is look at how much of the total time `compute_jacobian()` and `compute_residual()` are taking.  When pushing BoomerAMG far (trying to scale a problem out to many cores) what will happen is that `compute_jacobian()` and `compute_residual()` will take smaller and smaller portions of the total `solve()` time.  At any point you want to keep `compute_residual()` + `compute_jacobian()` to around 60%-70% of the total `solve()` time.  The remainder of the `solve()` time is solver / preconditioning time - with the majority of that going into BoomerAMG.  If BoomerAMG is taking more than 50% of the solve time then either you've scaled your problem too far (try to keep at least 5000 DoFs per processor - 10k is even better) or you need to start adjusting BoomerAMG options.

### More Options

Let's dive into some of the more advanced options.

#### Max Levels

The first option that I want to draw your attention to is also one you should probably leave alone for now, but I point it out because everyone wants to mess with it.  `-pc_hypre_boomeramg_max_levels` controls the number of "levels" in the multigrid solve: i.e. the number of coarser problems that are produced.  The default for this option is `25`.  You might think that you could save time by making this smaller or that you could make the algorithm more accurate by making this larger: actually *neither* of those are the truth!

One thing to understand about multigrid is that it's trying to generate a really small "coarsest" problem that it will actually solve (usually using a direct/Gauss elimination solver).  If you artificially limit the number of levels what you're doing is not allowing the algorithm to reach the coarsest level, which means that you're doing an expensive direct solve on a _larger_ problem... which is _slower_.  Yes, by reducing the number of levels you can actually slow down your solve!

What about increasing the levels?  Well, the problem can only get *so* coarse.  So increasing the number of levels typically has no effect.  Even a problem with millions of DoFs will typically only need ~15 levels or so.  You can see how many levels BoomerAMG is using by turning on `-pc_hypre_boomeramg_print_statistics`.

#### Coarsen Type

As mentioned before, speeding up Hypre is usually done by doing more coarsening.  The main option that controls coarsening is `-pc_hypre_boomeramg_coarsen_type`.  By default this is set to `Falgout` which is a good mix of efficiency and accuracy.  However, there is typically some performance to be gained by using more aggressive options.  In particular, when solving a 3D problem you should try using `HMIS` or `PMIS` (in that order).  Both of these use very little parallel communication but do an excellent job at removing matrix entries to get to coarser problems.

There are a lot more options other than `Falgout`, `HMIS` and `PMIS` - but I'm not going to list them here because those are really the ones you will want to use.

#### Agressive Coarsening

Another option that can do a lot of coarsening is "Aggressive Coarsening".  BoomerAMG actually has many parameters surrounding this - but currently only 2 are available to us as PETSc options: `-pc_hypre_boomeramg_agg_nl` and `-pc_hypre_boomeramg_agg_num_paths`.

`-pc_hypre_boomeramg_agg_nl` is the number of coarsening levels to apply "aggressive coarsening" to.  Aggressive coarsening does just what you think it does: it tries even harder to remove matrix entries.  The way it does this is looking at "second-order" connections: does there exist a path from one important entry to another important entry *through* several other entries.  By looking at these pathways the algorithm will decide whether or not to keep an entry.  Doing more aggressive coarsening will result in less time spent in BoomerAMG (and a lot less communication done) but will also impact the effectiveness of the preconditioner by quite a lot - so it's a balance.

`-pc_hypre_boomeramg_agg_num_paths` is the number of pathways to consider to find a connection and keep something.  That means increasing this value will _reduce_ the ammount of aggressive coarsening happening in each aggressive coarsening level.  What this means is that a higher `-pc_hypre_boomeramg_agg_num_paths` will improve accuracy/effectiveness but slow things down.  So it's a balance.

By default aggressive coarsening is off (`-pc_hypre_boomeramg_agg_nl 0`), so to turn it on set `-pc_hypre_boomeramg_agg_nl` to something higher than zero.  I recommend 2 or 3 to start with, but even 4 can be ok in 3D.  `-pc_hypre_boomeramg_agg_num_paths` defaults to `1`: which is the most aggressive setting.  If the aggressive coarsening levels are causing too many linear iterations, try increasing the number of paths _first_.  Go up to about 4,5 or 6 and see if it helps reduce the number of linear iterations.  If it doesn't, then you may need to back off on the number of aggressive coarsening levels you are doing.  All a balancing act...

#### Interpolation Truncation

You can also coarsen during the interpolation operation.  One way to do that is to set `-pc_hypre_boomeramg_truncfactor`.  This value should be between `0` and `1` and works similarly to the strong threshold: the higher you set it the more entries are ignored.  I recommend a value around `0.3` to start with.  You can adjust this up (maybe 0.4 or 0.5) for some speed or adjust it down (0.2, etc.) for more accuracy.  Balance.

#### Interpolation Type

Speaking of interpolation - it's expensive and how it's done can greatly effect the accuracy and efficiency of BoomerAMG.  Ideally, you should choose an interpolation operation that matches your physics (check the Hypre manual and various Hypre papers for discussion about which interpolation operators are better for which physics) but there are some good rules of thumb.

To change it set `-pc_hypre_boomeramg_interp_type`.  The default is `classic`.  This tends to be really slow and unnecessary - especially for 3D problems.  I recommend starting with `ext+i` (yes, that's what the value of the option is).  It stands for "extended+i".  This is a good all around option that has low communication overhead.

There are *many* more options here, but I'm not going to enumerate them for now.

#### P Max

I'm going to be honest: I don't quite understand what `-pc_hypre_boomeramg_P_max` does exactly.  I've read about it - but I still can't quite get it.  The description from PETSc is: "Max elements per row for interpolation operator".  Setting this low (~2) seems to do a good job.  Setting it higher seems to make the solve less accurate.  However: that goes against my intuition - which is why I don't quite understand what's going on.  If someone knows please email `moose-users` with a good eplanation!

### Putting it All Together

So - what does an "evolved" BoomerAMG options line look like?  Here's one I'm currently using for a 3D Laplacian solve with ~6M elements on ~500 cores:

```bash
-pc_type hypre -pc_hypre_type boomeramg -pc_hypre_boomeramg_strong_threshold 0.7  -pc_hypre_boomeramg_agg_nl 4 -pc_hypre_boomeramg_agg_num_paths 5 -pc_hypre_boomeramg_max_levels 25 -pc_hypre_boomeramg_coarsen_type HMIS -pc_hypre_boomeramg_interp_type ext+i -pc_hypre_boomeramg_P_max 2 -pc_hypre_boomeramg_truncfactor 0.3
```

This is a good place to start if you're looking for advanced usage.  If it's not "strong" enough for you (takes too many linear iterations) then reduce the number of aggressive coarsening levels (`-pc_hypre_boomeramg_agg_nl`) - if it's still too slow... reduce the number of aggressive coarsening paths or try a different coarsening type (like `PMIS`) or add a bit more truncation (maybe go to 0.4 or 0.5).  It's all about balance

## Full List of Options

Here is the full list of Hypre options present in PETSc 3.7.6 (found using `-help | grep -C 5 -i hypre`):

```
HYPRE preconditioner options
  -pc_hypre_type <boomeramg> (choose one of) pilut parasails boomeramg ams (PCHYPRESetType)
HYPRE BoomerAMG Options
  -pc_hypre_boomeramg_cycle_type <V> (choose one of) V W (None)
  -pc_hypre_boomeramg_max_levels <25>: Number of levels (of grids) allowed (None)
  -pc_hypre_boomeramg_max_iter <1>: Maximum iterations used PER hypre call (None)
  -pc_hypre_boomeramg_tol <0.>: Convergence tolerance PER hypre call (0.0 = use a fixed number of iterations) (None)
  -pc_hypre_boomeramg_truncfactor <0.>: Truncation factor for interpolation (0=no truncation) (None)
  -pc_hypre_boomeramg_P_max <0>: Max elements per row for interpolation operator (0=unlimited) (None)
  -pc_hypre_boomeramg_agg_nl <0>: Number of levels of aggressive coarsening (None)
  -pc_hypre_boomeramg_agg_num_paths <1>: Number of paths for aggressive coarsening (None)
  -pc_hypre_boomeramg_strong_threshold <0.25>: Threshold for being strongly connected (None)
  -pc_hypre_boomeramg_max_row_sum <0.9>: Maximum row sum (None)
  -pc_hypre_boomeramg_grid_sweeps_all <1>: Number of sweeps for the up and down grid levels (None)
  -pc_hypre_boomeramg_nodal_coarsen <0>: Use a nodal based coarsening 1-6 (HYPRE_BoomerAMGSetNodal)
  -pc_hypre_boomeramg_vec_interp_variant <0>: Variant of algorithm 1-3 (HYPRE_BoomerAMGSetInterpVecVariant)
  -pc_hypre_boomeramg_grid_sweeps_down <1>: Number of sweeps for the down cycles (None)
  -pc_hypre_boomeramg_grid_sweeps_up <1>: Number of sweeps for the up cycles (None)
  -pc_hypre_boomeramg_grid_sweeps_coarse <1>: Number of sweeps for the coarse level (None)
  -pc_hypre_boomeramg_smooth_type <Schwarz-smoothers> (choose one of) Schwarz-smoothers Pilut ParaSails Euclid (None)
  -pc_hypre_boomeramg_smooth_num_levels <25>: Number of levels on which more complex smoothers are used (None)
  -pc_hypre_boomeramg_eu_level <0>: Number of levels for ILU(k) in Euclid smoother (None)
  -pc_hypre_boomeramg_eu_droptolerance <0.>: Drop tolerance for ILU(k) in Euclid smoother (None)
  -pc_hypre_boomeramg_eu_bj: <FALSE> Use Block Jacobi for ILU in Euclid smoother? (None)
  -pc_hypre_boomeramg_relax_type_all <symmetric-SOR/Jacobi> (choose one of) Jacobi sequential-Gauss-Seidel seqboundary-Gauss-Seidel SOR/Jacobi backward-SOR/Jacobi  symmetric-SOR/Jacobi  l1scaled-SOR/Jacobi Gaussian-elimination      CG Chebyshev FCF-Jacobi l1scaled-Jacobi (None)
  -pc_hypre_boomeramg_relax_type_down <symmetric-SOR/Jacobi> (choose one of) Jacobi sequential-Gauss-Seidel seqboundary-Gauss-Seidel SOR/Jacobi backward-SOR/Jacobi  symmetric-SOR/Jacobi  l1scaled-SOR/Jacobi Gaussian-elimination      CG Chebyshev FCF-Jacobi l1scaled-Jacobi (None)
  -pc_hypre_boomeramg_relax_type_up <symmetric-SOR/Jacobi> (choose one of) Jacobi sequential-Gauss-Seidel seqboundary-Gauss-Seidel SOR/Jacobi backward-SOR/Jacobi  symmetric-SOR/Jacobi  l1scaled-SOR/Jacobi Gaussian-elimination      CG Chebyshev FCF-Jacobi l1scaled-Jacobi (None)
  -pc_hypre_boomeramg_relax_type_coarse <Gaussian-elimination> (choose one of) Jacobi sequential-Gauss-Seidel seqboundary-Gauss-Seidel SOR/Jacobi backward-SOR/Jacobi  symmetric-SOR/Jacobi  l1scaled-SOR/Jacobi Gaussian-elimination      CG Chebyshev FCF-Jacobi l1scaled-Jacobi (None)
  -pc_hypre_boomeramg_relax_weight_all <1.>: Relaxation weight for all levels (0 = hypre estimates, -k = determined with k CG steps) (None)
  -pc_hypre_boomeramg_relax_weight_level <1.>: Set the relaxation weight for a particular level (weight,level) (None)
  -pc_hypre_boomeramg_outer_relax_weight_all <1.>: Outer relaxation weight for all levels (-k = determined with k CG steps) (None)
  -pc_hypre_boomeramg_outer_relax_weight_level <1.>: Set the outer relaxation weight for a particular level (weight,level) (None)
  -pc_hypre_boomeramg_no_CF: <FALSE> Do not use CF-relaxation (None)
  -pc_hypre_boomeramg_measure_type <local> (choose one of) local global (None)
  -pc_hypre_boomeramg_coarsen_type <Falgout> (choose one of) CLJP Ruge-Stueben  modifiedRuge-Stueben   Falgout  PMIS  HMIS (None)
  -pc_hypre_boomeramg_interp_type <classical> (choose one of) classical   direct multipass multipass-wts ext+i ext+i-cc standard standard-wts   FF FF1 (None)
  -pc_hypre_boomeramg_print_statistics: Print statistics (None)
  -pc_hypre_boomeramg_print_statistics <3>: Print statistics (None)
  -pc_hypre_boomeramg_print_debug: Print debug information (None)
  -pc_hypre_boomeramg_nodal_relaxation: <FALSE> Nodal relaxation via Schwarz (None)
```
