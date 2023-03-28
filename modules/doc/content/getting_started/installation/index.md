# Installing MOOSE

To begin developing with MOOSE, click a link corresponding to your operating system or another
method of choice:

- [Linux and MacOS](installation/conda.md)
- [Docker](installation/docker.md)
- [Windows Subsystem for Linux](installation/windows.md)
- [INL's HPC Pre-Built MOOSE](installation/inl_hpc_prebuilt_moose.md)
- Optional packages:

  - [installation/install_libtorch.md]

When the above does not quite fit your needs, the following instructions are available:

- [Offline (air-gapped)](installation/offline_installation.md)
- [HPC Clusters](installation/hpc_install_moose.md)
- [INL's HPC Cluster](installation/inl_hpc_install_moose.md)
- [From Source (GCC)](installation/manual_installation_gcc.md)
- [From Source (LLVM/Clang)](installation/manual_installation_llvm.md)
- [installation/manual_installation_linux_lldb.md]

!include installation/installation_troubleshooting.md

!template load file=installation/update_moose.md.template PATH=~/projects

!content pagination use_title=True
                    previous=getting_started/index.md
                    next=getting_started/new_users.md
