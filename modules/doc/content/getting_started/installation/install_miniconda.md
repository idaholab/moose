## Install Miniconda3 id=installconda

Installing Miniconda3 is straight forward. Download, install, and configure. If you run into issues during these steps, please visit our [troubleshooting guide for Conda](troubleshooting.md#condaissues optional=True).

- +Linux Users:+

  ```bash
  curl -L -O https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh
  bash Miniconda3-latest-Linux-x86_64.sh -b -p ~/miniconda3
  ```

- +Macintosh Users:+

  ```bash
  curl -L -O https://repo.anaconda.com/miniconda/Miniconda3-latest-MacOSX-x86_64.sh
  bash Miniconda3-latest-MacOSX-x86_64.sh -b -p ~/miniconda3
  ```

With Miniconda installed to your home directory, export PATH, so that it may be used:

```bash
export PATH=$HOME/miniconda3/bin:$PATH
```

Configure Conda to work with conda-forge, and our INL public channel:

```bash
conda config --add channels conda-forge
conda config --add channels https://conda.software.inl.gov/public
```

!alert warning title=sudo conda
If you find yourself using `sudo conda`... something's not right. The most common reason for needing sudo, is due to an improper Conda installation. Conda *should* be installed to your home directory, without any use of `sudo`.

While in the `base` conda environment, we also encourage the usage of
[Mamba](https://github.com/mamba-org/mamba), a drop-in re-implementation of the conda
package manager that takes advantage of parallelization. This will lead to faster
installation of conda packages. To install mamba, simply perform:

```
conda install mamba
```

Now `mamba` can be used instead of `conda` for installing packages.
