# Anaconda MOOSE Environment

Our preferred method for obtaining libraries necessary for MOOSE based Application development, is through Anaconda. Follow these instructions to create an environment on your machine using Anaconda.

!include getting_started/minimum_requirements.md

## Prerequisites

- [Install Miniconda](https://docs.conda.io/projects/conda/en/latest/user-guide/install/index.html)
- Configure Conda to work with conda-forge, and our Idaholab channel:

  ```bash
  conda config --add channels conda-forge idaholab
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

  ```pre
  (moose) [x86_64-apple-darwin13]
  ```

!include getting_started/installation/install_moose.md
