//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsBorehole.h"
#include "RotationMatrix.h"

registerMooseObject("RichardsApp", RichardsBorehole);

InputParameters
RichardsBorehole::validParams()
{
  InputParameters params = PeacemanBorehole::validParams();
  params.addRequiredParam<UserObjectName>(
      "richardsVarNames_UO", "The UserObject that holds the list of Richards variable names.");
  params.addParam<std::vector<UserObjectName>>("relperm_UO",
                                               "List of names of user objects that "
                                               "define relative permeability.  Only "
                                               "needed if fully_upwind is used");
  params.addParam<std::vector<UserObjectName>>(
      "seff_UO",
      "List of name of user objects that define effective saturation as a function of "
      "pressure list.  Only needed if fully_upwind is used");
  params.addParam<std::vector<UserObjectName>>("density_UO",
                                               "List of names of user objects that "
                                               "define the fluid density.  Only "
                                               "needed if fully_upwind is used");
  params.addParam<bool>("fully_upwind", false, "Fully upwind the flux");
  params.addClassDescription("Approximates a borehole in the mesh with given bottomhole pressure, "
                             "and radii using a number of point sinks whose positions are read "
                             "from a file");
  return params;
}

RichardsBorehole::RichardsBorehole(const InputParameters & parameters)
  : PeacemanBorehole(parameters),
    _fully_upwind(getParam<bool>("fully_upwind")),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _num_p(_richards_name_UO.num_v()),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    // in the following, getUserObjectByName returns a reference (an alias) to a RichardsBLAH user
    // object, and the & turns it into a pointer
    _density_UO(_fully_upwind ? &getUserObjectByName<RichardsDensity>(
                                    getParam<std::vector<UserObjectName>>("density_UO")[_pvar])
                              : NULL),
    _seff_UO(_fully_upwind ? &getUserObjectByName<RichardsSeff>(
                                 getParam<std::vector<UserObjectName>>("seff_UO")[_pvar])
                           : NULL),
    _relperm_UO(_fully_upwind ? &getUserObjectByName<RichardsRelPerm>(
                                    getParam<std::vector<UserObjectName>>("relperm_UO")[_pvar])
                              : NULL),

    _num_nodes(0),
    _mobility(0),
    _dmobility_dv(0),
    _pp(getMaterialProperty<std::vector<Real>>("porepressure")),
    _dpp_dv(getMaterialProperty<std::vector<std::vector<Real>>>("dporepressure_dv")),
    _viscosity(getMaterialProperty<std::vector<Real>>("viscosity")),
    _permeability(getMaterialProperty<RealTensorValue>("permeability")),
    _dseff_dv(getMaterialProperty<std::vector<std::vector<Real>>>("ds_eff_dv")),
    _rel_perm(getMaterialProperty<std::vector<Real>>("rel_perm")),
    _drel_perm_dv(getMaterialProperty<std::vector<std::vector<Real>>>("drel_perm_dv")),
    _density(getMaterialProperty<std::vector<Real>>("density")),
    _ddensity_dv(getMaterialProperty<std::vector<std::vector<Real>>>("ddensity_dv"))
{
  _ps_at_nodes.resize(_num_p);
  for (unsigned int pnum = 0; pnum < _num_p; ++pnum)
    _ps_at_nodes[pnum] = _richards_name_UO.nodal_var(pnum);

  // To correctly compute the Jacobian terms,
  // tell MOOSE that this DiracKernel depends on all the Richards Vars
  const std::vector<MooseVariableFEBase *> & coupled_vars = _richards_name_UO.getCoupledMooseVars();
  for (unsigned int i = 0; i < coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

void
RichardsBorehole::prepareNodalValues()
{
  // NOTE: i'm assuming that all the richards variables are pressure values

  _num_nodes = (*_ps_at_nodes[_pvar]).size();

  Real p;
  Real density;
  Real ddensity_dp;
  Real seff;
  std::vector<Real> dseff_dp;
  Real relperm;
  Real drelperm_ds;
  _mobility.resize(_num_nodes);
  _dmobility_dv.resize(_num_nodes);
  dseff_dp.resize(_num_p);
  for (unsigned int nodenum = 0; nodenum < _num_nodes; ++nodenum)
  {
    // retrieve and calculate basic things at the node
    p = (*_ps_at_nodes[_pvar])[nodenum];    // pressure of fluid _pvar at node nodenum
    density = _density_UO->density(p);      // density of fluid _pvar at node nodenum
    ddensity_dp = _density_UO->ddensity(p); // d(density)/dP
    seff = _seff_UO->seff(_ps_at_nodes,
                          nodenum); // effective saturation of fluid _pvar at node nodenum
    _seff_UO->dseff(
        _ps_at_nodes, nodenum, dseff_dp); // d(seff)/d(P_ph), for ph = 0, ..., _num_p - 1
    relperm = _relperm_UO->relperm(seff); // relative permeability of fluid _pvar at node nodenum
    drelperm_ds = _relperm_UO->drelperm(seff); // d(relperm)/dseff

    // calculate the mobility and its derivatives wrt (variable_ph = porepressure_ph)
    _mobility[nodenum] =
        density * relperm / _viscosity[0][_pvar]; // assume viscosity is constant throughout element
    _dmobility_dv[nodenum].resize(_num_p);
    for (unsigned int ph = 0; ph < _num_p; ++ph)
      _dmobility_dv[nodenum][ph] = density * drelperm_ds * dseff_dp[ph] / _viscosity[0][_pvar];
    _dmobility_dv[nodenum][_pvar] += ddensity_dp * relperm / _viscosity[0][_pvar];
  }
}

void
RichardsBorehole::computeResidual()
{
  if (_fully_upwind)
    prepareNodalValues();
  DiracKernel::computeResidual();
}

Real
RichardsBorehole::computeQpResidual()
{
  const Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return 0.0;

  const Real bh_pressure =
      _p_bot +
      _unit_weight *
          (_q_point[_qp] -
           _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  Real pp;
  Real mob;
  if (!_fully_upwind)
  {
    pp = _pp[_qp][_pvar];
    mob = _rel_perm[_qp][_pvar] * _density[_qp][_pvar] / _viscosity[_qp][_pvar];
  }
  else
  {
    pp = (*_ps_at_nodes[_pvar])[_i];
    mob = _mobility[_i];
  }

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
    wc = wellConstant(_permeability[_qp],
                      _rot_matrix[current_dirac_ptid - 1],
                      _half_seg_len[current_dirac_ptid - 1],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow += _test[_i][_qp] * std::abs(character) * wc * mob * (pp - bh_pressure);
  }

  if (current_dirac_ptid + 1 < _zs.size() || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point, or we only have one point
  {
    wc = wellConstant(_permeability[_qp],
                      _rot_matrix[current_dirac_ptid],
                      _half_seg_len[current_dirac_ptid],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflow += _test[_i][_qp] * std::abs(character) * wc * mob * (pp - bh_pressure);
  }

  _total_outflow_mass.add(
      outflow * _dt); // this is not thread safe, but DiracKernel's aren't currently threaded
  return outflow;
}

void
RichardsBorehole::computeJacobian()
{
  if (_fully_upwind)
    prepareNodalValues();
  DiracKernel::computeJacobian();
}

Real
RichardsBorehole::computeQpJacobian()
{
  const Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return 0.0;
  return jac(_pvar);
}

Real
RichardsBorehole::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  const unsigned int dvar = _richards_name_UO.richards_var_num(jvar);
  return jac(dvar);
}

Real
RichardsBorehole::jac(unsigned int wrt_num)
{
  const Real character = _character.value(_t, _q_point[_qp]);
  if (character == 0.0)
    return 0.0;

  const Real bh_pressure =
      _p_bot +
      _unit_weight *
          (_q_point[_qp] -
           _bottom_point); // really want to use _q_point instaed of _current_point, i think?!

  Real pp;
  Real dpp_dv;
  Real mob;
  Real dmob_dv;
  Real phi;
  if (!_fully_upwind)
  {
    pp = _pp[_qp][_pvar];
    dpp_dv = _dpp_dv[_qp][_pvar][wrt_num];
    mob = _rel_perm[_qp][_pvar] * _density[_qp][_pvar] / _viscosity[_qp][_pvar];
    dmob_dv = (_drel_perm_dv[_qp][_pvar][wrt_num] * _density[_qp][_pvar] +
               _rel_perm[_qp][_pvar] * _ddensity_dv[_qp][_pvar][wrt_num]) /
              _viscosity[_qp][_pvar];
    phi = _phi[_j][_qp];
  }
  else
  {
    if (_i != _j)
      return 0.0; // residual at node _i only depends on variables at that node
    pp = (*_ps_at_nodes[_pvar])[_i];
    dpp_dv =
        (_pvar == wrt_num ? 1 : 0); // NOTE: i'm assuming that the variables are pressure variables
    mob = _mobility[_i];
    dmob_dv = _dmobility_dv[_i][wrt_num];
    phi = 1;
  }

  // Get the ID we initially assigned to this point
  const unsigned current_dirac_ptid = currentPointCachedID();

  // If getting the ID failed, fall back to the old bodge!
  // if (current_dirac_ptid == libMesh::invalid_uint)
  //  current_dirac_ptid = (_zs.size() > 2) ? 1 : 0;

  Real outflowp(0.0);

  Real wc(0.0);
  if (current_dirac_ptid > 0)
  // contribution from half-segment "behind" this point
  {
    wc = wellConstant(_permeability[_qp],
                      _rot_matrix[current_dirac_ptid - 1],
                      _half_seg_len[current_dirac_ptid - 1],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflowp += _test[_i][_qp] * std::abs(character) * wc *
                  (mob * phi * dpp_dv + dmob_dv * phi * (pp - bh_pressure));
  }

  if (current_dirac_ptid < _zs.size() - 1 || _zs.size() == 1)
  // contribution from half-segment "ahead of" this point
  {
    wc = wellConstant(_permeability[_qp],
                      _rot_matrix[current_dirac_ptid],
                      _half_seg_len[current_dirac_ptid],
                      _current_elem,
                      _rs[current_dirac_ptid]);
    if ((character < 0.0 && pp < bh_pressure) || (character > 0.0 && pp > bh_pressure))
      // injection, so outflow<0 || // production, so outflow>0
      outflowp += _test[_i][_qp] * std::abs(character) * wc *
                  (mob * phi * dpp_dv + dmob_dv * phi * (pp - bh_pressure));
  }

  return outflowp;
}
