!alert construction
This tutorial is incomplete, but feel free to browse the currently available content. Otherwise, refer back to the [examples_and_tutorials/index.md] page for other helpful training materials or check out the MOOSE [application_development/index.md] pages for more information.

# Tutorial 1: Application Development id=tutorial-1

In this tutorial, the reader shall work through the steps to create and solve physics problems using MOOSE while learning to develop their own custom application. The instructions for how to how to convert the problem's governing [!ac](PDEs) into an expression that is compatible with MOOSE, will be provided. As the tutorial progresses, the core C++ classes that are available to developers, as well as the basic components of a `MooseObject`, will be discussed. There will be demonstrations included showing how to set up the input files that invoke objects and associated methods. Meanwhile, the theory behind the Galerkin [!ac](FEM), as well as other, more advanced techniques that MOOSE employs, will be discussed.

This tutorial is the focus of the live hosted [examples_and_tutorials/index.md#lws]. New users are encouraged to engage themselves in this training by reading the content carefully and reproducing the steps, without simply copying and pasting code. This tutorial is designed to be an in-depth explanation of creating a complete, custom multiphysics application including the process of using a Git repository and tests. Rest assured, you will be working on a custom application in no time!

## Tutorial Steps id=steps

!content outline location=getting_started/examples_and_tutorials/tutorial01_app_development
                 hide=tutorial-1 steps
                 max_level=6

!!!
I'm thinking each step has the following basic components:
1. Outline/Goals
2. Physics
3. Concepts/Theory
4. Demonstration
  - Source code
  - Input File
  - Results
  - Commit
If we can make that work repeatedly, it'd be neat...
!!!
