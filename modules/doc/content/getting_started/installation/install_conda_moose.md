## Install MOOSE Conda Packages id=moosepackages

Install the moose-libmesh and moose-tools package from mooseframework.org, and name your environment 'moose':

```bash
conda create --name moose moose-libmesh moose-tools
```

Activate the moose environment (may result in an error):

```bash
conda activate moose
```

Follow any additional on-screen instructions while attempting to run `conda activate`, and try to activate again. If an error continues, please see `conda activate moose` section in our [troubleshooting guide for Conda](troubleshooting.md#condaissues optional=True).

You will have successfully activated the moose environment when you see (moose) prefixed within your prompt (and/or when no error occurs).

!alert note
Know that you will need to `conda activate moose` for +each terminal window you open+. If you wish to make this automatic, you can add that command to the end of your shell profile.
