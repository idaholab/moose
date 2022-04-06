# Debugging

At some point while developing a MOOSE-based application you will probably need to use a debugger.  A debugger will allow you to stop your program at certain points (or if things happen such as a memory segfault).  Once stopped you can then carefully step through the program, inspecting variable values as you go in order to find the source of the problem.

In particular, if you ever see a "Segfault" or a "Signal 11" that means it's time to pull out the debugger.  Any debugger will automatically stop once a segfault is reached, showing you exactly where the invalid memory access occurred.

For a good tutorial on debugging [see Example 21](/ex21_debugging.md optional=True)

## Debug Executable

The first step to debugging anything is to build a debug executable.  By default MOOSE-based applications are built in "optimized" (`opt`) mode.  That ensures the fastest solves.  However, an optimized executable is missing a lot of information that is useful to a debugger and the optimization process itself can cause code to get reordered (or even skipped!) making it difficult to step through a program.

To build an executable suitable for debugging you need to set the `METHOD` environment variable to `dbg`.  You can `export` it in your environment but it's usually simpler to use a UNIX shortcut that allows you to define environment variables at the same time you run a command, like so:

```bash
METHOD=dbg make -j 8
```

Always remember that the `8` should be modified to reflect the number of processors you want to use for the build (usually the number of cores in your computer).

Once the build is complete you should end up with a "debug executable" that look like: `yourapp-dbg`.  That executable is perfect for loading into a debugger.  However, that executable will run VERY slowly - so make sure that before you begin debugging you come up with a problem that is as small as possible but still shows the problem you're trying to fix.

## Debuggers

Many different debuggers exist: `lldb`, `gdb`, `ddd`, and Totalview are just a few.  While a command-line debugger (like `lldb` or `gdb`) might seem daunting at first, they are an invaluable tool for quick debugging and debugging in complicated scenarios such as when you're running on a cluster.  Learning one should be essential to any computational scientist.

For debugging MOOSE-based applications we recommend `lldb` if you're using the clang compiler (default on Mac OSX) and `gdb` for the `gcc` compiler (default on Linux).

### LLDB and GDB

`lldb` and `gdb` are very similar.  They work on the "command-line" taking text input and moving through your program as it executes.

With our MOOSE package on Mac OSX you actually need to run lldb using `sudo` so it has the elevated privileges it needs to attach to your program.  To invoke `lldb` with a MOOSE-based application on Mac OSX you would do:

```bash
sudo lldb -- ./yourapp-dbg -i inputfile.i
```

The `--` tells lldb that any command-line options after that point need to be passed to the executable you are running.  `sudo` will ask you for your password so that it can elevate the priveleges of `lldb`.  You can also look below to see how to allow `lldb` to run with sudo without using a password.

`gdb` can be run with a similar command:

```bash
gdb --args ./yourapp-dbg -i inputfile.i
```

(On Linux this will generally work, without the need for `sudo`)

Once this is done, your executable will be loaded but won't start running.  This is an opportune time to set breakpoints.  We usually recommmend setting a breakpoint on `MPI_Abort` using the `b` command:

```
b MPI_Abort
```

Then to start running your executable use the `r` command (type `r` then hit enter).

If a breakpoint (or fault) is reached: I recommend first using the `bt` command to output a "back-trace" so you can see exactly what the current call-stack looks like to figure out where you are.

