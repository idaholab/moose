!config navigation scrollspy=False
<!--I need more room for !gallery display here-->

# Examples and Tutorials class=center style=font-size:270%;color:rgba(0, 88, 151, 1.0);

This page includes various demonstrations of MOOSE usage and hands-on instructions intended to introduce the basics of [!ac](MOOSE) for creating custom applications to solve unique and challenging multiphysics problems. Each example or tutorial focuses on different aspects of MOOSE, primarily the fundamental C++ classes that are available to solve multiphysics problems.

Before proceeding, please visit the [installation/index.md] page and the [getting_started/new_users.md] page. Also, for the larger part of the context in the materials provided on this page, it is assumed that the viewer has some background in computer programming, specifically with [C++](#programming).

## Examples style=font-size:200%;font-weight:500; id=examples

The MOOSE repository has a directory named `examples/` with several subdirectories. Each subdirectory
has code for a full MOOSE-based application and input file(s) for running simulations. Each example can
be built by running binary that can be built by running `make` in the desired example's directory.
This will produce a MOOSE application binary (named e.g. `ex01-opt`) that can be used to run input
files (such as the ones in the example's own directory).  A guide explaining what each example
demonstrates and how to use it is provided here:

!!!
TODO: we should use !content outline here, but only once its possible to not use ordered list numbering so that the indices don't look like: "1. Example 1: As simple As It Gets." This would look silly and redundant.

!content outline max_level=1 location=getting_started/examples_and_tutorials/examples
!!!

!scroll!
- [Example 1: As Simple As It Gets](examples/ex01_inputfile.md)
- [Example 2: Adding a Custom Kernel](examples/ex02_kernel.md)
- [Example 3: Multiphysics Coupling](examples/ex03_coupling.md)
- [Example 4: Custom Boundary Conditions](examples/ex04_bcs.md)
- [Example 5: Automatic Mesh Adaptivity](examples/ex05_amr.md)
- [Example 6: Transient Analysis](examples/ex06_transient.md)
- [Example 7: Custom Initial Conditions](examples/ex07_ics.md)
- [Example 8: Material Properties](examples/ex08_materials.md)
- [Example 9: Stateful Materials Properties](examples/ex09_stateful_materials.md)
- [Example 10: Auxiliary Variables](examples/ex10_aux.md)
- [Example 11: Preconditioning](examples/ex11_prec.md)
- [Example 12: Physics Based Preconditioning](examples/ex12_pbp.md)
- [Example 13: Custom Functions](examples/ex13_functions.md)
- [Example 14: Postprocessors and Code Verification](examples/ex14_pps.md)
- [Example 15: Custom Actions](examples/ex15_actions.md)
- [Example 16: Creating a Custom Timestepper](examples/ex16_timestepper.md)
- [Example 17: Adding a Dirac Kernel](examples/ex17_dirac.md)
- [Example 18: ODE Coupling](examples/ex18_scalar_kernel.md)
- [Example 19: Newton Damping](examples/ex19_dampers.md)
- [Example 20: UserObjects](examples/ex20_user_objects.md)
- [Example 21: Debugging](examples/ex21_debugging.md)
!scroll-end!

## Tutorials style=font-size:200%;font-weight:500; id=tutorials

!gallery! large=6

!card! tutorial01_app_development/moose_intro.png title=[1. Application Development [!icon!link]](tutorial01_app_development/index.md)
Learn how to develop a MOOSE-based application to solve coupled systems of differential equations in a multiphysics setting. Our primary tutorial teaches you how to create custom MOOSE objects: `Kernels`, `Materials`, `BCs`, `Functions`, `Postprocessors`, and more. Also learn how to setup input files to invoke these objects and how to process and visualize the results of your simulations.
!card-end!

!card! media/phase_field/solutionrasterizer.png title=2. Phase Field Module
Coming soon.
!card-end!
!gallery-end!

## Workshop style=font-size:200%;font-weight:500; id=lws

The [!ac](MOOSE) development team at [!ac](INL) will occasionally host live workshops at [!ac](INL) and around the world. Registration is open for all who wish to attend, although space is typically limited. Below are links to the workshop slides as well as video recordings of the live workshops.

- [Workshop Slideshow](https://www.mooseframework.org/workshop)
- [MOOSE Workshop (Summer 2020)](https://www.youtube.com/watch?v=2tJwBsYaLaI)

MOOSE training events will typically be announced on the [home page](/), but be sure to join one of the [mailing lists](new_users.md#join) for further updates!

## C++ Programming References id=programming

MOOSE developers need only a basic understanding of computer programming with C and/or C++ to get started. If possible, it may be helpful to review the following resources:

- [help/c++/index.md]
- [cplusplus.com/doc/tutorial](http://www.cplusplus.com/doc/tutorial/)
- [en.cppreference.com](https://en.cppreference.com/)
- [w3schools.com/cpp/default.asp](https://www.w3schools.com/cpp/default.asp)
- [geeksforgeeks.org/c-plus-plus](https://www.geeksforgeeks.org/c-plus-plus/)
- [Bjarne Stroustrup (2013). The C++ Programming Language, 4th Edition.](https://www.stroustrup.com/4th.html)
- [Brian Kernighan & Dennis Ritchie (1988). The C Programming Language, 2nd Edition.](http://s3-us-west-2.amazonaws.com/belllabs-microsite-dritchie/cbook/index.html)

Besides checking out the references that are listed above, a strong and clear [Google](https://www.google.com/) search can go a long way. Also, remember that you can always find help from [our Community](help/contact_us.md).

!content pagination use_title=True
                    previous=new_users.md
