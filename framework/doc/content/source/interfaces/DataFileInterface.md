# DataFileInterface

This class provides an API for resolving paths to data files distributed alongside
MOOSE based apps and modules.

| Method | Description |
| - | - |
getDataFilePath | Finds a data file given a relative path

Files located in `<your_app>/data` can be registered as data paths for use in installed and in-tree
builds of applications. The MOOSE framework and MOOSE module data directories
of `moose/framework/data` and `moose/modules/*/data` are already registered. These
data directories (located at the root of an application) are installed automatically using the standard `make install`.

To make your data available for searching with this interface, register it with the following:

- the `registerAppDataFilePath` macro in `Registry.h`, where an applications data in its root `data` directory is registered (ex: `registerAppDataFilePath("foo_bar")` for `FooBarApp`)
- the `registerNonAppDataFilePath` macro in `Registry.h`, where a general data directory is registered

Once a data path is registered, it can be searched using this interface and via `DataFileName`
parameters. This search is consistent between both in-tree and installed builds of an application.

When a parameter is specified as `DataFileName` type, the corresponding value that you get
via `getParam<DataFileName>` is the searched value (the user's input is used for the search).
You may also utilize the `getDataFilePath()` method within this interface to manually
search for a relative path in the data without the use of a parameter (for hard-coded data). The search order for these path is the following:

- if the path is absolute, use the absolute path (only for `DataFileName` parameters)
- relative to the input file (only for `DataFileName` parameters)
- relative to all installed and registered data file directories (for an installed application)
- relative to all in-tree registered data file directories (for an in-tree build)

You can see the file paths that were found in the search for all `DataFileName` parameters
via the `--show-data-files` command line option.
