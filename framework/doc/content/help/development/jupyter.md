# Setup Jupyter Python Notebooks

[Jupyter](http://jupyter.org/) Notebooks are a web application that allows creating and sharing documents
that contain live (python) code, equations, visualizations and explanatory text.
It is an ideal vehicle to distribute worksheets with equation derivations and
graphs and can even contain interactive elements.

## Installing Jupyter

MOOSE comes bundled with a miniconda install. To install additional python modules such
as the python based Jupyter system it is suggested to create a custom conda environment

```bash
conda create -n jupyter --clone=/opt/moose/miniconda
```

The new environment needs to be activated and the necessary python packages installed

```bash
source activate jupyter
conda install jupyter sympy
```

## Running the Notebook server

In the directory in which the notebook files are to be stored execute

```bash
jupyter-notebook
```

A browser window sould open with a list of notebooks (`*.ipynb` files) and a few UI elements.

## Starting a new notebook

From the _New_ dropdown in the top right select _Python 2_ under _Notebooks_. A new browser tab
with an empty notebook should open.

!media media/jupyter_new.png

Start by importing [sympy](http://www.sympy.org/en/index.html) in `In [1]:` and enabling LaTeX rendering for equations.

```python
from sympy import *
from sympy import init_printing
init_printing(use_latex=True)
```

Next define a few variables and a function (for which we only declare an argument but not an implementation yet)

```python
c, eta = symbols("c eta")
h = Function("h")(eta)
```

Now lets define an expression `f` using these components

``` python
f = h*c**2 + (1-h)*(1-c)**2
f
```

The lone `f` on the second line causes the expression to be printed in `Out [3]:`. We can now build a few symbolic
deriuvatives of the expression `f` with respect to `c` and `eta`

```python
(diff(f, c), diff(f,eta))
```

This should yield

!media media/jupyter_example.png

## Enabling and disabling the environment

To switch back and forth between the pristine MOOSE miniconda environment the `jupyter` environment
use the following commands

```bash
source activate jupyter
```
```bash
source deactivate jupyter
```
