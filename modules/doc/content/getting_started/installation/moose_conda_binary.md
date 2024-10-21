# Pre-Built MOOSE

!style! halign=left
If you are not interested in developing your own MOOSE based application, and wish to use MOOSE's
many available physics solvers, you can install the pre-built fully-featured MOOSE binary.
!style-end!

If you are operating on a Windows machine, please first follow
[Windows Subsystem for Linux](installation/windows.md), and then come back to these instructions.

## Install Conda

!style! halign=left
Our preferred method for delivering pre-built MOOSE binaries is via Conda.
!style-end!

!include getting_started/installation/install_miniconda.md

## Install MOOSE

!style! halign=left
With Conda initialized, create the `moose` environment and install `moose`:
!style-end!

```bash
conda create -n moose moose
```

After the installation completes, activate the new environment:

```bash
conda activate moose
```

!alert note
Know that you will need to `conda activate moose` for +each terminal window you open, and each time
you wish to perform MOOSE related work+. If you wish to make this automatic, you can add that
command to the end of your shell profile.

Verify `moose` is available:

```bash
moose --help
```

Should produce the help page. This simple command demonstrates that you have successfully installed
the MOOSE Conda package.

!template load file=getting_started/installation/workshop_tutorial.md.template TUTORIAL=heat_transfer

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

The easiest solution is to open two terminal windows. While in one, you have `moose` activated.
While in the other, you have `paraview` activated. Open a new terminal window now, and create the
new `paraview` environment:

```bash
conda activate base # just in case you have `moose` auto-activating
conda create -n paraview paraview
conda activate paraview
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
Continue on to see more examples and tutorials using MOOSE! However, most of the next section is
geared towards developing your own application.
!style-end!

!content pagination use_title=True
                    previous=installation/index.md
                    next=examples_and_tutorials/index_without_new_users.md
