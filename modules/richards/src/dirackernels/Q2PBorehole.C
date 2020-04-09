//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Q2PBorehole.h"
#include "RotationMatrix.h"

registerMooseObject("RichardsApp", Q2PBorehole);

InputParameters
Q2PBorehole::validParams()
{
  InputParameters params = PeacemanBorehole::validParams();
  params.addRequiredParam<UserObjectName>(
      "fluid_density",
      "A RichardsDensity UserObject that defines the fluid density as a function of pressure.");
  params.addRequiredParam<UserObjectName>(
      "fluid_relperm",
      "A RichardsRelPerm UserObject (eg RichardsRelPermPower) that defines the "
      "fluid relative permeability as a function of the saturation Variable.");
  params.addRequiredCoupledVar("other_var",
                               "The other variable in the 2-phase system.  If "
                               "Variable=porepressure, the other_var=saturation, and "
                               "vice-versa.");
  params.addRequiredParam<bool>("var_is_porepressure",
                                "This flag is needed to correctly calculate the Jacobian entries.  "
                                "If set to true, this Sink will extract fluid from the phase with "
                                "porepressure as its Variable (usually the liquid phase).  If set "
                                "to false, this Sink will extract fluid from the phase with "
                                "saturation as its variable (usually the gas phase)");
  params.addRequiredParam<Real>("fluid_viscosity", "The fluid dynamic viscosity");
  params.addClassDescription("Approximates a borehole in the mesh with given bottomhole pressure, "
                             "and radii using a number of point sinks whose positions are read "
                             "from a file.  This DiracKernel is for use by Q2P models");
  return params;
}

