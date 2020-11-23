!config navigation collapsible-sections=[None, None, None, None, None, 'close']

# Examples and Tutorials

This page includes various demonstrations intended to introduce the basics of [!ac](MOOSE) for creating custom applications to solve unique and challenging multiphysics problems. Each example or tutorial focuses on different aspects of MOOSE, primarily the fundamental systems that are available to solve multiphysics problems.

Before proceeding, please visit the [installation/index.md] page and the [getting_started/new_users.md] page. It is assumed that the reader has a minimial understanding in computer programming with [C++](#programming).

## Examples

The [!ac](MOOSE) repository has a directory named `examples` with several subdirectories. Each subdirectory
has code for a MOOSE-based application and input file(s) for running simulations. Each example can
be used by running a binary that is first compiled by running `make` in the applicable subdirectory.
The resulting application binary (named e.g. `ex01-opt`) is used to run input
files.  A guide explaining what each example
demonstrates and how to use it is provided here:

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

### Module Use Examples

Example problems are provided for some of the individual [modules/index.md]. A comprehensive list of all that are available is given below. Simply expand the example set under a module header and follow their links.

###### Contact

!include modules/contact/contact_examples.md

###### Geochemistry

- [modules/geochemistry/tests_and_examples/index.md]

###### Level Set

!include modules/level_set/level_set_examples.md

###### Porous Flow

!include modules/porous_flow/porous_flow_examples.md

###### Reconstructed Discontinuous Galerkin

- [modules/rdg/index.md#example]

###### Stochastic Tools

!include modules/stochastic_tools/stochastic_tools_examples.md

#### <!--empty header for breaking out of the collapsed section-->

The following example combines the Heat Conduction, Tensor Mechanics, and Stochastic Tools modules and demonstrates one of the core purposes of MOOSE---solving multiphysics problems: [modules/combined/examples/stm_thermomechanics.md]

## Tutorials

!col! small=1 medium=1 large=1
!style halign=center
[!icon!school]
!col-end!

!col! small=11 medium=11 large=11
[tutorial01_app_development/index.md]\\
Learn how to develop a MOOSE-based application to solve coupled systems of differential equations in a multiphysics setting. This tutorial teaches you how to create custom MOOSE objects, setup input files to invoke these objects, and how to process and visualize the results of your simulations.
!col-end!

!col! small=1 medium=1 large=1
!style halign=center
[!icon!school]
!col-end!

!col! small=11 medium=11 large=11
[Porous Flow Tutorial](modules/porous_flow/tutorial_00.md)\\
This tutorial guides the user through some commonly-used aspects of the [Porous Flow](modules/porous_flow/index.md) module. It concerns fluid injection through a borehole into a large fluid-filled reservoir. The tutorial begins with simple Darcy flow, and gradually adds more complex phenomena such as coupling with heat and solid mechanics, multi-phase flows and chemical reactions.
!col-end!

## Workshop id=workshop

The [!ac](MOOSE) development team at [!ac](INL) occasionally hosts live workshops. Registration is typically open for all who wish to attend, although space is usually limited. Below are links to the workshop slides and a recent video recording of the workshop<!--change this sentence to be a plural reference, "recording(s)," when/if more workshop recordings become available-->.

- [Workshop Slideshow](https://www.mooseframework.org/workshop)
- [MOOSE Workshop (Summer 2020)](https://www.youtube.com/watch?v=2tJwBsYaLaI)

MOOSE training events will typically be announced on the [home page](index.md exact=true), but be sure to join the [mailing list](help/contact_us.md) for further updates!

## C++ Programming References id=programming

MOOSE developers need only a basic understanding of computer programming with C++ to get started. If possible, it may be helpful to review the following resources:

- [help/c++/index.md]
- [cplusplus.com/doc/tutorial](http://www.cplusplus.com/doc/tutorial/)
- [en.cppreference.com](https://en.cppreference.com/)
- [w3schools.com/cpp/default.asp](https://www.w3schools.com/cpp/default.asp)
- [geeksforgeeks.org/c-plus-plus](https://www.geeksforgeeks.org/c-plus-plus/)
- [Bjarne Stroustrup (2013). The C++ Programming Language, 4th Edition.](https://www.stroustrup.com/4th.html)
- [Brian Kernighan & Dennis Ritchie (1988). The C Programming Language, 2nd Edition.](http://s3-us-west-2.amazonaws.com/belllabs-microsite-dritchie/cbook/index.html)

Besides checking out the references that are listed above, a strong and clear [Google](https://www.google.com/) search can go a long way. Remember that you can always find help from [our Community](help/contact_us.md).

!content pagination use_title=True
                    previous=new_users.md
