//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeAnisotropicMultiGrainEigenstrain.h"
#include "RankTwoTensor.h"
#include "EulerAngleProvider.h"
#include "RotationTensor.h"

registerMooseObject("PhaseFieldApp",ComputeAnisotropicMultiGrainEigenstrain);

template <>
InputParameters
validParams<ComputeAnisotropicMultiGrainEigenstrain>()
{
  InputParameters params = validParams<ComputeEigenstrainBase>();
  params.addClassDescription(
    "Compute spatially and temporally dependent eigstrain by adding constant eigenstrain and the thermal expansion");
  // params.addCoupledVar("temperature", "Coupled temperature");
  params.addRequiredParam<MaterialPropertyName>("temperature", "Temperature");
  params.addRequiredCoupledVar("stress_free_temperature_xx",
                               "Reference temperature at which there is no "
                               "thermal expansion for thermal eigenstrain in "
                               "the xx direction "
                               "calculation");
  params.addRequiredCoupledVar("stress_free_temperature_yy",
                               "Reference temperature at which there is no "
                               "thermal expansion for thermal eigenstrain in "
                               "the yy direction "
                               "calculation");
  params.addRequiredCoupledVar("stress_free_temperature_zz",
                               "Reference temperature at which there is no "
                               "thermal expansion for thermal eigenstrain in "
                               "the zz direction "
                               "calculation");
  params.addRequiredParam<Real>("constant_eigenstrain_xx", "Constant eigenstrain along x");
  params.addRequiredParam<Real>("constant_eigenstrain_yy", "Constant eigenstrain along y");
  params.addRequiredParam<Real>("constant_eigenstrain_zz", "Constant eigenstrain along z");
  params.addRequiredParam<Real>("thermal_expansion_coeff_xx", "Thermal expansion coefficient along x");
  params.addRequiredParam<Real>("thermal_expansion_coeff_yy", "Thermal expansion coefficient along y");
  params.addRequiredParam<Real>("thermal_expansion_coeff_zz", "Thermal expansion coefficient qlong z");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  params.addRequiredParam<UserObjectName>("euler_angle_provider",
                                          "Name of Euler angle provider user object");
  params.addRequiredParam<UserObjectName>("grain_tracker",
                                          "the GrainTracker UserObject to get values from");

  return params;
}

