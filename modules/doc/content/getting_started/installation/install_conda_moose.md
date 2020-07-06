## Install MOOSE Conda Packages id=moosepackages

Install the moose-libmesh and moose-tools package from mooseframework.org, and name your environment 'moose':

```bash
conda create --name moose moose-libmesh moose-tools
```

Activate the moose environment +(do this for any new terminal opened)+:

```bash
conda activate moose
```

You may receive an error, accompanied by additional instructions when attempting to activate a profile. Follow those on-screen instructions, and try to activate the moose environment again. If you are unsure how to proceed, please see `conda activate moose` section in our [troubleshooting guide for Conda](troubleshooting.md#condaissues optional=True).

You will have successfully activated the moose environment when you see (moose) prefixed within your prompt.

!alert note
Know that you will need to `conda activate moose` again for +each terminal window you open+. If you wish to make this automatic, you can add that command to the end of your shell profile.
