# XMLDiff

`XMLDiff` tests compare `XML` output(s) for the test to a reference in the specified
`gold_dir` folder.

## Options

Test configuration options are added to the `tests` file.

- `xmldiff`: A list of `XML` files to compare
- `gold_dir`: The directory where the "gold standard" files reside relative to the TEST_DIR: (default: ./gold/)
- `abs_zero`: Absolute zero cutoff used in xmldiff comparisons, defaults to 1e-10
- `rel_err`: Relative error value used in xmldiff comparisons, defaults to 5.5e-6
- `ignored_attributes`: Ignore an attribute. For example, type and version in sample `XML` block below

```
<VTKFile type="Foo" version="0.1">
```

Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Comparison criteria id=comparison

`XMLDiff` compares the parsed elements and attributes of the two files, so their structure must
match. Because the contents of an `XML` file are text, every value that parses as a number is
compared with the tolerances `abs_zero` and `rel_err`, including each entry of a
space-separated list of numbers. All other text must match exactly. The tolerance equations are
the same as for `JSONDiff`, see [JSONDiff](JSONDiff.md#comparison): the relative difference is
normalized by the test value, and a pair is skipped if both values are below `abs_zero`.

## Example test configuration in the MOOSE test suite

In this example, the `XMLDiff` tester is used to check the `XML` output of two `VectorPostprocessors`.

!listing test/tests/outputs/xml/tests
