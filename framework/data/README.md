# Data file directory

Files located in this directory or in the `moose/modules/*/data` or `app/data` directories
can be retrieved using the `getDataFileName(const std::string & param)` function, where
`param` is an input parameter of type `FileName`

`getDataFileName` will search (in this order)

- relative to the input file
- relative to the running binary in the shared directory (assuming the application is installed)
- relative to all registered data file directories (which are determined by the source file locations when compiling)

This file is retrieved in the `test/tests/misc/data_file_name/test.i` test as data file.
