/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowFluidStateFlashBase.h"

template <>
InputParameters
validParams<PorousFlowFluidStateFlashBase>()
{
  InputParameters params = validParams<PorousFlowVariableBase>();
  params.addRequiredCoupledVar("gas_porepressure",
                               "Variable that is the porepressure of the gas phase");
  params.addRequiredCoupledVar("z", "Total mass fraction of component i summed over all phases");
  params.addParam<unsigned int>("liquid_phase_number", 0, "The phase number of the liquid phase");
  params.addParam<unsigned int>(
      "liquid_fluid_component", 0, "The fluid component number of the liquid phase");
  params.addParam<Real>("pc", 0.0, "Constant capillary pressure (Pa). Default is 0.0");
  params.addRangeCheckedParam<Real>(
      "sat_lr",
      0.0,
      "sat_lr >= 0 & sat_lr <= 1",
      "Liquid residual saturation.  Must be between 0 and 1. Default is 0");
  MooseEnum unit_choice("Kelvin=0 Celsius=1", "Kelvin");
  params.addParam<MooseEnum>(
      "temperature_unit", unit_choice, "The unit of the temperature variable");
  params.addClassDescription("Base class for fluid state calculations using persistent primary "
                             "variables and a vapor-liquid flash");
  return params;
}

PorousFlowFluidStateFlashBase::PorousFlowFluidStateFlashBase(const InputParameters & parameters)
  : PorousFlowVariableBase(parameters),

    _gas_porepressure(_nodal_material ? coupledNodalValue("gas_porepressure")
                                      : coupledValue("gas_porepressure")),
    _gas_gradp_qp(coupledGradient("gas_porepressure")),
    _gas_porepressure_varnum(coupled("gas_porepressure")),
    _pvar(_dictator.isPorousFlowVariable(_gas_porepressure_varnum)
              ? _dictator.porousFlowVariableNum(_gas_porepressure_varnum)
              : 0),

    _num_z_vars(coupledComponents("z")),

    _aqueous_phase_number(getParam<unsigned int>("liquid_phase_number")),
    _gas_phase_number(1 - _aqueous_phase_number),
    _aqueous_fluid_component(getParam<unsigned int>("liquid_fluid_component")),
    _gas_fluid_component(1 - _aqueous_fluid_component),

    _temperature(_nodal_material ? getMaterialProperty<Real>("PorousFlow_temperature_nodal")
                                 : getMaterialProperty<Real>("PorousFlow_temperature_qp")),
    _gradT_qp(getMaterialProperty<RealGradient>("PorousFlow_grad_temperature_qp")),
    _dtemperature_dvar(
        _nodal_material
            ? getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_nodal_dvar")
            : getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),

    _mass_frac(_nodal_material
                   ? declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")
                   : declareProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_qp")),
    _grad_mass_frac_qp(_nodal_material ? nullptr
                                       : &declareProperty<std::vector<std::vector<RealGradient>>>(
                                             "PorousFlow_grad_mass_frac_qp")),
    _dmass_frac_dvar(_nodal_material ? declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_nodal_dvar")
                                     : declareProperty<std::vector<std::vector<std::vector<Real>>>>(
                                           "dPorousFlow_mass_frac_qp_dvar")),

    _fluid_density(_nodal_material
                       ? declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal")
                       : declareProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_qp")),
    _dfluid_density_dvar(_nodal_material ? declareProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_fluid_phase_density_nodal_dvar")
                                         : declareProperty<std::vector<std::vector<Real>>>(
                                               "dPorousFlow_fluid_phase_density_qp_dvar")),
    _fluid_viscosity(_nodal_material
                         ? declareProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")
                         : declareProperty<std::vector<Real>>("PorousFlow_viscosity_qp")),
    _dfluid_viscosity_dvar(
        _nodal_material
            ? declareProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_nodal_dvar")
            : declareProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_qp_dvar")),

    _T_c2k(getParam<MooseEnum>("temperature_unit") == 0 ? 0.0 : 273.15),
    _R(8.3144621),
    _pc(getParam<Real>("pc")),
    _sat_lr(getParam<Real>("sat_lr")),
    _dseff_ds(1.0 / (1.0 - _sat_lr)),
    _nr_max_its(42),
    _nr_tol(1.0e-12),
    _is_initqp(false)
{
  // Check that the number of total mass fractions provided as primary variables is correct
  if (_num_z_vars != _num_components - 1)
    mooseError("The number of supplied mass fraction variables should be ",
               _num_components - 1,
               " in ",
               _name,
               " but ",
               _num_z_vars,
               " are supplied");

  // Store all total mass fractions and associated variable numbers
  _z.resize(_num_z_vars);
  _gradz_qp.resize(_num_z_vars);
  _z_varnum.resize(_num_z_vars);
  _zvar.resize(_num_z_vars);

  for (unsigned int i = 0; i < _num_z_vars; ++i)
  {
    _z[i] = (_nodal_material ? &coupledNodalValue("z", i) : &coupledValue("z", i));
    _gradz_qp[i] = &coupledGradient("z", i);
    _z_varnum[i] = coupled("z", i);
    _zvar[i] = (_dictator.isPorousFlowVariable(_z_varnum[i])
                    ? _dictator.porousFlowVariableNum(_z_varnum[i])
                    : 0);
  }
}

void
PorousFlowFluidStateFlashBase::initQpStatefulProperties()
{
  _is_initqp = true;
  // Set the size of pressure and saturation vectors
  PorousFlowVariableBase::initQpStatefulProperties();

  // Set the size of all other vectors
  setMaterialVectorSize();

  // Set the initial values of the properties at the nodes.
  // Note: not required for qp materials as no old values at the qps are requested
  if (_nodal_material)
    thermophysicalProperties();
}

void
PorousFlowFluidStateFlashBase::computeQpProperties()
{
  _is_initqp = false;
  // Prepare the derivative vectors
  PorousFlowVariableBase::computeQpProperties();

  // Set the size of all other vectors
  setMaterialVectorSize();

  // Calculate all required thermophysical properties
  thermophysicalProperties();
}

void
PorousFlowFluidStateFlashBase::setMaterialVectorSize() const
{
  _fluid_density[_qp].assign(_num_phases, 0.0);
  _fluid_viscosity[_qp].assign(_num_phases, 0.0);
  _mass_frac[_qp].resize(_num_phases);
  for (unsigned int ph = 0; ph < _num_phases; ++ph)
    _mass_frac[_qp][ph].assign(_num_components, 0.0);

  // Derivatives and gradients are not required in initQpStatefulProperties
  if (!_is_initqp)
  {
    _dfluid_density_dvar[_qp].resize(_num_phases);
    _dfluid_viscosity_dvar[_qp].resize(_num_phases);
    _dmass_frac_dvar[_qp].resize(_num_phases);

    if (!_nodal_material)
      (*_grad_mass_frac_qp)[_qp].resize(_num_phases);

    for (unsigned int ph = 0; ph < _num_phases; ++ph)
    {
      _dfluid_density_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dfluid_viscosity_dvar[_qp][ph].assign(_num_pf_vars, 0.0);
      _dmass_frac_dvar[_qp][ph].resize(_num_components);

      for (unsigned int comp = 0; comp < _num_components; ++comp)
        _dmass_frac_dvar[_qp][ph][comp].assign(_num_pf_vars, 0.0);

      if (!_nodal_material)
        (*_grad_mass_frac_qp)[_qp][ph].assign(_num_components, RealGradient());
    }
  }
}

Real
PorousFlowFluidStateFlashBase::rachfordRice(Real x, std::vector<Real> & Ki) const
{
  // Check that the size of the equilibrium constant vector is correct
  if (Ki.size() != _num_components)
    mooseError("The number of equilibrium components passed to rachfordRice is not correct");

  Real f = 0.0;
  Real z_total = 0.0;

  for (unsigned int i = 0; i < _num_z_vars; ++i)
  {
    f += (*_z[i])[_qp] * (Ki[i] - 1.0) / (1.0 + x * (Ki[i] - 1.0));
    z_total += (*_z[i])[_qp];
  }

  // Add the last component (with total mass fraction = 1 - z_total)
  f += (1.0 - z_total) * (Ki[_num_z_vars] - 1.0) / (1.0 + x * (Ki[_num_z_vars] - 1.0));

  return f;
}

Real
PorousFlowFluidStateFlashBase::rachfordRiceDeriv(Real x, std::vector<Real> & Ki) const
{
  // Check that the size of the equilibrium constant vector is correct
  if (Ki.size() != _num_components)
    mooseError("The number of equilibrium components passed to rachfordRiceDeriv is not correct");

  Real df = 0.0;
  Real z_total = 0.0;

  for (unsigned int i = 0; i < _num_z_vars; ++i)
  {
    df -= (*_z[i])[_qp] * (Ki[i] - 1.0) * (Ki[i] - 1.0) / (1.0 + x * (Ki[i] - 1.0)) /
          (1.0 + x * (Ki[i] - 1.0));
    z_total += (*_z[i])[_qp];
  }

  // Add the last component (with total mass fraction = 1 - z_total)
  df -= (1.0 - z_total) * (Ki[_num_z_vars] - 1.0) * (Ki[_num_z_vars] - 1.0) /
        (1.0 + x * (Ki[_num_z_vars] - 1.0)) / (1.0 + x * (Ki[_num_z_vars] - 1.0));

  return df;
}

Real
PorousFlowFluidStateFlashBase::vaporMassFraction(std::vector<Real> & Ki) const
{
  // Check that the size of the equilibrium constant vector is correct
  if (Ki.size() != _num_components)
    mooseError("The number of equilibrium components passed to vaporMassFraction is not correct");

  Real v;

  // If there are only two components, an analytical solution is possible
  if (_num_components == 2)
    v = ((*_z[0])[_qp] * (Ki[1] - Ki[0]) - (Ki[1] - 1.0)) / ((Ki[0] - 1.0) * (Ki[1] - 1.0));
  else
  {
    // More than two components - solve the Rachford-Rice equation using
    // Newton-Raphson method.
    // Initial guess for vapor mass fraction
    Real v0 = 0.5;
    unsigned int iter = 0;

    while (std::abs(rachfordRice(v0, Ki)) > _nr_tol)
    {
      v0 = v0 - rachfordRice(v0, Ki) / rachfordRiceDeriv(v0, Ki);
      iter++;

      if (iter > _nr_max_its)
        break;
    }
    v = v0;
  }
  return v;
}

Real
PorousFlowFluidStateFlashBase::effectiveSaturation(Real saturation) const
{
  return (saturation - _sat_lr) / (1.0 - _sat_lr);
}

Real PorousFlowFluidStateFlashBase::capillaryPressure(Real /* saturation */) const { return _pc; }

Real PorousFlowFluidStateFlashBase::dCapillaryPressure_dS(Real /* saturation */) const
{
  return 0.0;
}

Real PorousFlowFluidStateFlashBase::d2CapillaryPressure_dS2(Real /* saturation */) const
{
  return 0.0;
}
