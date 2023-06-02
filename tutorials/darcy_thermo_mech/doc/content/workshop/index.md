# MOOSE Workshop

!style halign=center fontsize=150%
!datetime today format=%B %Y

!style halign=center
[www.mooseframework.org](index.md exact=True)

!---

!include workshop/intro/inl_background.md

!---

!include workshop/intro/moose_introduction.md

!---

!include workshop/intro/moose_multiphysics.md

!---

!include workshop/intro/getting_started.md

!---

!include workshop/problem/overview.md

!---

!include workshop/problem/summary.md

!---

!include workshop/problem/step01.md

!---

!include workshop/numerical/fem_overview.md

!---

!include workshop/numerical/fem_shape.md

!---

!include workshop/numerical/fem_solve.md

!---

!include workshop/cpp/01_basics.md

!---

!include workshop/cpp/02_scope.md

!---

!include workshop/cpp/03_types.md

!---

!include workshop/cpp/04_classes.md

!---

!include workshop/cpp/standards.md

!---

!include workshop/problem/step02.md end=end-intro

!---

!include workshop/systems/mooseobject.md

!---

!include workshop/systems/inputparameters.md

!---

!include workshop/systems/kernels.md

!---

# Step 2: Pressure Kernel id=step02b

## (continued)

!include workshop/problem/step02.md start=end-intro

!---

!include workshop/problem/laplace_young.md

!---

!include workshop/systems/mesh.md

!---

!include workshop/systems/outputs.md

!---

!include workshop/problem/step03.md end=end-intro

!---

!include workshop/systems/materials.md

!---

!include workshop/systems/functions.md

!---

# Step 3: Pressure Kernel with Material id=step03b

## (continued)

!include workshop/problem/step03.md start=end-intro

!---

!include workshop/infrastructure/testing.md

!---

!include workshop/problem/step04.md end=end-intro

!---

!include workshop/systems/auxkernels.md

!---

# Step 4: Velocity Auxiliary Variable

## (continued)

!include workshop/problem/step04.md start=end-intro

!---

!include workshop/problem/step05.md end=end-steady

!---

!include workshop/systems/executioners.md

!---

!include workshop/systems/timeintegrators.md

!---

!include workshop/systems/timesteppers.md

!---

# Step 5: Heat Conduction id=step05b

## (continued)

!include workshop/problem/step05.md start=start-transient end=end-transient

!---

!include workshop/systems/boundaryconditions.md

!---

# Step 5: Heat Conduction id=step05c

## (continued)

!include workshop/problem/step05.md start=start-bc

!---

!include workshop/problem/step06.md

!---

!include workshop/infrastructure/troubleshooting.md

!---

!include workshop/problem/step07.md end=end-intro

!---

!include workshop/systems/adaptivity.md end=end-intro

!---

# Step 7: Mesh Adaptivity id=step07b

## (continued)

!include workshop/problem/step07.md start=end-intro

!---

!include workshop/problem/step08.md end=end-intro

All MOOSE Postprocessors are based on the UserObject System, so we will begin with an introduction there.

!---

!include workshop/systems/userobjects.md

!---

!include workshop/systems/postprocessors.md

!---

!include workshop/systems/vectorpostprocessors.md

!---

# Step 8: Postprocessors

## (continued)

!include workshop/problem/step08.md start=end-intro

!---

!include workshop/problem/step09.md end=end-intro

!---

!include workshop/modules/modules.md

!---

# Step 9: Mechanics id=step09b

## (continued)

!include workshop/problem/step09.md start=end-intro

!---

!include workshop/problem/step10.md end=end-intro

!---

!include workshop/systems/multiapps.md

!---

!include workshop/systems/transfers.md

!---

# Step 10: Multiscale Simulation id=step10b

## (continued)

!include workshop/problem/step10.md start=end-intro

!---

!include workshop/problem/step11.md end=end-intro

!---

!include workshop/systems/actions.md

!---

# Step 11: Custom Syntax id=step11b

## (continued)

!include workshop/problem/step11.md start=end-intro

!---

!include workshop/infrastructure/mms.md

!---

!include workshop/infrastructure/debugging.md

!---

!include workshop/infrastructure/restart.md

!---

!include workshop/systems/index.md

!---

!include workshop/numerical/fvm_overview.md

!---

!bibtex bibliography title-level=1
