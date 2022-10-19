# Local Binary Installation

Use the NCRC client to download binaries to your local workstation.

The NCRC client allows authorized users to search, install, and upgrade NCRC binaries available on INL servers.

## Install NCRC Client

The NCRC client is available via INL's public Conda channel repository.

!alert note
The client must be installed into the base Conda environment.

INL Conda Repository:

```bash
$> conda activate base
$> conda config --add channels https://conda.software.inl.gov/public
$> conda install ncrc
```

## Syntax

```pre
$> ncrc --help

usage: ncrc [-h] {install,remove,update,search,list} ...

Manage NCRC packages

positional arguments:
  {install,remove,update,search,list}
                        Available Commands.
    install             Install application
    remove              Prints information on how to remove application
    update              Update application
    search              Perform a regular expression search for an NCRC
                        application
    list                List all available NCRC applications

optional arguments:
  -h, --help            show this help message and exit
```

## Common Usage

##### List all available NCRC applications

```bash
 $> ncrc list

# Use 'ncrc search name-of-application' to list more detail
# NCRC applications available:

pronghorn
bison
griffin
sabertooth
relap7
```

##### Lists all available versions of a specific NCRC application

```bash
$> ncrc search griffin

Loading channels: done
# Name                       Version           Build  Channel
ncrc-griffin              2021_07_29         build_0  ncrc-applications
ncrc-griffin              2021_08_27         build_0  ncrc-applications
ncrc-griffin              2021_08_31         build_0  ncrc-applications
ncrc-griffin              2021_09_03         build_0  ncrc-applications
ncrc-griffin              2021_09_09         build_0  ncrc-applications
ncrc-griffin              2021_09_15         build_0  ncrc-applications
ncrc-griffin              2021_09_25         build_0  ncrc-applications
ncrc-griffin              2021_10_02         build_0  ncrc-applications
```

##### Install an NCRC application

Installing an NCRC application requires authentication. If you have sufficient privileges the package installation will commence:

```bash
$> conda activate base
$ (base) >
$ (base) > ncrc install bison

Username: johndoe
PIN+TOKEN:
Installing bison...
```

##### Install a specific NCRC application version

Don't want to use the latest? For whatever the reason, you can install a specific version of an available NCRC application in the following way:

```bash
$> conda activate base
$ (base) >
$ (base) > ncrc install griffin=2021_09_15

Username: johndoe
PIN+TOKEN:
Installing griffin=2021_09_15...
```

##### Update an NCRC application

To update your NCRC application to the latest available, you first must activate the environment it is in, and then perform the update:

```bash
$> conda activate bison
$ (bison) > ncrc update bison

Username: johndoe
PIN+TOKEN:
```

##### Uninstalling NCRC application

The NCRC script being a wrapper tool, is unable to perform such a function. The user must deactivate the environment
and remove that environment using the appropriate conda commands instead. The following is an error scenario instructing
the user how to achieve the task:

```bash
$> ncrc remove bison

Due to the way ncrc wraps itself into conda commands, it is best to
remove the environment in which the application is installed. Begin
by deactivating the application environment and then remove it:
  conda deactivate
  conda env remove -n bison
```

## Using the Application

Once your NCRC application is installed, you have access to use said binary and/or view the documentation.

!alert note
Where applicable, you only need to activate the environment once. Each example below assume the environment has not yet been activated.

##### Running Tests

To run the application's tests, you first instruct that application to copy its test files into a usable directory, enter said directory, and then run the tests. As follows:

```bash
$> conda activate bison
$ (bison) >
$ (bison) > bison-opt --copy-inputs tests

<Trimmed results. Output consists of tests being copied.>

$ (bison) > cd bison/tests
$ (bison) > bison-opt --run -j 5
fipd_rodlet_mesh_generator.test ........................................................................... OK
fill_gas_thermal_conductivity.legacy.He ................................................................... OK
<trimmed>
triso_pebble.3d_pebble_from_csv .............................................................. [min_cpus=5] OK
triso_pebble.cylinder_2d_rz ............................................................................... OK
triso_pebble.cylinder_3d .................................................................................. OK
triso_pebble.sphere_2d_rz ................................................................................. OK
triso_pebble.sphere_3d .................................................................................... OK
triso_pebble.plate ........................................................................................ OK
--------------------------------------------------------------------------------------------------------------
Ran 1606 tests in 274.6 seconds. Average test time 0.7 seconds, maximum test time 20.7 seconds.
1606 passed, 22 skipped, 0 pending, 0 failed
```

##### Some Applications contain more than tests

Some applications have different inputs that can be copied and executed. Each application can use whatever naming convention
they wish, so listing all possibilities is impossible. However, any runnable input directory will have a file called `testroot`
residing in it. The following *crazy* command will return those possibilities (using Bison as an example):

```bash
$> conda activate bison
$ (bison) > find $CONDA_PREFIX/$NCRC_APP/share/$NCRC_APP -maxdepth 2 -type f -name 'testroot' \
| awk 'BEGIN { FS = "/" } ; { print $(NF-1) }'
workshop
tools
tests
assessment
examples
benchmarks
```

##### Run a specific input file

Unquestionably the real reason you installed said NCRC application, you can run your own input files for solving in the following way:

```bash
$> conda activate bison
$ (bison) >
$ (bison) > cd ~/path/to/my/input/file
$ (bison) > bison-opt -i my_input.i
# Using MPI
$ (bison) > mpiexec -n 8 bison-opt -i my_input.i
```

##### View Documentation

Documentation exists as part of the binary installation process. You can access documentation by instructing your favorite web browser to open the index.html file.
We have made finding the location of this file via an environment variable, prefix with the NCRC application name:

```bash
$> conda activate bison
$ (bison) >
$ (bison) > echo $bison_DOCS
/home/johndoe/miniconda3/envs/bison/bison/share/bison/doc/index.html
```

You can then enter displayed results into your favorite web browser. Similarly on Macintosh machines, you can make use of the `open` command, to open the environment variable directly:

```bash
$> conda activate bison
$ (bison) > open $bison_DOCS
```
