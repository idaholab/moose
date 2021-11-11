# PerfGraphReporterReader

A python utility that provides an interface for reading [PerfGraphReporter.md] output. It rebuilds the graph for easy traversal via the `PerfGraphNode` and `PerfGraphSection` objects.

## Example usage

Take the following simple diffusion problem, which has a [PerfGraphReporter.md] set to output on final:

!listing test/tests/reporters/perf_graph_reporter/perf_graph_reporter.i

For more real-world-like timing, we will pass the command line arguments "`Mesh/gmg/nx=500 Mesh/gmg/ny=500`", as the test above is executed with only a single element. With this, we will execute the following (where `MOOSE_DIR` is an environment variable set to the directory that contains MOOSE):

!listing
$MOOSE_DIR/test/moose_test-opt -i $MOOSE_DIR/test/tests/reporters/perf_graph_reporter/perf_graph_reporter.i Mesh/gmg/nx=500 Mesh/gmg/ny=500

This run will generate the desired output in `$MOOSE_DIR/test/tests/reporters/perf_graph_reporter/perf_graph_reporter_json.json`.

Load a `PerfGraphReporterReader` with the given output as such:

!listing language=python
import os
from mooseutils.PerfGraphReporterReader import PerfGraphReporterReader
MOOSE_DIR = os.environ.get('MOOSE_DIR')
pgrr = PerfGraphReporterReader(MOOSE_DIR + '/test/tests/reporters/perf_graph_reporter/perf_graph_reporter_json.json')

