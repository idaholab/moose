/*****************************************/
/* Written by andrew.wilkins@csiro.au    */
/* Please contact me if you make changes */
/*****************************************/

#include "RichardsLumpedMassChange.h"

#include <iostream>


template<>
InputParameters validParams<RichardsLumpedMassChange>()
{
  InputParameters params = validParams<TimeKernel>();
  params.addRequiredParam<UserObjectName>("richardsVarNames_UO", "The UserObject that holds the list of Richards variables.");
  params.addRequiredParam<std::vector<UserObjectName> >("density_UO", "List of names of user objects that define the fluid density (or densities for multiphase).  In the multiphase case, for ease of use, the density, Seff and Sat UserObjects are the same format as for RichardsMaterial, but only the one relevant for the specific phase is actually used.");
  params.addRequiredParam<std::vector<UserObjectName> >("seff_UO", "List of name of user objects that define effective saturation as a function of porepressure(s)");
  params.addRequiredParam<std::vector<UserObjectName> >("sat_UO", "List of names of user objects that define saturation as a function of effective saturation");
  return params;
}

RichardsLumpedMassChange::RichardsLumpedMassChange(const std::string & name,
                                             InputParameters parameters) :
    TimeKernel(name,parameters),
    _richards_name_UO(getUserObject<RichardsVarNames>("richardsVarNames_UO")),
    _num_p(_richards_name_UO.num_v()),
    _pvar(_richards_name_UO.richards_var_num(_var.number())),

    _porosity(getMaterialProperty<Real>("porosity")),
    _porosity_old(getMaterialProperty<Real>("porosity_old")),

    // in the following:  first get the userobject names that were inputted, then get the _pvar one of these, then get the actual userobject that this corresponds to, then finally & gives pointer to RichardsDensity (or whatever) object.
    _seff_UO(&getUserObjectByName<RichardsSeff>(getParam<std::vector<UserObjectName> >("seff_UO")[_pvar])),
    _sat_UO(&getUserObjectByName<RichardsSat>(getParam<std::vector<UserObjectName> >("sat_UO")[_pvar])),
    _density_UO(&getUserObjectByName<RichardsDensity>(getParam<std::vector<UserObjectName> >("density_UO")[_pvar]))

{
  _ps_at_nodes.resize(_num_p);
  _ps_old_at_nodes.resize(_num_p);

  _nodal_pp.resize(_num_p);
  for (unsigned int i=0 ; i<_num_p; ++i)
    _nodal_pp[i] = _richards_name_UO.raw_var(i);

  _dseff.resize(_num_p);
}


// i wish all this stuff wasn't necessary,
// but we have to build the nodal values
// we also have to put them into a form that seff_UO can use
void
RichardsLumpedMassChange::prepareNodalPressureValues()
{
  for (unsigned int i=0 ; i<_num_p; ++i)
    _nodal_pp[i]->computeNodalValues();
  //_var.computeNodalValues();


  // we want to use seff_UO.seff, and this has arguments:
  // seff(std::vector<VariableValue *> p, unsigned int qp)
  // so i need a std::vector<VariableValue *> (ie a vector of pointers to VariableValues)
  // This is just crazy and surely there is a better way!
  for (unsigned int pnum=0 ; pnum<_num_p; ++pnum)
  {
    // _nodal_pp[pnum] is just like _var, and the ->nodalSln() returns a VariableValue &.  We want a pointer to this.
    _ps_at_nodes[pnum] = &(_nodal_pp[pnum]->nodalSln());
    _ps_old_at_nodes[pnum] = &(_nodal_pp[pnum]->nodalSlnOld());
  }
}



void
RichardsLumpedMassChange::computeResidual()
{
  prepareNodalPressureValues();
  TimeKernel::computeResidual();
}

Real
RichardsLumpedMassChange::computeQpResidual()
{
  // current values:
  Real density = (*_density_UO).density(_var.nodalSln()[_i]);
  Real seff = (*_seff_UO).seff(_ps_at_nodes, _i);
  Real sat = (*_sat_UO).sat(seff);
  Real mass = _porosity[_qp]*density*sat;


  // old values:
  // for snes_type = test (and perhaps others?) the Old values
  // aren't defined for some reason
  // hence all the fluffing around with checking of sizes
  Real density_old;
  Real seff_old;
  if (_var.nodalSlnOld().size()<=_i || (*_ps_old_at_nodes[0]).size() <= _i)
  {
    density_old = (*_density_UO).density(_var.nodalSln()[_i]);
    seff_old = (*_seff_UO).seff(_ps_at_nodes, _i);
  }
  else
  {
    density_old = (*_density_UO).density(_var.nodalSlnOld()[_i]);
    seff_old = (*_seff_UO).seff(_ps_old_at_nodes, _i);
  }
  Real sat_old = (*_sat_UO).sat(seff_old);
  Real mass_old = _porosity_old[_qp]*density_old*sat_old;


  return _test[_i][_qp]*(mass - mass_old)/_dt;
}


void
RichardsLumpedMassChange::computeJacobian()
{
  prepareNodalPressureValues();
  TimeKernel::computeJacobian();
}

Real
RichardsLumpedMassChange::computeQpJacobian()
{
  if (_i != _j)
    return 0.0;

  Real density = (*_density_UO).density(_var.nodalSln()[_i]);
  Real ddensity = (*_density_UO).ddensity(_var.nodalSln()[_i]);

  Real seff = (*_seff_UO).seff(_ps_at_nodes, _i);
  (*_seff_UO).dseff(_ps_at_nodes, _i, _dseff);

  Real sat = (*_sat_UO).sat(seff);
  Real dsat = (*_sat_UO).dsat(seff);

  Real mass_prime = _porosity[_qp]*(ddensity*sat + density*_dseff[_pvar]*dsat);

  return _test[_i][_qp]*mass_prime/_dt;
}



void
RichardsLumpedMassChange::computeOffDiagJacobian(unsigned int jvar)
{
  prepareNodalPressureValues();
  TimeKernel::computeOffDiagJacobian(jvar);
}

Real
RichardsLumpedMassChange::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_richards_name_UO.not_richards_var(jvar))
    return 0.0;
  if (_i != _j)
    return 0.0;
  unsigned int dvar = _richards_name_UO.richards_var_num(jvar);

  Real density = (*_density_UO).density(_var.nodalSln()[_i]);

  Real seff = (*_seff_UO).seff(_ps_at_nodes, _i);
  (*_seff_UO).dseff(_ps_at_nodes, _i, _dseff);

  Real dsat = (*_sat_UO).dsat(seff);

  Real mass_prime = _porosity[_qp]*density*_dseff[dvar]*dsat;

  return _test[_i][_qp]*mass_prime/_dt;
}
