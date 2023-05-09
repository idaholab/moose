# PerfGraphLivePrint

## Overview

The `PerfGraphLivePrint` (PGLP) object is responsible for printing information to the screen as a code is executing.  To perform this action PGLP is executing on a separate thread from the main thread.  This thread will be refered to as the "print thread".  The main thread keeps track of what is executing using the `TIME_SECTION` macro as part of [/PerfGraph.md].  PGLP then reads this information and decides whether or not to print information about what is currently happening.

By default, PGLP prints information any time a section takes longer than one second to execute.  It will print out the "live print message" at that point, then it will print a single `.` for every second until the section ends.  When the section ends, both the memory and the time will be printed. By default, the total memory used (by the simulation at this point) and the increment in time (e.g. the time for this section only) are printed.

The other reason that PGLP prints is for a large increase in memory (by default 100MB).  If, after a section executes, the memory usage increased by more than the limit, then PGLP will print the live print message and show the current total memory consumption as well as the time was used by the section.  The memory limit is configurable from the input file.

The PGLP primarily works by waking up every second and inspecting what the program has done / is currently doing, printing any necessary information and then going back to sleep.  It should be lightweight enough that it should not significantly disrupt the operation of the program.

## Objects/Data Structures Used By PerfGraphLivePrint

The PGLP utilizes several different objects and datastructures for reading the current state of the execution and for storing the current state of what's been printed.

### PerfGraphSectionInfo

The `PerfGraphRegistry` keeps track of which sections have been registered, including their unique id, short name, verbosity level, live message, and whether or not to print dots for that section.  This is all kept in a `PerfGraphSectionInfo` object that is part of the `PerfGraphRegistry`.

This information is heavily utilized by the PGLP to know what to print and when.  Due to the main thread registering this information and the print thread using it, the accessors surrounding `PerfGraphSectionInfo` have locks in them.  However, the `PerfGraph` executes on the main thread and therefore utilizes a protected member function on `PerfGraphRegistry` to access `PerfGraphSectionInfo` without a lock.

### PerfGraph::SectionIncrement

A `SectionIncrement` represents either the starting or ending state of a section.  `SectionIncrement`s are stored in an "execution list" (explained below) inside of `PerfGraph` as well as a `_print_thread_stack` in the PGLP.  The PGLP reads those in order to determine what the application is currently doing (and has done).

The `SectionIncrement` objects are stored in a `std::array` called `_execution_list` in the `PerfGraph`.  This means they are all allocated up-front.  When a timed section is started, the next available `SectionIncrement` in the execution list is filled with the current time, current memory, and an `IncrementState` of `STARTED`.  When the PGLP wakes up it inspects the execution list to determine what the program has done and is doing and whether or not it needs to print anything.  Note that there is zero memory allocation during the execution of the program.

### PerfGraph::_execution_list

The execution list is stored in the `PerfGraph` and keeps a running list of every timed section starting and stopping.  As mentioned above, it is a `std::array` of `SectionIncrement` objects that are statically allocated at the start of execution.  The execution list works as a "circular buffer"... when the end of the list is reached, filling continues by wrapping around to the beginning of the list.  To do this, a simple counter is kept for where the beginning and ending of the current execution list are.  The beginning and ending are kept in atomic variables so that they are threadsafe since they will be read and written to by both the main thread and the print thread.

The entire execution history is necessary because the PGLP only wakes up every second - and it needs to be able to see everything that transpired.  This allows it to know what has finished so that it can print finishing information and gives it the ability to keep track of what is currently executing by developing a print thread stack.

### PerfGraphLivePrint::_print_thread_stack

While `PerfGraph` keeps a stack, that stack is changing all the time during the execution of the code.  The PGLP needs to keep a stack of what is currently printed on the screen so that it knows how much to indent and how to "unfold" after sections end (i.e. how to print the "finished" statements for several sections that are nested).  This information is kept in the `PerfGraphLivePrint::_print_thread_stack`.

The print thread stack is developed by the PGLP by reading the execution list from the `PerfGraph` and "pushing" and "popping" `SectionInfo`.  An important detail is that the print thread stack must _copy_ from the execution list.  This is because the execution list is a circular array... meaning that old entries will get overwritten.  Things on the bottom of the stack could be there a long time (e.g. the entire execution of the program) - so to preserve that information it must be copied to the stack.

## Threading and Thread Safety

