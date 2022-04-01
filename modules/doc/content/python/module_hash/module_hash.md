# Module Hash Tool

The `module_hash.py` tool provides a method for suggesting a unique naming suffix for our contribution modules within our continuous integration environment.

The idea is to utilize git blob/commit hashes on specific items involved with updating or modifying any contribution library. Dependencies are taking into consideration; a change to PETSc requires a change to libMesh and therefore a new hash is generated for libMesh as well as PETSc.

The configuration file responsible for tracking changes is in yaml format:

```yaml
packages:
  petsc:
    - petsc
    - conda/petsc/meta.yaml
    - scripts/configure_petsc.sh
    - scripts/update_and_rebuild_petsc.sh
  libmesh:
    - libmesh
    - conda/libmesh/meta.yaml
    - scripts/configure_libmesh.sh
    - scripts/update_and_rebuild_libmesh.sh
zip_keys:
  - petsc
  - libmesh
```

`zip_keys` is present in order to produce an ordered list (a dependency chain) `['petsc', 'libmesh']`. As configured here, libMesh is set to depend on PETSc.

`packages` contain each contribution, and each contribution contains items that will be used to ultimately generate a hash using `git ls-tree`:

```bash
git ls-tree HEAD MOOSE_DIR/petsc
git ls-tree HEAD MOOSE_DIR/conda/petsc/meta.yaml
...etc
```

In order to support history, the module hash tool will perform a `git show <COMMIT>:module_hash.yaml` to understand any additions/subtractions that were being tracked at that time. That same `<COMMIT>` will then be used during `git ls-tree COMMIT` operations.

If the provided `<COMMIT>` does not produce valid results (module_hash.yaml does not exist at that time in history), then `arbitrary` will be produced, as the tool will not understand what to track.

### Syntax

```bash
❯ ./module_hash.py -h
usage: module_hash.py [-h] [-q] [-i] library [commit]

Supplies a hash for a given library

positional arguments:
  library            choose from: petsc, libmesh
  commit             default HEAD

optional arguments:
  -h, --help         show this help message and exit
  -q, --quiet        Do not print warnings
  -i, --influential  List influential files involved with hash generation then exit
```

### Examples

Basic usage, defaulting to HEAD:

```bash
❯ ./module_hash.py petsc
8328006

❯ ./module_hash.py libmesh
ae2fdf1
```

Example arbitrary return:

```bash
❯ ./module_hash.py libmesh abc123
warning: commit abc123 does not contain module_hash.yaml
arbitrary

❯ ./module_hash.py libmesh abc123 --quiet
arbitrary
```

Relative history support:

```bash
❯ ./module_hash.py libmesh HEAD~1
ae2fdf1

❯ ./module_hash.py libmesh HEAD~2
warning: commit HEAD~2 does not contain module_hash.yaml
arbitrary
```

Demonstrating dependency tracking. Because the module hash tool uses `git ls-tree`, the change must be committed to take effect.

```bash
❯ echo "" >> ../conda/petsc/meta.yaml
❯ git add ../conda/petsc/meta.yaml
❯ git commit -m "a change to petsc"
❯ ./module_hash.py libmesh
bf57cd8
```
