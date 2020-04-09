//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrystalPlasticityStateVarRateComponentVoce.h"
#include "MooseError.h"

registerMooseObject("TensorMechanicsApp", CrystalPlasticityStateVarRateComponentVoce);

InputParameters
CrystalPlasticityStateVarRateComponentVoce::validParams()
{

  InputParameters params = CrystalPlasticityStateVarRateComponent::validParams();
  params.addParam<std::string>(
      "uo_slip_rate_name",
      "Name of slip rate property: Same as slip rate user object specified in input file.");
  params.addParam<std::string>("uo_state_var_name",
                               "Name of state variable property: Same as "
                               "state variable user object specified in input "
                               "file.");
  params.addParam<MooseEnum>(
      "crystal_lattice_type",
      CrystalPlasticityStateVarRateComponentVoce::crystalLatticeTypeOptions(),
      "Type of crystal lattyce structure output");
  params.addParam<std::vector<unsigned int>>("groups",
                                             "To group the initial values on different "
                                             "slip systems 'format: [start end)', i.e.'0 "
                                             "12 24 48' groups 0-11, 12-23 and 24-48 ");
  params.addParam<std::vector<Real>>("h0_group_values",
                                     "h0 hardening constant for each group "
                                     " i.e. '0.0 1.0 2.0' means 0-11 = 0.0, "
                                     "12-23 = 1.0 and 24-48 = 2.0 ");
  params.addParam<std::vector<Real>>("tau0_group_values",
                                     "The initial critical resolved shear stress"
                                     "corresponding to each group"
                                     " i.e. '100.0 110.0 120.0' means 0-11 = 100.0, "
                                     "12-23 = 110.0 and 24-48 = 120.0 ");
  params.addParam<std::vector<Real>>("tauSat_group_values",
                                     "The saturation resolved shear stress"
                                     "corresponding to each group"
                                     " i.e. '150.0 170.0 180.0' means 0-11 = 150.0, "
                                     "12-23 = 170.0 and 24-48 = 180.0 ");
  params.addParam<std::vector<Real>>("hardeningExponent_group_values",
                                     "The hardening exponent m"
                                     "corresponding to each group"
                                     " i.e. '1.0 2.0 3.0' means 0-11 = 1.0, "
                                     "12-23 = 2.0 and 24-48 = 3.0 ");
  params.addParam<std::vector<Real>>("selfHardening_group_values",
                                     "The self hardening coefficient q_aa"
                                     "corresponding to each group"
                                     " i.e. '1.0 2.0 3.0' means 0-11 = 1.0, "
                                     "12-23 = 2.0 and 24-48 = 3.0 "
                                     " usually these are all 1.");
  params.addParam<std::vector<Real>>("coplanarHardening_group_values",
                                     "The coplanar latent hardening coefficient q_ab"
                                     "corresponding to each group"
                                     " i.e. '1.0 2.0 3.0' means 0-11 = 1.0, "
                                     "12-23 = 2.0 and 24-48 = 3.0 ");
  params.addParam<std::vector<Real>>("GroupGroup_Hardening_group_values",
                                     "The group-to-group latent hardening coefficient q_ab"
                                     "This is a NxN vector"
                                     " i.e. '1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0 9.0' "
                                     "means non-coplanar slip systems in gr_11,22,33= "
                                     "1.0, 5.0 and 9.0 respectively."
                                     "latent hardening between for gr_12,13 = 2.0 3.0"
                                     " respectively");
  params.addClassDescription("Phenomenological Voce constitutive model state variable evolution "
                             "rate component base class.");
  return params;
}

CrystalPlasticityStateVarRateComponentVoce::CrystalPlasticityStateVarRateComponentVoce(
    const InputParameters & parameters)
  : CrystalPlasticityStateVarRateComponent(parameters),
    _mat_prop_slip_rate(
        getMaterialProperty<std::vector<Real>>(parameters.get<std::string>("uo_slip_rate_name"))),
    _mat_prop_state_var(
        getMaterialProperty<std::vector<Real>>(parameters.get<std::string>("uo_state_var_name"))),
    _crystal_lattice_type(getParam<MooseEnum>("crystal_lattice_type")),
    _groups(getParam<std::vector<unsigned int>>("groups")),
    _h0_group_values(getParam<std::vector<Real>>("h0_group_values")),
    _tau0_group_values(getParam<std::vector<Real>>("tau0_group_values")),
    _tauSat_group_values(getParam<std::vector<Real>>("tauSat_group_values")),
    _hardeningExponent_group_values(getParam<std::vector<Real>>("hardeningExponent_group_values")),
    _selfHardening_group_values(getParam<std::vector<Real>>("selfHardening_group_values")),
    _coplanarHardening_group_values(getParam<std::vector<Real>>("coplanarHardening_group_values")),
    _GroupGroup_Hardening_group_values(
        getParam<std::vector<Real>>("GroupGroup_Hardening_group_values")),
    _n_groups(_groups.size())
{
  // perform input checks
  if (_n_groups < 2)
    paramError("groups",
               "the number of slip system groups provided is not "
               "correct. At least two values are expected");

  // check the size of all the user provided parameters
  if (_h0_group_values.size() != _n_groups - 1)
    paramError("h0_group_values",
               "the number of supplied parameters does not"
               " match the number of ip system groups");

  if (_tau0_group_values.size() != _n_groups - 1)
    paramError("tau0_group_values",
               "the number of supplied parameters does "
               "not match the number of slip system groups");

  if (_tauSat_group_values.size() != _n_groups - 1)
    paramError("tauSat_group_values",
               "the number of supplied parameters does "
               "not match the number of slip system groups");

  if (_hardeningExponent_group_values.size() != _n_groups - 1)
    paramError("hardeningExponent_group_values",
               "the number of supplied "
               "parameters does not match the number of slip system groups");

  if (_selfHardening_group_values.size() != _n_groups - 1)
    paramError("selfHardening_group_values",
               "the number of supplied parameters "
               "does not match the number of slip system groups");

  if (_coplanarHardening_group_values.size() != _n_groups - 1)
    paramError("coplanarHardening_group_values",
               "the number of supplied "
               "parameters does not match the number of slip system groups");

  if (_GroupGroup_Hardening_group_values.size() != (_n_groups - 1) * (_n_groups - 1))
    paramError("GroupGroup_Hardening_group_values",
               "the number of supplied "
               "parameters does not match the number of slip system groups");

  // initialize useful variables;
  initSlipSystemPlaneID(_slipSystem_PlaneID);
  initSlipSystemGroupID(_slipSystem_GroupID);
}

