# Local Binary Installation

Use the NCRC client to download binaries to your local workstation.

The NCRC client allows authorized users to search, install, and upgrade NCRC binaries available on INL servers.

## Install NCRC

The NCRC client is available via INL's public Conda channel repository.

!alert note
The client must be installed into the base Conda environment.

INL Conda Repository:

```bash
$> conda activate base
$> conda config --add channels https://conda.software.inl.gov/public
$> conda install ncrc
```

## NCRC Client Syntax

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

## Common NCRC Usage Examples

!alert note
Where applicable, you only need to activate the environment once. The examples below assume the environment has not yet been activated.

-  List all available NCRC applications

  ```bash
  $> ncrc list

  # Use 'ncrc search app' to list more detail
  # NCRC applications available:

	pronghorn
	bison
	griffin
	sabertooth
	relap7
  ```

- Lists all available versions of griffin

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

- Install the latest version of Bison (default behavior)

  ```bash
  $> conda activate base
  $ (base) >
  $ (base) > ncrc install bison

  Username: johndoe
  PIN+TOKEN:
  Installing bison...
  ```

- Install a specific version

  ```bash
  $> conda activate base
  $ (base) >
  $ (base) > ncrc install griffin=2021_09_15

  Username: johndoe
  PIN+TOKEN:
  Installing griffin=2021_09_15...
  ```

- Update Bison

  ```bash
  $> conda activate bison
  $ (bison) > ncrc update bison

  Username: johndoe
  PIN+TOKEN:
  ```

## Uninstalling NCRC Applications

The NCRC script being a wrapper tool, is unable to perform such a function. The user must deactivate the environment and remove that environment using the appropiate conda commands instead:

```bash
$> ncrc remove bison

 Due to the way ncrc wraps itself into conda commands, it is best to
 remove the environment in which the application is installed. Begin
 by deactivating the application environment and then remove it:
	conda deactivate
	conda env remove -n bison
```

## Using an NCRC Application

Once an NCRC application is installed, you then have access to use said binary and/or view the documentation.

!alert note
When you activate an NCRC application, that application is then available within your PATH.

- Run Bison tests

  ```bash
  $> conda activate bison
  $ (bison) >
  $ (bison) > bison-opt --tests -j 5
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

- Run a specific input file

  ```bash
  $> conda activate bison
  $ (bison) >
  $ (bison) > cd ~/path/to/my/input/file
  $ (bison) > bison-opt -i my_input.i

  # Using MPI
  $ (bison) > mpiexec -n 8 bison-opt -i my_input.i
  ```

- View Documentation

  ```bash
  $> conda activate bison
  $ (bison) >
  $ (bison) > echo $bison_DOCS
  /home/johndoe/miniconda3/envs/bison/bison/share/bison/doc/index.html
  ```

  You can then enter that location into your favorite web browser. Similarly on Macintosh machines, you can make use of the `open` command, to open the environment variable:

  ```bash
  $> conda activate bison
  $ (bison) > open $bison_DOCS
  ```
