# Reducing compile times with ccache

[ccache](http://ccache.samba.org/) is a compiler cache that speeds up recompilation by caching previous compilations and detecting when the same compilation is being done again. ccache can deliver significant speedups when developing MOOSE-based applications, or working on the framework itself.

## Usage

When using one of our [redistributable packages](getting_started/index.md), one must simply load an additional module to utilize ccache:

```bash
module load ccache
```

After loading this module you'll need to recompile libMesh using our `moose/scripts/update_and_rebuild_libmesh.sh` script. This script will detect the presence of ccache and append the appropriate configure arguments for you.  

Using the ccache module is a "*once you use it, you always have to use it*" type of module. If you forget to load this module after already having used it to build libMesh, you will receive `ccache: command not found` failures when attempting to build MOOSE and MOOSE-based apps.  

Our advice is to add this module to the list of modules that automatically get loaded when ever you open a new terminal. In order to do this add the `module load ccache` command to the end of your bash profile like so:

```bash
# Source MOOSE profile
if [ -f /opt/moose/environments/moose_profile ]; then
       . /opt/moose/environments/moose_profile
fi
module load ccache
```

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
