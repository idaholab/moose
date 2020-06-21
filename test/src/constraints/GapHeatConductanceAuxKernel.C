//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapHeatConductanceAuxKernel.h"

registerMooseObject("MooseTestApp", GapHeatConductanceAuxKernel);

InputParameters
GapHeatConductanceAuxKernel::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addParam<Real>("gap_conductance_constant",
                        0.03,
                        "The numerator in calculation of the heat transfer coefficient");
  params.addParam<MaterialPropertyName>(
      "slave_gap_conductance",
      "gap_conductance",
      "The material property name providing the gap conductance on the slave side");
  params.addParam<MaterialPropertyName>(
      "master_gap_conductance",
      "gap_conductance",
      "The material property name providing the gap conductance on the master side");
  params.addRequiredCoupledVar(
      "auxkernel_variable", "The auxiliary kernel variable that uses a lower-d for compute values");

  return params;
}

GapHeatConductanceAuxKernel::GapHeatConductanceAuxKernel(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _slave_gap_conductance(getADMaterialProperty<Real>("slave_gap_conductance")),
    _master_gap_conductance(getNeighborADMaterialProperty<Real>("master_gap_conductance")),
    _aux_variable(coupledValue("auxkernel_variable")),
    _aux_variable_old(coupledValueOld("auxkernel_variable"))
{
}

ADReal
GapHeatConductanceAuxKernel::computeQpResidual(Moose::MortarType type)
{
  switch (type)
  {
    case Moose::MortarType::Slave:
      return _lambda[_qp] * _test_slave[_i][_qp];

    case Moose::MortarType::Master:
      return -_lambda[_qp] * _test_master[_i][_qp];

    case Moose::MortarType::Lower:
    {
      ADReal heat_transfer_coeff(0);
      if (_has_master)
      {
        auto gap = (_phys_points_slave[_qp] - _phys_points_master[_qp]).norm();
        mooseAssert(MetaPhysicL::raw_value(gap) > TOLERANCE * TOLERANCE,
                    "Gap distance is too small in GapHeatConductanceAuxKernel");

        heat_transfer_coeff =
            (0.5 * (_slave_gap_conductance[_qp] + _master_gap_conductance[_qp])) / gap;
      }

      // Modify heat transfer coefficient with auxiliary kernel variable
      if (_aux_variable[_qp] != 0.0)
        heat_transfer_coeff *= 0.5 * (_aux_variable[_qp] + _aux_variable_old[_qp]);

      return _test[_i][_qp] *
             (_lambda[_qp] -
              heat_transfer_coeff * (_u_slave[_qp] - (_has_master ? _u_master[_qp] : 0)));
    }

    default:
      return 0;
  }
}
