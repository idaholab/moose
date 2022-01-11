## Install MOOSE Conda Packages id=moosepackages

Before we create our virtual conda environment, we first need to initialize mamba.
For this, execute the following command and +restart your terminal session+.

```bash
mamba init
```

Next, create a unique conda environment for moose, named `moose`, and attempt to activate it:

```bash
mamba create --name moose -q -y
mamba activate moose
```

Within the `moose` environment, install the necessary packages:

```bash
mamba install moose-tools moose-libmesh
```

Once the packages are installed, the `moose` environment needs to be deactivated and
reactivated to ensure that the environmental variables in the installed
packages are set properly.

```bash
mamba deactivate
mamba activate moose
```

If you are running into errors, please see our [troubleshooting guide for Conda](troubleshooting.md#condaissues optional=True).

!alert note
Know that you will need to `mamba activate moose` again for +each terminal window you open+. If you wish to make this automatic, you can add that command to the end of your shell profile.
