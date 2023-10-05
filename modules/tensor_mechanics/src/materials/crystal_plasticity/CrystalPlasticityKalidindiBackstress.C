//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/*
   Constitutive model form:
   Reference: Kalidindi, S. R., Bronkhorst, C. A., & Anand, L. (1992). Crystallographic texture evolution in bulk deformation processing of FCC metals. Journal of the Mechanics and Physics of Solids, 40(3), 537-569.

   Plus an Armstrong-Frederick backstress term:
   Reference: Armstrong, P. J., & Frederick, C. O. (1966). A mathematical representation of the multiaxial Bauschinger effect (Vol. 731). Berkeley, CA: Berkeley Nuclear Laboratories.

   The flow rule of the shearing slip rate:
   \dot{\gamma}^\alpha = \dot{\gamma}_o * |(\tau^\alpha - \chi^\alpha) / g^\alpha|^M * sgn(\tau^\alpha - \chi^\alpha)

   Armstrong-Frederick backstress term:
   \dot{\chi}^\alpha = h * \dot{\gamma}^\alpha - r * |\dot{\gamma}^\alpha| * \chi^\alpha

   Reference: https://github.com/ngrilli/c_pfor_am/blob/main/src/materials/CrystalPlasticityDislocationUpdate.C
*/

#include "CrystalPlasticityKalidindiBackstress.h"

registerMooseObject("TensorMechanicsApp", CrystalPlasticityKalidindiBackstress);
 
InputParameters
CrystalPlasticityKalidindiBackstress::validParams()
{
  InputParameters params = CrystalPlasticityKalidindiUpdate::validParams();
  params.addClassDescription("Kalidindi version of homogeneous crystal plasticity considering backstress term.");
  params.addParam<Real>("h",0.0,"Direct hardening coefficient for backstress");
  params.addParam<Real>("h_D",0.0,"Dynamic recovery coefficient for backstress");

  return params;
}

CrystalPlasticityKalidindiBackstress::CrystalPlasticityKalidindiBackstress(
    const InputParameters & parameters)
  : CrystalPlasticityKalidindiUpdate(parameters),

    // Backstress parameters
    _h(getParam<Real>("h")),
    _h_D(getParam<Real>("h_D")),

    // Backstress variable
    _backstress(declareProperty<std::vector<Real>>("backstress")),
    _backstress_old(getMaterialPropertyOld<std::vector<Real>>("backstress")),
    _backstress_increment(_number_slip_systems, 0.0),

    // resize local caching vectors used for substepping
    _previous_substep_backstress(_number_slip_systems, 0.0),
    _backstress_before_update(_number_slip_systems, 0.0)
{
}

void
CrystalPlasticityKalidindiBackstress::initQpStatefulProperties()
{
  // Initialize _tua, schmit tensor, g^\alpha, gamma^\alpha
  CrystalPlasticityKalidindiUpdate::initQpStatefulProperties();

  // Initialize the backstress size
  _backstress[_qp].resize(_number_slip_systems);
  for (const auto i : make_range(_number_slip_systems))
    _backstress[_qp][i] = 0.0;
}

void
CrystalPlasticityKalidindiBackstress::setInitialConstitutiveVariableValues()
{
  CrystalPlasticityKalidindiUpdate::setInitialConstitutiveVariableValues();

  _backstress[_qp] = _backstress_old[_qp];
  _previous_substep_backstress = _backstress_old[_qp];
}

void
CrystalPlasticityKalidindiBackstress::setSubstepConstitutiveVariableValues()
{
  CrystalPlasticityKalidindiUpdate::setSubstepConstitutiveVariableValues();

  _backstress[_qp] = _previous_substep_backstress;
}

