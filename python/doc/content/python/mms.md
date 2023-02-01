# Method of Manufactured Solutions (MMS)

The method of manufactured solutions (MMS) is one method that may be used to validate that a
system of partial differential equations (PDEs) is solving as expected. This method is general and
should be used heavily in testing, thus some basic utilities exist within MOOSE to aid in building
such tests.

The first step in the process is to be sure that the strong form of your equation is well understood.
For this example, a diffusion equation is being considered.

!equation id=mms_strong
\nabla \cdot \nabla u = 0.

Next, assume a solution to this equation. It is important to pick an equation that has the necessary
order to allow for all the derivatives to be computed, both spatially and temporally.

When performing a spatial convergence study for a transient problem the time portion
should be an appropriate order that allows for the temporal finite difference scheme to represent
solution exactly. To the contrary the spatial solution should not be exactly represented
by the finite element shape functions. Functions that include trigonometric or exponential terms
are often useful; higher order polynomials are also acceptable.

When performing a temporal convergence study the opposite should be true: the spatial solution
should be exactly represented by the finite elements functions and the temporal solution should
not be exactly represented by the temporal finite difference scheme.

For example, consider a problem using a second order time integration scheme, such as
Crank-Nicolson, with first order Lagrange finite elements. For a spatial study an appropriate
function would be $u=t^2x^3y^3$. If performing a temporal study then an adequate function would be
$u=t^3xy$.

For the example problem a spatial convergence study is being performed initially, so the assumed
exact solution is selected as


!equation id=mms_exact
u = \sin({2\pi x})\sin({2\pi y}).

Using the assumed solution a forcing function ($f$) can be computed from the strong form such that

!equation id=mms_pde
\nabla \cdot \nabla u - f = 0.

