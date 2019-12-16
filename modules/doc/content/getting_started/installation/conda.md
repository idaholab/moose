# Conda MOOSE Environment

Our preferred method for obtaining libraries necessary for MOOSE based Application development, is through Conda. Follow these instructions to create an environment on your machine using Conda. At this time, a Conda install via Windows is not supported.

!include getting_started/minimum_requirements.md

## Prerequisites

!include installation/remove_moose_environment.md

## Install Conda id=installconda

- [Install Miniconda or Anaconda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html) (we recommend Miniconda)

- Configure Conda to work with conda-forge, and our Idaholab channel:

  ```bash
  conda config --add channels conda-forge
  conda config --add channels idaholab
  ```

- Install the moose-env package from Idaholab and name your environment 'moose':

  ```bash
  conda create --name moose moose-env
  ```

- Activate the moose environment:

  ```bash
  conda activate moose
  ```

  Some folks may receive additional instructions when attempting to activate a profile. Follow those instructions, and try to activate the moose environment again.

  You will have successfully activated the moose environment when you see 'moose' within your prompt.

If you close, and then re-open this terminal window, know that you will need to `conda activate moose` again. You will need to do this for every terminal you open. If you wish to make this automatic, you can append `conda activate moose` to your bash or zsh profile.

!alert note title= sudo is not necessary
If you find yourself using `sudo` for any of the above conda commands... you shouldn't be. Conda should be installed to your home directory. Therefore you should already have write access. Try and figure out +why+ you don't have permissions before forcing a conda command with `sudo`.

!include getting_started/installation/clone_moose.md

!include getting_started/installation/test_moose.md

!include getting_started/installation/update_moose.md

Head back over to the [getting_started/index.md] page to continue your tour of MOOSE.
