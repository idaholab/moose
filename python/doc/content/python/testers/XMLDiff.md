# XMLDiff

`XMLDiff` tests compare `XML` output(s) for the test to a reference in the specified
`gold_dir` folder.

## Options

Test configuration options are added to the `tests` file.

All `XMLDiff` files also contain the options for differs found in [SchemaDiff](SchemaDiff.md), such as global changes to `rel_err` and `abs_zero`.

- `xmldiff`: Alias of `schemadiff`. A list of `XML` files to compare
- `ignored_attributes`: Alias for `ignored_items`. A list of keys, values, or attributes to ignore. For example, type and version in sample `XML` block below

```
<VTKFile type=\"Foo\" version=\"0.1\">")
```

Other test commands & restrictions may be found in the [TestHarness documentation](TestHarness.md).

## Example test configuration in the MOOSE test suite

In this example, the `XMLDiff` tester is used to check the `XML` output of two `VectorPostprocessors`.

!listing test/tests/outputs/xml/tests
