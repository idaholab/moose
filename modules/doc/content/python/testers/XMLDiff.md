# XMLDiff

`XMLDiff` tests compare `XML` output(s) for the test to a reference in the specified
`gold_dir` folder.

## Options

Test configuration options are added to the `tests` file.

- `xmldiff`: A list of `XML` files to compare
- `gold_dir`: The directory where the \"gold standard\" files reside relative to the TEST_DIR: (default: ./gold/)
- `abs_zero`: Absolute zero cutoff used in exodiff comparisons, defaults to 1e-10
- `rel_err`: Relative error value used in exodiff comparisons, defaults to 5.5e-6
- `ignored_attributes`: Ignore an attribute. For example, type and version in sample `XML` block below

```
<VTKFile type=\"Foo\" version=\"0.1\">")
```

Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in the MOOSE test suite

In this example, the `XMLDiff` tester is used to check the `XML` output of two `VectorPostprocessors`.

!listing test/tests/outputs/xml/tests
