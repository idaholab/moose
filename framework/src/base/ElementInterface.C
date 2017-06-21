/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ElementInterface.h"
#include "FEProblem.h"

#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"
#include "Assembly.h"

// libMesh
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/fe_base.h"
#include "libmesh/fe_interface.h"
#include "libmesh/serial_mesh.h"
#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/mesh_communication.h"
#include "libmesh/mesh_inserter_iterator.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_elem.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/parallel_node.h"
#include "libmesh/parallel_ghost_sync.h"
#include "libmesh/utility.h"
#include "libmesh/remote_elem.h"
#include "libmesh/linear_partitioner.h"
#include "libmesh/centroid_partitioner.h"
#include "libmesh/parmetis_partitioner.h"
#include "libmesh/hilbert_sfc_partitioner.h"
#include "libmesh/morton_sfc_partitioner.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/mesh_refinement.h"
#include "libmesh/quadrature.h"
#include "libmesh/boundary_info.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/default_coupling.h"
#include "libmesh/ghost_point_neighbors.h"

template <>
InputParameters
validParams<ElementInterface>()
{
  InputParameters params = emptyInputParameters();
  return params;
}

ElementInterface::ElementInterface(const MooseObject * moose_object)
  : _ei_params(moose_object->parameters()),
    _ei_subproblem(*_ei_params.get<SubProblem *>("_subproblem")),
    _ei_feproblem(*_ei_params.get<FEProblemBase *>("_fe_problem_base")),
    _ei_mesh(_ei_subproblem.mesh()),
    _ei_assembly(_ei_subproblem.assembly(0)),
    _ei_current_elem(_ei_assembly.elem())
{
}

void
ElementInterface::massProduct(const VariableValue & u, Real coef, DenseVector<Real> & prod) const
{
  Real volume = _ei_mesh.elemVolume(_ei_current_elem) * coef / 12;
  Real u0 = u[0] * volume;
  Real u1 = u[1] * volume;
  Real u2 = u[2] * volume;
  Real a = u0 + u1 + u2;
  prod(0) += a + u0;
  prod(1) += a + u1;
  prod(2) += a + u2;
}

void
ElementInterface::stiffnessProduct(const VariableValue & u, Real coef, DenseVector<Real> & prod) const
{
  const std::vector<Real> & elem_ar(_ei_mesh.elemAspectRatio(_ei_current_elem));
  Real u0 = u[0] * coef;
  Real u1 = u[1] * coef;
  Real u2 = u[2] * coef;
  Real r0 = elem_ar[1];
  Real r1 = elem_ar[2];
  Real r2 = elem_ar[0];
  Real c0 = r2 + r1 - r0;
  Real c1 = r2 + r0 - r1;
  Real c2 = r0 + r1 - r2;
  c0 /= 2;
  c1 /= 2;
  c2 /= 2;

  prod(0) += r0 * u0 - (c2 * u1 + c1 * u2);
  prod(1) += r1 * u1 - (c2 * u0 + c0 * u2);
  prod(2) += r2 * u2 - (c1 * u0 + c0 * u1);
}

void
ElementInterface::massMatrix(Real coef, DenseMatrix<Real> & mat) const
{
  Real v1 = _ei_mesh.elemVolume(_ei_current_elem) * coef / 12;
  Real v2 = v1 + v1;
  mat(0,0) += v2; mat(0,1) += v1; mat(0,2) += v1;
  mat(1,0) += v1; mat(1,1) += v2; mat(1,2) += v1;
  mat(2,0) += v1; mat(2,1) += v1; mat(2,2) += v2;
}

void
ElementInterface::stiffnessMatrix(Real coef, DenseMatrix<Real> & mat) const
{
  const std::vector<Real> & elem_ar(_ei_mesh.elemAspectRatio(_ei_current_elem));
  Real r0 = elem_ar[1] * coef / 2;
  Real r1 = elem_ar[2] * coef / 2;
  Real r2 = elem_ar[0] * coef / 2;
  Real c0 = r2 + r1 - r0;
  Real c1 = r2 + r0 - r1;
  Real c2 = r0 + r1 - r2;
  mat(0, 0) += r0 + r0;
  mat(1, 1) += r1 + r1;
  mat(2, 2) += r2 + r2;
  mat(0, 1) += -c2;
  mat(0, 2) += -c1;
  mat(1, 0) += -c2;
  mat(1, 2) += -c0;
  mat(2, 0) += -c1;
  mat(2, 1) += -c0;
}

ElementInterface::~ElementInterface() {}
