# Step 5: Creating a Kernel Object

In this step, the basic components of [#kernels] will be presented. To demonstrate their use, a new `Kernel` will be created to solve Darcy's Pressure equation, whose weak form was derived in the [previous step](tutorial01_app_development/step04_weak_form.md#demo). The concept of class *inheritance* shall also be demonstrated, as the object to solve Darcy's equation will inherit from the `Kernel` class. The [!ac](BVP) that will be enforced here shall be identical to that used in [Step 2](tutorial01_app_development/step02_input_file.md#physics). Except now, the pressure diffusion will not be so simple, and will depend on certain coefficients added to the Laplace equation.

## Statement of Physics id=physics

Recall that the foregoing zero-gravity, divergence-free form of Darcy's pressure law is given by,

!equation id=darcy-strong
-\nabla \cdot \dfrac{\bold{K}}{\mu} \nabla p = 0 \in \Omega

It was specified on the [Problem Statement](tutorial01_app_development/problem_statement.md#mats) page that the viscosity of the fluid (water), is $\mu_{f} = 7.98 \times 10^{-4} \textrm{Pa} \cdot \textrm{s}$. Also, assume that the permeability tensor takes on an isotropic, scalar value of $K = 0.8451 \times 10^{-9} \textrm{m}^{2}$ to represent the 1 mm steel sphere medium in the pipe<!--This should also be given in the problem statement, so long as it remains constant throughout the tutorial-->.

In the [previous step](tutorial01_app_development/step04_weak_form.md#demo), it was shown that the weak form of [darcy-strong] is the following:

!equation id=darcy-weak
(\nabla \psi, \dfrac{\bold{K}}{\mu} \nabla p) - \langle \psi, \dfrac{\bold{K}}{\mu} \nabla p \cdot \hat{n} \rangle = 0

Since [darcy-strong] must satisfy the following [!ac](BVP): $p = 4000 \, \textrm{Pa}$ at the inlet (left) boundary, $p = 0$ at the outlet (right) boundary, and $\nabla p \cdot \hat{n} = 0$, where $\hat{n}$ is the surface normal vector, on the remaining boundaries, it is possible to drop the second term in [darcy-weak] and express it, more simply, as the following:

!equation id=darcy-weak-kernel
(\nabla \psi, \dfrac{\bold{K}}{\mu} \nabla p) = 0

That is, there shall be no flux through any of the boundaries defined by $\hat{n}$.

There are several approaches for solving the systems of equations that emerge from [!ac](FEM) procedures using MOOSE. The typical approach is to evaluate the *Jacobian*, i.e., the current iterate of the global system of equations. The Jacobian of the residual given by [darcy-weak-kernel] is the following:

!equation id=darcy-jacobian
\mathbf{J} = \nabla \psi \cdot \dfrac{\mathbf{K}}{\mu} \nabla \phi

The term, $\phi$, is known as a *trial function*. Later in this tutorial<!--provide specific link when one becomes available-->, the exact meaning of a "Jacobian", and how to derive one for some residual, as in [darcy-jacobian], will be explained. It was necessary to introduce these concepts here so that a complete `Kernel` object may be developed.

## Kernel Objects id=kernels

The [syntax/Kernels/index.md] in MOOSE is one for computing the residual contribution from a volumetric term within a [!ac](PDE) using the Galerkin [!ac](FEM). One way to identify kernels is to check which term(s) in a weak form expression are multiplied by the gradient of the test function, $\nabla \psi$, or to check those which represent an integral over the whole domain, $\Omega$. By these definitions, or rules-of-thumb, it is clear that [darcy-weak-kernel] is a kernel object.

In [kernel-inheritance], notice that the `Diffusion` class is defined as a `Kernel` object, i.e., it *inherits* from the `Kernel` base class. This means that `Diffusion` is a unique object that builds upon what the MOOSE Kernels system is designed to do.

!listing framework/include/kernels/Diffusion.h
         line=class Diffusion : public Kernel
         id=kernel-inheritance
         caption=Syntax defining the `Diffusion` class as an object which inherits from the `Kernel` class.

Now, inspect the files that provide the `Kernel` base class to decide how to properly implement a new object of this type:

!listing framework/include/kernels/Kernel.h

!listing framework/src/kernels/Kernel.C

It can be seen that `Kernel` objects have access to all of the following members:

- `_u`, `_grad_u`\\
  Value and gradient of the variable this Kernel is operating on

- `_test`, `_grad_test`\\
  Value ($\psi$) and gradient ($\nabla \psi$) of the test functions at the quadrature points

- `_phi`, `_grad_phi`\\
  Value ($\phi$) and gradient ($\nabla \phi$) of the trial functions at the quadrature points

- `_q_point`\\
  Coordinates of the current quadrature point

- `_i`, `_j`\\
  Current index for test and trial functions, respectively

- `_qp`\\
  Current quadrature point index

!alert note title=There's a Lot Going on Here
The reader need not yet worry about what "quadrature points" or "trial functions" are, as these concepts will be discussed later in this tutorial<!--provide a specific hyperlink here once one exists-->. The [next step](tutorial01_app_development/step06_input_params.md) will discuss the purpose and use of the `validParams()` method. Also, while this tutorial assumes the reader has some background knowledge of C++, those who are unfamiliar with the syntax, `public` or `protected`, are encouraged to find reading material regarding [Access Specifiers](https://www.w3schools.com/cpp/cpp_access_specifiers.asp)<!--provide a link to our own in-house docs on access specifiers if we have any-->.

There are several methods that, specifically `Kernel` objects, may perform. Usually, the most important ones are `computeQpResidual()` and `computeQpJacobian()`, where "`Qp`" is an abbreviation for the term, *quadrature point*. `Kernel` objects must override these functions in order to perform their basic tasks that contribute to the solution of the [!ac](PDE). Notice that the `Diffusion` class does exactly this:

!listing framework/include/kernels/Diffusion.h
         start=protected
         end=}

## Demonstration id=demo

### Source Code id=source-demo

To solve [darcy-strong] in the form of [darcy-weak-kernel] and, ultimately, [darcy-jacobian], a new `Kernel` object must be created, and it shall be called `DarcyPressure`. In a MOOSE application, create a directory to store objects that are part of the Kernels system:

```bash
mkdir include/kernels src/kernels
```

In `include/kernels/`, create a file named `DarcyPressure.h` and add the code given in [darcy-header]. Here, the `DarcyPressure` class was defined as a type of `Kernel` object, and so the header file, `Kernel.h`, was included. A `MooseObject` must have a `validParams()` and a constructor method, and so these were included as part of the `public` members. For the object to actually compute something, the `Kernel` methods, `computeQpResidual()` and `computeQpJacobian()`, had to be overridden. Finally, two attributes, `_permeability` and `_viscosity` were created to store the values for $\mathbf{K}$ and $\mu$, respectively, and were assigned `const` types to ensure that their values aren't accidentally modified after they are set by the constructor method.

!listing tutorials/tutorial01_app_development/step05_kernel_object/include/kernels/DarcyPressure.h
         link=False
         id=darcy-header
         caption=Header file for the `DarcyPressure` object.

In `src/kernels`, create a file named `DarcyPressure.C` and add the code given in [darcy-source].
Here, the header file for this object was included. Next, the `registerMooseObject()` method was called for `"YourAppNameApp"` and the `DarcyPressure` object, so be sure to substitute "`YourAppName`". The `validParams()` method is the subject of the [next step](tutorial01_app_development/step06_input_params.md), so, for now, only a short class desciprtion was provided as part of this routine. In the constructor, `DarcyPressure()`, the `_permeability` and `_viscosity` attributes were set according to the values for $\mathbf{K}$ and $\mu$ that were given in the [#physics] section, respectively. Finally, the `computeQpResidual()` and `computeQpJacobian()` methods were programmed to return [darcy-weak-kernel] and [darcy-jacobian], respectively.

!listing tutorials/tutorial01_app_development/step05_kernel_object/src/kernels/DarcyPressure.C
         replace=["babbler", "YourAppName"]
         link=False
         id=darcy-source
         caption=Source file for the `DarcyPressure` object.

### Input File id=input-demo

Now that the source code for the MOOSE application has been modified, the executable must be recompiled:

```bash
cd ~/projects/YourAppName
make -j4 # use number of processors available on your system, i.e., 2, 4, ..., 12, or <N_procs>
```

Instead of the `Diffusion` class, the problem model should now use `DarcyPressure`. In the input file, `pressure_diffusion.i`, created in [Step 2](tutorial01_app_development/step02_input_file.md#input-demo), replace the `[Kernels]` block with the following:

!listing tutorials/tutorial01_app_development/step05_kernel_object/problems/pressure_diffusion.i
         block=Kernels
         link=False

### Results id=result-demo

Once the application has been recompiled, and `pressure_diffusion.i` has been updated, run the input file:

```bash
cd ~/projects/YourAppName/problems
../YourAppName-opt -i pressure_diffusion.i
```

Finally, visualize the solution with PEACOCK and verify that it resembles that which is shown in [results]:

```bash
cd ~/projects/YourAppName/problems
~/projects/moose/python/peacock/peacock -r pressure_diffusion_out.e
```

!media tutorial01_app_development/step05_result.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=results
       caption=Rendering of the [!ac](FEM) solution of [darcy-strong] subject to the [!ac](BVP) given in [#physics].

The reader should be convinced that, because the problem is one of steady-state diffusion, where the coefficient, $\mathbf{K} / \mu$, is constant, and since the foregoing [!ac](BVP) is a simple one that only involves essential boundary conditions,
the solution produced by the new `DarcyPressure` object, shown in [results],
is identical to the [previous one](tutorial01_app_development/step02_input_file.md#results) produced by the `Diffusion` object.

### Commit id=commit-demo

Add the two new files, `DarcyPressure.h` and `DarcyPressure.C`, to the git tracker and update the `pressure_diffusion.i` file:

```bash
cd ~/projects/YourAppName
git add include/kernels/ src/kernels/ problems/pressure_diffusion.i
```

Now, commit and push the changes to the remote repository:

```bash
git commit -m "Created kernel to solve Darcy pressure and updated the problem input file"
git push
```

!content pagination previous=tutorial01_app_development/step04_weak_form.md
                    next=tutorial01_app_development/step06_input_params.md
