# Method of Manufactured Solutions (MMS)

!---

The Method of Manufactured Solutions (MMS) is a useful tool for code verification
(making sure that a mathematical model is properly solved)

MMS works by assuming a solution, substituting it into the PDE, and obtaining a forcing term

The modified PDE (with forcing term added) is then solved numerically; the result can be compared to
the assumed solution

By checking the norm of the error on successively finer grids you can verify your code obtains the
theoretical convergence rates

!---

## Example 14: MMS

PDE:  $-\nabla \cdot \nabla u = 0$

Assumed solution:  $u = \sin(\alpha\pi x)$

Forcing function:  $f = \alpha^2 \pi^2 \sin (\alpha \pi x)$

Need to solve:  $-\nabla \cdot \nabla u - f = 0$

!---

## Example 14: Input File

!listing examples/ex14_pps/ex14.i

!---

## Example 14: Run via Command-line

```bash
cd ~/projects/moose/examples/ex14_pps
make -j 12 # use number of processors for your system
./ex14-opt -i ex14.i
```

!---

## MMS Python Package

```bash
export PYTHONPATH=$PYTHONPATH:~/projects/moose/python
```

!---

## Example 14: Exact Solution

#### mms_exact.py

!listing examples/ex14_pps/mms_exact.py

!---

```bash
cd ~/projects/moose/examples/ex14_pps
./mms_exact.py
```

```bash
pi^2*a^2*sin(x*pi*a)
[force]
  type = ParsedFunction
  value = 'pi^2*a^2*sin(x*pi*a)'
  vars = 'a'
  vals = '1.0'
[]
[exact]
  type = ParsedFunction
  value = 'sin(x*pi*a)'
  vars = 'a'
  vals = '1.0'
[]
```

!---

## Error Analysis

To compare two solutions (or a solution and an analytical solution) $f_1$ and $f_2$, the following
expressions are frequently used:

!equation
||f_1-f_2||^2_{L_2(\Omega)} = \int_{\Omega} (f_1-f_2)^2 \;\text{d}\Omega
\\
||f_1-f_2||^2_{H_{1,\text{semi}}(\Omega)} = \int_{\Omega} \left|\nabla \left(f_1-f_2\right)\right|^2 \;\text{d}\Omega


From finite element theory, the convergence rates are known for these quantities on successively
refined grids.  These two calculations are computed in MOOSE by utilizing the `ElementL2Error` or
`ElementH1SemiError` postprocessor objects, respectively.

!---

## Example 14: Convergence Study

#### mms_spatial.py

!listing examples/ex14_pps/mms_spatial.py

!---

## Example 14: Convergence Results

```bash
cd ~/projects/moose/examples/ex14_pps
./mms_spatial.py
```

!---

!media darcy_thermo_mech/ex14_mms.png
