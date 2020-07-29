# Example 05 : Automatic Mesh Adaptivity

MOOSE has support for mesh adaptivity that can automatically refine and coarsen the mesh in areas
of higher/lower error when solving problems.  This can improve the quality of your results in
addition to reducing computation time. You don't need to write any C++ code to use mesh
adaptivity. Instead, it can easily be enabled by filling out the `Adaptivity` section in an input
file:

!listing examples/ex05_amr/ex05.i block=Adaptivity

More details about this functionality are provided on the [Adaptivity](syntax/Adaptivity/index.md)
page.  MOOSE includes multiple [Indicators](Indicators/index.md) you can use to compute different
error estimates in addition to a few [Markers](Markers/index.md).

## Results

The results shown here are created using 6 refinement steps.  However, because it can take a while
to run, the `ex05.i` input file only specifies running 2 mesh refinement steps.  Mesh results from
each of the 6 refinement steps are shown below:

!media large_media/examples/ex5-mesh-0.png
       style=width:50%;margin-left:0%;
       caption=Initial mesh

!media large_media/examples/ex5-mesh-1.png
       style=width:50%;margin-left:0%;
       caption=Adaptivity Step 1

!media large_media/examples/ex5-mesh-2.png
       caption=Adaptivity Step 2
       style=width:50%;

!media large_media/examples/ex5-mesh-3.png
       caption=Adaptivity Step 3
       style=width:50%;

!media large_media/examples/ex5-mesh-4.png
       caption=Adaptivity Step 4
       style=width:50%;

!media large_media/examples/ex5-mesh-5.png
       caption=Adaptivity Step 5
       style=width:50%;

!media large_media/examples/ex5-mesh-6.png
       caption=Adaptivity Step 6
       style=width:50%;


And here is the final solution after all refinement steps are complete:

!media large_media/examples/ex5_out.png
       caption=Example 5 Output
       style=width:80%;

## Complete Source Files

- [examples/ex05_amr/ex05.i]
- [examples/ex05_amr/include/kernels/ExampleCoefDiffusion.h]
- [examples/ex05_amr/src/kernels/ExampleCoefDiffusion.C]

!content pagination use_title=True
                    previous=examples/ex04_bcs.md
                    next=examples/ex06_transient.md
