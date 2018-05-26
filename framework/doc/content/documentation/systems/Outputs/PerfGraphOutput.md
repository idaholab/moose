# PerfGraphOutput

!syntax description /Outputs/PerfGraphOutput

## Description

The `PerfGraph` object holds timing data for MOOSE.  With this object you can control when it's contents get printed to the screen and how detailed the data is.

Controlling when it gets printed is achieved by setting `execute_on`.  By default it's set to `final` which causes the information to be printed at the end of the simulation.

Controlling the detail is done by setting `level`.  The default level is `1` which print fairly basic performance information.  Setting the number higher causes more detail to come out.

Example with `level = 1`:

```
Root self: 0.04845 children: 0 total: 0.04845
  FEProblemBase::solve self: 0.011935 children: 0.00385 total: 0.015785
    FEProblemBase::computeResidualInternal self: 7e-06 children: 0.002923 total: 0.00293
    FEProblemBase::computeJacobianInternal self: 1e-06 children: 0.000899 total: 0.0009
  FEProblemBase::outputStep self: 5.7e-05 children: 0 total: 5.7e-05
```

Example with `level = 3`:

```
Root self: 0.064171 children: 0 total: 0.064171
  MooseApp::run self: -0.029871 children: 0.029871 total: 0
    MooseApp::runInputFile self: 5.4e-05 children: 0.016356 total: 0.01641
      Mesh::init self: 0.000356 children: 0 total: 0.000356
      Mesh::prepare self: 3.7e-05 children: 0.000111 total: 0.000148
        Mesh::update self: 1.7e-05 children: 9.3e-05 total: 0.00011
          Mesh::cacheInfo self: 5.4e-05 children: 0 total: 5.4e-05
      FEProblemBase::init self: 6.4e-05 children: 0.009568 total: 0.009632
        Mesh::meshChanged self: 4e-06 children: 0.000157 total: 0.000161
          Mesh::update self: 2e-05 children: 8.3e-05 total: 0.000103
            Mesh::cacheInfo self: 3.8e-05 children: 0 total: 3.8e-05
        FEProblemBase::EquationSystems::Init self: 0.009407 children: 0 total: 0.009407
    MooseApp::executeExecutioner self: -0.031231 children: 0.031231 total: 0
      FEProblemBase::initialSetup self: 0.000366 children: 0.006208 total: 0.006574
        Mesh::meshChanged self: 3e-06 children: 0.000137 total: 0.00014
          Mesh::update self: 2.1e-05 children: 7.5e-05 total: 9.6e-05
            Mesh::cacheInfo self: 3.7e-05 children: 0 total: 3.7e-05
        FEProblemBase::projectSolution self: 0.006032 children: 0 total: 0.006032
        FEProblemBase::reinitBecauseOfGhostingOrNewGeomObjects self: 0 children: 0 total: 0
      FEProblemBase::solve self: 0.0197 children: 0.004674 total: 0.024374
        FEProblemBase::computeResidualSys self: 2e-05 children: 0.003546 total: 0.003566
          FEProblemBase::computeResidualInternal self: 1.1e-05 children: 0.003535 total: 0.003546
            FEProblemBase::computeResidualTags self: 0.003526 children: 9e-06 total: 0.003535
        FEProblemBase::computeJacobianInternal self: 4e-06 children: 0.001093 total: 0.001097
          FEProblemBase::computeJacobianTags self: 0.001093 children: 0 total: 0.001093
      FEProblemBase::outputStep self: 0.000176 children: 0 total: 0.000176
```

## Levels

The following are the current level "recommendations"... note that Apps are free to add code sections to whatever level they wish... so this is just a suggestion!

- 0: Just the "root" - the whole application time
- 1: Minimal set of the most important routines (residual/jacobian computation, etc.)
- 2: Important initialization routines (setting up the mesh, initializing the systems, etc.)
- 3: More detailed information from levels `1` and `2`
- 4: This is where the Actions will start to print
- 5: Fairly unimportant, or less used routines
- 6: Routines that rarely take up much time

!syntax parameters /Outputs/PerfGraphOutput

!syntax inputs /Outputs/PerfGraphOutput

!syntax children /Outputs/PerfGraphOutput

!bibtex bibliography
