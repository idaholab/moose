# MOOSE Workshop

!style halign=center fontsize=150%
!datetime today format=%B %Y

!style halign=center
[www.mooseframework.org](index.md exact=True)

!---

!include user_workshop/intro/inl_background.md

!---

!include user_workshop/intro/moose_introduction.md

!---

!include user_workshop/intro/moose_multiphysics.md

!---

!include user_workshop/intro/getting_started.md

!---

!include user_workshop/problem/overview.md

!---

!include user_workshop/problem/summary.md

!---

!include user_workshop/problem/step01.md

!---

!include user_workshop/numerical/fem_overview.md

!---

!include user_workshop/numerical/fem_shape.md

!---

!include user_workshop/numerical/fem_solve.md

!---

!include user_workshop/problem/step02.md end=end-intro

!---

!include user_workshop/systems/mooseobject.md

!---

!include user_workshop/systems/inputparameters.md

!---

!include user_workshop/systems/kernels.md

!---

# Step 2: Heat Conduction Kernel id=step02b

## (continued)

!include user_workshop/problem/step02.md start=end-intro

!---

!include user_workshop/systems/mesh.md

!---

!include user_workshop/systems/outputs.md

!---

!include user_workshop/problem/step03.md end=end-intro

!---

!include user_workshop/systems/boundaryconditions.md

!---

# Step 3: Boundary conditions id=step03b

## (continued)

!include user_workshop/problem/step03.md start=end-intro

!---

!include user_workshop/problem/step04.md end=end-intro

!---

!include user_workshop/systems/materials.md

!---

!include user_workshop/systems/functions.md

!---

# Step 4: Heat Conduction with Material id=step04b

## (continued)

!include user_workshop/problem/step04.md start=end-intro

!---

!include user_workshop/infrastructure/testing.md

!---

!include user_workshop/problem/step05.md end=end-intro

!---

!include user_workshop/systems/auxkernels.md

!---

# Step 5: Heat Flux Auxiliary Variable id=step05b

## (continued)

!include user_workshop/problem/step05.md start=end-intro

!---

!include user_workshop/problem/step06.md end=end-steady

!---

!include user_workshop/systems/executioners.md

!---

!include user_workshop/systems/timeintegrators.md

!---

!include user_workshop/systems/timesteppers.md

!---

# Step 6: Transient Heat Conduction id=step06b

## (continued)

!---

!include user_workshop/problem/step06.md start=start-transient end=end-transient

!---

!include user_workshop/problem/step07.md

!---

!include workshop/infrastructure/troubleshooting.md

!---

!include user_workshop/problem/step08.md end=end-intro

!---

!include user_workshop/systems/adaptivity.md end=end-intro

!---

# Step 8: Mesh Adaptivity id=step08b

## (continued)

!include user_workshop/problem/step08.md start=end-intro

!---

!include user_workshop/problem/step09.md end=end-intro

All MOOSE Postprocessors are based on the UserObject System, so we will begin with an introduction there.

!---

!include user_workshop/systems/userobjects.md

!---

!include user_workshop/systems/postprocessors.md

!---

!include user_workshop/systems/vectorpostprocessors.md

!---

# Step 9: Postprocessors

## (continued)

!include user_workshop/problem/step09.md start=end-intro

!---

!include user_workshop/infrastructure/mms.md

!---

!include user_workshop/problem/step10.md end=end-intro

!---

!include user_workshop/modules/modules.md

!---

# Step 10: Mechanics id=step10b

## (continued)

!include user_workshop/problem/step10.md start=end-intro

!---

!include user_workshop/problem/step11.md end=end-intro

!---

!include user_workshop/systems/multiapps.md

!---

!include user_workshop/systems/transfers.md

!---

# Step 11: Multiscale Simulation id=step11b

## (continued)

!include user_workshop/problem/step11.md start=end-intro

!---

!include user_workshop/problem/step12.md end=end-intro

!---

!include user_workshop/systems/actions.md

!---

# Step 12: Custom Syntax id=step12b

## (continued)

!include user_workshop/problem/step12.md start=end-intro


!---

!include user_workshop/infrastructure/debugging.md

!---

!include user_workshop/infrastructure/restart.md

!---

!include user_workshop/systems/index.md

!---

!include user_workshop/numerical/fvm_overview.md

!---

!bibtex bibliography title-level=1
