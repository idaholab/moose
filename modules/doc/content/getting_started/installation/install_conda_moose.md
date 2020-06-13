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

The MOOSE team will make periodic updates to the conda packages. To stay up-to-date, activate the moose environment, and perform an update:

```bash
conda activate moose
conda update --all
```

!alert note title= sudo is not necessary
If you find yourself applying the use of `sudo` for any of the above conda commands... something's not right. The most common reason for needing sudo, is due to an improper installation. Conda *should* be installed to your home directory, and without any use of `sudo`.
