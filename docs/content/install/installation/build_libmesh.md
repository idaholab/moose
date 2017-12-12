## Compile libMesh
MOOSE directly relies on the [libMesh](http://libmesh.github.io/) finite-element framework. Because of this strong tie MOOSE contains a particular version of libMesh that we have vetted for our users. To pull down and compile this version of libMesh you simply need to run a script in MOOSE:

```bash
cd ~/projects/moose
./scripts/update_and_rebuild_libmesh.sh
```

!!! warning
     Do not use `sudo` when running update_and_rebuild_libmesh.sh.
