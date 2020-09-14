# Step 5: Creating a Kernel Object

In this step, the basic components of [#kernels] will be presented. To demonstrate their use, a new `Kernel` will be created to solve Darcy's Pressure equation, whose weak form was derived in the [previous step](tutorial01_app_development/step04_weak_form.md#demo). The concept of class *inheritance* shall also be demonstrated, as the object to solve Darcy's equation will inherit from the `ADKernel` class.

## Kernel Objects id=kernels

The [syntax/Kernels/index.md] in MOOSE is one for computing the residual contribution from a volumetric term within a [!ac](PDE) using the Galerkin [!ac](FEM). One way to identify kernels is to check which term(s) in a weak form expression are multiplied by the gradient of the test function, $\nabla \psi$, or to check those which represent an integral over the whole domain, $\Omega$.

In [kernel-inheritance], notice that the `ADDiffusion` object is declared as one that *inherits* from the `ADKernelGrad` base class, which, in turn, inherits from the `ADKernel` base class. This means that `ADDiffusion` is a unique object that builds upon what the MOOSE Kernels system is designed to do.

!listing framework/include/kernels/ADDiffusion.h
         line=class ADDiffusion
         id=kernel-inheritance
         caption=Syntax defining the `ADDiffusion` class as an object which inherits from the `ADKernelGrad` class.

!alert note title=Automatic Differentiation
"AD" (often used as a prefix for class names) stands for "Automatic Differentiation," which is a feature available to MOOSE application developers that drastically simplifies the implementation of new a `MooseObject`. <!--The non-AD counterparts of objects are the subject of the next tutorial-->

Now, inspect the files that provide the `ADKernel` base class to decide how to properly implement a new object of this type:

!listing framework/include/kernels/ADKernel.h

!listing framework/src/kernels/ADKernel.C

The most important members that `ADKernel` objects have access to are the following:

- `_u`, `_grad_u`\\
  Value and gradient of the variable this Kernel is operating on

- `_test`, `_grad_test`\\
  Value ($\psi$) and gradient ($\nabla \psi$) of the test functions at the quadrature points

- `_i`
  Current index for test function

- `_q_point`\\
  Coordinates of the current quadrature point

- `_qp`\\
  Current quadrature point index

There are are also several methods that `ADKernel` objects have access to. For now, the one to be aware of is `computeQpResidual` (or `precomputeQpResidual()` for special cases). "Qp" stands for "quadrature point," which is a characteristic feature of the [Gaussian quadrature](https://en.wikipedia.org/wiki/Gaussian_quadrature). These are the points at which the residual terms in the weak form of a [!ac](PDE) are integrated. Finite elements usually contain several quadrature points and, when the integrals at each are superposed, they produce an approximate solution.

## Demonstration id=demo

## Statement of Physics id=physics

Recall that the foregoing zero-gravity, divergence-free form of Darcy's pressure law is given by,

!equation id=darcy-strong
-\nabla \cdot \dfrac{\bold{K}}{\mu} \nabla p = 0 \in \Omega

It was specified on the [Problem Statement](tutorial01_app_development/problem_statement.md#mats) page that the viscosity of the fluid (water), is $\mu_{f} = 7.98 \times 10^{-4} \textrm{Pa} \cdot \textrm{s}$. Also, assume that the permeability tensor takes on an isotropic, scalar value of $K = 0.8451 \times 10^{-9} \textrm{m}^{2}$ to represent the 1 mm steel sphere medium in the pipe<!--This should also be given in the problem statement, so long as it remains constant throughout the tutorial-->.

In the [previous step](tutorial01_app_development/step04_weak_form.md#demo), it was shown that the weak form of [darcy-strong] is the following:

!equation id=darcy-weak
(\nabla \psi, \dfrac{K}{\mu} \nabla p) - \langle \psi, \dfrac{K}{\mu} \nabla p \cdot \hat{n} \rangle = 0

[darcy-strong] must satisfy the following [!ac](BVP): $p = 4000 \, \textrm{Pa}$ at the inlet (left) boundary, $p = 0$ at the outlet (right) boundary, and $\nabla p \cdot \hat{n} = 0$, where $\hat{n}$ is the surface normal vector, on the remaining boundaries. Therefore, it is possible to drop the second term in [darcy-weak] and express it, more simply, as the following:

!equation id=darcy-weak-kernel
(\nabla \psi, \dfrac{K}{\mu} \nabla p) = 0

### Source Code id=source-demo

To solve [darcy-strong] in the form of [darcy-weak-kernel], a new `ADKernelGrad` object must be created, and it shall be called `DarcyPressure`. Start by creating the directories to store files for objects that are part of the Kernels System:

```bash
cd ~/projects/babbler
mkdir include/kernels src/kernels
```

In `include/kernels/`, create a file named `DarcyPressure.h` and add the code given in [darcy-header]. Here, the `DarcyPressure` class was defined as a type of `ADKernelGrad` object, and so the header file, `ADKernelGrad.h`, was included. A `MooseObject` must have a `validParams()` and a constructor method, and so these were included as part of the `public` members. The `precomputeQpResidual()` method was overridden so that [darcy-weak-kernel] may set its returned value. Finally, two attributes, `_permeability` and `_viscosity` were created to store the values for $K$ and $\mu$, respectively, and were assigned `const` types to ensure that their values aren't accidentally modified after they are set by the constructor method.

!listing tutorials/tutorial01_app_development/step05_kernel_object/include/kernels/DarcyPressure.h
         link=False
         id=darcy-header
         caption=Header file for the `DarcyPressure` object.

In `src/kernels`, create a file named `DarcyPressure.C` and add the code given in [darcy-source].
Here, the header file for this object was included. Next, the `registerMooseObject()` method was called for `"BabblerApp"` and the `DarcyPressure` object. The `validParams()` method is the subject of the [next step](tutorial01_app_development/step06_input_params.md), so, for now, it is okay to simply copy and paste its definition. In the constructor method the `_permeability` and `_viscosity` attributes were set according to the values that were given in the [#physics] section. Finally, the `precomputeQpResidual()` method was programmed to return the left-hand side of [darcy-weak-kernel], except that the $\nabla \psi$ (`_grad_test`) term is automatically handled by the base class.

!listing tutorials/tutorial01_app_development/step05_kernel_object/src/kernels/DarcyPressure.C
         link=False
         id=darcy-source
         caption=Source file for the `DarcyPressure` object.

Since the application's source code has been modified, the executable must be recompiled:

```bash
cd ~/projects/babbler
make -j4
```

### Input File id=input-demo

Instead of the `ADDiffusion` class, the problem model should now use `DarcyPressure`. In the `pressure_diffusion.i` input file, which was created in [Step 2](tutorial01_app_development/step02_input_file.md#input-demo), replace the `[Kernels]` block with the following:

!listing tutorials/tutorial01_app_development/step05_kernel_object/problems/pressure_diffusion.i
         block=Kernels
         link=False

Now, execute the input file:

```bash
cd ~/projects/babbler/problems
../babbler-opt -i pressure_diffusion.i
```

### Results id=result-demo

Visualize the solution with PEACOCK and verify that it resembles that which is shown in [results]:

```bash
cd ~/projects/babbler/problems
~/projects/moose/python/peacock/peacock -r pressure_diffusion_out.e
```

!media tutorial01_app_development/step05_result.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=results
       caption=Rendering of the [!ac](FEM) solution of [darcy-strong] subject to the [!ac](BVP) given in [#physics].

One should be convinced that, because this is a steady-state problem, where the coefficient, $K / \mu$, is constant, and since the foregoing [!ac](BVP) is a simple one that only involves essential boundary conditions,
the solution produced by the new `DarcyPressure` object, shown in [results],
is identical to the [previous one](tutorial01_app_development/step02_input_file.md#results) produced by the `ADDiffusion` object.

### Commit id=commit-demo

Add the two new files, `DarcyPressure.h` and `DarcyPressure.C`, to the git tracker and update the `pressure_diffusion.i` file:

```bash
cd ~/projects/babbler
git add include/kernels/ src/kernels/ problems/pressure_diffusion.i
```

Now, commit and push the changes to the remote repository:

```bash
git commit -m "Created kernel to solve Darcy pressure and updated the problem input file"
git push
```

!content pagination previous=tutorial01_app_development/step04_weak_form.md
                    next=tutorial01_app_development/step06_input_params.md
