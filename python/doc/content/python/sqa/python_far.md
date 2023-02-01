!template load file=sqa/far.md.template project=MOOSE Tools

!template! item key=introduction
[!ac](MOOSE) Tools, as a collection of utilities, is designed to operate as a library of functionalities.
While each utility is tailored to perform a specific set of functions, the code can be arbitrarily expanded
and built upon to create extended capability. This flexibility exists by design within the utilities,
where they are often created with a central core set of code framework surrounded by extensions built
to house specific features (the best example of this is the [MooseDocs/index.md]). With respect to
performing failure analysis, the flexibility of the code base can be detrimental since there lacks a
well-defined problem to assess. To minimize the possibility of failure for a simulation, various
automated methods exist for developers. This document discusses these features and includes a list
of requirements associated with software failure analysis.
!template-end!

!template! item key=failure-analysis
[!ac](MOOSE) Tools has three primary methods for handling utility failures that range from test input
errors to output differential failures. These methods of failure are common across [!ac](MOOSE) and
[!ac](MOOSE)-based applications when testing aspects of Python-based support tools. The following
sections detail the handling of these sources of failures.

1. Test input parameter errors,
2. Test dependency errors, and
3. Missing objects/files failures.

Beyond these common sources of error, the `PythonUnitTest` tester (more information in Python
[python_stp.md#test-automation]) is also able to handle custom failure capture through Python script
input.

### Test Input Parameter Errors

Test specification files used to control [!ac](MOOSE) Tool testing, as in [!ac](MOOSE) itself, uses
the standard HIT syntax that MOOSE [uses](application_usage/input_syntax.md). File parsing automatically
handles syntax mistakes and reports them as errors. For example, consider the following test specification
file that contains a duplicate parameter:

!listing test_harness/parse_errors

If this test spec file is used to start a test, the system will automatically report the error and associated line number where it occurred as follows:

```
parse_errors:5: duplicate parameter "/Tests/syntax/check_input"
```

### Test Dependency Errors

Tests can depend on each other, and thus trees of test dependency can result. The testing system can automatically handle dependency issues (including depending on another test that is non-existent, or having a cyclic dependency chain). For an example of cyclic dependency, see the following:

!listing test_harness/cyclic_tests

Following the dependency chain (denoted through the `prereq` parameter), notice that `testB` and `testC` functionally depend on each other (through `testA`), which leads to a cyclic dependency. The system will detect this and report an error:

```
tests/test_harness.testC: Cyclic dependency error!
tests/test_harness.testC: 	testA --> testB --> testC <--> testB
```

### Missing Objects/Files Failure

When files required for the successful completion of a test are missing or incorrect, failures will also be encountered. In this example, consider the following test specification that compares outputs for [!ac](CSV) and Exodus output-based tests to gold files that do not exist:

!listing test_harness/diff_golds

When this test is executed, an error will be thrown during the file comparison. The missing file
content is considered empty, so the kind of error experienced will relate to differences between the
output and an empty file. In the case of the CSVDiff the following failing output is seen:

```
ERROR: In file csvdiff_diff_gold_out.csv: Columns with header 'time' aren't the same length
```
!template-end!

!template item key=failure-analysis-requirements
!sqa requirements link=True collections=FAILURE_ANALYSIS category=python
