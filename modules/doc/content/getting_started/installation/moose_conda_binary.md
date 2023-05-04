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

Should produce the help page. This simple command demonstrates that you have sucessfully installed
the MOOSE Conda package.

## Run an Example

!style! halign=left
A MOOSE installation binary comes with several examples you can run to make sure everything
is sound, as well as moving some of the example inputs into a safe location you can play with.
!style-end!

There are examples for each physic solver available by name, in the following directory:

```bash
ls $CONDA_PREFIX/moose/share/combined
```

!alert! note
Not everything you find in this directory is a physic library. We are working on an elegant way to
ask `moose` for all available solvers.

For now, lets copy the reactor module into a safe location for editing:

```bash
mkdir -p ~/projects/examples
cd ~/projects/examples
moose --copy-inputs reactor_workshop
```

With the reactor module's examples/inputs/tests copied, move into reactor workshop directory and
instruct `moose` to run the tests:

```bash
cd combined/reactor_workshop
moose --run -j 6
```


## More Examples

!style! halign=left
Continue on to see more examples and tuturials using MOOSE! However, most of the next section is
geared towards developing your own application.
!style-end!

!content pagination use_title=True
                    previous=installation/index.md
                    next=examples_and_tutorials/index_without_new_users.md
