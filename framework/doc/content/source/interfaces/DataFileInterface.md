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
`DataFileName`

If the provided path is absolute, no searching will take place and the absolute
path will be used. Otherwise, `getDataFileName` will search (in this order)

- relative to the input file
- relative to all installed and registered data file directories (for an installed application)
- relative to all in-tree registered data file directories (for an in-tree build)

The "registered" data file directories are directories that are registered via:

- the `registerAppDataFilePath` macro in `Registry.h`, where an applications data in its root `data` directory is registered
- the `registerDataFilePath` macro in `Registry.h`, where a general data directory is registered

## `getDataFileNameByName`

The `getDataFileNameByName` can be used for hard coded data file names. e.g.
data files that are tied to a specific model and are not meant to be user
replaceable. This method will not search relative to the input file location and
will only find data files in source repositories and installed binary
distributions.
