# Conda MOOSE Environment

Our preferred method for obtaining libraries necessary for MOOSE based Application development, is through Conda. Follow these instructions to create an environment on your machine using Conda. At this time, a Conda install via Windows is not supported.

!include sqa/minimum_requirements.md

## Prerequisites

!include installation/remove_moose_environment.md

## Install Conda id=installconda

- [Install Miniconda3 or Anaconda for Python 3.x](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html) (we recommend Miniconda)

  - +Catalina Users:+ It is recommended you +do not+ choose the pkg installation technique. Please download the 'bash' or 'command line' variant, as there appears to be issues installing the pkg version on Zsh enabled machines.

- Configure Conda to work with conda-forge, and our Idaholab channel:

  ```bash
  conda config --add channels conda-forge
  conda config --add channels https://mooseframework.org/conda/moose
  ```

- Install the moose-libmesh and moose-tools package from mooseframework.org, and name your environment 'moose':

  ```bash
  conda create --name moose moose-libmesh moose-tools
  ```

- Activate the moose environment +(do this for any new terminal opened)+:

  ```bash
  conda activate moose
  ```

  Some folks may receive additional instructions when attempting to activate a profile. Follow those instructions, and try to activate the moose environment again.

  You will have successfully activated the moose environment when you see 'moose' within your prompt.

If you close, and re-open this terminal window, know that you will need to `conda activate moose` again. You will need to do this for each terminal you open. If you wish to make this automatic, you may append `conda activate moose` within your bash or zsh profile.

The MOOSE team will make periodic updates to the conda packages. To stay up-to-date, activate the moose environment, and perform an update:

```bash
conda activate moose
conda update --all
```

!alert note title= sudo is not necessary
If you find yourself applying the use of `sudo` for any of the above conda commands... something's not right. The most common reason for needing sudo, is due to an improper installation. Conda *should* be installed to your home directory, and without any use of `sudo`.

!include getting_started/installation/clone_moose.md

!include getting_started/installation/test_moose.md

Head back over to the [getting_started/index.md] page to continue your tour of MOOSE.

## Uninstall Conda MOOSE Environment

If you wish to remove the moose environment at any time, you may do so using the following commands:

```bash
conda deactivate   # if 'moose' was currently activated
conda remove --name moose --all
```
