# MOOSE Tools

The MOOSE code itself is written in C++, but everything around it that supports it is written in
Python.  [python-packages] is a list of the tools/scripts developed in Python that are distributed
with MOOSE and [python/source/index.md] links to the source code documentation.

!table id=python-packages
| Tool | Description |
| :- | :- |
| [TestHarness.md] | Tool testing that applications work correctly as code is developed. |
| [memory_logger.md] | Tool for gathering memory usage of a running process. |
| [CSVDiff.md] | Tool for computing differences between comma separated value (CSV) files. |
| [python/mms.md] | Utilities for verifying solves with the method of manufactured solutions. |
| [free_energy.py](/CALPHAD_free_energies.md) | Tool for extracting MOOSE parsed function expressions from thermodynamic database files. |
| [moosetree](moosetree/index.md) | Tool for building and searching tree structures. |
| [pyhit](pyhit/index.md) | Tool for reading, writing, and manipulating MOOSE input files. |
| [combine_csv.md] | Tool for combining CSV files together. |
| [moosesqa/index.md] | Tools for managing SQA documentation. |
| [ReporterReader.md] | Tool for reading [JSON](JSONOutput.md) output of [Reporter](Reporters/index.md) data |
| [module_hash.md] | Tool for generating a hash suffix for our contribution modules. |
| [MooseDocs/index.md] | Tool for creating documentation. |

## Setup

MOOSE includes various python packages within the python directory. In order
to use these packages the python directory must be made available to the interpreter,
which is accomplished by setting the `PYTHONPATH` environment variable. The following can be set on
the command line or inserted into your bash environment.

```bash
export PYTHONPATH=$PYTHONPATH:~/projects/moose/python
```
