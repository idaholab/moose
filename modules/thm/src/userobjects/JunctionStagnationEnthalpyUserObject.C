#include "JunctionStagnationEnthalpyUserObject.h"
#include "libmesh/quadrature.h"

registerMooseObject("THMApp", JunctionStagnationEnthalpyUserObject);

template <>
InputParameters
validParams<JunctionStagnationEnthalpyUserObject>()
{
  InputParameters params = validParams<FlowJunctionUserObject>();

  params.addRequiredCoupledVar("A", "Cross-sectional flow channel area");
  params.addRequiredCoupledVar("rhoA", "rho*A");
  params.addRequiredCoupledVar("rhouA", "rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "rho*E*A");

  params.addRequiredParam<MaterialPropertyName>("rho", "Density material property");
  params.addRequiredParam<MaterialPropertyName>("vel", "Velocity material property");
  params.addRequiredParam<MaterialPropertyName>("H", "Stagnation enthalpy material property");

  return params;
}

JunctionStagnationEnthalpyUserObject::JunctionStagnationEnthalpyUserObject(
    const InputParameters & parameters)
  : DerivativeMaterialInterfaceTHM<FlowJunctionUserObject>(parameters),

    _dH_junction_drhoA(_n_bnd_ids, 0),
    _dH_junction_drhouA(_n_bnd_ids, 0),
    _dH_junction_drhoEA(_n_bnd_ids, 0),

    _dH_sum_drhoA(_n_bnd_ids, 0),
    _dH_sum_drhouA(_n_bnd_ids, 0),
    _dH_sum_drhoEA(_n_bnd_ids, 0),

    _denergy_rate_in_drhoA(_n_bnd_ids, 0),
    _denergy_rate_in_drhouA(_n_bnd_ids, 0),
    _denergy_rate_in_drhoEA(_n_bnd_ids, 0),

    _dmass_flow_rate_in_drhoA(_n_bnd_ids, 0),
    _dmass_flow_rate_in_drhouA(_n_bnd_ids, 0),

    _A(coupledValue("A")),
    _rhouA_old(coupledValueOld("rhouA")),

    _rho(getMaterialProperty<Real>("rho")),
    _drho_drhoA(getMaterialPropertyDerivativeTHM<Real>("rho", "rhoA")),

    _vel(getMaterialProperty<Real>("vel")),
    _dvel_drhoA(getMaterialPropertyDerivativeTHM<Real>("vel", "rhoA")),
    _dvel_drhouA(getMaterialPropertyDerivativeTHM<Real>("vel", "rhouA")),

    _H(getMaterialProperty<Real>("H")),
    _dH_drhoA(getMaterialPropertyDerivativeTHM<Real>("H", "rhoA")),
    _dH_drhouA(getMaterialPropertyDerivativeTHM<Real>("H", "rhouA")),
    _dH_drhoEA(getMaterialPropertyDerivativeTHM<Real>("H", "rhoEA"))
{
}

void
JunctionStagnationEnthalpyUserObject::initialize()
{
  _H_sum = 0;
  _energy_rate_in = 0;
  _mass_flow_rate_in = 0;

  std::fill(_dH_junction_drhoA.begin(), _dH_junction_drhoA.end(), 0);
  std::fill(_dH_junction_drhouA.begin(), _dH_junction_drhouA.end(), 0);
  std::fill(_dH_junction_drhoEA.begin(), _dH_junction_drhoEA.end(), 0);

  std::fill(_dH_sum_drhoA.begin(), _dH_sum_drhoA.end(), 0);
  std::fill(_dH_sum_drhouA.begin(), _dH_sum_drhouA.end(), 0);
  std::fill(_dH_sum_drhoEA.begin(), _dH_sum_drhoEA.end(), 0);

  std::fill(_denergy_rate_in_drhoA.begin(), _denergy_rate_in_drhoA.end(), 0);
  std::fill(_denergy_rate_in_drhouA.begin(), _denergy_rate_in_drhouA.end(), 0);
  std::fill(_denergy_rate_in_drhoEA.begin(), _denergy_rate_in_drhoEA.end(), 0);

  std::fill(_dmass_flow_rate_in_drhoA.begin(), _dmass_flow_rate_in_drhoA.end(), 0);
  std::fill(_dmass_flow_rate_in_drhouA.begin(), _dmass_flow_rate_in_drhouA.end(), 0);
}

void
JunctionStagnationEnthalpyUserObject::execute()
{
  const unsigned int j = getBoundaryIDIndex();

  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    const Real n = _normals[qp](0);
    const Real JxW = _coord[qp] * _JxW[qp];

    // numerator of H_junction if no flow into junction
    _H_sum += _H[qp] * JxW;
    _dH_sum_drhoA[j] += _dH_drhoA[qp] * JxW;
    _dH_sum_drhouA[j] += _dH_drhouA[qp] * JxW;
    _dH_sum_drhoEA[j] += _dH_drhoEA[qp] * JxW;

    if (_rhouA_old[qp] * n > 0)
    {
      // numerator of H_junction
      _energy_rate_in += _rho[qp] * _vel[qp] * _H[qp] * _A[qp] * n * JxW;
      _denergy_rate_in_drhoA[j] +=
          (_drho_drhoA[qp] * _vel[qp] * _H[qp] + _rho[qp] * _dvel_drhoA[qp] * _H[qp] +
           _rho[qp] * _vel[qp] * _dH_drhoA[qp]) *
          _A[qp] * n * JxW;
      _denergy_rate_in_drhouA[j] +=
          _rho[qp] * (_dvel_drhouA[qp] * _H[qp] + _vel[qp] * _dH_drhouA[qp]) * _A[qp] * n * JxW;
      _denergy_rate_in_drhoEA[j] += _rho[qp] * _vel[qp] * _dH_drhoEA[qp] * _A[qp] * n * JxW;

      // denominator of H_junction
      _mass_flow_rate_in += _rho[qp] * _vel[qp] * _A[qp] * n * JxW;
      _dmass_flow_rate_in_drhoA[j] +=
          (_drho_drhoA[qp] * _vel[qp] + _rho[qp] * _dvel_drhoA[qp]) * _A[qp] * n * JxW;
      _dmass_flow_rate_in_drhouA[j] += _rho[qp] * _dvel_drhouA[qp] * _A[qp] * n * JxW;
    }
  }
}

