# Pre-Built MOOSE

!style! halign=left
If you are not interested in developing your own MOOSE based application, and wish to use MOOSE's
many available physics solvers, you can install the pre-built fully-featured MOOSE binary.
!style-end!

## Install Conda

!style! halign=left
Our preferred method for deliverying pre-built MOOSE binaries is via Conda.
!style-end!

!include getting_started/installation/install_miniconda.md

## Install MOOSE

!style! halign=left
With Conda initialized, create the `moose` environment and install `moose`:
!style-end!

```bash
mamba create -n moose moose
```

After the installation completes, activate the new environment:

```bash
mamba activate moose
```

!alert note
Know that you will need to `mamba activate moose` for +each terminal window you open, and each time
you wish to perform MOOSE related work+. If you wish to make this automatic, you can add that
command to the end of your shell profile.

Verify `moose` is available:

```bash
moose --help
```

Should produce the help page.

You can now execute `moose` when attempting to run problems:

```bash
moose -i <some input file>.i
```

## Cloning MOOSE

!style! halign=left
While unnecessary, performing the following will provide numerous example input files to execute.
!style-end!

!template load file=installation/clone_moose.md.template PATH=~/projects

Example input files are located in `~/projects/moose/examples` directory.

!include getting_started/installation/uninstall_conda.md

## Examples

!style! halign=left
Most of the following examples and tutorials in the next section assume you are developing your own
application, or requiring that you have built MOOSE from source. You can ignore these requirements
while using the Pre-Built MOOSE binary. Where applicable, skip directly to creating an input file,
and attempt to run it using:
!style-end!

```bash
moose -i <some input file>.i
```

!content pagination use_title=True
                    previous=installation/index.md
                    next=examples_and_tutorials/index_without_new_users.md