To quit the debugger use `Ctrl+d` (by the way: that's a normal way to quit lots of command-line interpreters on Unix - including Python).

Another useful command is `p` (for 'print') which allows you to print out the value of a variable.  Just do:

```
p variablename
```

You can learn about the full set of commands by using `help` or looking at any number of tutorials online.

## Parallel Debugging

Firstly, if you don't have to debug in parallel *DON'T*!  Only do parallel debugging when you have a problem that can only be reproduced when running in parallel.  If your problem will show up in serial, it is MUCH easier to debug in serial.  If it takes a long time for your problem to run so you are wanting to run it in parallel: don't do that... instead, try to make your problem smaller so that you can debug it in serial.

With all of that said: if you actually do need to debug in parallel, MOOSE has a couple of command-line arguments to help.  However, before we get there, we need to do a bit of setup on Mac OSX:

### Mac OSX Parallel Debugging Setup

As noted above, when running `lldb` on Mac OSX we need to run with `sudo` to give it permission to attach to our running program.  `sudo` will ask for your password, but unfortunately when doing parallel debugging there is no way to enter that password.  Therefore, we need to make it so that you can run `lldb` with `sudo` without a password.

First thing is to get the full path to where `lldb` is using the command-line:

```bash
which lldb
```

If you are using our package on OSX this should return something like `/opt/moose/llvm-5.0.1/bin/lldb`.  Make note of this location (copy it, or set that Terminal aside) because you'll need it in the next step.

To set `lldb` to be able to run with `sudo` without a password we need to modify the `sudoers` file.  To do that issue this command in a terminal (preferably a new one so you can still see the path to `lldb` in the first one):

```bash
sudo visudo
```

Most-likely this is going to use `vi` (a text editor) to open the `sudoers` file (I say "most-likely" because it technically can open any editor based on the `EDITOR` environment variable).  If you are unfamiliar with `vi` don't worry.  Just press `i` to go into "insert" mode.  Navigate to the bottom of the file and add a line that looks like:

```
username ALL= NOPASSWD: /opt/moose/llvm-5.0.1/bin/lldb
```

Where `username` MUST be replaced with *your* username!  And the path to `lldb` needs to reflect what you got back from `which lldb` above.  Once that line is in place press `Esc` (to exit "insert mode") then type `:wq` (that's a `colon` then `w` then `q` - it's a command that says "write and quit") and press `Enter`.

Once you've completed that you should be able to run `sudo lldb` on the command-line and not need to enter your password.  If you still need to enter your password, create a new discussion on our [forum](https://github.com/idaholab/moose/discussions) so we can figure out what's wrong before you go further.

### Actually Parallel Debugging

With all of that setup we are ready to debug in parallel.  There are two options:

#### 1. Launch a terminal for each MPI process

What we're going to do is launch a terminal (using `xterm`) for each MPI process.  That terminal will run our debugger and attach to the running MPI process.  For this to work, you either need to be on your local box or have X-forwarding set up over SSH (which I'm not going to go into here).

Let's assume that you're working on your local Mac workstation using our package.  In order to launch your program with 4 MPI processes and 4 terminals for debugging you would do:

```bash
mpiexec -n 4 ./yourapp-dbg -i inputfile.i --start-in-debugger='sudo lldb'
```

(If you are on Linux - most-likely you will want to put `gdb` where `sudo lldb` is)

If everything is setup correctly you should see 4 XTerm windows show up with `lldb` command-line prompts.  Those debugger prompts are already attached to your running executable, but the executable is paused.  This is an opportune time to set breakpoints, but you have to do it in each terminal seperately.  For instance, you might want to go through each one and do:

```
b MPI_Abort
```

to be able to stop if MOOSE encounters an error.

Once you are ready to *continue* - you do just that.  Use the `c` command (type `c` and hit `Enter`) in each terminal window to tell it to continue.  Once you go through all of the open terminal windows you should see your application start to run in your original terminal.  If a breakpoint (or any fault) is reached on any process that terminal window will show the command-prompt again, allowing you to inspect variables, etc.

##### Passing debugger commands on the command line

`lldb` accepts debugger commands through the `-o` command line option that are executed as soon as the execuatble is loaded up and ready. This can be used to set breakpoints and immediately resume the execution of the app.

```bash
mpirun -n 4 ./yourapp-dbg -i inputfile.i --start-in-debugger "sudo lldb -o 'break set -E C++' -o cont"
```

The above command will set breakpoints that halt on thrown exceptions. Replace `break set -E C++` with `b MPI_Abort` to break on MOOSE errors. The `-o cont` option will automatically run the app after the breakpoints are set.

#### 2. Launch your application and tell it to wait so you can manually attach a debugger

This is going to be used in cases where you need to debug using LOTS of MPI processes, but you don't want a terminal window for each one.  This is also handy if you're working on a cluster that doesn't have any way of doing X-forwarding for option #1 to work.  The idea is to launch your application and have it wait during the initialization phase so you have time to attach a debugger manually to one of the processes.

To do this simply run your application like so:

```bash
mpiexec -n 4 ./yourapp-dbg -i inputfile.i --stop-for-debugger
```

This will cause your application to print something like the following and then wait 30 seconds (by default - if you need more time use `--stop-for-debugger=75` where `75` is the number of seconds you want it to wait):

```
> mpiexec -n 4 ../../../moose_test-opt -i simple_diffusion.i --stop-for-debugger=2

Stopping for 2 seconds to allow attachment from a debugger.

All of the processes you can connect to:
rank - hostname - pid
0 - dereksmacpro.local - 53403
1 - dereksmacpro.local - 53404
2 - dereksmacpro.local - 53405
3 - dereksmacpro.local - 53406

Waiting...
```

The message there is telling you where each process is running and what its "process ID" (`pid`) is.  That is the relevant information you need to be able to attach a debugger to that process.  In this case (on my local Mac) if I want to connect to "rank 2", in a separate Terminal I would do:

```
sudo lldb -p 53405
```

(`gdb` has a similar mechanism, see its docs)

That will launch `lldb` and attach to my running program.  Attaching to the program "pauses" it - allowing me to set breakpoints and then use the `c` command to tell it to continue.
