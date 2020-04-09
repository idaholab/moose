# MOOSE Conda Environment

This directory contains the conda recipes necessary for MOOSE Application based development.


## Installation Instructions

- [Install Miniconda or Anaconda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html) (we recommend Miniconda)

- Configure Conda to work with conda-forge, and our MOOSE Framework server:

  ```bash
  conda config --add channels conda-forge
  conda config --add channels https://mooseframework.org/conda/moose
  ```

- Install the moose-env package from Idaholab and name your environment 'moose':

  ```bash
  conda create --name moose-libmesh moose-tools
  ```

- Activate the moose environment +(do this for any new terminal opened)+:

  ```bash
  conda activate moose
  ```

  Some folks may receive additional instructions when attempting to activate a profile. Follow those instructions, and try to activate the moose environment again.

  You will have successfully activated the moose environment when you see 'moose' within your prompt.

If you close, and re-open this terminal window, know that you will need to `conda activate moose` again. You will need to do this for every terminal you open. If you wish to make this automatic, you can append `conda activate moose` to your shell profile.

The MOOSE team will make periodic updates to the conda packages. To stay up-to-date, activate the moose environment, and perform an update:

```bash
conda activate moose
conda update --all
```
