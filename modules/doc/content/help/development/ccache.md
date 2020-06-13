# Reducing compile times with ccache

[ccache](http://ccache.samba.org/) is a compiler cache that speeds up recompilation by caching previous compilations and detecting when the same compilation is being done again. ccache can deliver significant speedups when developing MOOSE-based applications, or working on the framework itself.

## Usage

In order to use ccache with MOOSE-based applications, it will be necessary to first build libMesh using ccache. This is due to the fact that each MOOSE-based application will ask libMesh how it was built, and in turn, use the same flags to build itself.

In other words If you are using our Conda moose-libmesh package, then you cannot use ccache. In order to begin using ccache, first create a new conda environment, free of moose-libmesh, and install everything necessary to build libMesh with ccache:

```bash
conda create --name ccache-env moose-petsc moose-tools
conda activate ccache-env
export CC="ccache mpicc" CXX="ccache mpicxx"
```

Next, build libMesh:

```bash
cd moose
scripts/update_and_rebuild_libmesh.sh
```

!alert note
Because ccache is a conda-forge package as opposed to one controlled by the MOOSE development team, it will be necessary for you set your CC and CXX variables manually, each time you `conda activate ccache-env`.

## Using ccache

All compile commands will now execute ccache through the CC/CXX aliases the ccache module added. ccache will automagically call the appropriate compiler when necessary. [user-ext] lists some useful commands.

!table id=user-ext caption=List of useful ccache commands
| Command | Description |
| :- | :- |
| `ccache -s` | Statistics on cache size, hits, and misses |
| `ccache -z` | Reset statistics |
| `ccache -C` | Clear compile cache |
| `ccache -M 10G` | Set size of the cache (e.g. 10 Gb) |


## Using LLDB when using ccache

You might find it difficult to set breakpoints when debugging a binary compiled with ccache. Fortunately there is a simple fix for this problem:

```bash
echo "settings set target.inline-breakpoint-strategy always" >> ~/.lldbinit
```

[See this for further discussion](http://lldb.llvm.org/use/troubleshooting.html)

## Backup considerations

If you are using a backup system on your computer you will most likely want to exclude or skip your ccache cache. The binaries in this folder will change very frequently causing you to run out of disk space on your backups very quickly. If you are using OS X TimeMachine, simply add `~/.ccache` to the list of excluded folders.