Q2PBorehole::Q2PBorehole(const InputParameters & parameters)
  : PeacemanBorehole(parameters),
    _density(getUserObject<RichardsDensity>("fluid_density")),
    _relperm(getUserObject<RichardsRelPerm>("fluid_relperm")),
    _other_var_nodal(coupledDofValues("other_var")),
    _other_var_num(coupled("other_var")),
    _var_is_pp(getParam<bool>("var_is_porepressure")),
    _viscosity(getParam<Real>("fluid_viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _num_nodes(0),
    _pp(0),
    _sat(0),
    _mobility(0),
    _dmobility_dp(0),
    _dmobility_ds(0)
{
}

void
Q2PBorehole::prepareNodalValues()
{
  _num_nodes = _other_var_nodal.size();

  // set _pp and _sat variables
  _pp.resize(_num_nodes);
  _sat.resize(_num_nodes);
  if (_var_is_pp)
  {
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
    {
      _pp[nodenum] = _var.dofValues()[nodenum];
      _sat[nodenum] = _other_var_nodal[nodenum];
    }
  }
  else
  {
    for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
    {
      _pp[nodenum] = _other_var_nodal[nodenum];
      _sat[nodenum] = _var.dofValues()[nodenum];
    }
  }

  Real density;
  Real ddensity_dp;
  Real relperm;
  Real drelperm_ds;
  _mobility.resize(_num_nodes);
  _dmobility_dp.resize(_num_nodes);
  _dmobility_ds.resize(_num_nodes);
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    density = _density.density(_pp[nodenum]);
    ddensity_dp = _density.ddensity(_pp[nodenum]);
    relperm = _relperm.relperm(_sat[nodenum]);
    drelperm_ds = _relperm.drelperm(_sat[nodenum]);
    _mobility[nodenum] = density * relperm / _viscosity;
    _dmobility_dp[nodenum] = ddensity_dp * relperm / _viscosity;
    _dmobility_ds[nodenum] = density * drelperm_ds / _viscosity;
  }
}

void
Q2PBorehole::computeResidual()
{
  prepareNodalValues();
  DiracKernel::computeResidual();
}

Real
Q2PBorehole::computeQpResidual()
{
  const Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return 0.0;

  const Real bh_pressure =
      _p_bot +
      _unit_weight *
          (_q_point[_qp] -
           _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  // Get the ID we initially assigned to this point
  const unsigned current_dirac_ptid = currentPointCachedID();

  // If getting the ID failed, fall back to the old bodge!
  // if (current_dirac_ptid == libMesh::invalid_uint)
  //  current_dirac_ptid = (_zs.size() > 2) ? 1 : 0;

  Real outflow(0.0); // this is the flow rate from porespace out of the system

  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point (must have >1 point for
  // current_dirac_ptid>0)
  {
    wc = wellConstant(_permeability[0],
                      _rot_matrix[current_dirac_ptid - 1],
                      _half_seg_len[current_dirac_ptid - 1],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow +=
          _test[_i][_qp] * std::abs(character) * wc * _mobility[_i] * (_pp[_i] - bh_pressure);
  }

  if (current_dirac_ptid + 1 < _zs.size() || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point, or we only have one point
  {
    wc = wellConstant(_permeability[0],
                      _rot_matrix[current_dirac_ptid],
                      _half_seg_len[current_dirac_ptid],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow +=
          _test[_i][_qp] * std::abs(character) * wc * _mobility[_i] * (_pp[_i] - bh_pressure);
  }

  _total_outflow_mass.add(
      outflow * _dt); // this is not thread safe, but DiracKernel's aren't currently threaded
  return outflow;
}

void
Q2PBorehole::computeJacobian()
{
  prepareNodalValues();
  DiracKernel::computeJacobian();
}

Real
Q2PBorehole::computeQpJacobian()
{
  return jac(_var.number());
}

Real
Q2PBorehole::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _other_var_num || jvar == _var.number())
    return jac(jvar);
  return 0.0;
}

Real
Q2PBorehole::jac(unsigned int jvar)
{
  if (_i != _j)
    return 0.0;

  const Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return 0.0;

  const Real bh_pressure =
      _p_bot +
      _unit_weight *
          (_q_point[_qp] -
           _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  const Real phi = 1;

  // Get the ID we initially assigned to this point
  const unsigned current_dirac_ptid = currentPointCachedID();

  // If getting the ID failed, fall back to the old bodge!
  // if (current_dirac_ptid == libMesh::invalid_uint)
  //  current_dirac_ptid = (_zs.size() > 2) ? 1 : 0;

  Real outflowp(0.0);

  const bool deriv_wrt_pp =
      (_var_is_pp && (jvar == _var.number())) || (!_var_is_pp && (jvar == _other_var_num));

  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point
  {
    wc = wellConstant(_permeability[0],
                      _rot_matrix[current_dirac_ptid - 1],
                      _half_seg_len[current_dirac_ptid - 1],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
    {
      // injection, so outflow<0 || // production, so outflow>0
      if (deriv_wrt_pp)
        outflowp += std::abs(character) * wc *
                    (_mobility[_i] * phi + _dmobility_dp[_i] * phi * (_pp[_i] - bh_pressure));
      else
        outflowp += std::abs(character) * wc * _dmobility_ds[_i] * phi * (_pp[_i] - bh_pressure);
    }
  }

  if (current_dirac_ptid < _zs.size() - 1 || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point
  {
    wc = wellConstant(_permeability[0],
                      _rot_matrix[current_dirac_ptid],
                      _half_seg_len[current_dirac_ptid],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && _pp[_i] < bh_pressure) || (character > 0.0 && _pp[_i] > bh_pressure))
    {
      // injection, so outflow<0 || // production, so outflow>0
      if (deriv_wrt_pp)
        outflowp += std::abs(character) * wc *
                    (_mobility[_i] * phi + _dmobility_dp[_i] * phi * (_pp[_i] - bh_pressure));
      else
        outflowp += std::abs(character) * wc * _dmobility_ds[_i] * phi * (_pp[_i] - bh_pressure);
    }
  }

  return _test[_i][_qp] * outflowp;
}
