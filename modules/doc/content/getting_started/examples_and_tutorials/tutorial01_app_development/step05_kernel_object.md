!content pagination previous=tutorial01_app_development/step04_weak_form.md
                    next=tutorial01_app_development/step06_input_params.md
                    margin-bottom=0px

# Step 5: Develop a Kernel Object

In this step, the basic components of [#kernels] will be presented. To demonstrate their use, a new `Kernel` will be created to solve Darcy's Pressure equation, whose weak form was derived in the [previous step](tutorial01_app_development/step04_weak_form.md#demo). The concept of class *inheritance* shall also be demonstrated, as the object to solve Darcy's equation will inherit from the `ADKernel` class.

## Kernel Objects id=kernels

The [syntax/Kernels/index.md] in MOOSE is one for computing the residual contribution from a volumetric term within a [!ac](PDE) using the Galerkin [!ac](FEM). One way to identify kernels is to check which term(s) in a weak form expression are multiplied by the gradient of the test function $\nabla \psi$, or to check those which represent an integral over the whole domain $\Omega$.

In [kernel-inheritance], notice that the `ADDiffusion` object is declared as one that *inherits* from the `ADKernelGrad` base class, which, in turn, inherits from the `ADKernel` base class. This means that `ADDiffusion` is a unique object that builds upon what the MOOSE Kernels system is designed to do.

!listing framework/include/kernels/ADDiffusion.h
         line=class ADDiffusion
         id=kernel-inheritance
         caption=Syntax defining the `ADDiffusion` class as an object which inherits from the `ADKernelGrad` class.

!alert note title=Automatic Differentiation
"AD" (often used as a prefix for class names) stands for "Automatic Differentiation," which is a feature available to MOOSE application developers that drastically simplifies the implementation of new a `MooseObject`. <!--The non-AD counterparts of objects are the subject of the next tutorial-->

Now, one might inspect the files that provide the base class, i.e., [`ADKernel.h`](framework/include/kernels/ADKernel.h) and [`ADKernel.C`](framework/src/kernels/ADKernel.C), to see what tools are available and decide how to properly implement a new object of this type. Variable members that `ADKernel` objects have access to include, but are not limited to, the following:

- `_u`, `_grad_u`\\
  Value and gradient of the variable being operated on.

- `_test`, `_grad_test`\\
  Value ($\psi$) and gradient ($\nabla \psi$) of the test function.

- `_i`\\
  Current index for the test function component.

- `_q_point`\\
  Coordinates of the current quadrature point.

- `_qp`\\
  Current quadrature point index.

There are several methods that `ADKernel` (and `Kernel`) objects may override. The most important ones are those which work to evaluate the kernel term in the weak form and they are explained in [kernel-methods].

!table id=kernel-methods caption=Methods used by different types of Kernel objects that compute the residual term.
| Base | Override | Use |
| :- | :- | :- |
| Kernel\\ +ADKernel+ | computeQpResidual | Use when the term in the [!ac](PDE) is multiplied by both the test function and the gradient of the test function (`_test` and `_grad_test` must be applied) |
| KernelValue\\ +ADKernelValue+ | precomputeQpResidual | Use when the term computed in the [!ac](PDE) is only multiplied by the test function (do not use `_test` in the override, it is applied automatically) |
| KernelGrad\\ +ADKernelGrad+ | precomputeQpResidual | Use when the term computed in the [!ac](PDE) is only multiplied by the gradient of the test function (do not use `_grad_test` in the override, it is applied automatically) |

For this step, the method to be aware of is `precomputeQpResidual()`; the same one used by [`ADDiffusion`](framework/src/kernels/ADDiffusion.C). The "`Qp`" stands for "quadrature point," which is a characteristic feature of the [Gaussian quadrature](https://en.wikipedia.org/wiki/Gaussian_quadrature). These are the points at which the weak form of a [!ac](PDE) is numerically integrated. Finite elements usually contain multiple quadrature points at specific, symmetrically placed locations and the Gauss quadrature formula is a summation over all of these points that yields an exact value for the integral of a polynomial over $\Omega$.

## Demonstration id=demo

In the [previous step](tutorial01_app_development/step04_weak_form.md#demo), it was shown that the weak form of the Darcy pressure equation is the following:

!equation id=darcy-weak
(\nabla \psi, \dfrac{\mathbf{K}}{\mu} \nabla p) - \langle \psi, \dfrac{\mathbf{K}}{\mu} \nabla p \cdot \hat{n} \rangle = 0

[!eqref](darcy-weak) must satisfy the following [!ac](BVP): $p = 4000 \, \textrm{Pa}$ at the inlet (left) boundary, $p = 0$ at the outlet (right) boundary, and $\nabla p \cdot \hat{n} = 0$ on the remaining boundaries. Therefore, it is possible to drop the second term in [!eqref](darcy-weak) and express it, more simply, as

!equation id=darcy-weak-kernel
(\nabla \psi, \dfrac{K}{\mu} \nabla p) = 0

where $K$ is a scalar and was substituted under the assertion that the permeability be isotropic, such that the full tensor $\mathbf{K}$ may be consolidated into a single value. It was specified on the [Problem Statement](tutorial01_app_development/problem_statement.md#mats) page that the viscosity of the fluid (water) is $\mu = \mu_{f} = 7.98 \times 10^{-4} \, \textrm{Pa} \cdot \textrm{s}$. Also, assume that the isotropic permeability $K = 0.8451 \times 10^{-9} \, \textrm{m}^{2}$ represents the 1 mm steel sphere medium inside the pipe (obtained by setting $d = 1$ in [!eqref](tutorial01_app_development/problem_statement.md#permeability)).

### Source Code id=source-demo

To evaluate [!eqref](darcy-weak-kernel), a new `ADKernelGrad` object can be created and it shall be called `DarcyPressure`. Start by making the directories to store files for objects that are part of the Kernels System:

!include commands/mkdir.md
         replace=['<d>', 'include/kernels src/kernels']

In `include/kernels`, create a file named `DarcyPressure.h` and add the code given in [darcy-header]. Here, the `DarcyPressure` class was defined as a type of `ADKernelGrad` object, and so the header file `ADKernelGrad.h` was included. A `MooseObject` must have a `validParams()` method and a constructor, and so these were included as part of the `public` members. The `precomputeQpResidual()` method was overridden so that [!eqref](darcy-weak-kernel) may set its returned value in accordance with [kernel-methods]. Finally, two variables, `_permeability` and `_viscosity`, were created to store the values for $K$ and $\mu$, respectively, and were assigned `const` types to ensure that their values aren't accidentally modified after they are set by the constructor method.

!listing tutorials/tutorial01_app_development/step05_kernel_object/include/kernels/DarcyPressure.h
         link=False
         id=darcy-header
         caption=Header file for the `DarcyPressure` object.

In `src/kernels`, create a file named `DarcyPressure.C` and add the code given in [darcy-source].
Here, the header file for this object was included. Next, the `registerMooseObject()` method was called for `"BabblerApp"` and the `DarcyPressure` object. The `validParams()` method is the subject of the [next step](tutorial01_app_development/step06_input_params.md), so, for now, it is okay to simply copy and paste its definition. In the constructor method the `_permeability` and `_viscosity` attributes were set according to the values that were given earlier. Finally, the `precomputeQpResidual()` method was programmed to return the left-hand side of [!eqref](darcy-weak-kernel), except that the $\nabla \psi$ (`_grad_test`) term is automatically handled by the base class.

!listing tutorials/tutorial01_app_development/step05_kernel_object/src/kernels/DarcyPressure.C
         link=False
         id=darcy-source
         caption=Source file for the `DarcyPressure` object.

Since the source code has been modified, the executable must be recompiled:

!include commands/make.md

### Input File id=input-demo

Instead of the `ADDiffusion` class, the problem model should now use `DarcyPressure`. In the `pressure_diffusion.i` input file, which was created in [Step 2](tutorial01_app_development/step02_input_file.md#input-demo), replace the `[Kernels]` block with the following:

!listing tutorials/tutorial01_app_development/step05_kernel_object/problems/pressure_diffusion.i
         block=Kernels
         link=False

Now, execute the input file:

!listing language=bash
cd ~/projects/babbler/problems
../babbler-opt -i pressure_diffusion.i

### Results id=result-demo

Visualize the solution with PEACOCK and confirm that it resembles that which is shown in [results]:

!include commands/peacock_r.md
         replace=['<d>', 'problems',
                  'peacock', '~/projects/moose/python/peacock/peacock',
                  '<e>', 'pressure_diffusion_out']

!media tutorial01_app_development/step05_result.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=results
       caption=Rendering of the [!ac](FEM) solution of Darcy's pressure equation subject to the given [!ac](BVP).

One should be convinced that, because this is a steady-state problem, where the coefficient $K / \mu_{f}$ is constant and since the foregoing [!ac](BVP) is a simple one that involves only essential boundary conditions,
the solution produced by the new `DarcyPressure` object, shown in [results],
is identical to the [previous one](tutorial01_app_development/step02_input_file.md#results) produced by the `ADDiffusion` object.

### Commit id=commit-demo

Add the two new files, `DarcyPressure.h` and `DarcyPressure.C`, to the git tracker and update the `pressure_diffusion.i` file:

!include commands/git_add.md
         replace=['*', 'include/kernels src/kernels problems/pressure_diffusion.i']

Now, commit and push the changes to the remote repository:

!include commands/git_commit.md
         replace=['<m>', '"developed kernel to solve Darcy pressure and updated the problem input file"']

!content pagination previous=tutorial01_app_development/step04_weak_form.md
                    next=tutorial01_app_development/step06_input_params.md
