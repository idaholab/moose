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

#include "DiracKernel.h"

// Moose includes
#include "SystemBase.h"
#include "Problem.h"

// libMesh includes
#include "parallel.h"
#include "point_locator_base.h"
#include "libmesh_common.h"

template<>
InputParameters validParams<DiracKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<SetupInterface>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this kernel operates on");

  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_dirac_kernel");
  return params;
}

DiracKernel::DiracKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    Coupleable(parameters, false),
    FunctionInterface(parameters),
    TransientInterface(parameters),
    MaterialPropertyInterface(parameters),
    GeometricSearchInterface(parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),
    _dirac_kernel_info(_subproblem.diracKernelInfo()),

    _current_elem(_var.currentElem()),
    _q_point(_subproblem.points(_tid)),
    _physical_point(_subproblem.physicalPoints(_tid)),
    _qrule(_subproblem.qRule(_tid)),
    _JxW(_subproblem.JxW(_tid)),

    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi()),
    _second_phi(_assembly.secondPhi()),

    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _second_test(_var.secondPhi()),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),
    _grad_u(_var.gradSln()),
    _grad_u_old(_var.gradSlnOld()),
    _grad_u_older(_var.gradSlnOlder()),
    _second_u(_var.secondSln()),
    _second_u_old(_var.secondSlnOld()),
    _second_u_older(_var.secondSlnOlder()),

    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu()),

    _real_zero(_problem._real_zero[_tid]),
    _zero(_problem._zero[_tid]),
    _grad_zero(_problem._grad_zero[_tid]),
    _second_zero(_problem._second_zero[_tid])
{
}

void
DiracKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point=_physical_point[_qp];
    if(isActiveAtPoint(_current_elem, _current_point))
    {
      for (_i = 0; _i < _test.size(); _i++)
        re(_i) += computeQpResidual();
    }
  }
}

void
DiracKernel::computeJacobian()
{
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    _current_point=_physical_point[_qp];
    if(isActiveAtPoint(_current_elem, _current_point))
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < _phi.size(); _j++)
        {
          ke(_i, _j) += computeQpJacobian();
        }
  }
}

Real
DiracKernel::computeQpJacobian()
{
  return 0;
}

void
DiracKernel::addPoint(const Elem * elem, Point p)
{
  if(elem->processor_id() != libMesh::processor_id())
    return;

  _dirac_kernel_info.addPoint(elem, p);
  _elements.insert(elem);
  _points[elem].insert(p);
}

void
DiracKernel::addPoint(Point p)
{
  AutoPtr<PointLocatorBase> pl = PointLocatorBase::build(TREE, _mesh);
  const Elem * elem = (*pl)(p);
  addPoint(elem, p);
}

bool
DiracKernel::hasPointsOnElem(const Elem * elem)
{
  return _elements.count(_mesh.elem(elem->id())) != 0;
}

bool
DiracKernel::isActiveAtPoint(const Elem * elem, const Point & p)
{
  return _points[elem].count(p) != 0;
}

void
DiracKernel::clearPoints()
{
  _elements.clear();
  _points.clear();
}
