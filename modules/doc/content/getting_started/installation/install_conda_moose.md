## Install MOOSE id=moosepackages

!style! halign=left
Create a unique conda environment for [!ac](MOOSE), named `moose`, and install the MOOSE dependency
packages:
!style-end!

!package! code
conda create -n moose moose-dev=__MOOSE_DEV__
!package-end!

After the installation completes, activate the new environment:

```bash
conda activate moose
```

If you are running into errors, please see our
[troubleshooting guide for Conda](help/troubleshooting.md#condaissues optional=True).

!alert note
Know that you will need to `conda activate moose` for +each terminal window you open, and each time
you wish to perform MOOSE related work+. If you wish to make this automatic, you can add that
command to the end of your shell profile with the command below:

```bash
if [[ "$0" = *"bash" ]]; then
    echo "conda activate moose" >> ~/.bash_profile
elif [[ "$0" = "zsh" ]]; then
    echo "conda activate moose" >> ~/.zshrc
fi
```
