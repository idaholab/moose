
Follow the steps below depending on your platform to install Miniforge. If you run into issues
during these steps, please visit our [Conda Troubleshooting](help/faq/conda_issues.md optional=True)
guide.

- +Linux Users:+

  ```bash
  curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-Linux-x86_64.sh
  bash Miniforge3-Linux-x86_64.sh -b -p ~/miniforge
  ```

- +Macintosh Users with Intel processors:+

  ```bash
  curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-x86_64.sh
  bash Miniforge3-MacOSX-x86_64.sh -b -p ~/miniforge
  ```

- +Macintosh Users with Apple Silicon processors:+

  ```bash
  curl -L -O https://github.com/conda-forge/miniforge/releases/latest/download/Miniforge3-MacOSX-arm64.sh
  bash Miniforge3-MacOSX-arm64.sh -b -p ~/miniforge
  ```

With Miniforge installed in your home directory, export PATH so that it may be used:

```bash
export PATH=$HOME/miniforge/bin:$PATH
```

Now that we can execute `conda`, initialize it and then exit the terminal:

```bash
conda init --all
exit
```

Upon restarting your terminal, you should see your prompt prefixed with (base). This indicates you
are in the base environment, and Conda is ready for operation:

```bash
$ (base) ~>
```

The next thing you should do after a fresh install, is perform an update to the base Conda
environment:

```bash
conda update --all --yes
```

Add [!ac](INL)'s public channel to gain access to [!ac](INL)'s Conda package library:

```bash
conda config --add channels https://conda.software.inl.gov/public
```

!alert warning title=Do not use sudo
If you find yourself using `sudo` commands while engaging Conda commands... something is not right.
The most common reason for needing sudo is due to an improper Conda installation. Conda *should* be
installed to your home directory, without any use of `sudo`.
