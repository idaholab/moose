# PerfGraphOutput

!syntax description /Outputs/PerfGraphOutput

## Description

The [/PerfGraph.md] object holds timing data for MOOSE.  With this object you can control when it's contents get printed to the screen and how detailed the data is.  The easiest way to print out the `PerfGraph` is simply to add `perf_graph = true` to your `[Outputs]` block like so:

```
[Outputs]
  perf_graph = true
[]
```

This will cause a simple, short printing of the `PerfGraph`.  For more detailed printing you'll want to create a sub-block in `[Outputs]` like so:

```
[Outputs]
  [pgraph]
    type = PerfGraphOutput
    execute_on = 'initial final'  # Default is "final"
    level = 2                     # Default is 1
    heaviest_branch = true        # Default is false
    heaviest_sections = 7         # Default is 0
  []
[]
```

Controlling when it gets printed is achieved by setting `execute_on`.  By default it's set to `final` which causes the information to be printed at the end of the simulation.

Controlling the detail is done by setting `level`.  The default level is `1` which print fairly basic performance information.  Setting the number higher causes more detail to come out.

Example with `level = 1`:

```
-------------------------------------------------------------------------------------------------------------
|                 Section                |   Self(s)  |    %   | Children(s) |    %   |  Total(s)  |    %   |
-------------------------------------------------------------------------------------------------------------
| App                                    |      0.004 |   1.95 |       0.207 |  98.05 |      0.212 | 100.00 |
|   FEProblem::computeUserObjects        |      0.000 |   0.07 |       0.000 |   0.00 |      0.000 |   0.07 |
|   FEProblem::solve                     |      0.014 |   6.59 |       0.119 |  56.44 |      0.133 |  63.03 |
|     FEProblem::computeResidualInternal |      0.000 |   0.01 |       0.079 |  37.45 |      0.079 |  37.45 |
|     FEProblem::computeJacobianInternal |      0.000 |   0.01 |       0.040 |  18.83 |      0.040 |  18.84 |
|     Console::outputStep                |      0.000 |   0.12 |       0.000 |   0.00 |      0.000 |   0.12 |
|   FEProblem::outputStep                |      0.000 |   0.04 |       0.001 |   0.42 |      0.001 |   0.46 |
|     PerfGraphOutput::outputStep        |      0.000 |   0.00 |       0.000 |   0.00 |      0.000 |   0.00 |
|     Console::outputStep                |      0.001 |   0.32 |       0.000 |   0.00 |      0.001 |   0.32 |
|     CSV::outputStep                    |      0.000 |   0.10 |       0.000 |   0.00 |      0.000 |   0.10 |
-------------------------------------------------------------------------------------------------------------
```

Example with `level = 3`:

```
-------------------------------------------------------------------------------------------------------------------------------------
|                             Section                            |   Self(s)  |    %   | Children(s) |    %   |  Total(s)  |    %   |
-------------------------------------------------------------------------------------------------------------------------------------
| App                                                            |      0.004 |   1.87 |       0.218 |  98.13 |      0.222 | 100.00 |
|   MooseApp::run                                                |      0.000 |   0.00 |       0.218 |  98.13 |      0.218 |  98.13 |
|     MooseApp::setup                                            |      0.000 |   0.00 |       0.065 |  29.29 |      0.065 |  29.29 |
|       MooseApp::runInputFile                                   |      0.000 |   0.05 |       0.039 |  17.64 |      0.039 |  17.69 |
|         GeneratedMesh::init                                    |      0.005 |   2.18 |       0.000 |   0.00 |      0.005 |   2.18 |
|         GeneratedMesh::prepare                                 |      0.001 |   0.25 |       0.002 |   1.03 |      0.003 |   1.28 |
|           GeneratedMesh::update                                |      0.000 |   0.17 |       0.002 |   0.86 |      0.002 |   1.03 |
|             GeneratedMesh::cacheInfo                           |      0.001 |   0.51 |       0.000 |   0.00 |      0.001 |   0.51 |
|         FEProblem::init                                        |      0.000 |   0.04 |       0.019 |   8.62 |      0.019 |   8.66 |
|           GeneratedMesh::meshChanged                           |      0.000 |   0.00 |       0.002 |   0.95 |      0.002 |   0.95 |
|             GeneratedMesh::update                              |      0.000 |   0.18 |       0.001 |   0.64 |      0.002 |   0.82 |
|               GeneratedMesh::cacheInfo                         |      0.001 |   0.37 |       0.000 |   0.00 |      0.001 |   0.37 |
|           FEProblem::EquationSystems::Init                     |      0.017 |   7.67 |       0.000 |   0.00 |      0.017 |   7.67 |
|     MooseApp::execute                                          |      0.000 |   0.00 |       0.153 |  68.84 |      0.153 |  68.84 |
|       MooseApp::executeExecutioner                             |      0.000 |   0.05 |       0.152 |  68.79 |      0.153 |  68.84 |
|         FEProblem::initialSetup                                |      0.001 |   0.30 |       0.006 |   2.74 |      0.007 |   3.04 |
|           GeneratedMesh::meshChanged                           |      0.000 |   0.00 |       0.002 |   0.98 |      0.002 |   0.99 |
|             GeneratedMesh::update                              |      0.000 |   0.17 |       0.002 |   0.69 |      0.002 |   0.87 |
|               GeneratedMesh::cacheInfo                         |      0.001 |   0.39 |       0.000 |   0.00 |      0.001 |   0.39 |
|           FEProblem::projectSolution                           |      0.003 |   1.57 |       0.000 |   0.00 |      0.003 |   1.57 |
|           FEProblem::reinitBecauseOfGhostingOrNewGeomObjects   |      0.000 |   0.00 |       0.000 |   0.00 |      0.000 |   0.00 |
|           FEProblem::updateGeometricSearch                     |      0.000 |   0.00 |       0.000 |   0.00 |      0.000 |   0.00 |
|         FEProblem::computeUserObjects                          |      0.000 |   0.10 |       0.000 |   0.00 |      0.000 |   0.10 |
|         FEProblem::solve                                       |      0.014 |   6.44 |       0.130 |  58.64 |      0.144 |  65.07 |
|           FEProblem::computeResidualSys                        |      0.000 |   0.01 |       0.101 |  45.42 |      0.101 |  45.43 |
|             FEProblem::computeResidualInternal                 |      0.000 |   0.01 |       0.101 |  45.41 |      0.101 |  45.42 |
|               FEProblem::computeResidualTags                   |      0.000 |   0.12 |       0.100 |  45.30 |      0.101 |  45.41 |
|                 NonlinearSystemBase::computeResidualTags       |      0.001 |   0.23 |       0.100 |  45.06 |      0.100 |  45.30 |
|                   NonlinearSystemBase::computeResidualInternal |      0.000 |   0.22 |       0.098 |  44.22 |      0.098 |  44.44 |
|                     NonlinearSystemBase::Kernels               |      0.098 |  44.22 |       0.000 |   0.00 |      0.098 |  44.22 |
|                   NonlinearSystemBase::NodalBCs                |      0.001 |   0.62 |       0.000 |   0.00 |      0.001 |   0.62 |
|           FEProblem::computeJacobianInternal                   |      0.000 |   0.00 |       0.029 |  13.10 |      0.029 |  13.10 |
|             FEProblem::computeJacobianTags                     |      0.000 |   0.03 |       0.029 |  13.07 |      0.029 |  13.10 |
|               NonlinearSystemBase::computeJacobianTags         |      0.029 |  13.07 |       0.000 |   0.00 |      0.029 |  13.07 |
|           Console::outputStep                                  |      0.000 |   0.10 |       0.000 |   0.00 |      0.000 |   0.10 |
|         FEProblem::outputStep                                  |      0.000 |   0.05 |       0.001 |   0.49 |      0.001 |   0.53 |
|           PerfGraphOutput::outputStep                          |      0.000 |   0.00 |       0.000 |   0.00 |      0.000 |   0.00 |
|           Console::outputStep                                  |      0.001 |   0.41 |       0.000 |   0.00 |      0.001 |   0.41 |
|           CSV::outputStep                                      |      0.000 |   0.07 |       0.000 |   0.00 |      0.000 |   0.07 |
-------------------------------------------------------------------------------------------------------------------------------------
```

