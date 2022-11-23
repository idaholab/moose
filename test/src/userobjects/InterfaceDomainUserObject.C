//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceDomainUserObject.h"
#include "MooseMesh.h"
#include "Function.h"
#include "MooseVariableFE.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", InterfaceDomainUserObject);

InputParameters
InterfaceDomainUserObject::validParams()
{
  InputParameters params = DomainUserObject::validParams();
  params.addRequiredCoupledVar("u", "The variable we are diffusion for");
  params.addRequiredCoupledVar("v", "The variable on the other side of the interface");
  params.addParam<FunctionName>("function", "1", "The forcing function.");
  params.addParam<Real>("robin_coef", 2, "The cofficient multiplying the Robin boundary condition");
  params.addParam<std::vector<BoundaryName>>(
      "robin_boundaries", "The boundaries on which to apply the Robin boundary condition");
  params.addRequiredParam<Real>(
      "interface_penalty",
      "The penalty that penalizes jump between primary and neighbor variables.");
  params.addRequiredParam<Real>("nl_abs_tol", "The absolute tolerance of the nonlinear solver");
  return params;
}

InterfaceDomainUserObject::InterfaceDomainUserObject(const InputParameters & parameters)
  : DomainUserObject(parameters),
    _u(coupledValue("u")),
    _v_neighbor(coupledNeighborValue("v")),
    _grad_u(coupledGradient("u")),
    _grad_v_neighbor(*coupledNeighborGradients("v")[0]),
    _var(*getVar("u", 0)),
    _v_var(*getInterfaceFieldVar("v", 0)),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _test_face(_var.phiFace()),
    _grad_test_face(_var.gradPhiFace()),
    _test_face_neighbor(_var.phiFaceNeighbor()),
    _grad_test_face_neighbor(_var.gradPhiFaceNeighbor()),
    _func(getFunction("function")),
    _robin_coef(getParam<Real>("robin_coef")),
    _interface_penalty(getParam<Real>("interface_penalty")),
    _nl_abs_tol(getParam<Real>("nl_abs_tol"))
{
  const auto & robin_boundaries = getParam<std::vector<BoundaryName>>("robin_boundaries");
  const auto & robin_bnd_ids_vec = _mesh.getBoundaryIDs(robin_boundaries);
  _robin_bnd_ids = std::set<BoundaryID>(robin_bnd_ids_vec.begin(), robin_bnd_ids_vec.end());
}

void
InterfaceDomainUserObject::initialize()
{
  _nodal_integrals.clear();
  _nodal_integrals.resize(_subproblem.mesh().getMesh().max_node_id());
}

void
InterfaceDomainUserObject::executeOnElement()
{
  for (const auto i : index_range(_test))
  {
    auto & integral = _nodal_integrals[_current_elem->node_ref(i).id()];
    for (const auto qp : make_range(qRule().n_points()))
      integral += JxW()[qp] * coord()[qp] *
                  (_grad_u[qp] * _grad_test[i][qp] - _test[i][qp] * _func.value(_t, qPoints()[qp]));
  }
}

void
InterfaceDomainUserObject::executeOnBoundary()
{
  if (!_robin_bnd_ids.count(_current_boundary_id))
    return;

  for (const auto i : index_range(_test_face))
  {
    auto & integral = _nodal_integrals[_current_elem->node_ref(i).id()];
    for (const auto qp : make_range(qRule().n_points()))
      integral += JxW()[qp] * coord()[qp] * _test_face[i][qp] * _robin_coef * _u[qp];
  }
}

void
InterfaceDomainUserObject::executeOnInterface()
{
  mooseAssert(_interface_bnd_ids.count(_current_boundary_id),
              "We should only get called if shouldExecuteOnInterface returns true");

  for (const auto i : index_range(_test_face))
  {
    auto & integral = _nodal_integrals[_current_elem->node_ref(i).id()];
    for (const auto qp : make_range(qRule().n_points()))
      integral += JxW()[qp] * coord()[qp] * _test_face[i][qp] * _interface_penalty *
                  (_u[qp] - _v_neighbor[qp]);
  }
}

void
InterfaceDomainUserObject::finalize()
{
  gatherSum(_nodal_integrals);
  for (const auto integral : _nodal_integrals)
    if (std::abs(integral) > _nl_abs_tol)
      mooseError("Interface diffusion problem not converged");
}

void
InterfaceDomainUserObject::threadJoin(const UserObject & y)
{
  const InterfaceDomainUserObject & interface_uo =
      dynamic_cast<const InterfaceDomainUserObject &>(y);
  for (unsigned int i = 0; i < _nodal_integrals.size(); i++)
    _nodal_integrals[i] += interface_uo._nodal_integrals[i];
}
