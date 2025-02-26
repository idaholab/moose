# Step 8: Mesh Adaptivity id=step08

!!end-intro

!---

## Step 8: Uniform Refinement

!listing step08_adaptivity/step8_uniform.i diff=step04_heat_conduction/step4.i block=Mesh

!---

## Step 8: Run coarse problem

Without modification, running the input will produce the same solution as in Step 4:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step08_adaptivity
moose-opt -i step8_uniform.i
```

!---

## Step 8: Run Uniform Refinement

There are several ways to instigate uniform refinement:

1. Directly modify the input to change `uniform_refine = 0` to `uniform_refine = 2`

2. Modify parameter using command-line interface:

   ```bash
   moose-opt -i step8_uniform.i Mesh/uniform_refine=2
   ```

3. Use the `-r` command-line option:

   ```bash
   moose-opt -i step8_uniform.i -r 2
   ```

!---

Each refinement increases the number of elements by a factor of $2^{\texttt{dim}}$

!media results/step8_uniform.png

!---

## Step 8: Mesh Adaptivity

!listing step8_adapt.i block=Adaptivity

!---

## Step 8c: Run Mesh Adaptivity

```bash
moose-opt -i step8_adapt.i
```

!---

!media results/step8_adapt.mp4
