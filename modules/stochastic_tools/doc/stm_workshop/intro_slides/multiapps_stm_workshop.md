# Execution Using MultiApps

!style halign=center
[https://mooseframework.inl.gov/source/multiapps/SamplerFullSolveMultiApp.html](https://mooseframework.inl.gov/source/multiapps/SamplerFullSolveMultiApp.html)

!---

# Parallelism of Sampling Matrix

- Every row of the sampling matrix is assigned one or more processors.
- The processors(s) assigned to a row are responsible for evaluating that sample.
- Here are some examples:

!style! fontsize=80%
!row!
!col! width=33%

!style halign=center
5 Rows and 3 Processors

!equation
\mathbf{X} = \begin{bmatrix}
x_{1,1} & \dots & x_{1,M} \\
x_{2,1} & \dots & x_{2,M} \\
x_{3,1} & \dots & x_{3,M} \\
x_{4,1} & \dots & x_{4,M} \\
x_{5,1} & \dots & x_{5,M}
\end{bmatrix}
\begin{array}{l}
\rightarrow \text{CPU 0} \\
\rightarrow \text{CPU 0} \\
\rightarrow \text{CPU 1} \\
\rightarrow \text{CPU 1} \\
\rightarrow \text{CPU 2} \\
\end{array}

!col-end!

!col! width=33%

!style halign=center
5 Rows and 8 Processors

!equation
\mathbf{X} = \begin{bmatrix}
x_{1,1} & \dots & x_{1,M} \\
x_{2,1} & \dots & x_{2,M} \\
x_{3,1} & \dots & x_{3,M} \\
x_{4,1} & \dots & x_{4,M} \\
x_{5,1} & \dots & x_{5,M}
\end{bmatrix}
\begin{array}{l}
\rightarrow \text{CPU 0/1} \\
\rightarrow \text{CPU 2/3} \\
\rightarrow \text{CPU 4/5} \\
\rightarrow \text{CPU 6} \\
\rightarrow \text{CPU 7} \\
\end{array}

!col-end!

!col! width=33%

!style halign=center
5 Rows and 8 Processors (ensuring every row has 2 processors)


!equation
\mathbf{X} = \begin{bmatrix}
x_{1,1} & \dots & x_{1,M} \\
x_{2,1} & \dots & x_{2,M} \\
x_{3,1} & \dots & x_{3,M} \\
x_{4,1} & \dots & x_{4,M} \\
x_{5,1} & \dots & x_{5,M}
\end{bmatrix}
\begin{array}{l}
\rightarrow \text{CPU 0/1} \\
\rightarrow \text{CPU 0/1} \\
\rightarrow \text{CPU 2/3} \\
\rightarrow \text{CPU 4/5} \\
\rightarrow \text{CPU 6/7} \\
\end{array}

!col-end!

!row-end!
!style-end!

- This matrix partitioning all happens within the `Sampler` and `MultiApp` objects based on `num_rows` and `mpiexec -n <nprocs>`

!---

# Sampling Using MultiApps

!row!
!col! width=60%
- Create/run sub-applications for each sampler row.
- `SamplerFullSolveMultiApp` fully evaluates each row before transferring data.

  - The +vast majority+ of applications will use `SamplerFullSolveMultiApp`

- `SamplerTransientMultiApp` progresses each sample in time and transfers data at each step.

- `min_procs_per_app = <n>` parameter ensures that at least `n` processor are used for each sample

  - Use for large problems where memory is an issue and problem already scales well
!col-end!

!col! width=40%
```
[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
    mode = batch-reset
  []
[]
```

```
[MultiApps]
  [runner]
    type = SamplerTransientMultiApp
    sampler = mc
    input_files = 'sub.i'
    mode = batch-restore
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  end_time = 5
[]
```
!col-end!
!row-end!

!---

# MultiApp Mode

1. +normal+: One sub-application is created for each row of data supplied by the Sampler object.
1. +batch-reset+: One sub-application is created for each processor, this sub-application is
                  destroyed and re-created for each row of data supplied by the Sampler object.
1. +batch-restore+: One sub-application is created for each processor, this sub-application is
                    backed up after initialization. Then for each row of data supplied by the
                    Sampler object the sub-application is restored to the initial state prior to
                    execution.

!row!
!col! width=50%
### Normal Mode


!equation
\mathbf{X} = \begin{bmatrix}
x_{1,1} & \dots & x_{1,M} \\
x_{2,1} & \dots & x_{2,M} \\
x_{3,1} & \dots & x_{3,M} \\
x_{4,1} & \dots & x_{4,M} \\
x_{5,1} & \dots & x_{5,M}
\end{bmatrix}
\begin{array}{lll}
\rightarrow & \text{CPU 0} & \text{Sub-app 0} \\
\rightarrow & \text{CPU 0} & \text{Sub-app 1} \\
\rightarrow & \text{CPU 1} & \text{Sub-app 2} \\
\rightarrow & \text{CPU 1} & \text{Sub-app 3} \\
\rightarrow & \text{CPU 2} & \text{Sub-app 4} \\
\end{array}

!col-end!

!col! width=50%
### Batch Mode

!equation
\mathbf{X} = \begin{bmatrix}
x_{1,1} & \dots & x_{1,M} \\
x_{2,1} & \dots & x_{2,M} \\
x_{3,1} & \dots & x_{3,M} \\
x_{4,1} & \dots & x_{4,M} \\
x_{5,1} & \dots & x_{5,M}
\end{bmatrix}
\begin{array}{lll}
\rightarrow & \text{CPU 0} & \text{Sub-app 0} \\
\rightarrow & \text{CPU 0} & \text{Sub-app 0} \\
\rightarrow & \text{CPU 1} & \text{Sub-app 1} \\
\rightarrow & \text{CPU 1} & \text{Sub-app 1} \\
\rightarrow & \text{CPU 2} & \text{Sub-app 2} \\
\end{array}

!col-end!
!row-end!

!---

### MultiApp Mode (cont.)

!style! fontsize=75%
!row!
!col! width=65%
- +normal+ mode

  - Traditional method of running multiple sub-applications (`positions`).
  - Very memory intensive and not recommended for production use.
!col-end!

!col! width=5%
!!
!col-end!

!col! width=30%
```
[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    ...
    mode = normal
  []
[]
```
!col-end!
!row-end!

!row!
!col! width=65%
- +batch-reset+ mode

  - Recommended mode for general problem types and requires the least intrusion.
  - Not memory intensive, but not the most efficient by run-time.
!col-end!

!col! width=5%
!!
!col-end!

!col! width=30%
```
[MultiApps]
  [runner]
    ...
    mode = batch-reset
  []
[]
```
!col-end!
!row-end!

!row!
!col! width=65%
- +batch-restore+ mode

  - Works if parameters are controllable.
!col-end!

!col! width=5%
!!
!col-end!

!col! width=30%
```
[MultiApps]
  [runner]
    ...
    mode = batch-restore
  []
[]
```
!col-end!
!row-end!

!row!
!col! width=65%
- +batch-restore+ mode with re-used solution

  - Works for steady-state and pseudo-transient problems.
  - Keeps solution around for next sample that is usually a better initial guess for the solve
!col-end!

!col! width=5%
!!
!col-end!

!col! width=30%
```
[MultiApps]
  [runner]
    ...
    mode = batch-restore
    keep_solution_during_restore = true
  []
[]
```
!col-end!
!row-end!

!row!
!col! width=65%
- +batch-restore+ mode with no restore

  - Most efficient method, but only works for steady-state problems.
  - Skips all restoring operations and just calls `solve()` again.
!col-end!

!col! width=5%
!!
!col-end!

!col! width=30%
```
[MultiApps]
  [runner]
    ...
    mode = batch-restore
    no_backup_and_restore = true
  []
[]
```
!col-end!
!row-end!
!style-end!

!---

# MultiApp Mode (cont.)

!media full_solve_memory_mpi.svg style=width:50%;margin-left:auto;margin-right:auto;

!media full_solve_memory_mpi_time.svg style=width:50%;margin-left:auto;margin-right:auto;

!---

# Transferring Sampler Quantities

!style! fontsize=90%
!row!
!col! width=48%
Using command-line syntax (+normal+ and +batch-reset+ modes)

```
[Samplers]
  [sample]
    ...
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    input = sub.i
    sampler = sample
    mode = batch-reset
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = runner
    sampler = sample
    param_names = 'Mesh/xmax Mesh/ymax'
  []
[]
```
!col-end!

!col! width=4%
!!
!col-end!

!col! width=48%
Using controllable parameters (+batch-restore+ modes)

```
[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    input = sub.i
    sampler = sample
    mode = batch-restore
  []
[]

[Transfers]
  [params]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = sample
    parameters = 'Mesh/xmax Mesh/ymax'
    to_control = stochastic
  []
[]
```

In `sub.i`:

```
[Controls]
  [stochastic]
    type = SamplerReceiver
  []
[]
```
!col-end!
!row-end!
!style-end!

!---

# Quantities of Interest (QoIs)

- First we define a QoIs using `Postprocessors`, `VectorPostprocessors`, or `Reporters`:

```
[Postprocessors]
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
[]

[VectorPostprocessors]
  [u_line]
    type = LineValueSampler
    variable = u
    start_point = '0 0.5 0'
    end_point = '1 0.5 0'
    num_points = 11
  []
[]
```

- STM is able to process scalar values (postprocessors), vector values (vector-postprocessors), and/or matrix values.

  - This is easily extendable as the need arises.

!---

# Transferring QoIs

!row!
!col! width=70%
!style! fontsize=85%
- `SamplerReporterTransfer` can gather all the different types of values into a single object

  - Postprocessors have the syntax `<pp_name>/value`
  - VectorPostprocessors have the syntax `<vpp_name>/<vector_name>`
  - Reporters have the syntax `<reporter_name>/<value_name>`

- Unlike other transfers, the data does not need to be declared explicitly on the main application.

  - A `StochasticReporter` object just needs to exist.
  - The transfer will declare values programmatically into the `stochastic_reporter` object.

- The resulting value will be a vector of with the length of the number of sampler rows

  - A vector of scalars for postprocessors
  - A vector of vectors for vector-postprocessors

- `parallel_type = ROOT` gathers all the data accross processors so that there is a single file on output.
!style-end!
!col-end!

!col! width=30%
```
[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = sample
    from_reporters = 'avg_u/value u_line/u'
    stochastic_reporter = storage
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    parallel_type = ROOT
  []
[]

[Outputs]
  csv = true
  json = true
[]
```
!col-end!
!row-end!

!---

# QoI Output

- The resulting data will have the name `<stochastic_reporter_name>/<transfer_name>:<object_name>:<value_name>`

- The `converged` vector tells you whether or not the sample's sub-application was able to solve

- Vector QoIs are not able to be outputted to CSV

!row!
!col! width=45%

`main_out.json`:

!listing! language=json max-height=200px
"storage": {
    "data:avg_u:value": [
        290.0493845242595,
        248.6050947481139,
        ...
    ],
    "data:u_line:u": [
        [
            290.0493845242595,
            283.2398742039828,
            ...
        ],
        [
            248.6050947481139,
            245.2938472398621,
            ...
        ],
        ...
    ],
    "data:converged": [
        true,
        true,
        ...
    ]
}
!listing-end!

!col-end!

!col! width=10%
!!
!col-end!

!col! width=45%

`main_out_storage_0001.csv`:

```
data:avg_u:value,data:converged
290.0493845242595,True
248.6050947481139,True
...
```
!col-end!
!row-end!

!---

# Dealing with Failed Solves

!row!
!col! width=60%
- It may happen that the sub-application is not able to solve with a certain set of parameters.
- Having `ignore_solve_not_converge = true` will ignore the error caused when a sample's sub-application fails to solve.
- Transient problems must also have `error_on_dtmin = false` in the sub-application executioner.
- Transfers will send whatever the last computed value of the QoI is.
!col-end!

!col! width=40%
```
[MultiApps]
  [runner]
    ...
    ignore_solve_not_converge = true
  []
[]
```

```
[Executioner]
  type = Transient
  ...
  error_on_dtmin = false
[]
```
!col-end!
!row-end!
