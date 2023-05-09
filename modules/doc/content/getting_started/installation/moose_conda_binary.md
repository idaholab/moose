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

There are examples for each physics solver available by name, in the following directory:

```bash
ls $CONDA_PREFIX/moose/share/combined
```

!alert! note
Not everything you find in this directory is a physics library. We are working on an elegant way to
ask `moose` for all available solvers.
!alert-end!

For now, lets copy the reactor module into a safe location for editing:

```bash
mkdir -p ~/projects/examples
cd ~/projects/examples
moose --copy-inputs reactor_tutorial
  <output trimmed>
Directory successfully copied into ./combined/reactor_tutorial/
```

!alert! note
Take note of the information being displayed in the output. `moose` is alerting to the directory
structure it created (`the last line`). Which can sometimes not represent the exact wordage you
provided as arguments.
!alert-end!

With the reactor module's examples and inputs copied, move into the reactor workshop directory and
instruct `moose` to run the tests:

```bash
cd combined/reactor_tutorial
moose --run -j 6
```

Testing will commence and take a few moments to finish. There may be several skipped tests for one
reason or another. This is normal. However none of the tests should fail.

Next, we will run a single input file manually, to demonstrate how you will ultimately be using
`moose`. Peruse the subdirectories and find an input file you wish to run:

!alert! tip
You can list all available input files by running:

```bash
cd ~/projects/examples/combined/reactor_tutorial
find . -name '*.i'
```

!alert-end!

```bash
cd <desired directory where input file of your choice resides>
moose -i <the input file you chose>.i --mesh-only
```

You will see some information scroll by, and ultimately end back at your prompt. If you perform a
directory listing (`ls`) you should see an exodus file was generated in the process (a `_in.e`
file).

## Viewing Results

!style! halign=left
`<the input file you chose>_in.e` can be opened with [Paraview](https://www.paraview.org/). A free
tool available for all major operating systems for viewing mesh files of many sorts (including
Exodus). Paraview is also available from Conda!
!style-end!

!alert! warning
If you are interested in installing this package using Conda, you will need to do so in a new
environment. As the `moose` environment you are using now is incompatible with some of the
dependencies required. This means while you are running `moose` problems, you will need to be in the
`moose` Conda environment. When you want to view results, you will need to be in `paraview`'s
environment. Conda makes this easy, but it will be up to you to watch your prompt and understand
when to activate one or the other.
!alert-end!

The easiest sollution is to open two terminal windows. While in one, you have `moose` activated.
While in the other, you have `paraview` activated. Open a new terminal window now, and create the
new `paraview` environment:

```bash
mamba activate base # just in case you have `moose` auto-activating
mamba create -n paraview paraview
mamba activate paraview
```

With paraview installed, you can now open `<the input file you chose>_in.e` with the following
command:

```bash
paraview <the input file you chose>_in.e
```

!alert! note
The very first time you attempt to run `paraview` it can take *minutes* before it launches.
Consecutive launches are quick.
!alert-end!

## More Examples

!style! halign=left
Continue on to see more examples and tuturials using MOOSE! However, most of the next section is
geared towards developing your own application.
!style-end!

!content pagination use_title=True
                    previous=installation/index.md
                    next=examples_and_tutorials/index_without_new_users.md
