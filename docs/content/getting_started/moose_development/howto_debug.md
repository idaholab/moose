# Using GDB (Executables built with GCC)
```
gdb --args <your program> <your program args>
e.g.
gdb --args ../../../moosetest-dbg -i 2d_diffusion_test.i
```

| Command | Description |
| ------------- | -------------|
| help | get some help |
| r | run |
| bt | back trace (stack trace) |
| n | next |
| s | step |
| b <function name\> | set breakpoint |
| c | continue |
| b | breakpoint |
| advance <line number\> | run until the execution hits the <line number\> |
| p <variable\> | prints a value of <variable\> |
| call <function\> | calls a function and prints its return value |
| info breakpoints | shows the breakpoints |
| disable <n\> | disable a breakpoint |

A comprehensive guide can be found at
[http://www.yolinux.com/TUTORIALS/GDB-Commands.html](http://www.yolinux.com/TUTORIALS/GDB-Commands.html)

## Viewing the contents of STL Containers ##
First download the file [dbinit_stl_views-1.03.txt](http://www.yolinux.com/TUTORIALS/src/dbinit_stl_views-1.03.txt) and save it as ~/.gdbinit

|Data type|	        |GDB command|
| --- | --- |
|std::vector<T\>	||pvector stl_variable
|std::list<T\>	        ||plist stl_variable T
|std::map<T,T\>	||pmap stl_variable
|std::multimap<T,T\>||pmap stl_variable
|std::set<T\>	         ||pset stl_variable T
|std::multiset<T\>	||pset stl_variable
|std::deque<T\>	||pdequeue stl_variable
|std::stack<T\>	||pstack stl_variable
|std::queue<T\>	||pqueue stl_variable
|std::priority_queue<T\>	||ppqueue stl_variable
|std::bitset<n\>	||pbitset stl_variable
|std::string	||pstring stl_variable
|std::widestring	||pwstring stl_variable

Now when you are debugging your program and need to view an STL container you simply type the desired function followed by the variable name.
```
(gdb) pvector my_vector
```

Note: When using one of the printer functions that requires you supply a type (pmap, pset, plist, etc...), and one or more of those types is std::string, things get a little trickier.  You need to specify the full string type which usually looks like this:

```
(gdb) pmap my_map int 'std::basic_string<char, std::char_traits<char>, std::allocator<char> >'
```

There are a few things to pay attention to in the above example:
 * Make sure you have single quotes around the full string type, double quotes do not work
 * When using a map type you will need to supply the both the "left" and "right" types before it will print anything


# Using LLDB (Executables built with Clang)
```
lldb -- <your program> <your program args>
e.g.
lldb -- ../../../moosetest-dbg -i 2d_diffusion_test.i
```
Most of the commands from gdb work in lldb too

# Getting Tracefiles from libMesh in parallel
Run Libmesh configure with ```--enable-tracefiles```. This option can be passed to the ```update_and_rebuild_libmesh``` script when it is run:
```bash
./update_and_rebuild_libmesh.sh --enable-tracefiles
```

## Using LLDB when using ccache
You might find it difficult to set breakpoints when debugging a binary compiled with ccache. Fortunately there is a simple fix for this problem:

```bash
echo "settings set target.inline-breakpoint-strategy always" >> ~/.lldbinit
```

See this link for the discussion: [http://lldb.llvm.org/troubleshooting.html](http://lldb.llvm.org/troubleshooting.html)


$ cd <MOOSE DIR>
$ scripts/update_and_rebuild_libmesh.sh --enable-tracefiles
```
When your program hits a mooseError() it will dump tracefiles on all processors to your disk.

Note: On OS X, there are a few more steps you will need to take in order to enable
```
$ ulimit -c unlimited
$ sudo mkdir /cores
$ sudo chown root:admin /cores
$ sudo chmod 1775 /cores
```
