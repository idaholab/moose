## Conda Issues id=condaissues

Conda issues can be the root cause for just about any issue on this page. Scroll through this section, for what may look familiar, and follow those instructions:

- #### command not found: conda

  You have yet to install conda, or your path to it is incorrect or not set. You will need to recall how you installed conda. Our instructions ask to have Miniconda3 installed to your home directory: `~/miniconda3`. Which requires you to set your PATH accordingly:

  ```bash
  export PATH=~/miniconda3/bin:$PATH
  ```

  With PATH set, try to run again, what ever command you were initially attempting.

- #### conda activate moose

  If `conda activate moose` is failing like so:

  ```bash
  CommandNotFoundError: Your shell has not been properly configured to use 'conda activate'.
  To initialize your shell, run

      $ conda init <SHELL_NAME>

  Currently supported shells are:
    - bash
    - fish
    - tcsh
    - xonsh
    - zsh
    - powershell

  See 'conda init --help' for more information and options.

  IMPORTANT: You may need to close and restart your shell after running 'conda init'.
  ```

  ...it's possible you have yet to perform a `conda init` *properly*. See conda init below.

  It could also mean you have an older version of Conda, or that the environment you are trying to activate is somewhere other than where conda thinks it should be, or simply missing / not yet created. Unfortunately, much of what can be diagnosed, is going to be beyond the scope of this document, and better left to the support teams at [Conda](https://docs.conda.io/en/latest/help-support.html). What we can attempt, is to create a new environment and go from there:

  ```bash
  conda create --name testing --quiet --yes
  ```

  The above should create an empty environment. Try and activate it:

  ```bash
  conda activate testing
  ```

  If the above is asking you to initialize conda, see `conda init` below.

  If the command failed, or the `conda create` command before it, the error will likely be involved with how Conda was first installed (perhaps with sudo rights, or as another user). You should look into removing this installation of conda, and starting over with our [Getting Started](getting_started/installation/conda.md) instructions. Failures of this nature can also mean your conda resource file (~/.condarc) is in bad shape. We have no way of diagnosing this in a troubleshooting fashion, as this file can contain more than just moose-related configs. For reference, the bare minimum should resemble the following:

  ```bash
  channels:
    - https://mooseframework.org/conda/moose
    - conda-forge
    - defaults
  ```

- #### conda init

  If `conda init` is failing, or similarly doing nothing, it is possible that Conda simply does not know what shell you are operating in, and it created a 'configuration' for the wrong shell, or not at all.

  To figure out what shell you are operating in:

  ```bash
  echo $0
  ```

  What ever returns here, is the type of shell you are operating in, and is also what you should be instructing Conda to 'initialize'. Example:

  ```bash
  conda init zsh
  ```

  +Hint:+ For Linux, this will most likely be `conda init bash`. For Macintosh, this can either be bash or zsh.

- #### Your issue not listed

  The quick fix-attempt, is to re-install the moose-packages:

  ```bash
  conda deactivate
  conda remove moose --all --yes
  conda create moose moose-libmesh moose-tools
  ```

  If the above re-install method ultimately failed, it is time to submit your errors to the [mailing list](faq/mailing_list.md).
