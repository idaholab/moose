!content pagination previous=tutorial01_app_development/step02_input_file.md
                    next=tutorial01_app_development/step04_weak_form.md
                    margin-bottom=0px

# Step 3: Introduction to MOOSE Objects

In this step, [#objects], and the purposes they serve, will be introduced.
To demonstrate this concept, consider the same problem discussed in [Step 2](tutorial01_app_development/step02_input_file.md#demo) and explore the basics of how the Laplace equation was developed in the form of a C++ class.

Anyone who feels that they need to review C++ and/or object-oriented programming are highly encouraged to do so before proceeding with the remainder of this tutorial. The [getting_started/examples_and_tutorials/index.md#programming] provides some helpful starting points for this.

## MOOSE Objects id=objects

The core framework of MOOSE is built around systems, where each system uses object-oriented programming to provide a well-defined interface for using the system via C++ inheritance. In short, the framework provides a set of base class(es) for each system. These base classes are specialized using custom C++ objects within an application to achieve the desired behavior for the problem.

In practice, all objects in MOOSE and within an application being developed follow the same pattern. Thus, only a broad understanding is required to begin working with MOOSE. For example, consider the `ADDiffusion` object contained within the framework as shown in [diffusion-hdr] and [diffusion-src].

!listing framework/include/kernels/ADDiffusion.h
         id=diffusion-hdr
         caption=Header file that declares the `ADDiffusion` object.

!listing framework/src/kernels/ADDiffusion.C
         id=diffusion-src
         caption=Source file that defines the `ADDiffusion` object.

This class can be used to demonstrate the form in which all objects created in a MOOSE-based application will follow. First, begin with the declarations in the header file. Foremost, all class headers should begin with:

!listing framework/include/kernels/ADDiffusion.h line=#pragma link=False

This line ensures that the compiler does not include the contents of this file more then once.

Next, the base class that is being inherited from is included. For the `ADDiffusion` object, the base class is `ADKernelGrad`:

!listing framework/include/kernels/ADDiffusion.h line=#include link=False

Finally, the class being created is defined. This definition contains three main components common to all objects that will be created within an application:

1. a static validParams function for creating input syntax,

   !listing framework/include/kernels/ADDiffusion.h line=static Input link=False

1. a constructor that is given the same name as the `MooseObject` that owns it and that consumes `InputParameters` objects, and

   !listing framework/include/kernels/ADDiffusion.h line=const InputParameters & link=False

1. one or more methods that are overridden to provide the custom functionality desired for the application.

   !listing framework/include/kernels/ADDiffusion.h line=precomputeQpResidual() link=False

   !alert note title=The method(s) overridden differ with each system.
   Each system within MOOSE has one or more base class objects. These objects are specialized by
   overriding one or more methods. The method required to be overridden varies with the base class.
   Please refer to the documentation of each system for more information.

The source (`.C`) file simply defines the functionality of these three components. As the tutorial progresses, the basic structure and syntax described here will be seen regularly and the purpose and use of methods, like `validParams()`, will become more clear.

## Demonstration id=demo

Recall, from [Step 2](tutorial01_app_development/step02_input_file.md#demo), that the foregoing [!ac](BVP) is governed by the following [!ac](PDE):

!equation id=laplace
-\nabla \cdot \nabla u = 0 \in \Omega

It is necessary to acknowledge that [!ac](PDEs) must be expressed in a certain format to be able to solve them with MOOSE. That is, assuming that the natural boundary condition, $\nabla u \cdot \hat{n} = 0$, is satisfied, then the *weak form* of [!eqref](laplace) is

!equation id=weak
\int_{\Omega} \nabla \psi \cdot \nabla u \, d\Omega = 0, \enspace \forall \, \psi

In the [next step](tutorial01_app_development/step04_weak_form.md), exactly what is meant by the terminology, "weak form," will be explained, so do not worry about this just yet.

### Source Code

A `MooseObject` that was capable of solving the Laplace equation was required for [Step 2](tutorial01_app_development/step02_input_file.md) and it was demonstrated that `ADDiffusion` provides this capability. To understand how it works, consider [diffusion-src], and notice that the $\nabla u$ term of the integrand in [!eqref](weak) was coded:

!listing framework/src/kernels/ADDiffusion.C
         start=ADRealVectorValue
         link=False

Here, the $\nabla \psi$ term in [!eqref](weak),
is automatically handled by the `precomputeQpResidual()` method of the [`ADKernelGrad`](source/kernels/ADKernelGrad.md) base class, so there was no need to include this term manually.

!alert tip title=MOOSE does a lot already.
Before developing a new `MooseObject` in an application, confirm that something like it does not already exist. All MOOSE applications posses the full capability of the framework and physics modules (more on this later), in addition to their own capabilities.

!content pagination previous=tutorial01_app_development/step02_input_file.md
                    next=tutorial01_app_development/step04_weak_form.md
