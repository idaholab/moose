# Step03 Coupling

!---

## MultiApp Coupling

Now we need to combine these capabilities to perform coupled calculations.

Knowing that it is trivially easy for MOOSE-based calculations to couple to Auxiliary Variables or Postprocessor values - it should be obvious how this is going to work:

1. Develop standalone calculations that have stand-in values in Auxiliary fields that are coupled to.
2. Put them together into a MultiApp hierarchy
3. Utilize Transfers to move values from one App to another to fill the Auxiliary fields with "real" values
4. ...
5. Profit!

All that is left to explore is how we *control* such a calculation.

!---

## Loose Coupling

!row!
!col! width=50%
Remembering back to the intro - the simplest coupling is "Loose".  Here we will simply exchange values once per timestep and move on.

This is easily achieved with MultiApps and Transfers.  By default, the MultiApp will be executed at the correct moment during a timestep (as dictated by `execute_on`) and the Transfers associated with that MultiApp are also executed at that time.  Then the simulation will move on to the next timestep.

No problem!

!col-end!

!col width=50%
!media images/coupling.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;box-shadow:none;

!row-end!

!---

## Loose Coupling Example (01_parent.i)

Here we continue with the microstructure calculation.  But now we will add Kernels and MaterialProperties that couple to the transferred fields.

The parent app can be thought of as computing a "source" term - that is then used as a forcing function in the sub-apps.

The sub-apps then compute a "material property"... which is then "upscaled" to the parent app via a PostprocessorInterpolationTransfer.

What we end up with is then a smooth field over the whole parent app domain that represents "Diffusivity" - which is then fed as the "Diffusivity" into the diffusion equation being solved by the parent app.

!---

## Run 01_parent.i

!listing step03_coupling/01_parent.i
         caption=01_parent.i

!---

## Picard

Loose coupling, though easy and stable, may not be accurate.  Since data is only exchanged once per timestep there is the very real possibility that the solutions in both apps are not at an equilibrium with each other.  This is especially true as timesteps grow larger.

To fix this, you can iterate back and forth between the apps until you reach a "stationary point".  In MOOSE we call this Picard iteration, though it has many other names including "Tight Coupling".

To get this behavior with MultiApps, all that is needed is to set `picard_max_its` in the `Executioner` block of the parent app to something greater than 1.  Note that you can do this at any point in a large MultiApp hierarchy!

One caveat: in order for this to work, both apps need to have Backup/Restore capability.  All MOOSE-based applications already have this, but some work is necessary for MOOSE-wrapped apps.

!---

## Picard Cont.

!media images/coupling_02_picard.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;background:white;


!---

## Run 02_parent.i

This solves the same problem as a moment ago - but now using Picard iteration.

- Note how the iterations unfold...
- Watch how the Picard residual continues to go down with each Picard iteration until convergence is met

!---

## Picard With Subcycling

Due to the advanced Backup/Restore capability within MOOSE, we actually have the ability to do Picard iteration even in instances where the two (or more!) apps are not utilizing the same timesteps.  One case where this is useful is with subcycling.

This can allow you to take much larger timesteps with one physics (say solid-mechanics) and much smaller timesteps with another (CFD), while still finding a stationary point between the two.

The important bit here is that an app needs to have its state restored back to the *start time* for the current timestep the parent app is trying to take.

The graphic on the next slide should help...

!---

## Picard With Subcycling Cont.

!media images/coupling_03_subcycling_picard.png
       style=width:80%;margin-left:auto;margin-right:auto;display:block;background:white;

!---

## Run 03_parent.i

Same problem solved again - but now the sub-app is taking smaller timesteps and it sub_cycling.

Everything still works - even in parallel!
