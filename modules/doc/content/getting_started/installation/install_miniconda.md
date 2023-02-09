
Follow the steps below depending on your platform to install mambaforge. If you run into issues
during these steps, please visit our [Conda Troubleshooting](help/faq/conda_issues.md optional=True)
guide. This installation guide relies on the utilization of `mamba`, an optimized package manager
for Conda.

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

With Mambaforge installed in your home directory, export PATH so that it may be used:

```bash
export PATH=$HOME/mambaforge3/bin:$PATH
```

Now that we can execute `mamba`, initialize it and then exit the terminal:

```bash
mamba init
exit
```

Upon restarting your terminal, you should see your prompt prefixed with (base). This indicates you
are in the base environment, and Conda is ready for operation:

```bash
$ (base) ~>
```

Add [!ac](INL)'s public channel to gain access to [!ac](INL)'s Conda package library:

```bash
conda config --add channels https://conda.software.inl.gov/public
```

!alert warning title=Do not use sudo
If you find yourself using `sudo` commands while engaging Conda commands... something is not right.
The most common reason for needing sudo is due to an improper Conda installation. Conda *should* be
installed to your home directory, without any use of `sudo`.
