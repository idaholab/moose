# Docker

## Minimum System Requirements

- Some flavor of Linux, MacOS, or Windows with Docker installed
- Memory: 16 GBs (debug builds)
- Processor: 64-bit x86
- Disk: 3 GB (image size)

## Obtaining MOOSE and Running Tests

Containers of MOOSE are currently hosted on Docker Hub in the repository [idaholab/moose](https://hub.docker.com/r/idaholab/moose) for Ubuntu 20.04. The tag "latest" is kept current with the master branch of the repository, and the other tags are commit hashes to be used by codes with MOOSE as a Git submodule.  As the Docker image already has the framework compiled, it is possible to go from no extant, local copy of MOOSE to running the tests with a single command.

```bash
docker run -ti idaholab/moose:latest /bin/bash -c 'cd test; ./run_tests'
```

## Extending the Image With MOOSE Apps

With the fully configured MOOSE framework in a Docker image, the next logical step is to extend this image with whichever MOOSE app is of interest.  For example, consider another open sourced INL code, Blackbear. To build an image of Blackbear, start with a Dockerfile as follows:

```docker
FROM idaholab/moose:latest

WORKDIR /opt

RUN git clone -b master https://github.com/idaholab/blackbear.git ; \
cd blackbear ; \
git submodule update --init ; \
make -j $(grep -c ^processor /proc/cpuinfo)

WORKDIR /opt/blackbear
```

The first line of this file tells Docker to use MOOSE as a base image.  The tag "latest" is used, because Blackbear does not have MOOSE as a submodule.  If it did, a commit hash would be used as the tag instead.  From here, the ```RUN``` instruction handles the cloning, submodule checkout, and building steps.  Last of all, to support end-use, the working directory is set to the root of the image's Blackbear repository.  To build an image using this Dockerfile, run the following.

```bash
docker build -t blackbear .
```

After this, like before, the tests can be ran with a single command.

```bash
docker run -ti blackbear ./run_tests
```
