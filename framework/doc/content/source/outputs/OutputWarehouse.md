# OutputWarehouse

The `OutputWarehouse` is a storage container for all [Output](syntax/Outputs/index.md)
objects.

Warehouses are used to store objects. Using warehouses is preferable in terms of encapsulation and code design
to storing everything in central simulation classes like the [FEProblem.md]. The `OutputWarehouse` does not
derive from the other warehouse classes, such as the `MooseObjectWarehouse`.

The `OutputWarehouse` inherits the
[SetupInterface.md] and forward the setup calls (such as `timestepSetup`) to the objects they hold. Similarly,
on a call to `outputStep` on the warehouse, all the objects the `OutputWarehouse` holds will also perform
`outputStep`.

In addition to keeping track of all `Outputs` objects, the `OutputWarehouse` also:

- keeps track of the `sync_times` at which output is forced
- handles the output to the console. The `ConsoleStream` that many objects use to output, uses the `OutputWarehouse`
  under the hood to output its messages.
