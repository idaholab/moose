!config navigation collapsible-sections=[None, None, None, None, 'close', None]

# Examples & Tutorials

This page includes various demonstrations intended to introduce the basics of [!ac](MOOSE) for creating custom applications to solve unique and challenging multiphysics problems. Each example or tutorial focuses on different aspects of MOOSE, primarily the fundamental systems that are available to solve multiphysics problems.

## Framework Workshops id=workshop

The [!ac](MOOSE) development team at [!ac](INL) occasionally hosts live training. Registration is
typically open for all who wish to attend, although space is usually limited. Below is a recording
of a MOOSE training webinar given in Summer 2020, as well as links to the training presentation and
other targeted, advanced workshop presentations.

!media https://www.youtube.com/embed/2tJwBsYaLaI
       id=training-webinar
       caption=MOOSE training workshop webinar given on June 9--10, 2020.

- [MOOSE Training Workshop](workshop/index.md alternative=missing_config.md)
- [MOOSE MultiApps Workshop](tutorial02_multiapps/presentation/index.md alternative=missing_config.md)
- [MOOSE Verification Workshop](tutorial03_verification/presentation/index.md alternative=missing_config.md)

MOOSE training events will typically be announced on the [home page](index.md exact=true), but be
sure to join the [mailing list](help/contact_us.md) for further updates!

## Framework Examples

The [!ac](MOOSE) repository has a directory named `examples` with several subdirectories. Each subdirectory
has code for a MOOSE-based application and input file(s) for running simulations. Each example can
be used by running a binary that is first compiled by running `make` in the applicable subdirectory.
The resulting application binary (named e.g. `ex01-opt`) is used to run input
files. A guide explaining what each example demonstrates and how to use it is provided here:

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

## Guided Framework Tutorials

Beyond individual examples, there are also general, guided tutorials that showcase application development,
the [MOOSE MultiApp system](MultiApps/index.md), and verification of calculated solutions.

- [Tutorial 1: Application Development](tutorial01_app_development/index.md) - Learn how to develop
  a MOOSE-based application to solve coupled systems of differential equations in a multiphysics
  setting. This tutorial teaches you how to create custom MOOSE objects, setup input files to invoke
  these objects, and how to process and visualize the results of your simulations.

- [Tutorial 2: MultiApp Demonstration](tutorial02_multiapps/index.md) - Learn how to use the
  [Multiapp](MultiApps/index.md) and [Transfer](Transfers/index.md) systems to couple many multiphysics
  applications together across differing time or length scales.

- [Tutorial 3: Code Verification](tutorial03_verification/index.md) - Demonstrates the use of analytical
  solutions and the [!ac](MMS) for code verification.

- [Tutorial 4: MOOSE Meshing](tutorial04_meshing/index.md) - Demonstrates MOOSE meshing with the
  [Reactor](modules/reactor/index.md) module meshing tools.

## Physics Module Examples and Tutorials

The [modules/index.md] provide models based on the MOOSE framework for a variety of physical phenomena.
These modules provide capabilities that can either be used by themselves, or serve as the basis for
application-specific models tailored to specific problems. Example problems and tutorials are available
for some of the individual physics modules as listed below.

- [Combined](modules/combined/tutorials/index.md) (coupling models from multiple modules)
- [Contact](modules/contact/tutorials/index.md)
- [Geochemistry](modules/geochemistry/tests_and_examples/index.md)
- [Heat Conduction](modules/heat_conduction/tutorials/introduction/index.md)
- [Level Set](modules/level_set/level_set_examples.md)
- [Porous Flow](modules/porous_flow/porous_flow_examples.md)
- [Reconstructed Dicontinuous Galerkin](modules/rdg/index.md#example)
- [Stochastic Tools](modules/stochastic_tools/examples/index.md)
- [Tensor Mechanics](modules/tensor_mechanics/examples_index.md)

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
