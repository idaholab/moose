// STL
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <math.h>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

// C and C++
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <limits>
#include <stddef.h>
#include <utility>
#include <time.h>

// MPI
#include "mpi.h"

// Petsc
#if defined(LIBMESH_HAVE_PETSC) && defined(PETSC_VERSION_LE)
#if !PETSC_VERSION_LE(3,3,0)
#include "petscdmmesh.h"
#endif
#endif

// libmesh
#include "libmesh/partitioner.h"
#include "libmesh/boundary_info.h"
#include "libmesh/dense_matrix.h"
#include "libmesh/dense_submatrix.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/dense_vector.h"
#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/elem_range.h"
#include "libmesh/equation_systems.h"
#include "libmesh/explicit_system.h"
#include "libmesh/fe.h"
#include "libmesh/fe_base.h"
#include "libmesh/fe_interface.h"
#include "libmesh/getpot.h"
#include "libmesh/id_types.h"
#include "libmesh/implicit_system.h"
#include "libmesh/libmesh.h"
#include "libmesh/libmesh_base.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/libmesh_config.h"
#include "libmesh/linear_implicit_system.h"
#include "libmesh/mesh.h"
#include "libmesh/mesh_base.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/node.h"
#include "libmesh/node_range.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/parameters.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/point.h"
#include "libmesh/parallel.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/quadrature.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/sparse_matrix.h"
#include "libmesh/stored_range.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/system.h"
#include "libmesh/system_norm.h"
#include "libmesh/tensor_value.h"
#include "libmesh/threads.h"
#include "libmesh/transient_system.h"
#include "libmesh/utility.h"
#include "libmesh/variant_filter_iterator.h"
#include "libmesh/vector_value.h"

// moose
#include "AuxiliarySystem.h"
#include "BndNode.h"
#include "Coupleable.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "DisplacedProblem.h"
#include "Factory.h"
#include "FEProblem.h"
#include "FunctionInterface.h"
#include "InputParameters.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "MooseObjectAction.h"
#include "MooseTypes.h"
#include "MooseVariable.h"
#include "MooseVariableInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "NonlinearSystem.h"
#include "ParallelUniqueId.h"
#include "Parser.h"
#include "PenetrationLocator.h"
#include "Problem.h"
#include "Resurrector.h"
#include "SetupInterface.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "ThreadedElementLoop.h"
#include "TimeScheme.h"
#include "Transient.h"
#include "TransientInterface.h"

// moose systems
#include "Action.h"
#include "ActionFactory.h"
#include "ActionWarehouse.h"
#include "AddVariableAction.h"

#include "Kernel.h"
#include "Material.h"
#include "AuxKernel.h"

#include "BoundaryCondition.h"
#include "IntegratedBC.h"
#include "NodalBC.h"

#include "Damper.h"
#include "DGKernel.h"
#include "DiracKernel.h"
#include "Executioner.h"
#include "Function.h"
#include "InitialCondition.h"
#include "Marker.h"

#include "Indicator.h"
#include "IndicatorWarehouse.h"

#include "Postprocessor.h"
#include "PostprocessorWarehouse.h"
#include "ElementPostprocessor.h"
#include "NodalPostprocessor.h"
#include "SidePostprocessor.h"
#include "GeneralPostprocessor.h"

#include "UserObject.h"
#include "UserObjectInterface.h"
#include "UserObjectWarehouse.h"
#include "ElementUserObject.h"
#include "NodalUserObject.h"
#include "SideUserObject.h"
#include "GeneralUserObject.h"

