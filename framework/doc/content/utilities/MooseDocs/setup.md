# MooseDocs Setup

The following instructions are for setting up existing applications to use the MooseDocs
system for their own documentation.

## Dependencies

If you are using a current [MOOSE package](getting_started/installation/index.md) then the setup is
complete.

If you are not using a MOOSE package, then the following packages must be installed, which can
be done using [pip](https://pip.pypa.io/en/stable/).

```bash
pip install --user pybtex livereload pylatexenc anytree pandas
```

## Documentation Location

Create a directory for documentation within your repository where your documentation-related files
will be stored. Most existing applications will have a `doc` directory---this can be used if desired,
or another location can be created. The location and name of this directory is arbitrary.

## MooseDocs Executable

To use MooseDocs, an executable is required---this main executable is simply copied from the
executable within [MOOSE]:

```bash
cp ~/projects/moose/modules/doc/moosedocs.py ~/projects/your_application_name/doc
```

This executable contains command-line based help, which can be accessed using the "-h" flag as
follows.

```
cd ~/projects/your_application_name/docs
./moosedocs.py -h
```

Next, a configuration file must be created. Details regarding this file may be found at
[MooseDocs/config.md].

## Source Documentation

The first step for creating a web-site is to document your code, it is best to refer to the
MOOSE instructions for documentation (see [MooseDocs/generate.md]). In general, applications
mimic the MOOSE process.
