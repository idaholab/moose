# Method of Manufactured Solutions (MMS)

The method of manufactured solutions (MMS) is one method that may be used to validate that your
system of partial differential equations (PDEs) is solving as expected. This method is useful and
should be used heavily in testing, thus some basic utilities exist within MOOSE to aid in building
such tests.

The first step in the process is to be sure that the strong form of your equation is well understood.
For this example, a diffusion equation is being considered.

\begin{equation}
\label{strong}
\nabla \cdot \nabla u = 0.
\end{equation}

Next, assumed a solution to this equation. It is important to pick an equation that has the
necessary order to allow for all the derivatives to be computed. It is also important that the
finite element shape functions can not represent the solution exactly. Functions that
include trigonometric or exponential terms are often an adequate choice. For this example the assumed
solution is given in [exact].

\begin{equation}
\label{exact}
u = a \cdot \sin({2\pi axy}),
\end{equation}
where $a$ is an arbitrary constant.

Using the assumed solution a forcing function ($f$) can be computed by from the strong form such that

\begin{equation}
\label{mms}
\nabla \cdot \nabla u + f = 0.
\end{equation}

This is simply done by inserting the assumed solution ([exact]) into the left-hand side of the
strong form of the equation ([strong]) and computing the result. The negative of this result is
the forcing function $f$.

[mms] is now a problem that can be solved and compared against the exact solution ([exact]).

## Computing Forcing Function

Computing the forcing function can be done by hand or with a computer algebra system. MOOSE includes
a [sympy](https://www.sympy.org) based utility for aiding with the computation of the forcing
function (the sympy package can be installed via [pip](https://pip.pypa.io) or
[miniconda](https://docs.conda.io)). For example, the following python script computes and prints the
forcing function for the current problem.

!listing input.py end=mms.plot

This basic script will output the forcing function (including the taking the negative) as needed for
inserting info a `ParsedFunction` object within a MOOSE input file.

```text
4*x^2*pi^2*a^3*sin(2*x*y*pi*a) + 4*y^2*pi^2*a^3*sin(2*x*y*pi*a)
```

The `evaluate` function automatically invokes 'x', 'y', 'z', and 't' as available variable; the first
argument is the partial differential equation to solve (the strong form) and the second is the
assumed known solution.

The following table lists the additional arguments that may be passed to the "evaluate" function.

| Keyword | Type | Default | Description |
| :- | :- | :- | :- |
| `variable` | `str` | `u` | The primary variable to be solved for within the PDE |
| `scalars` | `list` | | A list of arbitrary +scalar+ variables included in the solution or PDE |
| `vectors` | `list` | | A list of arbitrary +vector+ variables included in the solution or PDE |

When arbitrary vectors are supplied, the output will include the components named using the
vector name with "_x", "_y", or "_z". For example, the following example executed in
an interactive python shell includes an arbitrary vector ("u").

```python
>>> import mms
>>> f = mms.evaluate('diff(h, t) + div(u*h) + div(grad(r*h))', 'cos(x*y*t)', variable='h', scalars='r', vectors='u')
>>> mms.print_fparser(f)
x^2*r*t^2*cos(x*y*t) + x*y*sin(x*y*t) + x*t*u_y*sin(x*y*t) + y^2*r*t^2*cos(x*y*t) + y*t*u_x*sin(x*y*t)
```

## Using Forcing Function

Using the computed forcing function from above it is possible to perform a convergence study
for the problem to ensure that the appropriate rate of convergence is achieved as the mesh
is refined. The file listed in [mms_input] is the complete input file that performs the solve
for 5 levels of adaptivity. This is accomplished using a transient executioner and uniform
refinement.

Note that it is important that the boundary conditions are satisfied exactly, often the easiest
method to achieve this is to use a [FunctionDirichletBC.md] using the exact solution, which is
what was done for the current problem.

!listing mms/test/input.i id=mms_input caption=Complete input file for the example diffusion method of manufactured solutions problem.

For this problem the error is computed using the [ElementL2Error.md] postprocessor and the
element size is computed using the [AverageElementSize.md] postprocessor. If the mesh is not
uniform it may be more applicable to use the number of degrees of freedom as a proxy for element
size, which can be computed using the [NumDOFs.md] postprocessor.

## Convergence Plot

To determine if the convergence is correct the results from the above input file may be analyzed
by plotting the data on a log-log plot. The 'mms' package includes a 'plot' function designed for
this application that accepts the of the outputed CSV file(s). For this problem the following
python code produces the plot shown in [convergence].

!media convergence.png id=convergence caption=Example convergence plot created by 'mms.plot' function.

The 'plot' function accepts a filename for the first argument. If the filename contains an asterisk
then [glob](https://docs.python.org/2/library/glob.html) is used to get a list of filenames
from the supplied pattern, the last values for the error and element size from each file are
appended together. This feature is useful when performing a convergence study from a transient solve,
where each refinement is output to a separate file.

The following table lists the additional options that may be passed to the 'mms.plot' function.

| Keyword | Type | Default | Description |
| :- | :- | :- | :- |
| `x` | `str` | `h` | The name of the element length postprocessor |
| `y` | `str` | `error` | The name of the solution error postprocessor |
| `xlabel` | `str` | | The text to use for labeling the x-axis, if not provided the value of 'x' is used |
| `ylabel` | `str` | | The text to use for labeling the y-axis, if not provided the value of 'y' is used |
| `output` | `str` | | The name of the file to create (e.g., "convergence.pdf") |
| `show` | `bool` | True | Flag for disabling the live window of the plot |
| `fit` | `bool` | True | Flag for disabling the computation of the slop fit |

The rate of convergence is expected to be two for first order elements and third for second order
elements, for more information regarding convergence rates refer to [citet!fish2007first].

!bibtex bibliography
