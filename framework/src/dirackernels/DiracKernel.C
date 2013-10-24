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
#include "libmesh/parallel.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/libmesh_common.h"

template<>
InputParameters validParams<DiracKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The name of the variable that this kernel operates on");

  params.addParam<bool>("use_displaced_mesh", false, "Whether or not this object should use the displaced mesh for computation.  Note that in the case this is true but no displacements are provided in the Mesh block the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addPrivateParam<std::string>("built_by_action", "add_dirac_kernel");
  return params;
}

DiracKernel::DiracKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    SetupInterface(parameters),
    CoupleableMooseVariableDependencyIntermediateInterface(parameters, false),
    FunctionInterface(parameters),
    UserObjectInterface(parameters),
    TransientInterface(parameters, name, "dirac_kernels"),
    MaterialPropertyInterface(parameters),
    PostprocessorInterface(parameters),
    GeometricSearchInterface(parameters),
    Restartable(name, parameters, "DiracKernels"),
    Reportable(name, parameters),
    ZeroInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),
    _coord_sys(_assembly.coordSystem()),
    _dirac_kernel_info(_subproblem.diracKernelInfo()),

    _current_elem(_var.currentElem()),
    _q_point(_assembly.qPoints()),
    _physical_point(_assembly.physicalPoints()),
    _qrule(_assembly.qRule()),
    _JxW(_assembly.JxW()),

    _phi(_assembly.phi()),
    _grad_phi(_assembly.gradPhi()),

    _test(_var.phi()),
    _grad_test(_var.gradPhi()),

    _u(_var.sln()),
    _grad_u(_var.gradSln()),
    _u_dot(_var.uDot()),
    _du_dot_du(_var.duDotDu())
{
}

void
DiracKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.index());

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
  DenseMatrix<Number> & ke = _assembly.jacobianBlock(_var.index(), _var.index());

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
  if (!elem || (elem->processor_id() != libMesh::processor_id()))
    return;

  _dirac_kernel_info.addPoint(elem, p);
  _elements.insert(elem);
  _points[elem].insert(p);
}

const Elem *
DiracKernel::addPoint(Point p)
{
  AutoPtr<PointLocatorBase> pl = PointLocatorBase::build(TREE, _mesh);
  const Elem * elem = (*pl)(p);
  addPoint(elem, p);
  return elem;
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

MooseVariable &
DiracKernel::variable()
{
  return _var;
}

SubProblem &
DiracKernel::subProblem()
{
  return _subproblem;
}
