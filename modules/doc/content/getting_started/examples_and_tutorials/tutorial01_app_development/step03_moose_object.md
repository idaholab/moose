# Step 3: Introduction to MOOSE Objects

In this step, [#objects], and the purposes they serve, will be introduced.
To demonstrate this concept, consider the same problem discussed in [Step 2](tutorial01_app_development/step02_input_file.md#physics) and explore the basics of how the Laplace equation was developed in the form of a C++ object.

## Statement of Physics id=physics

Recall, from [Step 2](tutorial01_app_development/step02_input_file.md#physics), that the foregoing [!ac](BVP) is governed by the following [!ac](PDE):

!equation id=laplace
-\nabla \cdot \nabla u = 0 \in \Omega

It is necessary to acknowledge that [!ac](PDEs) must be expressed in a certain format to be able to solve them with MOOSE. That is, assuming that the natural boundary condition, $\nabla u \cdot \hat{n}$, is satisfied, then the *weak form* of [laplace] is

!equation id=weak
\int_{\Omega} \nabla \psi \cdot \nabla u \, d\Omega = 0, \, \forall \,\,\, \psi

In the [next step](tutorial01_app_development/step04_weak_form.md), exactly what is meant by the terminology, "weak form," will be explained when the Galerkin [!ac](FEM) is discussed, so do not worry about this just yet.

## MOOSE Objects id=objects

The core framework of MOOSE is built around systems, where each system uses object-oriented programming to provide a well-defined interface for using the system via C++ inhertience. In short, the framework provides a set of base class(es) for each system. These base classes are specialized using custom C++ objects within an application to achieve the desired behavior for the problem. Resources for object orientiented are abundant, but a solid starting point is [cplusplus.com](https://www.cplusplus.com). This tutorial assumes a basic understanding of both C++ and object-oriented programming.

In practice, all objects in MOOSE and within an application being developed follow the same pattern. Thus, only a broad understanding is required to begin working with MOOSE. For example, consider the `Diffusion` object contianed within the framework as shown in [diffusion-hdr] and [diffusion-src].

!listing framework/include/kernels/Diffusion.h
         re=(?P<remove>\S*^class Diffusion;$.*?<Diffusion>\(\);)
         id=diffusion-hdr
         caption=Header file that declares the `Diffusion` object.

!listing framework/src/kernels/Diffusion.C
         id=diffusion-src
         caption=Source file that define the `Diffusion` object.

This class can be used to demonstrate the form in which all object created in a MOOSE-based application will follow. First, begin with definition in the header. Foremost, all header classes should start with:

!listing framework/include/kernels/Diffusion.h line=#pragma

This line ensures that the compiler does not include the contents of this file more then once.

Next, the base class that is being specialized is included. For the `Diffusion` object, this base class is a `Kernel`, thus the object being created is part of the [Kernels/index.md] system for describing the volumentric term of the weak form. Again, details regarding how the weak form is generated will be covered later in the tutorial.

!listing framework/include/kernels/Diffusion.h line=#include

Finally, the class being created is defined. This definition contains three main compoments common to all object that will be created within an application:

1. a static validParams function for creating input syntax;

   !listing framework/include/kernels/Diffusion.h line=static Input

1. a constructor the has a single input; and

   !listing framework/include/kernels/Diffusion.h line=const InputParameters &

1. one or many methods that are overridden to provide the custo functionality desired for the application.

   !listing framework/include/kernels/Diffusion.h line=virtual

The source (.C) file simply defines the behavior of these three components for the object being created. As the tutorial progresses, this basic structure and syntax of the will be seen regularly and the purpose and use of methods, like `validParams()`, will become more clear.

A `MooseObject` that is capable of solving the Laplace equation was required for [Step 2](tutorial01_app_development/step02_input_file.md). By now, it should be obvious that the `Diffusion` object was capable of solving [laplace] and so there was no need to develop one for this problem.

!alert tip title=The MOOSE does a lot already
Before developing a new `MooseObject` in an application, confirm that something like it does not already exist. All MOOSE applications posses the full capability of the framework and physics modules (more on this later), in addition to their own capabilities.

!content pagination previous=tutorial01_app_development/step02_input_file.md
                    next=tutorial01_app_development/step04_weak_form.md