Threading is accomplished using C++ threading mechanisms.  The PGLP is started in the constructor of `PerfGraph` as a `std::thread`.  The destructor of `PerfGraph` tells the PGLP to finish and calls `join()` on it to delete it.

### Memory Synchronization

With the main thread writing to the execution list and the PGLP reading from it on its own thread, memory synchronization is critical.  In `PerfGraph:addToExecutionList` a `SectionIncrement` has its data filled in, then a `std::atomic_thread_fence()` is used to make sure the changes to that `SectionIncrement`'s data is "published" to all threads _before_ incrementing the end of the execution list.  That thread fence "synchronizes with" the a thread fence in `PerfGraphLivePrint::start()`.  This means that if the PGLP wakes up as the `SectionInfo` is being written to, that it won't ever access the data mid-change.  All of this is achieved without any locking on the main thread.

### Waiting and Waking Up

The main execution loop for PGLP is in `start()` and is a `while()` loop that is dependent on the the object-local `_currently_destructing` variable.  Details about `_currently_destructing` are below - but the gist is that the loop will continue until `PerfGraph` tells it not to.

PGLP waits on a section completing so that we can instantly print the finishing info for it.  To do this, we use a `std::condition_variable`.  A `condition_variable` can be used to wait for a notification from the main thread.  By using `wait_for()` it is possible to wait until either the specified amount of time has passed or the thread is signaled to wakeup.  This allows the PGLP to sleep for 1 second, or until the main thread tells it that a section is complete.

However, the actual workings of a `condition_variable` are complex.  There are actually three reasons why a `wait_for()` may wakeup:

1. The timeout is over
2. The notification is sent
3. A "spurious wakeup"

The third type of wakeup occurs when an OS wakes the thread up early, which can happen randomly.  This is ok with us, because it gives us a moment to check to see if sections have completed while the thread has been asleep.  To do that, there is a predicate functor passed to `wait_for` that does exactly this check.  In addition, it also checks to see if the `PerfGraph` is `_destructing`... which would mean the PGLP should stop waiting and print the final info.

I want to take a moment and talk about locking and `condition_variable`.  Usually, you want to use a lock to guard the evaluation of the predicate and the setting of variable values in the main thread.  In our case, we really don't want locking in the execution pathway of the main thread because it could slow down execution.  To get around this, the variables used in the predicate are atomic.  However, this isn't without its own flaws.  There is a tiny chance that the predicate can be evaluated when it is false, but then, in-between checking the predicate and waiting for the signal the main thread signals, then the PGLP starts looking for a signal.  In this case, the PGLP may wait one second more before finding out that there is work to be done.  This is incredibly rare, and is not worth fixing by having a lock on the main thread.

### Destructing

As mentioned above, the PGLP is looping until the `PerfGraph` signals that it is destructing.  The `_destructing` variable in `PerfGraph` is guarded by a mutex that is also utilized when checking the predicate of the condition variable mentioned above.  This is done because we don't want to accidentally miss the destructing signal and wait for 1 more second - delaying the ending of the program.  That could be hugely detrimental to a run that is running thousands of cases (like a stocahstic sampling or even just running tests).

One more small detail about destructing is that the value of it needs to be captured _before_ capturing the value of the end of the execution list.  This is to make sure that the final end of the execution list is available for the last run-through of the PGLP... so that everything can be completely printed.

## Printing

After making it past the condition variable mentioned above, the PGLP then needs to read the execution list, build the print stack, and decide if anythig should be printed.

The simplest case is that the execution is still in the "same place" - meaning that the execution list is has not been changed.  In that case, either the current sections live print message needs to be printed or dots need to be printed.

If the execution list is not in the same place, then it must be iterated through.  Note that the current end of the execution list is only captured one time, during the condition_variable predicate.  This same ending is utilized throughout the entire time the PGLP is executing for that iteration.  The main thread may be moving on past that point - but for an entire cycle of the PGLP the ending of the execution list is held constant.  This is required to make sure there are no race conditions.

The different reasons (ways) things are printed are:

1. Has been running for 1 second (print live message)
2. Is still running (print dots)
3. Was the current thing running that had already been printed and finished (print stats)
4. Is continuing to run after an internal scope was running and was printed (print "Still")
5. Finished running, hasn't been printed, but is over the memory limit (print "Finished")

In each case the entire "stack" is printed up to where the current section that needs to be printed is.  This is to give context to inner scopes and get proper indenting.