void
JunctionStagnationEnthalpyUserObject::finalize()
{
  gatherSum(_H_sum);
  gatherSum(_energy_rate_in);
  gatherSum(_mass_flow_rate_in);

  if (std::fabs(_mass_flow_rate_in) <
      1e-10) // no flow into junction; compute H_junction as average H
  {
    _H_junction = _H_sum / _n_bnd_ids;
    for (unsigned int j = 0; j < _n_bnd_ids; j++)
    {
      _dH_junction_drhoA[j] = _dH_sum_drhoA[j] / _n_bnd_ids;
      _dH_junction_drhouA[j] = _dH_sum_drhouA[j] / _n_bnd_ids;
      _dH_junction_drhoEA[j] = _dH_sum_drhoEA[j] / _n_bnd_ids;
    }
  }
  else // some flow into junction
  {
    _H_junction = _energy_rate_in / _mass_flow_rate_in;
    for (unsigned int j = 0; j < _n_bnd_ids; j++)
    {
      _dH_junction_drhoA[j] = _denergy_rate_in_drhoA[j] / _mass_flow_rate_in -
                              _energy_rate_in / (_mass_flow_rate_in * _mass_flow_rate_in) *
                                  _dmass_flow_rate_in_drhoA[j];
      _dH_junction_drhouA[j] = _denergy_rate_in_drhouA[j] / _mass_flow_rate_in -
                               _energy_rate_in / (_mass_flow_rate_in * _mass_flow_rate_in) *
                                   _dmass_flow_rate_in_drhouA[j];
      _dH_junction_drhoEA[j] = _denergy_rate_in_drhoEA[j] / _mass_flow_rate_in;
    }
  }
}

void
JunctionStagnationEnthalpyUserObject::threadJoin(const UserObject & uo)
{
  const JunctionStagnationEnthalpyUserObject & pp =
      static_cast<const JunctionStagnationEnthalpyUserObject &>(uo);
  _H_sum += pp._H_sum;
  _energy_rate_in += pp._energy_rate_in;
  _mass_flow_rate_in += pp._mass_flow_rate_in;
}

Real
JunctionStagnationEnthalpyUserObject::getJunctionStagnationEnthalpy() const
{
  return _H_junction;
}

const std::vector<Real> &
JunctionStagnationEnthalpyUserObject::getJunctionStagnationEnthalpyMassDerivatives() const
{
  return _dH_junction_drhoA;
}

const std::vector<Real> &
JunctionStagnationEnthalpyUserObject::getJunctionStagnationEnthalpyMomentumDerivatives() const
{
  return _dH_junction_drhouA;
}

const std::vector<Real> &
JunctionStagnationEnthalpyUserObject::getJunctionStagnationEnthalpyEnergyDerivatives() const
{
  return _dH_junction_drhoEA;
}