bool
CrystalPlasticityStateVarRateComponentVoce::calcStateVariableEvolutionRateComponent(
    unsigned int qp, std::vector<Real> & val) const
{
  val.assign(_variable_size, 0.0);

  unsigned int group_i;
  Real h0;
  Real tau_0;
  Real tau_sat;
  Real hardening_exponenet;
  Real delta_tau;

  DenseVector<Real> hb(_variable_size);

  for (unsigned int i = 0; i < _variable_size; ++i)
  {
    group_i = _slipSystem_GroupID[i];
    h0 = _h0_group_values[group_i];
    tau_0 = _tau0_group_values[group_i];
    tau_sat = _tauSat_group_values[group_i];
    hardening_exponenet = _hardeningExponent_group_values[group_i];

    delta_tau = tau_sat - tau_0;

    hb(i) = h0 *
            std::pow(std::abs(1.0 - (_mat_prop_state_var[qp][i] - tau_0) / delta_tau),
                     hardening_exponenet) *
            std::copysign(1.0, 1.0 - (_mat_prop_state_var[qp][i] - tau_0) / delta_tau);
  }

  for (unsigned int i = 0; i < _variable_size; ++i)
    for (unsigned int j = 0; j < _variable_size; ++j)
    {
      const Real q_ab = getHardeningCoefficient(i, j);
      val[i] += std::abs(_mat_prop_slip_rate[qp][j]) * q_ab * hb(j);
    }

  return true;
}

MooseEnum
CrystalPlasticityStateVarRateComponentVoce::crystalLatticeTypeOptions()
{
  return MooseEnum("FCC BCC", "FCC");
}

void
CrystalPlasticityStateVarRateComponentVoce::initSlipSystemPlaneID(
    std::vector<unsigned int> & _slipSystem_PlaneID) const
{
  _slipSystem_PlaneID.assign(_variable_size, 0);

  for (unsigned int slipSystemIndex = 0; slipSystemIndex < _variable_size; ++slipSystemIndex)
    switch (_crystal_lattice_type)
    {
      case 0: // FCC
        if (slipSystemIndex < 12)
          _slipSystem_PlaneID[slipSystemIndex] = slipSystemIndex / 3;
        else
          mooseError("FCC with more than 12 slip planes is not implemented ");

        break;

      case 1: // BCC
        if (slipSystemIndex < 12)
          _slipSystem_PlaneID[slipSystemIndex] = slipSystemIndex / 2;

        else if (slipSystemIndex >= 12 && slipSystemIndex < 48)
          _slipSystem_PlaneID[slipSystemIndex] = (slipSystemIndex - 6);

        else
          mooseError("BCC with more than 48 slip systems is not implemented ");

        break;

      default:
        mooseError("VoceHardeningError: Pass valid crustal_structure_type ");
    }
}

void
CrystalPlasticityStateVarRateComponentVoce::initSlipSystemGroupID(
    std::vector<unsigned int> & _slipSystem_GroupID) const
{
  _slipSystem_GroupID.assign(_variable_size, 0);

  for (unsigned int slipSystemIndex = 0; slipSystemIndex < _variable_size; ++slipSystemIndex)
    for (unsigned int i = 0; i < _n_groups - 1; ++i)
      if (slipSystemIndex >= _groups[i] && slipSystemIndex < _groups[i + 1])
      {
        _slipSystem_GroupID[slipSystemIndex] = i;
        break;
      }
}

Real
CrystalPlasticityStateVarRateComponentVoce::getHardeningCoefficient(
    unsigned int slipSystemIndex_i, unsigned int slipSystemIndex_j) const
{
  // collect slip system plane and group
  const unsigned int group_i = _slipSystem_GroupID[slipSystemIndex_i];
  const unsigned int group_j = _slipSystem_GroupID[slipSystemIndex_j];
  const unsigned int plane_i = _slipSystem_PlaneID[slipSystemIndex_i];
  const unsigned int plane_j = _slipSystem_PlaneID[slipSystemIndex_j];

  // create check for clarity
  const bool same_slipSystem = slipSystemIndex_i == slipSystemIndex_j;
  const bool same_group = group_i == group_j;
  const bool same_plane = plane_i == plane_j;

  // retrieve appropriate coefficient
  Real q_ab;
  if (same_slipSystem)
    q_ab = _selfHardening_group_values[group_i];
  else if (same_plane)
    q_ab = _coplanarHardening_group_values[group_i];
  else if (same_group) // here for debugging purposes this if could be removed
    q_ab = _GroupGroup_Hardening_group_values[group_i * (_n_groups - 1) + group_i];
  else if (!same_group)
    q_ab = _GroupGroup_Hardening_group_values[group_i * (_n_groups - 1) + group_j];
  else // here for debugging purposes
    mooseError("VoceHardeningError:getHardeningCoefficient: case not listed, abort ");

  return q_ab;
}
