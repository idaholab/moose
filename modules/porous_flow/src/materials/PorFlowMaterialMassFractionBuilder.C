/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "PorFlowMaterialMassFractionBuilder.h"

#include "Conversion.h"

template<>
InputParameters validParams<PorFlowMaterialMassFractionBuilder>()
{
  InputParameters params = validParams<Material>();

  params.addRequiredRangeCheckedParam<unsigned int>("num_phases", "num_phases>0", "The number of fluid phases in the simulation");
  params.addRequiredRangeCheckedParam<unsigned int>("num_components", "num_components>0", "The number of fluid components in the simulation");
  params.addCoupledVar("mass_fraction_vars", "List of variables that represent the mass fractions.  Format is 'f_ph0^c0 f_ph0^c1 f_ph0^c2 ... f_ph0^c(N-1) f_ph1^c0 f_ph1^c1 fph1^c2 ... fph1^c(N-1) ... fphP^c0 f_phP^c1 fphP^c2 ... fphP^c(N-1)' where N=num_components and P=num_phases, and it is assumed that f_ph^cN=1-sum(f_ph^c,{c,0,N-1}) so that f_ph^cN need not be given.  If no variables are provided then num_phases=1=num_components.");
  params.addRequiredParam<UserObjectName>("PorFlowVarNames_UO", "The UserObject that holds the list of Porous-Flow variable names.");
  params.addClassDescription("This Material forms a std::vector<std::vector ...> of mass-fractions out of the individual mass fractions");
  return params;
}

PorFlowMaterialMassFractionBuilder::PorFlowMaterialMassFractionBuilder(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),

    _num_phases(getParam<unsigned int>("num_phases")),
    _num_components(getParam<unsigned int>("num_components")),
    _porflow_name_UO(getUserObject<PorFlowVarNames>("PorFlowVarNames_UO")),

    _mass_frac(declareProperty<std::vector<std::vector<Real> > >("PorFlow_mass_frac")),
    _mass_frac_old(declarePropertyOld<std::vector<std::vector<Real> > >("PorFlow_mass_frac")),
    _grad_mass_frac(declareProperty<std::vector<std::vector<RealGradient> > >("PorFlow_grad_mass_frac")),
    _dmass_frac_dvar(declareProperty<std::vector<std::vector<std::vector<Real> > > >("dPorFlow_mass_frac_dvar")),

    _num_passed_mf_vars(coupledComponents("mass_fraction_vars"))
{
  if (_num_passed_mf_vars != _num_phases*(_num_components - 1))
    mooseError("PorFlowMaterialMassFractionBuilder: The number of mass_fraction_vars is " << _num_passed_mf_vars << " which must be equal to num_phases (" << _num_phases << ") multiplied by num_components-1 (" << _num_components - 1 << ")");

  _mf_vars_num.resize(_num_passed_mf_vars);
  _mf_vars.resize(_num_passed_mf_vars);
  _grad_mf_vars.resize(_num_passed_mf_vars);
  for (unsigned i = 0; i < _num_passed_mf_vars; ++i)
  {
    _mf_vars_num[i] = coupled("mass_fraction_vars", i);
    _mf_vars[i] = &coupledNodalValue("mass_fraction_vars", i);
    _grad_mf_vars[i] = &coupledGradient("mass_fraction_vars", i);
  }
}

void
PorFlowMaterialMassFractionBuilder::initQpStatefulProperties()
{
  const unsigned int num_var = _porflow_name_UO.num_v();
  _mass_frac[_qp].resize(_num_phases);
  _mass_frac_old[_qp].resize(_num_phases);
  _grad_mass_frac[_qp].resize(_num_phases);
  _dmass_frac_dvar[_qp].resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    _mass_frac[_qp][ph].resize(_num_components);
    _mass_frac_old[_qp][ph].resize(_num_components);
    _grad_mass_frac[_qp][ph].resize(_num_components);
    _dmass_frac_dvar[_qp][ph].resize(_num_components);
    for (unsigned int comp = 0; comp < _num_components; ++comp)
      _dmass_frac_dvar[_qp][ph][comp].assign(num_var, 0.0);
  }

  // the derivative matrix is fixed for all time
  // so it can be built here instead of in computeQpProperties
  unsigned int i = 0;
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    for (unsigned int comp = 0; comp < _num_components - 1; ++comp)
    {
      if (!(_porflow_name_UO.not_porflow_var(_mf_vars_num[i])))
      {
        // _mf_vars[i] is a PorFlow variable
        const unsigned int pf_var_num = _porflow_name_UO.porflow_var_num(_mf_vars_num[i]);
        _dmass_frac_dvar[_qp][ph][comp][pf_var_num] = 1.0;
        _dmass_frac_dvar[_qp][ph][_num_components - 1][pf_var_num] = -1.0;
      }
      i++;
    }
  }
}

void
PorFlowMaterialMassFractionBuilder::computeQpProperties()
{
  build_mass_frac(_qp);

  /*
   *  YAQI HACK !!
   *
   * I really just want to simply call
   * build_mass_frac(_qp)
   * from initQpStatefulProperties, but the Variables
   * aren't initialised at that point so moose crashes
   */
  if (_t_step == 1)
    for (unsigned int ph = 0; ph < _num_phases; ++ph)
      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _mass_frac_old[_qp][ph][comp] = _mass_frac[_qp][ph][comp];
}

void
PorFlowMaterialMassFractionBuilder::build_mass_frac(unsigned int qp)
{
  unsigned int i = 0;
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
  {
    Real total_mass_frac = 0;
    _grad_mass_frac[_qp][ph][_num_components - 1] = 0.0;
    for (unsigned int comp = 0; comp < _num_components - 1; ++comp)
    {
      _mass_frac[qp][ph][comp] = (*_mf_vars[i])[qp];
      total_mass_frac += (*_mf_vars[i])[qp];
      _grad_mass_frac[qp][ph][comp] = (*_grad_mf_vars[i])[qp];
      _grad_mass_frac[_qp][ph][_num_components - 1] -= (*_grad_mf_vars[i])[qp];
      i++;
    }
    _mass_frac[qp][ph][_num_components - 1] = 1 - total_mass_frac;
  }
}
