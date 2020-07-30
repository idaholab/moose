# Step 2: Creating an Input File

In this step, the way in which [!ac](FE) models are built by invoking several C++ objects in a neat and organized fashion with MOOSE [#inputs] will be explained. To demonstrate this concept, the reader shall consider a steady-state diffusion of pressure from one end of the pipe, between the pressure vessels, to the other (see the [tutorial01_app_development/problem_statement.md] page). For this, a [!ac](BVP) that is so simple that the reader need not concern themselves with the physics, and focus only on how to write the input code to solve the governing equation, will be enforced. While there are important [!ac](FEM) and C++ concepts at work here, for now, simply consider them as a given.

## Statement of Physics id=physics

The Laplace equation shall be employed to model the steady-state diffusion of pressure, $u$, on $\Omega$. Thus, find $u$, such that

!equation id=laplace
-\nabla \cdot \nabla u = 0 \in \Omega

satisfies the following [!ac](BVP): $u = 4000 \, \textrm{Pa}$ at the inlet (left) boundary, $u = 0$ at the outlet (right) boundary, and $\nabla u \cdot \hat{n} = 0$, where $\hat{n}$ is the surface normal vector, on the remaining boundaries.

## Input Files id=inputs

C++ objects may be used to solve problems by writing an input file, for which one appends the `.i` extension to. For such files, MOOSE uses the "hierarchical input text" (hit) format. More often, this input style is referred to as being block structured. To demonstrate this format, consider a C++ class designed to model the diffusion of some field variable, $u$. An object, or a single instance of this class, would be created in a MOOSE input file by the following syntax:

!listing tutorials/tutorial01_app_development/step02_input_file/test/tests/kernels/simple_diffusion/simple_diffusion.i
         block=Kernels
         link=False

The syntax, `type = Diffusion`, denotes the object's reference to a C++ class named [`Diffusion`](source/kernels/Diffusion.md) that is capable of solving [laplace].
This input format is referred to as a hierarchy because `Diffusion` is an object of the [`Kernel`](syntax/Kernels/index.md) type, which is also a C++ class that others inherit from.

For each block, which usually refers to some base class, e.g., `Kernel`, one adds sub-blocks, which usually refer to some inherited class, like `Diffusion`. Within each sub-block, any number of name/value pairs are provided, and these correspond to the parameters required for the given class. Although there are usually several input parameters required, `Diffusion` doesn't need any except the variable for which to operate on. Variables also have their own block in input files. At the very least, a MOOSE input file requires the following six block types:

- [`[Mesh]`](syntax/Mesh/index.md): Define the geometry of the domain
- [`[Variables]`](syntax/Variables/index.md): Define the unknown(s) of the problem
- [`[Kernels]`](syntax/Kernels/index.md): Define the equation(s) to solve
- [`[BCs]`](syntax/BCs/index.md): Define the boundary condition(s) of the problem
- [`[Executioner]`](syntax/Executioner/index.md): Define how the problem will solve
- [`[Outputs]`](syntax/Outputs/index.md): Define how the solution will be written

It should be obvious that, for the most basic [!ac](FE) analysis problems, one must define the mesh, variables, equations (kernels), and boundary conditions. One must also decide how they're going to handle the algebra and record the results, so for these purposes, one includes the `[Executioner]` and `[Outputs]` blocks. Thus, these six blocks shall be included when the input file is written for the given diffusion problem.

!alert tip title=Block Types
The names of blocks suggest what types of objects belong to them and the tasks they perform. If you're ever wondering where to put a sub-block type, visit the [Source Documentation](source/index.md) page and check which block type that object is filed under. You'll also notice that the block structure of an input file is directly related to how the `src/` directories in the MOOSE repository are organized. <!--May I address the reader personally when I include things like helpful tips?-->

*For more information about input files, please visit the [application_usage/input_syntax.md] page.*

## Execute an Input File id=execute

There are a couple of ways to go about executing an input file, but, regardless of which, the major requirement is an executable. From [Step 1](tutorial01_app_development/step01_moose_app.md), the reader should have already created an executable for their application, but, if not, be sure to run the following commands:

```bash
cd ~/projects/YourAppName
make -j4 # use number of processors available on your system, i.e., 2, 4, ..., 12, or <N_procs>
```

The most basic way to execute an input file is from a system's terminal. All a user needs to do is navigate to the directory where their input file is stored and then pass it to the application's executable with the `-i` command:

```bash
cd ~/projects/YourAppName/test/tests/kernels/simple_diffusion
../../../../YourAppName-opt -i simple_diffusion.i
```

MOOSE will then process the [!ac](FE) simulation and populate the input file directory with all of the requested outputs, e.g., ExodusII or comma delimited files. Here, a file called `simple_diffusion_out.e` should have been created. As mentioned in [Helpful Software](getting_started/new_users.md#helpful-software), users can open this file to visualize the results of their solution with PEACOCK or ParaView. It was also mentioned that PEACOCK is the MOOSE in-house [!ac](GUI). Coincidentally, PEACOCK provides the second way in which an input file may be executed. To demonstrate this, run the following commands:

```bash
cd ~/projects/YourAppName/test/tests/kernels/simple_diffusion
~/projects/moose/python/peacock/peacock -i step1.i
```

This will open the PEACOCK window and the first thing one may notice is a planar mesh used for the [simple diffusion test](tutorial01_app_development/step01_moose_app.md#test) problem. To execute the input file, click on the "Execute" tab and then click "Run". These two steps are shown in [peacock-execute] and [peacock-run], respectively. Once the solve completes, click on the "ExodusViewer" tab to view the results.

!media tutorial01_app_development/peacock_execute.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=peacock-execute
       caption=After PEACOCK opens for a specified input file, click on the "Execute" tab.

!media tutorial01_app_development/peacock_run.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=peacock-run
       caption=In the PEACOCK "Execute" tab, click on "Run" to execute the input file.
<!--This image shows my machine's directories, i.e., /home/crswong888/... is that tacky?-->

!alert tip title=Bash Alias for PEACOCK
The command-line syntax for running PEACOCK can be simplified by creating a bash alias for it. Be sure to visit the [documentation page for PEACOCK](application_usage/peacock.md) to learn how to do this.

## Demonstration id=demo

### Input File id=input-demo

In this section<!--chapter?-->, the reader will be guided through the setup of an input file to model the steady-state diffusion of pressure over the length of the pipe. Recall that six basic blocks - those which were mentioned in the [#inputs] section - are required. To begin, the reader shall create a directory named "problems" in their MOOSE application:

```bash
cd ~/projects/YourAppName
mkdir problems
```

Next, they shall open their preferred text editor and create a file named `pressure_diffusion.i` in the `problems/` directory. This file is where the block structure format will be used to setup the inputs needed to solve the [!ac](BVP) given in the [#physics].

For the `[Mesh]` block, the geometry of the pipe, whose dimensions were provided on the [tutorial01_app_development/problem_statement.md] page, shall be defined using the MOOSE standard rectilinear mesh generator object, [`GeneratedMesh`](source/mesh/GeneratedMesh.md):

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Mesh
         link=False
<!--Using the block parameter with the !listing command fails to render the exact format of the corresponding lines of the source file. For example, in this file, the comments following all of the input params were vertically aligned along the same column, but all of the extra white space was removed in this rendering. If the block parameter is not used, and the whole file is shown, this is not the case. TODO: fix this.-->

Although this generates a planar mesh with a height equal to the pipe's radius, it is possible to model the true effect of the cylindrical space by solving the problem in a special way. To do this, a [`[Problem]`](syntax/Problem/index.md) block shall be included in addition to the basic six:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Problem
         link=False

The purpose of this block is to indicate that the planar mesh is part of an asymmetric body and to solve the [!ac](FE) problem with respect to a cylindrical coordinate system. There are, of course, several block types beyond the six core ones - many of which will be seen throughout this tutorial.

The pressure variable, $u$, shall be added to the `[Variables]` block with an unambiguous name, i.e., `pressure`:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Variables
         link=False

Following that, the `[Kernels]` block, whose syntax was demonstrated earlier, shall be included. As in the previous case, the `Diffusion` class will be used to solve [laplace]. Here, the `pressure` variable will be operated on:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Kernels
         link=False

Now that the domain, $\Omega$, the primary variable, $u$, and the [!ac](PDE) have been specified, the [!ac](BVP) must be enforced. The [`DirichletBC`](source/bcs/DirichletBC.md) class enforces a Dirichlet, or essential, boundary condition, e.g., the pressures prescribed at the pipe's inlet and outlet. For this problem, two separate `DirichletBC` objects will fall under the `[BCs]` block:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=BCs
         link=False

The reader should be convinced that it is not necessary to enforce the Von Neumann (natural) boundary conditions, $\nabla u \cdot \hat{n}$, since they are zero-valued fluxes.

!alert note title=Input File Block Order
The blocks need not be developed in any particular order, but one which corresponds with the procedure for a typical [!ac](FE) analysis might be followed.

Finally, a steady-state executioner object to solve the resulting system of equations, using Newton's method, shall be called upon, and its solution shall be output in ExodusII format by adding the `[Executioner]` and `[Outputs]` blocks, respectively:

!listing tutorials/tutorial01_app_development/step02_input_file/problems/pressure_diffusion.i
         block=Executioner Outputs
         link=False

There are a lot of ways to control the solution when using the [`Steady`](source/executioners/Steady.md) class (or the [`Transient`](source/executioners/Transient.md) class for time-dependent simulations). For instance, the parameters which set the PETSc solver options can be very influential on the convergence rate of the NEWTON solve. Throughout this tutorial, the reader is encouraged to test different combinations of input parameters, especially for `Executioner` objects. After running the input file, as it was given here, the reader may remove the PETSc options parameters, run it again, and observe the difference in the convergence rate.

### Results id=result-demo

In [#execute],
some ways in which to execute and visualize the results of an input file were discussed. The reader should review these methods and find their preferred means of interacting with MOOSE. As a reminder, one could run [`pressure_diffusion.i`](#input-demo) from the terminal by entering the following:

```bash
cd ~/projects/YourAppName/problems
../YourAppName-opt -i pressure_diffusion.i
```

If the input file was ran from the terminal, and the `pressure_diffusion_out.e` file exists, the results can be rendered in PEACOCK by running the following commands:

```bash
cd ~/projects/YourAppName/problems
~/projects/moose/python/peacock/peacock -r pressure_diffusion_out.e
```

After running the above commands, the PEACOCK window opens to the "ExodusViewer" tab and a result which resembles that shown in [results] should be displayed.

!media tutorial01_app_development/step02_result.png
       style=width:100%;margin-left:auto;margin-right:auto;
       id=results
       caption=Rendering of the [!ac](FEM) solution of [laplace] subject to the [!ac](BVP) given in [#physics].
<!--this image needs to be cropped - way too much white space-->

Notice that the [!ac](FEM) solution depicted in [results] satisfies the boundary conditions given in the [#physics], i.e., a pressure of 4000 Pa can be observed at the inlet and zero pressure at the outlet. The pressure distribution over the length of the pipe also appears to be uniform across its radius, indicating that there is no flux through any of the remaining boundaries. Thus, $\nabla u \cdot \hat{n} = 0$ is also satisfied at those boundaries.

### Commit id=commit-demo

The reader should have already created a GitHub repository to store their MOOSE application as part of the [previous step](tutorial01_app_development/step01_moose_app.md#repo) of this tutorial. They should have also pushed their first commit to the `origin` remote (the online copy). Now that new a file exists in the local repository, i.e., `problems/pressure_diffusion.i`, and since this input file has been verified to produce good results, it should be committed and pushed to the remote. Before proceeding, inspect the status of the local repository as it compares to the `HEAD`, which denotes the version that existed following the most recent commit:

```bash
cd ~/projects/YourAppName
git status
```

The terminal output should read something like the following:

```
Untracked files:
  (use "git add <file>..." to include in what will be committed)

	problems/

nothing added to commit but untracked files present (use "git add" to track)
```

This indicates that a new directory, `problems/`, has not been staged, and that there may or may not be more unstaged files in that directory. Of course, in this case, there are. Proceed with the instructions provided in the output for adding the new files and then reinspect the status of the local repository:

```bash
git add problems
git status
```

Now, the terminal output should be the following:

```
Changes to be committed:
  (use "git reset HEAD <file>..." to unstage)

	new file:   problems/pressure_diffusion.i
```

To commit these changes, simply enter `git commit`. A user will then be prompted to enter a message describing the changes. For this, the reader may consider writing something like "created an input file to solve diffusion problem," as shown in the example below.

```
created an input file to solve diffusion problem                            
# Please enter the commit message for your changes. Lines starting
# with '#' will be ignored, and an empty message aborts the commit.
#
# On branch master
# Your branch is up to date with 'origin/master'.
#
# Changes to be committed:
#       new file:   problems/pressure_diffusion.i
#
```

After exiting the `git commit` prompt, simply enter `git push` to update the `origin` remote. The reader may proceed to `github.com/YourGitHubUserName/YourAppName` and confirm that their changes have been published.

!alert tip title=Interacting With Git
As mentioned in [Step 1](tutorial01_app_development/step01_moose_app.md#repo), there are a lot of ways to interact with Git. There are even [!ac](GUI) applications for using Git - the terminal is not the only way. You should become comfortable with this approach to software development, especially for MOOSE Framework development.

!content pagination previous=tutorial01_app_development/step01_moose_app.md
                    next=tutorial01_app_development/step03_moose_object.md