ComputeAnisotropicMultiGrainEigenstrain::ComputeAnisotropicMultiGrainEigenstrain(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBase>(parameters),
    // _temperature(coupledValue("temperature")),
    _temperature(getMaterialProperty<Real>("temperature")),
    // _deigenstrain_dT(declarePropertyDerivative<RankTwoTensor>(_eigenstrain_name,
                                                              // getVar("temperature", 0)->name())),
    _stress_free_temperature_xx(coupledValue("stress_free_temperature_xx")),
    _stress_free_temperature_yy(coupledValue("stress_free_temperature_yy")),
    _stress_free_temperature_zz(coupledValue("stress_free_temperature_zz")),
    _constant_eigenstrain_xx(getParam<Real>("constant_eigenstrain_xx")),
    _constant_eigenstrain_yy(getParam<Real>("constant_eigenstrain_yy")),
    _constant_eigenstrain_zz(getParam<Real>("constant_eigenstrain_zz")),
    _thermal_expansion_coeff_xx(getParam<Real>("thermal_expansion_coeff_xx")),
    _thermal_expansion_coeff_yy(getParam<Real>("thermal_expansion_coeff_yy")),
    _thermal_expansion_coeff_zz(getParam<Real>("thermal_expansion_coeff_zz")),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _euler(getUserObject<EulerAngleProvider>("euler_angle_provider")),
    _grain_tracker(getUserObject<GrainTrackerInterface>("grain_tracker")) //,
    // _orientation_name("orientation_name"),//////////////////////////////////////////////////////////////////////
    // _orientation(declareProperty<Real>(_orientation_name)) ///////////////////////////////////
{
    // Loop over variables (ops)
    for (auto op_index = decltype(_op_num)(0); op_index < _op_num; ++op_index)
    {
      // Initialize variables
      _vals[op_index] = &coupledValue("v", op_index);
    }
}

void
ComputeAnisotropicMultiGrainEigenstrain::computeQpEigenstrain()
{
  Real theta1 = _constant_eigenstrain_xx + _thermal_expansion_coeff_xx * (_temperature[_qp] - _stress_free_temperature_xx[_qp]);
  Real theta2 = _constant_eigenstrain_yy + _thermal_expansion_coeff_yy * (_temperature[_qp] - _stress_free_temperature_yy[_qp]);
  Real theta3 = _constant_eigenstrain_zz + _thermal_expansion_coeff_zz * (_temperature[_qp] - _stress_free_temperature_zz[_qp]);

  RankTwoTensor I1(1, 0, 0, 0, 0, 0);
  RankTwoTensor I2(0, 1, 0, 0, 0, 0);
  RankTwoTensor I3(0, 0, 1, 0, 0, 0);

  RankTwoTensor theta0 = theta1 * I1 + theta2 * I2 + theta3 * I3;
  RankTwoTensor dtheta_dt0 = _thermal_expansion_coeff_xx * I1 + _thermal_expansion_coeff_yy * I2 +
                             _thermal_expansion_coeff_zz * I3;

  _eigenstrain[_qp].zero();
  // _deigenstrain_dT[_qp].zero();

  Real sum_h = 0.0;

  //get the vector that maps active order parameters to grain ids
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  //loop over the active OPs
  for (auto op_index = beginIndex(op_to_grains); op_index < op_to_grains.size(); ++op_index)
  {

    auto grain_id = op_to_grains[op_index];
    // _console << "printing the grain ID \n"; //////////////////////////////////////////////////////////////////////
    // _console << beginIndex(op_to_grains) << "\n";
    // _console << op_to_grains.size() << "\n";
    // _console << op_to_grains[op_index] << "\n";
    // _console << grain_id << "\n";
    // _console << _euler.getGrainNum() << "\n";
    // _console << (FeatureFloodCount::invalid_id) << "\n";
    // _console << (op_to_grains[op_index] == FeatureFloodCount::invalid_id) << "\n";
    // printf("%u", grain_id);//////////////////////////////////////////////////////////////////////
    // printf("%u", _euler.getGrainNum());//////////////////////////////////////////////////////////////////////
    if (op_to_grains[op_index] == FeatureFloodCount::invalid_id)
        continue;

    EulerAngles angles;

    //make sure you have enough Euler angles in the file and grab the right one
    if (grain_id < _euler.getGrainNum())
    {
      angles = _euler.getEulerAngles(grain_id);
    }
    else
      mooseError("ComputeAnisotropicMultiGrainEigenstrain has run out of grain rotation data.");

    //Interpolation factor for the eigenstrain tensors - this goes between 0 and 1 if eta is 0 or 1
    //Using standard PF interpolation function.
    // Real n = (*_vals[op_index])[_qp];
    // Real h = n*n*n*(6*n*n - 15*n + 10);

    RankTwoTensor theta = theta0;
    // Real angle1 = angles.phi1;//////////////////////////////////////////////////////////////////////
    RankTwoTensor dtheta_dt = dtheta_dt0;
    theta.rotate(RotationTensor(RealVectorValue(angles)));
    dtheta_dt.rotate(RotationTensor(RealVectorValue(angles)));


    // Interpolation factor for elasticity tensors
    Real h = (1.0 + std::sin(libMesh::pi * ((*_vals[op_index])[_qp] - 0.5))) / 2.0;

    RankTwoTensor local_eigenstrain;
    // Real local_orientation; //////////////////////////////////////////////////////////////////////
    // RankTwoTensor local_deigenstrain_dT;

    local_eigenstrain = theta * h;
    // local_orientation = angle1 * h;//////////////////////////////////////////////////////////////////////
    // local_deigenstrain_dT = dtheta_dt * h;

    // local_eigenstrain.rotate(RotationTensor(RealVectorValue(angles)));
    // local_deigenstrain_dT.rotate(RotationTensor(RealVectorValue(angles)));

    _eigenstrain[_qp] += local_eigenstrain;
    // _orientation[_qp] += local_orientation;//////////////////////////////////////////////////////////////////////
    // _deigenstrain_dT[_qp] += local_deigenstrain_dT;

    sum_h += h;
  }

  //Normalize the tensor to a sum of order parameters = 1...
  //anything divided by near-zero is going to explode
  const Real tol = 1.0e-10;
  sum_h = std::max(sum_h, tol);

  _eigenstrain[_qp] /= sum_h;
  // _orientation[_qp] /= sum_h; //////////////////////////////////////////////////////////////////////
  // _deigenstrain_dT[_qp] /= sum_h;
}
