# MOOSE SQA Tools

The "moosesqa" python package is a set of utilities for gathering requirements from
test specifications as well as generating software quality reports.

## Software Requirements

The software requirement information for a directory can be collected using the
`get_requirements_from_tests` function. This must be provided two inputs: a list of directories to
search and a list of test specification names to consider. It will return a `dict` of lists. The key
is the top level folder (or "group") and the list entries contain a `Requirement` object that
contains data for each requirement. For example, the following can be executed within the MOOSE
repository to load all the requirement information from the moose test application.

```bash
$ cd ~/projects/moose/test
$ python
Python 3.7.6 | packaged by conda-forge | (default, Jun  1 2020, 18:33:30)
[Clang 9.0.1 ] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import moosesqa
>>> req = moosesqa.get_requirements_from_tests(['tests'], ['tests'])
```

## SQA Reports

The information that is output from the SQA check command (i.e., `./moosedocs.py check`) and
the shown in the [MOOSE SQA](sqa/index.md exact=True optional=True) documentation are generated from report objects.
There exists three types of report objects:

- +`SQADocumentReport`+\\
  Provides information regarding the existence of the various necessary documents needed
  to satisfy the NQA-1 standard (e.g., the Software Test Plan).
- +`SQARequirementReport`+\\
  Provides information regarding the requirements that are defined within the test
  specifications, including checks for duplicates and missing information.
- +`SQAMooseAppReport`+\\
  Collects data regarding the inclusion of source and syntax design documentation based on
  MOOSE-based application registered syntax.

The SQA software reports may be retrieved by using the `get_sqa_reports` function. This function
requires one argument, the name of a report configuration YAML file. This is comprised of
three sections: "Applications", "Documents", and "Requirements". These sections include the
options for the `SQAMooseAppReport`, `SQADocumentReport`, and the `SQARequirementReport`,
respectively. The file shown in [example-config] is configuration file from the
[Stochastic Tools SQA page](stochastic_tools/sqa/index.md optional=True). The available options for
each report should be gathered from the source code for the various report objects. If you
are creating reports for a new application and need assistance getting started please
contact MOOSE developers via the [Discussions forum](https://github.com/idaholab/moose/discussions).

!listing stochastic_tools/doc/sqa_reports.yml id=example-config caption=Example SQA report configuration file.

It is possible to generate the reports manually by running the following commands, but in general
it is recommended that the SQA "check" command be used from the application doc directory.

```bash
$ cd ~/projects/moose/modules/stochastic_tools/doc
$ python
Python 3.7.6 | packaged by conda-forge | (default, Jun  1 2020, 18:33:30)
[Clang 9.0.1 ] on darwin
Type "help", "copyright", "credits" or "license" for more information.
>>> import moosesqa
>>> report_tuple = moosesqa.get_sqa_reports('sqa_reports.yml')
>>> for reports in report_tuple:
>>>     for r in reports:
>>>         print(r.getReport())
>>>
```
