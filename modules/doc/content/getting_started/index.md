
# Getting Started

First you need to install the MOOSE framework. To do this, click the link below corresponding to
your operating system/platform and follow the instructions:

- [getting_started/installation/mac_os.md]
- [getting_started/installation/ubuntu.md]
- [getting_started/installation/mint.md]
- [getting_started/installation/opensuse.md]
- [getting_started/installation/fedora.md]
- [getting_started/installation/centos.md]
- [getting_started/installation/windows10.md]
- [getting_started/installation/hpc_install_moose.md]
- [getting_started/installation/manual_installation_gcc.md]
- [getting_started/installation/manual_installation_llvm.md]

When installation is complete, return to this page to continue.

## Create an Application id=create-an-app

MOOSE is designed for building custom applications, therefore if you plan on working with MOOSE
then you should create an application.

Your application is where code and input files should be created for your particular problem.

To create an application, run the stork.sh script while sitting outside the MOOSE repository with
a single argument providing the name you wish to use to name your application:

```bash
cd ~/projects
./moose/scripts/stork.sh YourAppName
```

Running this script will create a folder named "YourAppName" in the projects directory, this
application will automatically link against MOOSE. Obviously, the "YourAppName" should be the name
you want to give to your application; consider the use of an acronym. We prefer animal names for
applications, but you are free to choose whatever name suits your needs.

!alert note
You should not attempt to run this script while sitting inside the MOOSE repository. Doing so will result in an error.

## Compile and Test Your Application

```bash
cd ~/projects/YourAppName
make -j4
./run_tests -j4
```

If your application is working correctly, you should see one passing test. This indicates that
your application is ready to be further developed.

!include getting_started/installation/post_moose_install.md

!include getting_started/installation/installation_troubleshooting.md