The columns `Self`, `Children` and `Total` represent the time spent *in* the section, *in sections underneath* the section, and the sum of those two.

The percentage columns following each one are the percent that the time to the left is of the total run-time of the App.


## Levels

The following are the current level "recommendations"... note that Apps are free to add code sections to whatever level they wish... so this is just a suggestion!

- 0: Just the "root" - the whole application time
- 1: Minimal set of the most important routines (residual/jacobian computation, etc.)
- 2: Important initialization routines (setting up the mesh, initializing the systems, etc.)
- 3: More detailed information from levels `1` and `2`
- 4: This is where the Actions will start to print
- 5: Fairly unimportant, or less used routines
- 6: Routines that rarely take up much time

## Heaviest Branch

To output the most expensive ("heaviest") trace through the code set `heaviest_branch = true`.  It will print something like this:

```
-------------------------------------------------------------------------------------------------------------------------------------
|                             Section                            |   Self(s)  |    %   | Children(s) |    %   |  Total(s)  |    %   |
-------------------------------------------------------------------------------------------------------------------------------------
| App                                                            |      0.004 |   1.86 |       0.218 |  98.14 |      0.222 | 100.00 |
|   MooseApp::run                                                |      0.000 |   0.00 |       0.218 |  98.14 |      0.218 |  98.14 |
|     MooseApp::execute                                          |      0.000 |   0.00 |       0.153 |  68.88 |      0.153 |  68.88 |
|       MooseApp::executeExecutioner                             |      0.000 |   0.05 |       0.153 |  68.83 |      0.153 |  68.88 |
|         FEProblem::solve                                       |      0.014 |   6.43 |       0.130 |  58.56 |      0.144 |  64.99 |
|           FEProblem::computeResidualSys                        |      0.000 |   0.01 |       0.101 |  45.36 |      0.101 |  45.37 |
|             FEProblem::computeResidualInternal                 |      0.000 |   0.01 |       0.101 |  45.35 |      0.101 |  45.36 |
|               FEProblem::computeResidualTags                   |      0.000 |   0.11 |       0.100 |  45.24 |      0.101 |  45.35 |
|                 NonlinearSystemBase::computeResidualTags       |      0.001 |   0.23 |       0.100 |  45.01 |      0.100 |  45.24 |
|                   NonlinearSystemBase::computeResidualInternal |      0.000 |   0.22 |       0.098 |  44.17 |      0.098 |  44.38 |
|                     NonlinearSystemBase::Kernels               |      0.098 |  44.16 |       0.000 |   0.00 |      0.098 |  44.16 |
-------------------------------------------------------------------------------------------------------------------------------------
```

## Heaviest Sections

By setting `heaviest_sections = 10` (or any positive integer) you can get a print out of the 10 sections that take the most "Self" time like so:

```
---------------------------------------------------------------------
|                   Section                   |   Self(s)  |    %   |
---------------------------------------------------------------------
| NonlinearSystemBase::Kernels                |      0.098 |  44.14 |
| NonlinearSystemBase::computeJacobianTags    |      0.029 |  13.05 |
| MooseApp::setupOptions                      |      0.026 |  11.58 |
| FEProblem::EquationSystems::Init            |      0.017 |   7.65 |
| FEProblem::solve                            |      0.014 |   6.42 |
| GeneratedMesh::init                         |      0.005 |   2.17 |
| App                                         |      0.004 |   1.86 |
| FEProblem::projectSolution                  |      0.003 |   1.57 |
| GeneratedMesh::cacheInfo                    |      0.003 |   1.26 |
| Action::CreateProblemAction::FEProblem::act |      0.003 |   1.25 |
---------------------------------------------------------------------
```

!syntax parameters /Outputs/PerfGraphOutput

!syntax inputs /Outputs/PerfGraphOutput

!syntax children /Outputs/PerfGraphOutput

!bibtex bibliography
