/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowMassFraction.h"

template <>
InputParameters
validParams<PorousFlowMassFraction>()
{
  InputParameters params = validParams<PorousFlowMaterialVectorBase>();
  params.addCoupledVar("mass_fraction_vars",
                       "List of variables that represent the mass fractions.  Format is 'f_ph0^c0 "
                       "f_ph0^c1 f_ph0^c2 ... f_ph0^c(N-1) f_ph1^c0 f_ph1^c1 fph1^c2 ... "
                       "fph1^c(N-1) ... fphP^c0 f_phP^c1 fphP^c2 ... fphP^c(N-1)' where "
                       "N=num_components and P=num_phases, and it is assumed that "
                       "f_ph^cN=1-sum(f_ph^c,{c,0,N-1}) so that f_ph^cN need not be given.  If no "
                       "variables are provided then num_phases=1=num_components.");
  params.addClassDescription("This Material forms a std::vector<std::vector ...> of mass-fractions "
                             "out of the individual mass fractions");
  return params;
}

PorousFlowMassFraction::PorousFlowMassFraction(const InputParameters & parameters)
  : PorousFlowMaterialVectorBase(parameters),

    _mass_frac(_nodal_material
                   ? declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")
                   : declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _grad_mass_frac(_nodal_material ? nullptr
                                    : &declareProperty<std::vector<std::vector<RealGradient>>>(
                                          "PorousFlow_grad_mass_frac_qp")),
    _dmass_frac_dvar(_nodal_material ? declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_nodal_dvar")
                                     : declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_qp_dvar")),

    _num_passed_mf_vars(coupledComponents("mass_fraction_vars"))
{
  if (_num_phases < 1 || _num_components < 1)
    mooseError("PorousFlowMassFraction: The Dictator proclaims that the number of phases is ",
               _num_phases,
               " and the number of components is ",
               _num_components,
               ", and stipulates that you should not use PorousFlowMassFraction in this case");
  if (_num_passed_mf_vars != _num_phases * (_num_components - 1))
    mooseError("PorousFlowMassFraction: The number of mass_fraction_vars is ",
               _num_passed_mf_vars,
               " which must be equal to the Dictator's num_phases (",
               _num_phases,
               ") multiplied by num_components-1 (",
               _num_components - 1,
               ")");

  _mf_vars_num.resize(_num_passed_mf_vars);
  _mf_vars.resize(_num_passed_mf_vars);
  _grad_mf_vars.resize(_num_passed_mf_vars);
  for (unsigned i = 0; i < _num_passed_mf_vars; ++i)
  {
    _mf_vars_num[i] = coupled("mass_fraction_vars", i);
    _mf_vars[i] = (_nodal_material ? &coupledNodalValue("mass_fraction_vars", i)
                                   : &coupledValue("mass_fraction_vars", i));
    _grad_mf_vars[i] = &coupledGradient("mass_fraction_vars", i);
  }
}

void
PorousFlowMassFraction::initQpStatefulProperties()
{
  // all we need to do is compute _mass_frac for _nodal_materials
  // but the following avoids code duplication
  computeQpProperties();
}

void
PorousFlowMassFraction::computeQpProperties()
{
  // size all properties correctly
  _mass_frac[_qp].resize(_num_phases);
  _dmass_frac_dvar[_qp].resize(_num_phases);
  if (!_nodal_material)
    (*_grad_mass_frac)[_qp].resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _mass_frac[_qp][ph].resize(_num_components);
    _dmass_frac_dvar[_qp][ph].resize(_num_components);
    for (unsigned int comp = 0; comp < _num_components; ++comp)
      _dmass_frac_dvar[_qp][ph][comp].assign(_num_var, 0.0);
    if (!_nodal_material)
      (*_grad_mass_frac)[_qp][ph].resize(_num_components);
  }

  // compute the values and derivatives
  unsigned int i = 0;
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    Real total_mass_frac = 0;
    if (!_nodal_material)
      (*_grad_mass_frac)[_qp][ph][_num_components - 1] = 0.0;
    for (unsigned int comp = 0; comp < _num_components - 1; ++comp)
    {
      _mass_frac[_qp][ph][comp] = (*_mf_vars[i])[_qp];
      total_mass_frac += _mass_frac[_qp][ph][comp];
      if (!_nodal_material)
      {
        (*_grad_mass_frac)[_qp][ph][comp] = (*_grad_mf_vars[i])[_qp];
        (*_grad_mass_frac)[_qp][ph][_num_components - 1] -= (*_grad_mf_vars[i])[_qp];
      }
      if (_dictator.isPorousFlowVariable(_mf_vars_num[i]))
      {
        // _mf_vars[i] is a PorousFlow variable
        const unsigned int pf_var_num = _dictator.porousFlowVariableNum(_mf_vars_num[i]);
        _dmass_frac_dvar[_qp][ph][comp][pf_var_num] = 1.0;
        _dmass_frac_dvar[_qp][ph][_num_components - 1][pf_var_num] = -1.0;
      }
      i++;
    }
    _mass_frac[_qp][ph][_num_components - 1] = 1.0 - total_mass_frac;
  }
}
