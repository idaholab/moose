## Install Mambaforge3 id=installconda

Follow the steps below depending on your platform to install mambaforge. If you run into issues during these steps, please visit our [Conda Troubleshooting](help/faq/conda_issues.md optional=True) guide. This installation
guide relies on the utilization of `mamba`, an optimized package manager for Conda.

- +Linux Users:+

  ```bash
  curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Mambaforge-Linux-x86_64.sh
  bash Mambaforge-Linux-x86_64.sh -b -p ~/mambaforge3
  ```

- +Macintosh Users with Intel processors:+

  ```bash
  curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Mambaforge-MacOSX-x86_64.sh
  bash Mambaforge-MacOSX-x86_64.sh -b -p ~/mambaforge3
  ```

- +Macintosh Users with Apple Silicon processors:+

  ```bash
  curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Mambaforge-MacOSX-arm64.sh
  bash Mambaforge-MacOSX-arm64.sh -b -p ~/mambaforge3
  ```

With Mambaforge installed to your home directory, export PATH, so that it may be used:

```bash
export PATH=$HOME/mambaforge3/bin:$PATH
```

Configure Conda to work with our INL public channel:

```bash
conda config --add channels https://conda.software.inl.gov/public
```

!alert warning title=sudo conda
If you find yourself using `sudo conda`/`sudo mamba`... something is not right. The most common reason for needing sudo is due to an improper Conda installation. Conda *should* be installed to your home directory, without any use of `sudo`.
