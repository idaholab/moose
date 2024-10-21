## Conda Issues id=condaissues

Conda issues can be the root cause for just about any issue on this page. Scroll through this
section for what may look familiar, and follow those instructions:

- #### 404 error, The channel is not accessible or is invalid.

  If you are receiving this, you may be victim of us changing the channel name out from underneath
  you (Sorry!). Remove the offending channel(s):

  ```bash
  conda config --remove channels https://mooseframework.org/conda/moose
  conda config --remove channels https://mooseframework.com/conda/moose
  conda config --remove channels https://mooseframework.inl.gov/conda/moose
  ```

  If you receive errors about a channel not present (CondaKeyError), please ignore. You most likely
  will not have all three 'old' channels. Next, add the correct channel:

  ```bash
  conda config --add channels https://conda.software.inl.gov/public
  ```

  When you're finished, a `conda config --show channels` should resemble the following:

  ```bash
  $ conda config --show channels
  channels:
    - https://conda.software.inl.gov/public
    - conda-forge
    - defaults
  ```

- #### Download error, Timeouts

  Conda packages we produce can be quite large, and can trigger the default download timeout imposed
  upon by Conda's download routines. You can increase this time in the following way:

  ```bash
  conda config --set remote_read_timeout_secs new_timeout
  ```

  Where `new_timeout` is an integer greater than `60` (the default in seconds).

- #### command not found: conda

  You have yet to install conda, or your path to it is incorrect or not set. You will need to recall
  how you installed conda. Our instructions ask to have Miniforge installed to your home
  directory: `~/miniforge`. Which requires you to set your PATH accordingly:

  ```bash
  export PATH=~/miniforge/bin:$PATH
  ```

  With PATH set, try to run again, what ever command you were initially attempting.

- #### conda activate moose

  If `conda activate moose` is failing like so:

  ```bash
  Run 'conda init --all' to be able to run conda activate/deactivate
  and start a new shell session. Or use conda to activate/deactivate.
  ```

  ...it's possible you have yet to perform a `conda init --all`.

  It could also mean you have an older version of Conda, or that the environment you are trying to
  activate is somewhere other than where conda thinks it should be, or simply missing / not yet
  created. Unfortunately, much of what can be diagnosed, is going to be beyond the scope of this
  document, and better left to the support teams at
  [Conda](https://docs.conda.io/en/latest/help-support.html). What we can attempt, is to create a
  new environment and go from there:

  ```bash
  conda create -n testing -q -y
  ```

  The above should create an empty environment. Try and activate it:

  ```bash
  conda activate testing
  ```

  If the command continues to ask you to perform a `conda init --all` or the command failed, or the
  `conda create` command before it, the error will likely be involved with how Conda was first
  installed (perhaps with sudo rights, or as another user). You should look into removing this
  installation of conda, and starting over with our
  [Getting Started](getting_started/installation/conda.md) instructions. Failures of this nature can
  also mean your conda resource file (`~/.condarc`) is in bad shape. We have no way of diagnosing
  this in a troubleshooting fashion, as this file can contain more than just moose-related configs.
  For reference, the bare minimum should resemble the following:

  ```bash
  channels:
    - https://conda.software.inl.gov/public
    - conda-forge
    - defaults
  ```

- #### conda init

  If `conda init --all` is failing, or similarly doing nothing, it is possible that Conda simply
  does not support the shell you are operating in. To figure out what shell you are operating in run
  the following:

  ```bash
  echo $0
  ```

  What ever returns here, is the type of shell you are operating in. Please verify this is a shell
  that Conda supports.

- #### Your issue not listed

  The quick fix-attempt, is to delete the faulty environment and re-install it:

  !versioner! code
  conda activate base
  conda env remove -n moose
  conda create -n moose moose-dev=__VERSIONER_CONDA_VERSION_MOOSE_DEV__
  conda activate moose
  !versioner-end!

  If the above re-install method ultimately failed, it is time to submit your errors to the
  [discussion forum](faq/discussion_forum.md).
