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

#include "DGKernel.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"

const BoundaryID DGKernel::InternalBndId = 12345;

template<>
InputParameters validParams<DGKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this boundary condition applies to");
  params.addPrivateParam<BoundaryID>("_boundary_id", DGKernel::InternalBndId);

  params.addPrivateParam<std::string>("built_by_action", "add_dg_kernel");
  return params;
}


DGKernel::DGKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    TransientInterface(parameters, name, "dgkernels"),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(parameters, false),
    TwoMaterialPropertyInterface(parameters),
    Restartable(name, parameters, "DGKernels"),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
//    _dim(_mesh.dimension()),

    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),

    _neighbor_elem(_assembly.neighbor()),
    _neighbor_elem_volume(_assembly.neighborVolume()),

    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),

    _coord_sys(_assembly.coordSystem()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),

    _boundary_id(parameters.get<BoundaryID>("_boundary_id")),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),

    _phi(_assembly.phiFace()),
    _grad_phi(_assembly.gradPhiFace()),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),

    _normals(_var.normals()),

    _phi_neighbor(_assembly.phiFaceNeighbor()),
    _grad_phi_neighbor(_assembly.gradPhiFaceNeighbor()),

    _test_neighbor(_var.phiFaceNeighbor()),
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()),

    _u_neighbor(_var.slnNeighbor()),
    _grad_u_neighbor(_var.gradSlnNeighbor())
{
}

DGKernel::~DGKernel()
{
}

void
DGKernel::computeResidual()
{
//  Moose::perf_log.push("computeResidual()","DGKernel");

  DenseVector<Number> & re = _assembly.residualBlock(_var.index());
  DenseVector<Number> & neighbor_re = _assembly.residualBlockNeighbor(_var.index());

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_test.size(); _i++)
      re(_i) += _JxW[_qp]*_coord[_qp]*computeQpResidual(Moose::Element);

    for (_i=0; _i<_test_neighbor.size(); _i++)
      neighbor_re(_i) += _JxW[_qp]*_coord[_qp]*computeQpResidual(Moose::Neighbor);
  }

//  Moose::perf_log.pop("computeResidual()","DGKernel");
}

void
DGKernel::computeJacobian()
{
//  Moose::perf_log.push("computeJacobian()","DGKernel");

  DenseMatrix<Number> & Kee = _assembly.jacobianBlock(_var.index(), _var.index());
  DenseMatrix<Number> & Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.index(), _var.index());

  DenseMatrix<Number> & Kne = _assembly.jacobianBlockNeighbor(Moose::NeighborElement, _var.index(), _var.index());
  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _var.index(), _var.index());

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_test.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        Kee(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::ElementElement);

    for (_i=0; _i<_test.size(); _i++)
      for (_j=0; _j<_phi_neighbor.size(); _j++)
        Ken(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::ElementNeighbor);

    for (_i=0; _i<_test_neighbor.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        Kne(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::NeighborElement);

    for (_i=0; _i<_test_neighbor.size(); _i++)
      for (_j=0; _j<_phi_neighbor.size(); _j++)
        Knn(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::NeighborNeighbor);
  }

//  Moose::perf_log.pop("computeJacobian()","DGKernel");
}

void
DGKernel::computeOffDiagJacobian(unsigned int jvar)
{
//  Moose::perf_log.push("computeOffDiagJacobian()","DGKernel");

  DenseMatrix<Number> & Kee = _assembly.jacobianBlock(_var.index(), jvar);
  DenseMatrix<Number> & Ken = _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.index(), jvar);

  DenseMatrix<Number> & Kne = _assembly.jacobianBlockNeighbor(Moose::NeighborElement, _var.index(), jvar);
  DenseMatrix<Number> & Knn = _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _var.index(), jvar);

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_test.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
      {
        if(jvar == _var.index())
          Kee(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::ElementElement);
        else
          Kee(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpOffDiagJacobian(Moose::ElementElement, jvar);
      }

    for (_i=0; _i<_test.size(); _i++)
      for (_j=0; _j<_phi_neighbor.size(); _j++)
      {
        if(jvar == _var.index())
          Ken(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::ElementNeighbor);
        else
          Ken(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpOffDiagJacobian(Moose::ElementNeighbor, jvar);
      }

    for (_i=0; _i<_test_neighbor.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
      {
        if(jvar == _var.index())
          Kne(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::NeighborElement);
        else
          Kne(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpOffDiagJacobian(Moose::NeighborElement, jvar);
      }

    for (_i=0; _i<_test_neighbor.size(); _i++)
      for (_j=0; _j<_phi_neighbor.size(); _j++)
      {
        if(jvar == _var.index())
          Knn(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpJacobian(Moose::NeighborNeighbor);
        else
          Knn(_i,_j) += _JxW[_qp]*_coord[_qp]*computeQpOffDiagJacobian(Moose::NeighborNeighbor, jvar);
      }
  }

//  Moose::perf_log.pop("computeOffDiagJacobian()","DGKernel");
}

Real
DGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}

MooseVariable &
DGKernel::variable()
{
  return _var;
}

SubProblem &
DGKernel::subProblem()
{
  return _subproblem;
}
