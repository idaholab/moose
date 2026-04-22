## MOOSE Development Environment id=moosepackages

!style! halign=left
Create a unique conda environment for [!ac](MOOSE) development, named `moose`, and install the MOOSE dependency
packages:
!style-end!

!versioner! code
conda create -n moose moose-dev=__VERSIONER_CONDA_VERSION_MOOSE_DEV__
!versioner-end!

After the installation completes, activate the new environment:

```bash
conda activate moose
```

If you are on a Macintosh machine, a supported version of the Mac OS X Software Development Kit (SDK) is required to use this environment. If a supported version has not been installed, an error will appear when calling the previous activation command. Follow the instructions in the error message to install a supported version of the SDK.

If you are running into errors, please see our
[troubleshooting guide for Conda](help/troubleshooting.md#condaissues optional=True).
