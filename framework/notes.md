
* Be able to sort objects for a specific executeflag using dependencyresolver

* Put pre-ic, pre/post-aux attributes in database so we can select on them directly - this data is
  not a part of objects and must be added later, so we need the ability to update objects that are
  already in the database with manually-specified attributes (i.e. not read off the object).

* Canonical way to do (arbitrary) operations on  _enabled_ objects only - skipping not enabled objects. - this
  is for things like residualSetup, jacobianSetup, initialize, finalize, execute, etc.

Questions:
- are there ever user objects that only exist on certain threads (i.e. not on all threads)?
- Are there duplicate instances of objects for each thread (I think so)?
- Currently the warehouse doesn't know about threading-copy relationships between objects.  We
    need to store this information somehow so we can query objects as a set (maybe?).  Actually
    we need some sort of ThreadedInterface that keeps a pointer to the master thread object
    pointer for joining.

- How to deal with partial queries but still be able to cache them?
    - pre-build queries that will be needed by compute loop objects (i.e. objects of interest
        grouped by thread+subdomain and thread+boundary combos) and pass in a data structure
        containing the prebuilt query ids that the compute loops just execute.

- Keeping track of query ids is a huge pain.  Is there a better way to track+cache queries?
    - macro that caches on the warehouse by a file and line number string of the call location?
        But this doesn't handle thread locking.

- add a way to "lock" the warehouse as immutable (except the cache) to prevent new objects from
    being added (but still allow updating existing objects.
