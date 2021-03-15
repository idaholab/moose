# CALPHAD

This is a work in progress.

A script to export MOOSE readable free energy expressions from `*.tdb` thermodynamic database files can be found at

```text
moose/python/calphad/free_energy.py
```

The `free_energy.py` tool allows users to extract free energy expressions from `*.tdb`
thermodynamic database files (ThermoCalc format). The tool exports a list of MOOSE Material blocks
for each phase (or a user specified subset of phases). The CALPHAD functional expressions are
implemented using the [`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md) class.

It is up to the user to construct a full MOOSE input file around these material blocks and to rename
the variables used in the exported form to more suitable names.

## Installation

The [pycalphad](https://github.com/richardotis/pycalphad) Python module is required to perform the
parsing of the `*.tdb` files. [SymPy](https://github.com/sympy/sympy) is required to build the
functional forms of the calphad expressions.

To install and/or upgrade these prerequisites use pip:

```text
pip install --upgrade sympy
pip install --upgrade pycalphad
```

## Thermodynamic databases

Database files can be obtained online at

* [Computational Phase Diagram Database](http://cpddb.nims.go.jp/index_en.html) (CPDDB)
