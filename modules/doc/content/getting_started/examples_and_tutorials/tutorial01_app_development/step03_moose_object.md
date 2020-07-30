# Step 3: Introduction to MOOSE Objects

In this step, [#objects], and the purposes they serve, will be introduced.
To demonstrate this concept, the reader shall consider the same problem discussed in [Step 2](tutorial01_app_development/step02_input_file.md#physics) and explore the basics of how the Laplace equation was developed in the form of a C++ object.

## Statement of Physics id=physics

Recall, from [Step 2](tutorial01_app_development/step02_input_file.md#physics), that the foregoing [!ac](BVP) is governed by the following [!ac](PDE):

!equation id=laplace
-\nabla \cdot \nabla u = 0 \in \Omega

It is necessary to acknowledge that [!ac](PDEs) must be expressed in a certain format to be able to solve them with MOOSE. That is, assuming that the natural boundary condition, $\nabla u \cdot \hat{n}$, is satisfied, then the *weak form* of [laplace] is

!equation id=weak
\int_{\Omega} \nabla \psi \cdot \nabla u \, d\Omega = 0, \, \forall \,\,\, \psi

In the [next step](tutorial01_app_development/step04_weak_form.md), exactly what is meant by the terminology, "weak form," will be explained when the Galerkin [!ac](FEM) is discussed, so do not worry about this just yet.

## MOOSE Objects id=objects

The idea of object-oriented programming is to create many different functions that process a well-defined and unique task, and then call upon them wherever they are needed<!--Provide link to discussion of OOP somewhere on the website here-->. Generally, when one refers to [objects in C++ programming](https://www.w3schools.com/cpp/cpp_classes.asp) it could mean several different things. As a basic example, the creation and use of a C++ object might look like that shown in [c++-object]. The reader is encouraged to try to predict what this code would output.

!listing! language=C++
          id=c++-object
          caption=Example use of some C++ object, `myObj`, of type, `myClass` (from [w3schools.com](https://www.w3schools.com/cpp/cpp_classes.asp)).
class MyClass {       // The class
  public:             // Access specifier
    int myNum;        // Attribute (int variable)
    string myString;  // Attribute (string variable)
};

int main() {
  MyClass myObj;  // Create an object of MyClass

  // Access attributes and set values
  myObj.myNum = 15;
  myObj.myString = "Some text";

  // Print attribute values
  cout << myObj.myNum << "\n";
  cout << myObj.myString;
  return 0;
}
!listing-end!

More specifically, when one mentions objects in MOOSE, these are usually a reference to a particular C++ *subclass* that exists within the MOOSE source code, or a particular *instance* of a class. A subclass would be one which inherits from another one and uses it as a template. For example, the `Diffusion` class is a MOOSE object that is a subclass of the `Kernel` *base class*. Any instance of a certain class becomes an object. As in the above example, *myObj* is an instance of *myClass*.

Object-oriented programming in MOOSE forms the edifice for the structure and organization of MOOSE input files, which were discussed in the [previous step](tutorial01_app_development/step02_input_file.md#inputs). For example, recall that the `[BCs]` block in [`pressure_diffusion.i`](tutorial01_app_development/step02_input_file.md#input-demo) was the following:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=BCs
         link=False

Here, since both the `[inlet]` and `[outlet]` blocks become instances of the `DirichletBC` class, they are both objects. The `DirichletBC` class is also an object, since it is an instance of the `BoundaryCondition` base class.

### Custom MOOSE Objects id=custom

All user-facing objects in MOOSE derive from [`MooseObject`](src/base/MooseObject.h), this allows for a common structure for all applications and is the basis for the modular design of MOOSE. A new `MooseObject` requires a *header file* and a *source file*. The basic setup for each is shown in [basic-header] and [basic-source], respectively.
In a MOOSE application, header files will be found in the `include/` directory, while source files will be found in the `src/` directory. Within each of those directories, a `MooseObject` must be filed in the subdirectory whose name corresponds with the base class for which the object inherits from. For example, the files for the `Diffusion` object, which will be discussed in the [next section](#demo), are located at [`moose/framework/include/kernels/`](https://github.com/idaholab/moose/tree/master/framework/include/kernels) and [`moose/framework/src/kernels/`](https://github.com/idaholab/moose/tree/master/framework/src/kernels).

!listing! language=C++
          id=basic-header
          caption=A basic template used for creating a `MooseObject` header file.
#pragma once

#include "BaseObject.h"

class CustomObject : public BaseObject
{
public:
  static InputParameters validParams();

  CustomObject(const InputParameters & parameters);

protected:

  virtual Real doSomething() override;

  const Real & _scale;
};
!listing-end!

!listing! language=C++
          id=basic-source
          caption=A basic template used for creating a `MooseObject` source file.
#include "CustomObject.h"

registerMooseObject("CustomApp", CustomObject);

InputParameters
CustomObject::validParams()
{
  InputParameters params = BaseObject::validParams();
  params.addClassDescription("The CustomObject does something with a scale parameter.");
  params.addParam<Real>("scale", 1, "A scale factor for use when doing something.");
  return params;
}

CustomObject::CustomObject(const InputParameters & parameters) :
    BaseObject(parameters),
    _scale(getParam<Real>("scale"))
{
}

double
CustomObject::doSomething()
{
  // Do some sort of import calculation here that needs a scale factor
  return _scale;
}
!listing-end!

As the tutorial progresses, the basic structure and syntax of the templates given in [basic-header] and [basic-source] will be seen regularly and the purpose and use of functions, like `validParams()`, should become more clear.

## Demonstration id=demo

A `MooseObject` that is capable of solving the Laplace equation was required for [Step 2](tutorial01_app_development/step02_input_file.md). By now, it should be obvious that the `Diffusion` object was perfectly capable of solving [laplace] and so there was no need to develop one for this problem.

!alert tip title=The MOOSE Framework Does a Lot Already
Before one considers developing a new `MooseObject` that they need for their application, they should confirm that something like it does not already exist in the Framework. All MOOSE Applications posses the full capability of the MOOSE Framework, in addition to their own capabilities.

Still, one should wonder about how exactly this object was developed. First off, consider the header file, `Diffusion.h`:

!listing framework/include/kernels/Diffusion.h
         start=class

In this file, the `Diffusion` class has been identified, as well as the data types and names of the functions it performs. In essence, this is the purpose of header files, which have the `.h` extension, in C++ programming.

Notice that there are no indications of [laplace] nor [weak] here. Actually, [weak] is provided as part of the source file:

!listing framework/src/kernels/Diffusion.C
         re=Real\sDiffusion::computeQpResidual.*?^}

In the above code, one should recognize a resemblance to the integrand on the left-hand side of [weak].
This is why it was necessary to present [weak] - MOOSE is designed to solve [!ac](PDEs) in this format. Now, all that remains is to evaluate the integral. Once that is taken care of, a completed `MooseObject` that can solve [laplace] will be realized. The means by which integration is handled will be discussed later in this tutorial.

<!--Need to come back here and verify that all of this accurate once I work through more steps and get a clearer picture of how this all works. I DO NOT want to confuse the reader.-->

!content pagination previous=tutorial01_app_development/step02_input_file.md
                    next=tutorial01_app_development/step04_weak_form.md
