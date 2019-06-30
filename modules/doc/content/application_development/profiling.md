# Profiling MOOSE code

## MacOS

Follow the steps below to get profiling information for your application:

- Download the full Xcode distribution (iprofiler is not included with the Xcode command line tools distribution).
- Compile MOOSE and your application in `oprof` mode.
- Run your application through the profiler:

### Newer versions of MacOS (Mojave):

```
instruments -t Time\ Profiler ./mooseapp-oprof -i input.i
```

This will create a directory `instrumentscli[0-9]+.trace`, where `[0-9]+`
denotes a number, which you can open using

```
open instrumentscli[0-9]+.trace
```

The Instruments application will open in a new window with the profile.

### Older versions of MacOS (Sierra)

```
iprofiler -timeprofiler -T 10m ./mooseapp-oprof -i input.i
```

This will create a directory `mooseapp-oprof.dtps` which you can open using

```
open mooseapp-oprof.dtps
```

The Instruments application will open in a new window with the profile.

## Linux

Profiling on Linux can be done using [gperftools](https://github.com/gperftools/gperftools).
