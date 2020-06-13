## Environment

Lets try to make our environment as sane as possible, while setting up all the locations we will need.

```bash
bash --noprofile
export PACKAGES_DIR=/some/path/with/write/access
export STACK_SRC=`mktemp -d /tmp/moose_stack_src.XXXXXX`
umask 022
mkdir -p $PACKAGES_DIR
```

!alert warning title=Proceed while using same terminal
What ever terminal you were in, while you performed the above commands, you *MUST* remain in that terminal, for the remainder of the instructions. If this terminal is closed, it will be necessary to *START OVER*.