This is simply done by inserting the assumed solution ([mms_exact]) into the left-hand side of the
strong form of the equation ([mms_strong][) and computing the result. The result of this calculation is
the forcing function $f$:

!equation id=mms_force
f = 8\pi^2\sin(2x\pi)\sin(2y\pi)

[mms_pde] is now a problem that can be solved and compared against the exact solution ([mms_exact]).

## Computing Forcing Function

Computing the forcing function can be done by hand or with a computer algebra system. MOOSE includes
[sympy](https://www.sympy.org) based tools for aiding with the computation of the forcing
function (the sympy package can be installed via [pip](https://pip.pypa.io) or
[miniconda](https://docs.conda.io)).

For example, the following python script computes and prints the forcing function for the current
example.

!listing mms/test/mms_exact.py end=ft

The "mms.print_fparser" function will output the forcing function as needed for
use with a [MooseParsedFunction.md] object within a MOOSE input file. The calls to the
"mms.print_hit" function output, in MOOSE input file format (hit), the forcing function and the
exact solution. The complete output for this function is given below.

```text
8*pi^2*sin(2*x*pi)*sin(2*y*pi)
[force]
  type = ParsedFunction
  value = '8*pi^2*sin(2*x*pi)*sin(2*y*pi)'
[]
[exact]
  type = ParsedFunction
  value = 'sin(2*x*pi)*sin(2*y*pi)'
[]
```

The `mms.evaluate` function automatically invokes 'x', 'y', 'z', and 't' as available variables; the
first argument is the partial differential equation to solve (the strong form) and the second is the
assumed known solution. The variable 'R' is reserved as the coordinate system
(see [sympy.vector.CoordSys3D](https://docs.sympy.org/latest/modules/vector/coordsys.html optional=True) for more information.)
Finally, the variables 'e_i', 'e_j', and 'e_k' are also reserved, each represents the basis vectors
for a 3D coordinate system. These are simply alternate variables for 'R.i', 'R.j', and 'R.k'.

The following table lists the additional arguments that may be passed to the "evaluate" function.

| Keyword | Type | Default | Description |
| :- | :- | :- | :- |
| `variable` | `str` | `u` | The primary variable to be solved for within the PDE |
| `scalars` | `list` | A list of constant +scalar+ variables included in the solution or PDE |
| `vectors` | `list` | A list of constant +vector+ variables included in the solution or PDE |
| `functions` | `list` | A list of arbitrary functions of 'x', 'y', 'z', and 't' in the solution or PDE |
| `vectorfunctions` | `list` | A list of arbitrary vector functions of 'x', 'y', 'z', and 't' in the solution or PDE |
| `negative` | `bool` | `False` | Flag for returning the negative of the evaluated function, by default this is false thus the function returned is correct for placing in the [BodyForce](BodyForce.md) Kernel object. |
| `**kwargs` | `dict` | All additional key, value pairs supplied are evaluated as additional functions that may include the 'x', 'y', 'z', and 't' or any other variables defined in the previous arguments. If 'e_i', 'e_j', 'e_k' are supplied the supplied function components (e.g., '_x', '_y', and '_z') are automatically defined. |

When arbitrary vectors are supplied, the output will include the components named using the
vector name with "_x", "_y", or "_z". For example, the following example executed in
an interactive python shell includes an arbitrary vector ("u") and scalar ("r").

```python
>>> import mms
>>> f = mms.evaluate('diff(h, t) + div(u*h) + div(grad(r*h))', 'cos(x*y*t)', variable='h', scalars=['r'], vectors=['u'])
>>> mms.print_fparser(f)
x^2*r*t^2*cos(x*y*t) + x*y*sin(x*y*t) + x*t*u_y*sin(x*y*t) + y^2*r*t^2*cos(x*y*t) + y*t*u_x*sin(x*y*t)
```

## Spatial Convergence

Using the computed forcing function from above it is possible to perform a spatial convergence study
for the problem to ensure that the appropriate rate of convergence is achieved as the mesh
is refined. The file in [mms_spatial] is the complete input file that performs the desired
solve.

!listing mms/test/mms_spatial.i id=mms_spatial
         caption=Complete input file for the solving the diffusion equation for use with the method
                 of manufactured solutions for a spatial convergence study.

It is important that the boundary conditions are satisfied exactly, often the easiest
method to achieve this is to use a [FunctionDirichletBC.md] using the exact solution, which is
what was done for the current problem.

For this problem the error is computed using the [ElementL2Error.md] postprocessor and the
element size is computed using the [AverageElementSize.md] postprocessor. If the mesh is not
uniform it may be more applicable to use the number of degrees of freedom as a proxy for element
size, which can be computed using the [NumDOFs.md] postprocessor.

To perform a convergence study this problem must be solved with increasing levels of refinement.
There are many ways to accomplish this, including using the automated tools included in the "mms"
python package. The code in [mms_spatial_script] demonstrates the use of the `run_spatial` function
to perform 4 levels of refinement for both first and second order finite elements.

!listing mms/test/mms_spatial.py end=TESTING id=mms_spatial_script
         caption=Python script for performing and plotting a spatial convergence study.

To determine if the convergence is correct the results from the above input file may be analyzed
by plotting the data on a log-log plot. The 'mms' package includes a `ConvergencePlot` object
designed for this application. The `plot` method of this object accepts the outputted data from the
run functions, as shown in [mms_spatial_script]. The resulting output from executing the script
is shown in [spatial_plot].

!media mms_spatial.png id=spatial_plot caption=Results for spatial MMS convergence study.

The legend of this plot includes the slope of each of the plotted lines. Both achieve the
theoretical convergence rates of two and three for first and second order elements, respectively.
For more information regarding convergence rates refer to [!citet](fish2007first).

## Temporal Convergence

In similar fashion it is possible to perform a temporal convergence study. In this example the
same strong form is considered, see [mms_strong]. However, the assumed solution is modified
such that the spatial solution can be exactly represented by first order finite elements. The
assumed solution is given in [time_exact] and the resulting forcing function in [time_force].

\begin{equation}
\label{time_exact}
u = xyt^3
\end{equation}

\begin{equation}
\label{time_force}
f = 3xyt^2
\end{equation}

Again, the MOOSE "mms" package can be used for computing the input file syntax needed to represent
these functions as follows.

!listing mms/test/mms_exact.py start=ft,st

The output of this script, as shown below, prints the forcing function as well as the MOOSE input
file blocks needed to represent both the forcing function and the exact solution. The complete input
file for the temporal problem is shown in [mms_temporal].


```text
3*x*y*t^2
[force]
  type = ParsedFunction
  value = '3*x*y*t^2'
[]
[exact]
  type = ParsedFunction
  value = 'x*y*t^3'
[]
```

!listing mms/test/mms_temporal.i id=mms_temporal
                       caption=Complete input file for the solving the diffusion equation for use
                               with the method of manufactured solutions for a temporal convergence
                               study.

To perform a temporal convergence study the simulation must be performed with decreasing time
step size. When setting up an input file for a temporal study be sure to set the "end_time" parameter
to guarantee that the each run of the simulation solves to the same point in time, allowing the
resulting solutions to be compared. The "mms" package includes `run_temporal` method for performing
the various runs with varying time step; the convergence plot is created using the same object as
shown previously. The complete python code for the temporal MMS convergence study is given
in [mms_temporal_script] and the result plot is shown in [temporal_plot].

!listing mms_temporal.py end=TESTING id=mms_temporal_script
                         caption=Python script for performing and plotting a temporal convergence
                                 study.

!media mms_temporal.png id=temporal_plot caption=Results for temporal MMS convergence study.

The rate of converged as computed via the slope of the lines is shown in the legend for each of the
entries. In this case the expected rate of converge of one and two is achieved for the
first and second order finite difference schemes used for time integration.

## Creating Test(s)

After creating the aforementioned scripts for running the spatial and temporal convergence studies
it is desirable to convert these scripts into actual tests (see [TestHarness.md]). This is easily
achieved using by creating a "tests" file within the test directory of an application.

For example, the following "tests" file is contained within MOOSE for executing the scripts
presented in the above examples.

!listing mms/test/tests id=mms_tests caption=Test specification within MOOSE for running the
                                             example MMS convergence study scripts.

This specification includes tests that check for the output of the resulting convergence plot image
as well as performs a "diff" with the data generated from the calls to the `run_spatial` and
`run_temporal` methods. The CSV files are created within the script by calling the "to_csv"
method on the object returned from the run functions. For example, the spatial script contains
the following lines at the end of the script to create the files that is tested.

!listing mms/test/mms_spatial.py start=TESTING include-start=False


!bibtex bibliography
