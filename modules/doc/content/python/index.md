# MOOSE Python Tools

The MOOSE code itself is written in C++, but everything around it that supports it is written in
Python.  Here we detail some of the tools/scripts developed in Python that are distributed with
MOOSE.  Click on each one for further information

| Tool | Description |
| :- | :- |
| [TestHarness.md] | Tool testing that applications work correctly as code is developed. |
| [memory_logger.md] | Tool for gathering memory usage of a running process. |
| [CSVDiff.md] | Tool for computing differences between comma separated value (CSV) files. |
| [mms.md] | Utilities for verifying solves with the method of manufactured solutions. |

## Setup

MOOSE includes various python packages within the python directory. In order
to use these packages the python directory must be made available to the interpreter,
which is accomplished by setting the `PYTHONPATH` environment variable. The following can be set on
the command line or inserted into your bash environment.

```bash
export PYTHONPATH=$PYTHONPATH:~/projects/moose/python
```
