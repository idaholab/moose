//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PCNSFVLaxFriedrichs.h"
#include "NS.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("MooseApp", PCNSFVLaxFriedrichs);

InputParameters
PCNSFVLaxFriedrichs::validParams()
{
  InputParameters params = FVFluxKernel::validParams();
  params.addClassDescription("Computes the residual of advective term using finite volume method.");
  params.addRequiredParam<UserObjectName>(NS::fluid, "Fluid userobject");
  MooseEnum eqn("mass momentum energy");
  params.addRequiredParam<MooseEnum>("eqn", eqn, "The equation you're solving.");
  MooseEnum momentum_component("x=0 y=1 z=2");
  params.addParam<MooseEnum>("momentum_component",
                             momentum_component,
                             "The component of the momentum equation that this kernel applies to.");
  return params;
}

PCNSFVLaxFriedrichs::PCNSFVLaxFriedrichs(const InputParameters & params)
  : FVFluxKernel(params),
    _fluid(dynamic_cast<FEProblemBase *>(&_subproblem)
               ->getUserObject<SinglePhaseFluidProperties>(NS::fluid)),
    _superficial_vel_elem(getADMaterialProperty<RealVectorValue>(NS::superficial_velocity)),
    _superficial_vel_neighbor(
        getNeighborADMaterialProperty<RealVectorValue>(NS::superficial_velocity)),
    _rho_elem(getADMaterialProperty<Real>(NS::density)),
    _rho_neighbor(getNeighborADMaterialProperty<Real>(NS::density)),
    _rhou_elem(getADMaterialProperty<RealVectorValue>(NS::momentum)),
    _rhou_neighbor(getNeighborADMaterialProperty<RealVectorValue>(NS::momentum)),
    _rho_ht_elem(getADMaterialProperty<Real>(NS::total_enthalpy_density)),
    _rho_ht_neighbor(getNeighborADMaterialProperty<Real>(NS::total_enthalpy_density)),
    _T_fluid_elem(getADMaterialProperty<Real>(NS::T_fluid)),
    _T_fluid_neighbor(getNeighborADMaterialProperty<Real>(NS::T_fluid)),
    _pressure_elem(getADMaterialProperty<Real>(NS::pressure)),
    _pressure_neighbor(getNeighborADMaterialProperty<Real>(NS::pressure)),
    _eps_elem(getMaterialProperty<Real>(NS::porosity)),
    _eps_neighbor(getNeighborMaterialProperty<Real>(NS::porosity)),
    _eqn(getParam<MooseEnum>("eqn")),
    _index(getParam<MooseEnum>("momentum_component"))
{
  if ((_eqn == "momentum") && !isParamValid("momentum_component"))
    paramError("eqn",
               "If 'momentum' is specified for 'eqn', then you must provide a parameter "
               "value for 'momentum_component'");
}

void
PCNSFVLaxFriedrichs::computeResidual(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  const auto r = MetaPhysicL::raw_value(computeQpResidual());

  const auto ft = fi.faceType(_var.name());
  if (ft != FaceInfo::VarFaceNeighbors::BOTH)
    mooseError("This object should only be run on internal faces. If on a boundary face, then "
               "PCNSFVLaxFriedrichsBC should be used instead");

  // residual contribution of this kernel to the elem element
  prepareVectorTag(_assembly, _var.number());
  _local_re(0) = r;
  accumulateTaggedLocalResidual();

  // residual contribution of this kernel to the neighbor element
  prepareVectorTagNeighbor(_assembly, _var.number());
  _local_re(0) = -r;
  accumulateTaggedLocalResidual();
}

void
PCNSFVLaxFriedrichs::computeJacobian(const FaceInfo & fi)
{
  if (skipForBoundary(fi))
    return;

  _face_info = &fi;
  _normal = fi.normal();
  const auto r = computeQpResidual();

  const auto ft = fi.faceType(_var.name());
  if (ft != FaceInfo::VarFaceNeighbors::BOTH)
    mooseError("This object should only be run on internal faces. If on a boundary face, then "
               "PCNSFVLaxFriedrichsBC should be used instead");

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");
  _assembly.processDerivatives(r, _var.dofIndices()[0], _matrix_tags);

  mooseAssert(_var.dofIndicesNeighbor().size() == 1,
              "We're currently built to use CONSTANT MONOMIALS");
  _assembly.processDerivatives(-r, _var.dofIndicesNeighbor()[0], _matrix_tags);
}

void
PCNSFVLaxFriedrichs::computeAValues()
{
  _Sf = _face_info->faceArea() * _face_info->faceCoord() * _face_info->normal();
  _vSf_elem = _superficial_vel_elem[_qp] * _Sf;
  _vSf_neighbor = _superficial_vel_neighbor[_qp] * _Sf;
  _cSf_elem = _fluid.c_from_p_T(_pressure_elem[_qp], _T_fluid_elem[_qp]) * _Sf.norm();
  _cSf_neighbor = _fluid.c_from_p_T(_pressure_neighbor[_qp], _T_fluid_neighbor[_qp]) * _Sf.norm();
  // Create this to avoid new nonzero mallocs
  const ADReal dummy_psi = 0 * (_vSf_elem + _cSf_elem + _vSf_neighbor + _cSf_neighbor);
  _psi_elem = std::max({_vSf_elem + _cSf_elem, _vSf_neighbor + _cSf_neighbor, ADReal(0)});
  _psi_elem += dummy_psi;
  _psi_neighbor = std::min({_vSf_elem - _cSf_elem, _vSf_neighbor - _cSf_neighbor, ADReal(0)});
  _psi_neighbor += dummy_psi;
  _alpha_elem = _psi_elem / (_psi_elem - _psi_neighbor);
  _alpha_neighbor = 1. - _alpha_elem;
  _psi_max = std::max(std::abs(_psi_elem), std::abs(_psi_neighbor));
  _omega = _psi_neighbor * _alpha_elem;
  _vSf_elem *= _alpha_elem;
  _vSf_neighbor *= _alpha_neighbor;
  _adjusted_vSf_elem = _vSf_elem - _omega;
  _adjusted_vSf_neighbor = _vSf_neighbor + _omega;
  _adjusted_vSf_max = std::max(std::abs(_adjusted_vSf_elem), std::abs(_adjusted_vSf_neighbor));
  _adjusted_vSf_max += 0 * (_adjusted_vSf_elem + _adjusted_vSf_neighbor);
}

ADReal
PCNSFVLaxFriedrichs::computeQpResidual()
{
  computeAValues();

  if (_eqn == "mass")
    return _adjusted_vSf_elem * _rho_elem[_qp] + _adjusted_vSf_neighbor * _rho_neighbor[_qp];
  else if (_eqn == "momentum")
    return _adjusted_vSf_elem * _rhou_elem[_qp](_index) +
           _adjusted_vSf_neighbor * _rhou_neighbor[_qp](_index) +
           (_alpha_elem * _eps_elem[_qp] * _pressure_elem[_qp] +
            _alpha_neighbor * _eps_neighbor[_qp] * _pressure_neighbor[_qp]) *
               _Sf(_index);
  else if (_eqn == "energy")
    return _adjusted_vSf_elem * _rho_ht_elem[_qp] + _adjusted_vSf_neighbor * _rho_ht_neighbor[_qp] +
           // This term removes artifacts at boundaries in the particle velocity solution. Note that
           // if instead of using pressure, one uses porosity*pressure then the solution is totally
           // wrong
           _omega * (_pressure_elem[_qp] - _pressure_neighbor[_qp]);
  else
    mooseError("Unrecognized enum type ", _eqn);
}
