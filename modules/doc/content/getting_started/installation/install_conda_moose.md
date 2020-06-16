## Install MOOSE Conda Packages id=moosepackages

Install the moose-libmesh and moose-tools package from mooseframework.org, and name your environment 'moose':

```bash
conda create --name moose moose-libmesh moose-tools
```

Activate the moose environment +(do this for any new terminal opened)+:

```bash
conda activate moose
```

Some folks may receive additional instructions when attempting to activate a profile. Follow those instructions, and try to activate the moose environment again.

You will have successfully activated the moose environment when you see 'moose' within your prompt.

If you close, and re-open this terminal window, know that you will need to `conda activate moose` again. You will need to do this for each terminal you open. If you wish to make this automatic, you may append `conda activate moose` to your bash or zsh profiles.

## Keeping Conda up to date

The MOOSE team will make periodic updates to the conda packages. To stay up-to-date, activate the moose environment, and perform an update:

```bash
conda activate moose
conda update --all
```

!alert note title=Keep Conda updates and MOOSE updates in sync
Know, that after performing a conda update, it is always advisable to update and rebuild MOOSE, and/or your Application(s).
