# Anaconda MOOSE Environment

Our preferred method for obtaining libraries necessary for MOOSE based Application development, is through Anaconda. Follow these instructions to create an environment on your machine using Anaconda.

!include getting_started/minimum_requirements.md

## Prerequisites

!include installation/remove_moose_environment.md

## Install Miniconda id=installminiconda

- [Install Miniconda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html)
- Configure Conda to work with conda-forge, and our Idaholab channel:

  ```bash
  conda config --add channels conda-forge
  conda config --add channels idaholab
  ```

- Install the moose-env package from Idaholab:

  ```bash
  conda create --name moose moose-env
  ```

- Activate the moose environment:

  ```bash
  conda activate moose
  ```

  Some folks may receive additional instructions when attempting to activate a profile. Follow those instructions, and try to activate the moose environment again. You have successfully activated the environment when you see the name of this environment prefixed within your prompt:

  ```bash
  (moose) [x86_64-apple-darwin13]
  ```

If you close, and then re-open this terminal window, know that you will need to `conda activate moose` again. You will need to do this for every terminal you open. If you wish to make this automatic, you can append `conda activate moose` to your bash or zsh profile.

!alert note title= sudo is not necessary
If you find yourself using `sudo` for any of the above conda commands... you shouldn't be. Miniconda should be installed to your home directory. Therefore you should already have write access. Try and figure out +why+ you don't have permissions before forcing a conda command with `sudo`.

!include getting_started/installation/clone_moose.md

!include getting_started/installation/test_moose.md

!include getting_started/installation/update_moose.md

Head back over to the [getting_started/index.md] page to continue your tour of MOOSE.
