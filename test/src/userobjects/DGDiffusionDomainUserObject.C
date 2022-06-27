//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGDiffusionDomainUserObject.h"
#include "MooseMesh.h"
#include "Function.h"
#include "MooseVariableFE.h"
#include "Assembly.h"

registerMooseObject("MooseTestApp", DGDiffusionDomainUserObject);

InputParameters
DGDiffusionDomainUserObject::validParams()
{
  InputParameters params = DomainUserObject::validParams();
  params.addRequiredCoupledVar("u", "The variable we are running diffusion with DG for");
  params.addRequiredParam<FunctionName>("function", "The forcing function.");
  params.addRequiredParam<Real>("epsilon", "Epsilon");
  params.addRequiredParam<Real>("sigma", "Sigma");
  return params;
}

DGDiffusionDomainUserObject::DGDiffusionDomainUserObject(const InputParameters & parameters)
  : DomainUserObject(parameters),
    _u(coupledValue("u")),
    _u_neighbor(coupledNeighborValue("u")),
    _grad_u(coupledGradient("u")),
    _grad_u_neighbor(coupledNeighborGradient("u")),
    _var(*getVar("u", 0)),
    _test(_var.phi()),
    _grad_test(_var.gradPhi()),
    _test_face(_var.phiFace()),
    _grad_test_face(_var.gradPhiFace()),
    _test_face_neighbor(_var.phiFaceNeighbor()),
    _grad_test_face_neighbor(_var.gradPhiFaceNeighbor()),
    _func(getFunction("function")),
    _epsilon(getParam<Real>("epsilon")),
    _sigma(getParam<Real>("sigma"))
{
}

void
DGDiffusionDomainUserObject::initialize()
{
  _elem_integrals.clear();
  _elem_integrals.resize(_subproblem.mesh().getMesh().max_elem_id());
}

void
DGDiffusionDomainUserObject::executeOnElement()
{
  auto & local_integrals = _elem_integrals[_current_elem->id()];
  local_integrals.resize(_test.size());

  for (const auto i : make_range(_test.size()))
    for (const auto qp : make_range(qRule().n_points()))
      local_integrals[i] += JxW()[qp] * coord()[qp] * _grad_u[qp] * _grad_test[i][qp];
}

void
DGDiffusionDomainUserObject::executeOnBoundary()
{
  auto & local_integrals = _elem_integrals[_current_elem->id()];
  mooseAssert(local_integrals.size() == _test_face.size(),
              "These should be equal or we're in trouble.");

  for (const auto i : make_range(_test_face.size()))
    for (const auto qp : make_range(qRule().n_points()))
    {
      const unsigned int elem_b_order = _var.order();
      const double h_elem =
          _current_elem_volume / _current_side_volume * 1. / Utility::pow<2>(elem_b_order);

      Real fn = _func.value(_t, qPoints()[qp]);
      Real r = 0;
      r -= (_grad_u[qp] * _normals[qp] * _test_face[i][qp]);
      r += _epsilon * (_u[qp] - fn) * _grad_test_face[i][qp] * _normals[qp];
      r += _sigma / h_elem * (_u[qp] - fn) * _test_face[i][qp];
      local_integrals[i] += JxW()[qp] * coord()[qp] * r;
    }
}

void
DGDiffusionDomainUserObject::executeOnInternalSide()
{
  const unsigned int elem_b_order = _var.order();
  const double h_elem =
      _current_elem_volume / _current_side_volume * 1. / Utility::pow<2>(elem_b_order);

  // elem
  {
    auto & local_integrals = _elem_integrals[_current_elem->id()];
    mooseAssert(local_integrals.size() == _test_face.size(),
                "These should be equal or we're in trouble.");

    for (const auto i : make_range(_test_face.size()))
      for (const auto qp : make_range(qRule().n_points()))
      {
        Real r = 0;
        r -= 0.5 * (_grad_u[qp] * _normals[qp] + _grad_u_neighbor[qp] * _normals[qp]) *
             _test_face[i][qp];
        r += _epsilon * 0.5 * (_u[qp] - _u_neighbor[qp]) * _grad_test_face[i][qp] * _normals[qp];
        r += _sigma / h_elem * (_u[qp] - _u_neighbor[qp]) * _test_face[i][qp];
        local_integrals[i] += JxW()[qp] * coord()[qp] * r;
      }
  }

  // neighbor
  {
    auto & local_integrals = _elem_integrals[_neighbor_elem->id()];
    if (local_integrals.size() < _test_face_neighbor.size())
      local_integrals.resize(_test_face_neighbor.size());

    for (const auto i : make_range(_test_face_neighbor.size()))
      for (const auto qp : make_range(qRule().n_points()))
      {
        Real r = 0;
        r += 0.5 * (_grad_u[qp] * _normals[qp] + _grad_u_neighbor[qp] * _normals[qp]) *
             _test_face_neighbor[i][qp];
        r += _epsilon * 0.5 * (_u[qp] - _u_neighbor[qp]) * _grad_test_face_neighbor[i][qp] *
             _normals[qp];
        r -= _sigma / h_elem * (_u[qp] - _u_neighbor[qp]) * _test_face_neighbor[i][qp];
        local_integrals[i] += JxW()[qp] * coord()[qp] * r;
      }
  }
}

void
DGDiffusionDomainUserObject::finalize()
{
  for (auto & local_integrals : _elem_integrals)
  {
    auto size = local_integrals.size();
    _communicator.max(size);
    local_integrals.resize(size);
    gatherSum(local_integrals);
    for (const auto integral : local_integrals)
      if (std::abs(integral) > TOLERANCE * TOLERANCE)
        mooseError("DG diffusion problem not converged");
  }
}

void
DGDiffusionDomainUserObject::threadJoin(const UserObject & y)
{
  const DGDiffusionDomainUserObject & dg_uo = dynamic_cast<const DGDiffusionDomainUserObject &>(y);
  for (unsigned int i = 0; i < _elem_integrals.size(); i++)
  {
    auto & my_local_integrals = _elem_integrals[i];
    auto & their_local_integrals = dg_uo._elem_integrals[i];
    if (my_local_integrals.size() < their_local_integrals.size())
      my_local_integrals.resize(their_local_integrals.size());
    for (const auto j : index_range(their_local_integrals))
      my_local_integrals[j] += their_local_integrals[j];
  }
}
