## Install MOOSE Conda Packages id=moosepackages

!alert! note
If conda was installed previously, and the [install_miniconda.md] instructions were skipped,
we highly encourage the use of [Mamba](https://github.com/mamba-org/mamba) to install
packages, due to speed. We recommend that you perform:

```
conda deactivate
conda install mamba
```

in your base environment.
!alert-end!

Begin by creating the moose environment within conda, and attempt to activate it:

```bash
conda create --name moose -q -y
conda activate moose
```

During activation, you may be presented with an error such as the following:

```pre
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

This error is trying to exlain how to initialize Conda for the first time for your given shell environment. Different operating systems use different shells. To understand what shell you are using, perform the following:

```bash
echo $0
```

What ever is returned is your shell. With that information in hand (if you received the above error to begin with) perform the following init command:

```bash
conda init SHELL_NAME
```

Where SHELL_NAME is the shell you discovered in the previous step. Once complete, close any terminals you have opened, and re-open them. Then run `conda activate moose` again. If successful proceed to installing packages:

```bash
mamba install moose-tools
mamba install moose-libmesh
```

If you are running into additional errors, please see our [troubleshooting guide for Conda](troubleshooting.md#condaissues optional=True).

!alert note
Know that you will need to `conda activate moose` again for +each terminal window you open+. If you wish to make this automatic, you can add that command to the end of your shell profile.
