# MooseDocs Setup

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
cp ~/projects/moose/doc/moosedocs.py ~/projects/your_application_name/docs
```

This executable contains command-line based help, which can be accessed using the "-h" flag as
follows.

```
cd ~/projects/your_application_name/docs
./moosedocs.py -h
```

!include MooseDocs/config.md

## Source Documentation

The first step for creating a web-site is to document your code, it is best to refer to the
MOOSE instruction for documentation for more information (see [MooseDocs/generate.md]), your
application will generally mimic the MOOSE process.
