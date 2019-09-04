
# Getting Started

First you need to install the MOOSE framework. To do this, click the link below corresponding to
your operating system/platform and follow the instructions:

- +MacOS+:

  - [Mojave](getting_started/installation/macos_mojave.md)
  - [High Sierra or older](getting_started/installation/macos_other.md)

- +Linux+:

  - [getting_started/installation/ubuntu.md]
  - [getting_started/installation/mint.md]
  - [getting_started/installation/opensuse.md]
  - [getting_started/installation/fedora.md]
  - [getting_started/installation/centos.md]

- [getting_started/installation/windows10.md]
- [getting_started/installation/docker.md]
- Advanced Instructions:

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

## Customizing MOOSE configuration

MOOSE can be customized by running a `configure` script in
`$MOOSE_DIR/framework`. Below we summarize the configuration options available:

### Automatic differentiation

- `--with-derivative-type`: Specify the derivative storage type to use for
  MOOSE's `DualReal` object. Options are `sparse` and `nonsparse`. `sparse`
  selects `SemiDynamicSparseNumberArray` as the derivative storage type;
  `nonsparse` selects `NumberArray`. A more detailed overview of these storage
  types can be found in the [`DualReal` documentation](/DualReal.md).
- `--with-derivative-size=<n>`: Specify the length of the underlying derivative
  storage array. The default is 50. A smaller number may be chosen for increased
  speed; a larger number may be required for 3D problems or problems with
  coupling between many variables.
