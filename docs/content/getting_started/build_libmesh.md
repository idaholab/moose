---
## Compile libMesh
* MOOSE directly relies on the libMesh finite-element framework. Because of this strong tie MOOSE contains a particular version of libMesh that we have vetted for our users. We will periodically update that version (about once a month) so stay tuned to the MOOSE-users mailing list for those announcements. To pull down and compile this version of libMesh you simply need to run a script in MOOSE:

```bash
cd ~/projects/moose
scripts/update_and_rebuild_libmesh.sh
```

!!! Important
     Do not use `sudo` when running update_and_rebuild_libmesh.sh.

---
## Test It!
* All that's left is to make sure that everything is working properly...

```bash
cd ~/projects/moose/test
make -j 4
./run_tests -j 4
```

* If everything is good then all of the tests will pass.

!!! note
    Some will be skipped depending on your environment
    