!alert note
The examples that follow show detail by calling the `info()` method on each `PerfGraphSection` and `PerfGraphNode`. This is typically not the best way to report the data (you should likely use the member methods on said classes to access the data you need), but it is appropriate here for the purpose of showing usage. For more information on the member methods in said classes, see [#reference] for references to methods in each of the previous listed classes.


### Heaviest sections

We can determine the five heaviest sections with:

!listing language=python
for section in pgrr.heaviestSections(5):
    print(section)

!listing
PerfGraphSection "NonlinearSystemBase::Kernels"
PerfGraphSection "FEProblem::EquationSystems::Init"
PerfGraphSection "NonlinearSystemBase::computeJacobianTags"
PerfGraphSection "FEProblem::solve"
PerfGraphSection "MeshGeneratorMesh::cacheInfo"

Similarly, we can obtain more detailed output with:

!listing language=python
for section in pgrr.heaviestSections(5):
    print(section.info())

!listing
PerfGraphSection "NonlinearSystemBase::Kernels":
  Num calls: 15
  Level: 3
  Time (41.36%): Self 4.34 s, Children 0.00 s, Total 4.34 s
  Memory (0.20%): Self 1 MB, Children 0 MB, Total 1 MB
  Nodes:
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::execute
         MooseApp::executeExecutioner
          Steady::PicardSolve
           FEProblem::solve
            FEProblem::computeResidualSys
             FEProblem::computeResidualInternal
              FEProblem::computeResidualTags
               NonlinearSystemBase::nl::computeResidualTags
                NonlinearSystemBase::computeResidualInternal
                 NonlinearSystemBase::Kernels (14 call(s), 38.3% time, 0.0% memory)
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::execute
         MooseApp::executeExecutioner
          Steady::PicardSolve
           FEProblem::solve
            NonlinearSystemBase::nlInitialResidual
             FEProblem::computeResidualSys
              FEProblem::computeResidualInternal
               FEProblem::computeResidualTags
                NonlinearSystemBase::nl::computeResidualTags
                 NonlinearSystemBase::computeResidualInternal
                  NonlinearSystemBase::Kernels (1 call(s), 3.0% time, 0.2% memory)
PerfGraphSection "FEProblem::EquationSystems::Init":
  Num calls: 1
  Level: 2
  Time (25.50%): Self 2.68 s, Children 0.00 s, Total 2.68 s
  Memory (17.59%): Self 86 MB, Children 0 MB, Total 86 MB
  Nodes:
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::setup
         MooseApp::runInputFile
          Action::InitProblemAction::act
           FEProblem::init
            FEProblem::EquationSystems::Init (1 call(s), 25.5% time, 17.6% memory)
PerfGraphSection "NonlinearSystemBase::computeJacobianTags":
  Num calls: 2
  Level: 5
  Time (11.29%): Self 1.19 s, Children 0.00 s, Total 1.19 s
  Memory (4.50%): Self 22 MB, Children 0 MB, Total 22 MB
  Nodes:
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::execute
         MooseApp::executeExecutioner
          Steady::PicardSolve
           FEProblem::solve
            FEProblem::computeJacobianInternal
             FEProblem::computeJacobianTags
              NonlinearSystemBase::computeJacobianTags (2 call(s), 11.3% time, 4.5% memory)
PerfGraphSection "FEProblem::solve":
  Num calls: 1
  Level: 1
  Time (60.96%): Self 0.85 s, Children 5.55 s, Total 6.40 s
  Memory (53.58%): Self 239 MB, Children 23 MB, Total 262 MB
  Nodes:
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::execute
         MooseApp::executeExecutioner
          Steady::PicardSolve
           FEProblem::solve (1 call(s), 61.0% time, 53.6% memory)
PerfGraphSection "MeshGeneratorMesh::cacheInfo":
  Num calls: 2
  Level: 3
  Time (3.69%): Self 0.39 s, Children 0.00 s, Total 0.39 s
  Memory (5.11%): Self 25 MB, Children 0 MB, Total 25 MB
  Nodes:
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::setup
         MooseApp::runInputFile
          Action::InitProblemAction::act
           FEProblem::init
            MeshGeneratorMesh::meshChanged
             MeshGeneratorMesh::update
              MeshGeneratorMesh::cacheInfo (1 call(s), 2.1% time, 0.2% memory)
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::setup
         MooseApp::runInputFile
          Action::SetupMeshCompleteAction::Mesh::act
           Action::SetupMeshCompleteAction::Mesh::completeSetupUndisplaced
            MeshGeneratorMesh::prepare
             MeshGeneratorMesh::update
              MeshGeneratorMesh::cacheInfo (1 call(s), 1.6% time, 4.9% memory)

### Section by name

Let's say we know that we want to look the timing of all kernel evaluations. With this, we're interested in the section `NonlinearSystemBase::Kernels`.

Obtain the section in question and show its information with:

!listing language=python
kernels_section = pgrr.section('NonlinearSystemBase::Kernels')
print(kernels_section.info())

!listing
PerfGraphSection "NonlinearSystemBase::Kernels":
  Num calls: 15
  Level: 3
  Time (41.36%): Self 4.34 s, Children 0.00 s, Total 4.34 s
  Memory (0.20%): Self 1 MB, Children 0 MB, Total 1 MB
  Nodes:
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::execute
         MooseApp::executeExecutioner
          Steady::PicardSolve
           FEProblem::solve
            FEProblem::computeResidualSys
             FEProblem::computeResidualInternal
              FEProblem::computeResidualTags
               NonlinearSystemBase::nl::computeResidualTags
                NonlinearSystemBase::computeResidualInternal
                 NonlinearSystemBase::Kernels (14 call(s), 38.3% time, 0.0% memory)
    - MooseTestApp (main)
       MooseApp::run
        MooseApp::execute
         MooseApp::executeExecutioner
          Steady::PicardSolve
           FEProblem::solve
            NonlinearSystemBase::nlInitialResidual
             FEProblem::computeResidualSys
              FEProblem::computeResidualInternal
               FEProblem::computeResidualTags
                NonlinearSystemBase::nl::computeResidualTags
                 NonlinearSystemBase::computeResidualInternal
                  NonlinearSystemBase::Kernels (1 call(s), 3.0% time, 0.2% memory)

From this, we can see that we had 14 residual solve evaluations that took 38.3% of the total run time and one residual evaluation on initial that took 3.0% of the total run time.

## Reference

!pysyntax class name=mooseutils.PerfGraphReporterReader

!pysyntax class name=mooseutils.PerfGraphNode

!pysyntax class name=mooseutils.PerfGraphSection