bool
CrystalPlasticityKalidindiBackstress::calculateSlipRate()
{
  // Difference between RSS and backstress temporary variable for each slip system
  Real effective_stress;

  for (const auto i : make_range(_number_slip_systems))
  {
    effective_stress = _tau[_qp][i] - _backstress[_qp][i];

    _slip_increment[_qp][i] =
        _ao * std::pow(std::abs(effective_stress / _slip_resistance[_qp][i]), 1.0 / _xm);
    if (effective_stress < 0.0)
      _slip_increment[_qp][i] *= -1.0;

    if (std::abs(_slip_increment[_qp][i]) * _substep_dt > _slip_incr_tol)
    {
      if (_print_convergence_message)
        mooseWarning("Maximum allowable slip increment exceeded ",
                     std::abs(_slip_increment[_qp][i]) * _substep_dt);

      return false;
    }
  }
  return true;
}

void
CrystalPlasticityKalidindiBackstress::calculateConstitutiveSlipDerivative(
    std::vector<Real> & dslip_dtau)
{
  Real effective_stress;

  for (const auto i : make_range(_number_slip_systems))
  {
    effective_stress = _tau[_qp][i] - _backstress[_qp][i];

    if (MooseUtils::absoluteFuzzyEqual(effective_stress, 0.0))
      dslip_dtau[i] = 0.0;
    else
      dslip_dtau[i] = _ao / _xm *
                      std::pow(std::abs(effective_stress / _slip_resistance[_qp][i]), 1.0 / _xm - 1.0) /
                      _slip_resistance[_qp][i];
  }
}

bool
CrystalPlasticityKalidindiBackstress::areConstitutiveStateVariablesConverged()
{
  return isConstitutiveStateVariableConverged(_slip_resistance[_qp],
                                              _slip_resistance_before_update,
                                              _previous_substep_slip_resistance,
                                              _resistance_tol);                                        
}

void
CrystalPlasticityKalidindiBackstress::updateSubstepConstitutiveVariableValues()
{
  _previous_substep_slip_resistance = _slip_resistance[_qp];
  _previous_substep_backstress = _backstress[_qp];
}

void
CrystalPlasticityKalidindiBackstress::cacheStateVariablesBeforeUpdate()
{
  _slip_resistance_before_update = _slip_resistance[_qp];
  _backstress_before_update = _backstress[_qp];
}

void
CrystalPlasticityKalidindiBackstress::calculateStateVariableEvolutionRateComponent()
{
  CrystalPlasticityKalidindiUpdate::calculateStateVariableEvolutionRateComponent();
  
  ArmstrongFrederickBackstressUpdate();
}

bool
CrystalPlasticityKalidindiBackstress::updateStateVariables()
{
  // Now perform the check to see if the slip system should be updated
  for (const auto i : make_range(_number_slip_systems))
  {
    _slip_resistance_increment[i] *= _substep_dt;
    if (_previous_substep_slip_resistance[i] < _zero_tol && _slip_resistance_increment[i] < 0.0)
      _slip_resistance[_qp][i] = _previous_substep_slip_resistance[i];
    else
      _slip_resistance[_qp][i] =
          _previous_substep_slip_resistance[i] + _slip_resistance_increment[i];

    if (_slip_resistance[_qp][i] < 0.0)
      return false;
  }

  // Backstress: can be both positive or negative
  for (const auto i : make_range(_number_slip_systems))
  { 
    _backstress_increment[i] *= _substep_dt;
    _backstress[_qp][i] = _previous_substep_backstress[i] + _backstress_increment[i];
  }

  return true;
}

/*
   Update of the Armstrong-Frederick backstress term.
   Reference: Armstrong, P.J., Frederick, C.O., 1966.
   G.E.G.B. Report RD/B/N. Central Electricity Generating Board.
   Backstress term modified in this function.
*/
void
CrystalPlasticityKalidindiBackstress::ArmstrongFrederickBackstressUpdate()
{
  for (const auto i : make_range(_number_slip_systems)) 
  {
    _backstress_increment[i] = _h * _slip_increment[_qp][i];
    _backstress_increment[i] -= _h_D * _backstress[_qp][i] * std::abs(_slip_increment[_qp][i]);  
  }
}