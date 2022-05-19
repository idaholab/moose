# DataFileInterface

This class provides API for resolving paths to data files distributed alongside
MOOSE based apps and modules.

| Method | Description |
| - | - |
getDataFileName | Finds a data file given a `FileName` input parameter
getDataFileNameByName | Finds a data file given a relative path

## `getDataFileName`

Files located in `moose/framework/data`, the `moose/modules/*/data`, or
`<your_app>/data` directories can be retrieved using the `getDataFileName(const
std::string & param)` function, where `param` is an input parameter of type
`FileName`

`getDataFileName` will search (in this order)

- relative to the input file
- relative to the running binary in the shared directory (assuming the application is installed)
- relative to all registered data file directories (which are determined by the source file locations when compiling and registered using the `registerDataFilePath` macro in `Registry.h`)

## `getDataFileNameByName`

The `getDataFileNameByName` can be used for hard coded data file names. e.g.
data files that are tied to a specific model and are not meant to be user
replaceable. This method will not search relative to the input file location and
will only find data files in source repositories and installed binary
distributions.
