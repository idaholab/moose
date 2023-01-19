!content pagination previous=tutorial01_app_development/step01_moose_app.md
                    next=tutorial01_app_development/step03_moose_object.md
                    margin-bottom=0px

# Step 2: Write an Input File

In this step, the concept of an input file is introduced. These files provide the means for controlling [!ac](FE) simulations with MOOSE. To demonstrate this concept, a steady-state diffusion of pressure from one end of the pipe, between the pressure vessels, to the other (see the [tutorial01_app_development/problem_statement.md] page) will be considered. The goal, here, is to create an input file that solves this simple [!ac](BVP). This problem is detailed in the [#demo] section, but, first, some basic information regarding input files and their execution are provided. As for many steps of this tutorial, concepts will be introduced and a hands-on demonstration will follow.

## Input File Format id=inputs

MOOSE input files use a `.i` extension and are formatted using the [!ac](HIT) format, which is a nested block structure. By convention, blocks beginning with capital letters are syntax that is dictated by the application. Blocks starting with lower case letters are arbitrary names assigned by the creator of the input file. A complete list of the available syntax and the associated objects for MOOSE-based applications is available on the [syntax/index.md] page.

To demonstrate this format, consider a C++ class designed to model the diffusion of some field variable $u$. An object, or a single instance of this class, is invoked in an input file by the following syntax:

!listing tutorials/tutorial01_app_development/step02_input_file/test/tests/kernels/simple_diffusion/simple_diffusion.i
         block=Kernels
         link=False

The syntax `type = Diffusion` denotes the C++ class that is named [`Diffusion`](source/kernels/Diffusion.md) and is capable of solving the Laplace operator, also commonly referred to as the diffusion term.

Within each sub-block, any number of name/value pairs are provided, and these correspond to the parameters required for the given class. Although there are usually several input parameters required, `Diffusion` does not need any except the variable for which to operate on. Variables also have their own block in input files. At the very least, a MOOSE input file requires the following six block types:

- [`[Mesh]`](syntax/Mesh/index.md): Define the geometry of the domain
- [`[Variables]`](syntax/Variables/index.md): Define the unknown(s) of the problem
- [`[Kernels]`](syntax/Kernels/index.md): Define the equation(s) to solve
- [`[BCs]`](syntax/BCs/index.md): Define the boundary condition(s) of the problem
- [`[Executioner]`](syntax/Executioner/index.md): Define how the problem will solve
- [`[Outputs]`](syntax/Outputs/index.md): Define how the solution will be written

It should be obvious that a basic [!ac](FE) analysis problem requires a mesh, variables, equations, and boundary conditions. Also, methods to handle the numerical solve and record its results are necessary, so the `[Executioner]` and `[Outputs]` blocks are required for these purposes.

!alert tip title=Block types
The names of blocks suggest what types of objects can be used within those blocks and the tasks they perform. Please visit the [Syntax Documentation](syntax/index.md) page for a complete list of available syntax and associated objects.

*For more information about input files, please visit the [application_usage/input_syntax.md] page.*

## Execute an Input File id=execute

There are a several ways to execute an input file---a couple of which shall be explained here. From [Step 1](tutorial01_app_development/step01_moose_app.md), the reader should have already created an executable for their application, but, if not, be sure to run the following commands:

!include commands/make.md

### Execute Using the Terminal

The most basic way to execute an input file is from a terminal. To do this, a user may navigate to the directory where their input file is stored and then pass it to the executable with the `-i` argument. For example, the input file for the application's default test may be executed as follows:

!listing language=bash
cd ~/projects/babbler/test/tests/kernels/simple_diffusion
../../../../babbler-opt -i simple_diffusion.i

The input file could also be executed relative to the application's root directory:

!listing language=bash
cd ~/projects/babbler
./babbler-opt -i test/tests/kernels/simple_diffusion/simple_diffusion.i

The application will run the [!ac](FE) simulation and populate the directory with all of the requested outputs, e.g., ExodusII or comma delimited files. Here, a file called `simple_diffusion_out.e` should have been created. As mentioned in [Helpful Software](getting_started/new_users.md#helpful-software), this file can be opened to visualize results of the solution with [ParaView](https://www.paraview.org) or Peacock (the MOOSE in-house [!ac](GUI)).

### Execute Using Peacock

Peacock provides a second way to execute an input file. To demonstrate this, run the following commands:

!listing language=bash
cd ~/projects/babbler/test/tests/kernels/simple_diffusion
~/projects/moose/python/peacock/peacock -i simple_diffusion.i

This will open the Peacock window and display the mesh used for the [simple diffusion test](tutorial01_app_development/step01_moose_app.md#test). To execute the input file, click on the "Execute" tab and then click "Run". Once the solve completes, click on the "ExodusViewer" tab to view the results.

!alert warning title=First execution of Peacock is slow
Peacock is a python application. As such, during the initial execution binary files are cached, which often takes a few minutes. Subsequent executions of Peacock should be quicker.

!alert note title=Bash alias for Peacock
The command-line syntax for running Peacock can be simplified by creating a bash alias for it. Be sure to visit the [python/peacock.md] page to learn how to do this.

## Demonstration id=demo

The Laplace equation shall be employed to model the steady-state diffusion of pressure $u$ on the domain $\Omega$. Thus, find $u$ such that

!equation id=laplace
-\nabla \cdot \nabla u = 0 \in \Omega

satisfies the following [!ac](BVP): $u = 4000 \, \textrm{Pa}$ at the inlet (left) boundary, $u = 0$ at the outlet (right) boundary, and $\nabla u \cdot \hat{n} = 0$, where $\hat{n}$ is the surface normal vector, on the remaining boundaries.

### Input File

In this section, the setup of an input file to model the steady-state diffusion of pressure over the length of the pipe will be detailed. To begin, create a directory named "problems:"

!include commands/mkdir.md
         replace=['<d>', 'problems']

Next, use a [text editor](getting_started/new_users.md#helpful-software) to create a file named `pressure_diffusion.i` in the `problems` directory. This file is where the block-structured ([!ac](HIT)) format will be used to setup the inputs needed to solve the given [!ac](BVP).

!alert note title=Block order doesn't matter.
The blocks need not be listed in any particular order within the input file.

Recall that six basic blocks---those which were mentioned in the [#inputs] section---are required. For the `[Mesh]` block, the geometry of the pipe, whose dimensions were provided on the [tutorial01_app_development/problem_statement.md] page, shall be defined using the MOOSE standard rectilinear mesh generator object, [`GeneratedMesh`](source/mesh/GeneratedMesh.md):

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Mesh
         link=False

!!!
TODO: Maintain whitespace when using block= in the listing command
!!!

This creates a planar mesh with a height equal to the pipe radius. Thus, this problem shall be solved using cylindrical coordinates. To do this, a [`[Problem]`](syntax/Problem/index.md) block shall be included in addition to the basic six blocks:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Problem
         link=False

The purpose of this block is to indicate that the planar mesh represents an axisymmetric body defined with respect to a cylindrical coordinate system.

The pressure variable $u$ shall be added to the `[Variables]` block with an unambiguous name, i.e., `pressure`:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Variables
         link=False

The variables listed here are the variables to be solved for using the [!ac](FEM). Next, the `[Kernels]` block, whose syntax was demonstrated earlier, shall be included. Here, the [`ADDiffusion`](source/kernels/ADDiffusion.md) class will be used to solve [!eqref](laplace) and the `pressure` variable will be operated on:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Kernels
         link=False

Now that the domain $\Omega$, the primary variable $u$, and the [!ac](PDE) have been specified, the [!ac](BVP) must be enforced. The [`ADDirichletBC`](source/bcs/ADDirichletBC.md) class enforces a Dirichlet (essential) boundary condition, e.g., the pressures prescribed at the pipe inlet and outlet. For this problem, two separate `ADDirichletBC` objects are setup under the `[BCs]` block---one for each end of the pipe:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=BCs
         link=False

It is not necessary to enforce the Neumann (natural) boundary conditions, $\nabla u \cdot \hat{n}$, since they are zero-valued fluxes.

The problem being solved does not have a time component, so a steady-state executioner object, [`Steady`](source/executioners/Steady.md), is invoked to solve the resulting equation:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Executioner
         link=False

Finally, the computed [!ac](FE) solution shall be output in ExodusII format by adding the following syntax for the `[Outputs]` block:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Outputs
         link=False

There are a lot of ways to control the solution when using the `Steady` class (or the [`Transient`](source/executioners/Transient.md) class for time-dependent simulations). For instance, the parameters which set the PETSc solver options can be very influential on the convergence rate of the `NEWTON` solve. Throughout this tutorial, the reader is encouraged to test different combinations of input parameters, especially for `Executioner` objects. After running the input file, as it was given here, the reader may remove the PETSc options parameters, run it again, and observe the difference in the convergence rate.

### Results id=result-demo

In [#execute],
some ways in which to execute and visualize the results of an input file were discussed. These methods should be reviewed to find a preferred means of interacting with the application. As a reminder, the [`pressure_diffusion.i`](#input-demo) file can be exectued from the terminal by entering the following:

!listing language=bash
cd ~/projects/babbler/problems
../babbler-opt -i pressure_diffusion.i

If the input file was ran from the terminal, and the `pressure_diffusion_out.e` file exists, the results can be rendered in Peacock by running the following commands:

!include commands/peacock_r.md
         replace=['<d>', 'problems',
                  'peacock', '~/projects/moose/python/peacock/peacock',
                  '<e>', 'pressure_diffusion_out']

After running the above commands, the Peacock window opens to the "ExodusViewer" tab and a result which resembles that shown in [results] will be displayed.

!media tutorial01_app_development/step02_result.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=results
       caption=Rendering of the [!ac](FEM) solution of [!eqref](laplace) subject to the given [!ac](BVP).

Notice that the [!ac](FEM) solution depicted in [results] satisfies the boundary conditions, i.e., a pressure of 4000 Pa can be observed at the inlet and zero pressure at the outlet. The pressure distribution over the length of the pipe also appears to be uniform across its radius, indicating that there is no flux through any of the remaining boundaries. Thus, $\nabla u \cdot \hat{n} = 0$ is also satisfied at those boundaries.

### Commit id=commit-demo

A GitHub repository to store the new MOOSE-based application was created as part of the [previous step](tutorial01_app_development/step01_moose_app.md#repo) of this tutorial. The repository should also contain the first commit to the `origin` remote (the online copy). Now that a new file exists in the local repository, i.e., `problems/pressure_diffusion.i`, and since this input file has been verified to produce good results, it should be committed and pushed to the remote. Before proceeding, inspect the status of the local repository as it compares to the `HEAD`, which denotes the version that existed following the most recent commit:

!listing language=bash
cd ~/projects/babbler
git status

The terminal output should read something like the following:

!listing! language=git
On branch main
Your branch is up to date with 'origin/main'.

Untracked files:
  (use "git add <file>..." to include in what will be committed)
	problems/

nothing added to commit but untracked files present (use "git add" to track)
!listing-end!

This indicates that a new directory `problems` has not been staged, and that there may or may not be more unstaged files in that directory. Of course, in this case, there are. Proceed with the instructions provided in the output for adding the new files and then reinspect the status of the local repository:

!listing language=bash
git add problems
git status

Now, the terminal output should be the following:

!listing! language=git
On branch main
Your branch is up to date with 'origin/main'.

Changes to be committed:
  (use "git restore --staged <file>..." to unstage)
	new file:   problems/pressure_diffusion.i
!listing-end!

To commit these changes, simply enter `git commit`. A user will then be prompted to enter a message describing the changes. For this change, the message added might be "created an input file to solve diffusion problem," as shown in the example below.

!listing! language=git
created an input file to solve diffusion problem
# Please enter the commit message for your changes. Lines starting
# with '#' will be ignored, and an empty message aborts the commit.
#
# On branch main
# Your branch is up to date with 'origin/main'.
#
# Changes to be committed:
#       new file:   problems/pressure_diffusion.i
#
!listing-end!

After exiting the `git commit` prompt, simply enter `git push` to update the `origin` remote. The remote repository on `github.com/YourGitHubUserName/babbler` can be inspected to confirm that the changes have been published.

!alert tip title=Interacting with Git
As mentioned in [Step 1](tutorial01_app_development/step01_moose_app.md#repo), there are a lot of ways to interact with Git. There are even [!ac](GUI) applications for using Git---the terminal is not the only way. It is important to become comfortable with this approach to software development, especially for MOOSE application development.

!content pagination previous=tutorial01_app_development/step01_moose_app.md
                    next=tutorial01_app_development/step03_moose_object.md
