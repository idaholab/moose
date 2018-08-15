## Environment

Lets try to make our environment as sane as possible, while setting up all the locations we will need.

```bash
module purge   #(may fail with command not found)
unset LD_LIBRARY_PATH
unset CPLUS_INCLUDE_PATH
unset C_INCLUDE_PATH
export PACKAGES_DIR=/some/path/with/write/access
export STACK_SRC=`mktemp -d /tmp/moose_stack_src.XXXXXX`
umask 022
```

!alert note
What ever terminal window you were in, when you performed the above commands, you _MUST_ use that same window,
for the remainder of these instructions. If this window is closed, or the machine is rebooted, it will be
necessary to perform the above commands again, before continuing any step. You will also _need_ to perform any
exports in any previous steps you continued from.


Create the target installation location:

```bash
mkdir -p $PACKAGES_DIR
```
