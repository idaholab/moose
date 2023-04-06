## Compile libMesh

MOOSE directly relies on the [libMesh] finite-element framework. Because
of this strong tie MOOSE contains a particular version of libMesh that we have vetted for our
users. To pull down and compile this version of libMesh you simply need to run a script in MOOSE:

```bash
export MOOSE_JOBS=6 METHODS=opt
cd ~/projects/moose
./scripts/update_and_rebuild_libmesh.sh
```

!alert! tip
`MOOSE_JOBS` is a loose influential environment variable that dictates how many cores to use when
executing many of our scripts.

`METHODS` is an influential environment variable that dictates how to
build libMesh. If this variable is not set, libMesh will by default build 4 methods (taking 4x
longer to finish).
!alert-end!
