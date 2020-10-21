# Example 20 : UserObjects

The problem is time-dependent diffusion with Dirichlet boundary conditions of 0 on the left and 1 on the right. The diffusion coefficient being calculated bu the `Material` is dependent on the average value of a variable on each block. Thus, as the concentration diffuses, the diffusion coefficient increases, but the coefficient is different for each block (based on the average of the variable on that block).

To achieve this, we need 3 objects working together :

-  `BlockAverageValue` : A `UserObject` that computes the average value of a variable on each block of the domain and provides `averageValue()` for retrieving the average value on a particular block.
-  `BlockAverageDiffusionMaterial` : A `Material` that computes "diffusivity" based on the average value of a variable as computes by a `BlockAverageValue` `UserObject`.
-  `ExampleDiffusion` : The same `Kernel` we have seen before that uses a "diffusivity" material property. The main purpose of this class is to provide the `averageValue` method that acepts a `SubdomainID`, which is simply an integer value specifying whcih block of the mesh to perform the average value calculation.

[](---)

## Create BlockAverageValue UserObject

The first step is to create a `UserObject` for computing the average value of a variable on block (subdomain). The complete header and source file for this custom `UserObject` are linked below, within each file the comments detail the functionality of the class.

[BlockAverageValue.h](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/include/userobjects/BlockAverageValue.h)

[BlockAverageValue.C](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/src/userobjects/BlockAverageValue.C)

[](---)

## Create BlockAverageDiffusionMaterial Material

The second step is to create the Material object that will utilize the block average value for computing a diffusion coefficient. The complete header and source file for this custom `Material` object are linked below, which includes comments detailing the functionality of the `Material`. This class simply creates a `Material` object that creates a material property, "diffusivity", that is computed by the `BlockAverageValue` class and accessed via the `averageValue` method.

[BlockAverageDiffusionMaterial.h](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/include/materials/BlockAverageDiffusionMaterial.h)

[BlockAverageDiffusionMaterial.C](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/src/materials/BlockAverageDiffusionMaterial.C)

[](---)

## Create ExampleDiffusion Kernel

In order to utilize the "diffusivity" material property a Kernel that uses a material property as a coefficient is required. This is accomplished by creating a new Kernel, in this case a Kernel that inherits from the MOOSE `Diffusion` Kernel. This newly created Kernel simply ;ultiplies the `Diffusion` Kernel `computeQpResidual()` and `computeQpJacobian()` methods with a material propety. The complete code for this custom is supplied in the links below, agian the comments in the source detail the behavior of the class.

[ExampleDiffusion.h](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/include/kernels/ExampleDiffusion.h)

[ExampleDiffusion.C](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/src/kernels/ExampleDiffusion.C)

[](---)

## Running the Problem

This example may be run using Peacock or by running the following commands from the command line.

```
cd ~/projects/moose/examples/ex20_user_objects
make -j8
./ex20-opt -i ex20.i
```

This will generate the results file, ex2_out.e, as shown in Figure 1 and 2. This file may be viewed using Peacock or an external application that supports the Exodus II format (e.g., Paraview).

!media large_media/examples/ex20_out_4.png
       caption=Figure 1 : Example 20 results after four time steps
       style=width:50%;

!media large_media/examples/ex20_out_10.png
       caption=Figure 2: Example 20 results after ten time steps
       style=width:50%;


[](---)

## Complete Input Files

[ex20.i](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/ex20.i)

[ExampleApp.C](https://github.com/idaholab/moose/blob/devel/examples/ex20_user_objects/src/base/ExampleApp.C)

!content pagination use_title=True
                    previous=examples/ex19_dampers.md
                    next=examples/ex21_debugging.md
