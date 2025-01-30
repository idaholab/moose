# Step 8: Mesh Adaptivity id=step08

!!end-intro

!---

## Step 8a: Coarse Solution

!listing step8a_coarse.i

!---

## Step 8a: Run

With the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step08_adaptivity
../executable/shield_multiphysics-opt -i step8a_coarse.i
```

With a conda MOOSE executable:

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step08_adaptivity
moose-opt -i step8a_coarse.i
```

!---

!media results/step8_coarse.png caption="Coarse mesh"

!---

## Step 8b: Fine Solution

!listing step8b_fine.i

!---

## Step 8b: Run

With the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step08_adaptivity
../executable/shield_multiphysics-opt -i step8b_fine.i
```

With a conda MOOSE executable:

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step08_adaptivity
moose-opt -i step8b_fine.i
```

!---

!media results/step8_fine.png caption="2x uniform refinement"

!---

## Step 8c: Adaptive Mesh Solution

!listing step8c_adapt.i

!---

## Step 8c: Run

With the tutorial executable:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step08_adaptivity
../executable/shield_multiphysics-opt -i step8c_adapt.i
```

With a conda MOOSE executable:

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step08_adaptivity
moose-opt -i step8c_adapt.i
```

!---

!media results/step8_adapt.png caption="Using adaptivity"
