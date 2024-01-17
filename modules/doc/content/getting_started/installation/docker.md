# Docker

We produce two Docker images, one that is a development environment (the `moose-dev` image) for MOOSE-based applications and one with an installed MOOSE application (the `moose` image) with all modules enabled.

If you are modifying a MOOSE-based application, you should use the `moose-dev` image described in [#development-image]. Otherwise, you should use the `moose` image described in [#installed-image].

These images are hosted on Docker Hub in the [idaholab](https://hub.docker.com/r/idaholab) organization and are based on [rockylinux:[!package!apptainer_rockylinux]](https://hub.docker.com/_/rockylinux) with GCC [!package!apptainer_gcc] and MPICH [!package!apptainer_mpich].

## Development image

The [idaholab/moose-dev](https://hub.docker.com/r/idaholab/moose-dev) image contains all of the dependencies needed to build a MOOSE-based application. This includes a compiler, MPI, PETSc, libMesh, and the necessary python tools.

How you use this image is dependent on your development style, but a simple example that includes creating a volume, cloning, building, and then testing the MOOSE framework follows:

```bash
docker volume create projects
docker run -it -v projects:/projects idaholab/moose-dev:latest
cd /projects
git clone https://github.com/idaholab/moose.git
cd moose/test
make -j 4
./run_tests -j 4
```

!alert note
If you are building another image from this one, you may need to source the file at `/environment` first to obtain the proper environment.

This image is versioned based on a script in the MOOSE repository that hashes it based on the state of the dependencies. To obtain the proper version of `moose-dev` to use given the current state of moose, you can run (within MOOSE):

```bash
./scripts/versioner.py moose-dev
```

## Installed image

The [idaholab/moose](https://hub.docker.com/r/idaholab/moose) is based on [idaholab/moose-dev](https://hub.docker.com/r/idaholab/moose-dev), and also contains a fully installed version of MOOSE compiled with all of the modules in `/opt/moose`. The executable is `moose-opt`, which is located in the path.

An example of running the electromagnetics module tests with this image is as follows:

```
docker volume create projects
docker run -it -v projects:/projects idaholab/moose:latest
cd /projects
moose-opt --copy-inputs electromagnetics
cd moose/electromagnetics
moose-opt --run -j 4
```

This image is versioned based on the master hash of MOOSE that it was compiled with.
