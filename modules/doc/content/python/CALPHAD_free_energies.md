# CALPHAD Thermodynamic Database free energy extraction

The `free_energy.py` tool (`python/calphad/free_energy.py`) allows users to
extract free energy expressions from `*.tdb` thermodynamic database files
(ThermoCalc format). The tool exports a list of MOOSE Material blocks for each
phase (or a user specified subset of phases). The CALPHAD functional expressions
are implemented using the `DerivativeParsedMaterial` class.

It is up to the user to construct a full MOOSE input file around these material
blocks and to rename the variables used in the exported form to more suitable
names.

## Installation

The [pycalphad](https://github.com/richardotis/pycalphad)
([docs](https://pycalphad.org/docs/latest/)) Python module is required to
perform the parsing of the `*.tdb` files.
[SymPy](https://github.com/sympy/sympy) is required to build the functional
forms of the CALPHAD expressions.

To install and/or upgrade these prerequisites use conda:
```
conda create -n pycalphad
conda install -n pycalphad -c pycalphad -c msys2 -c conda-forge pycalphad
conda activate pycalphad
```
(or follow the installation instructions on the pycalphad website).

## Thermodynamic databases

Database files can be obtained online at

* [Computational Phase Diagram Database](http://cpddb.nims.go.jp/index_en.html/) (CPDDB)
* [NIST Materials Data Repository](https://materialsdata.nist.gov)
