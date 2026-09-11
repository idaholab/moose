With Miniforge installed in your home directory, initialize it so that it can be used in other terminal windows:

```bash
PATH=$HOME/miniforge/bin:$PATH conda init --all
```

Next, +close the terminal window that was used to run the command above and open a new one+. In the new terminal window, you should see your prompt prefixed with (base). This indicates you are in the base environment, and that Conda is ready for operation.

In this new terminal window, update the base Conda environment:

```bash
conda update --all --yes
```

Lastly, add [!ac](INL)'s public channel to gain access to [!ac](INL)'s Conda package library:

```bash
conda config --add channels https://conda.software.inl.gov/public
```

## Create Environment id=create_environment

After installing Miniforge (by following the instructions immediately above), a unique Conda environment will be created named `moose` with the packages that contain the environment:

!versioner! code
conda create -n moose-dev moose-dev=__VERSIONER_CONDA_VERSION_MOOSE_DEV__
!versioner-end!

If you are running into errors, please see our
[troubleshooting guide for Conda](help/troubleshooting.md#condaissues optional=True).

## Activate Environment id=activate_environment

Now that the `moose-dev` Conda environment has been installed, run the following to activate it:

```bash
conda activate moose-dev
```

To utilize this environment in other terminal windows, the command above must be ran first. Once this activation command is ran, the compilers and dependencies for building MOOSE or a MOOSE-based application will be available.

## Build and Test id=build_and_test

After following the instructions above to setup the environment, we will next build and test MOOSE or a MOOSE-based application.

All of the commands that follow +must+ be ran within a terminal window in which you have ran `conda activate moose` first as described in [#activate_environment].

Follow the instructions in [#build_and_test_moose] if you are building MOOSE. Otherwise, follow the instructions in [#build_and_test_moose_application].

### Build and Test: MOOSE id=build_and_test_moose

!template load file=installation/developer/includes/build_test_moose.md.template moose_jobs=4

### Build and Test: MOOSE Application id=build_and_test_moose_application

!template load file=installation/developer/includes/build_test_moose_app.md.template moose_jobs=4
