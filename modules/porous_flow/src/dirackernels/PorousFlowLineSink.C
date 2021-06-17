//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowLineSink.h"
#include "libmesh/utility.h"

InputParameters
PorousFlowLineSink::validParams()
{
  InputParameters params = PorousFlowLineGeometry::validParams();
  MooseEnum p_or_t_choice("pressure=0 temperature=1", "pressure");
  params.addParam<MooseEnum>("function_of",
                             p_or_t_choice,
                             "Modifying functions will be a function of either pressure and "
                             "permeability (eg, for boreholes that pump fluids) or "
                             "temperature and thermal conductivity (eg, for boreholes that "
                             "pump pure heat with no fluid flow)");
  params.addRequiredParam<UserObjectName>(
      "SumQuantityUO",
      "User Object of type=PorousFlowSumQuantity in which to place the total "
      "outflow from the line sink for each time step.");
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names");
  params.addParam<unsigned int>(
      "fluid_phase",
      0,
      "The fluid phase whose pressure (and potentially mobility, enthalpy, etc) "
      "controls the flux to the line sink.  For p_or_t=temperature, and without "
      "any use_*, this parameter is irrelevant");
  params.addParam<unsigned int>("mass_fraction_component",
                                "The index corresponding to a fluid "
                                "component.  If supplied, the flux will "
                                "be multiplied by the nodal mass "
                                "fraction for the component");
  params.addParam<bool>(
      "use_relative_permeability", false, "Multiply the flux by the fluid relative permeability");
  params.addParam<bool>("use_mobility", false, "Multiply the flux by the fluid mobility");
  params.addParam<bool>("use_enthalpy", false, "Multiply the flux by the fluid enthalpy");
  params.addParam<bool>(
      "use_internal_energy", false, "Multiply the flux by the fluid internal energy");
  params.addCoupledVar("multiplying_var", 1.0, "Fluxes will be moultiplied by this variable");
  params.addClassDescription("Approximates a line sink in the mesh by a sequence of weighted Dirac "
                             "points whose positions are read from a file");
  return params;
}

PorousFlowLineSink::PorousFlowLineSink(const InputParameters & parameters)
  : PorousFlowLineGeometry(parameters),
    _dictator(getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _total_outflow_mass(
        const_cast<PorousFlowSumQuantity &>(getUserObject<PorousFlowSumQuantity>("SumQuantityUO"))),

    _has_porepressure(
        hasMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_porepressure_qp_dvar")),
    _has_temperature(hasMaterialProperty<Real>("PorousFlow_temperature_qp") &&
                     hasMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")),
    _has_mass_fraction(
        hasMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal") &&
        hasMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
            "dPorousFlow_mass_frac_nodal_dvar")),
    _has_relative_permeability(
        hasMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>(
            "dPorousFlow_relative_permeability_nodal_dvar")),
    _has_mobility(
        hasMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>(
            "dPorousFlow_relative_permeability_nodal_dvar") &&
        hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>(
            "dPorousFlow_fluid_phase_density_nodal_dvar") &&
        hasMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>("dPorousFlow_viscosity_nodal_dvar")),
    _has_enthalpy(hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_enthalpy_nodal") &&
                  hasMaterialProperty<std::vector<std::vector<Real>>>(
                      "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")),
    _has_internal_energy(
        hasMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_internal_energy_nodal") &&
        hasMaterialProperty<std::vector<std::vector<Real>>>(
            "dPorousFlow_fluid_phase_internal_energy_nodal_dvar")),

    _p_or_t(getParam<MooseEnum>("function_of").getEnum<PorTchoice>()),
    _use_mass_fraction(isParamValid("mass_fraction_component")),
    _use_relative_permeability(getParam<bool>("use_relative_permeability")),
    _use_mobility(getParam<bool>("use_mobility")),
    _use_enthalpy(getParam<bool>("use_enthalpy")),
    _use_internal_energy(getParam<bool>("use_internal_energy")),

    _ph(getParam<unsigned int>("fluid_phase")),
    _sp(_use_mass_fraction ? getParam<unsigned int>("mass_fraction_component") : 0),

    _pp((_p_or_t == PorTchoice::pressure && _has_porepressure)
            ? &getMaterialProperty<std::vector<Real>>("PorousFlow_porepressure_qp")
            : nullptr),
    _dpp_dvar((_p_or_t == PorTchoice::pressure && _has_porepressure)
                  ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                        "dPorousFlow_porepressure_qp_dvar")
                  : nullptr),
    _temperature((_p_or_t == PorTchoice::temperature && _has_temperature)
                     ? &getMaterialProperty<Real>("PorousFlow_temperature_qp")
                     : nullptr),
    _dtemperature_dvar(
        (_p_or_t == PorTchoice::temperature && _has_temperature)
            ? &getMaterialProperty<std::vector<Real>>("dPorousFlow_temperature_qp_dvar")
            : nullptr),
    _fluid_density_node(
        (_use_mobility && _has_mobility)
            ? &getMaterialProperty<std::vector<Real>>("PorousFlow_fluid_phase_density_nodal")
            : nullptr),
    _dfluid_density_node_dvar((_use_mobility && _has_mobility)
                                  ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                        "dPorousFlow_fluid_phase_density_nodal_dvar")
                                  : nullptr),
    _fluid_viscosity((_use_mobility && _has_mobility)
                         ? &getMaterialProperty<std::vector<Real>>("PorousFlow_viscosity_nodal")
                         : nullptr),
    _dfluid_viscosity_dvar((_use_mobility && _has_mobility)
                               ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                     "dPorousFlow_viscosity_nodal_dvar")
                               : nullptr),
    _relative_permeability(
        ((_use_mobility && _has_mobility) ||
         (_use_relative_permeability && _has_relative_permeability))
            ? &getMaterialProperty<std::vector<Real>>("PorousFlow_relative_permeability_nodal")
            : nullptr),
    _drelative_permeability_dvar(((_use_mobility && _has_mobility) ||
                                  (_use_relative_permeability && _has_relative_permeability))
                                     ? &getMaterialProperty<std::vector<std::vector<Real>>>(
                                           "dPorousFlow_relative_permeability_nodal_dvar")
                                     : nullptr),
    _mass_fractions(
        (_use_mass_fraction && _has_mass_fraction)
            ? &getMaterialProperty<std::vector<std::vector<Real>>>("PorousFlow_mass_frac_nodal")
            : nullptr),
    _dmass_fractions_dvar((_use_mass_fraction && _has_mass_fraction)
                              ? &getMaterialProperty<std::vector<std::vector<std::vector<Real>>>>(
                                    "dPorousFlow_mass_frac_nodal_dvar")
                              : nullptr),
    _enthalpy(_has_enthalpy ? &getMaterialPropertyByName<std::vector<Real>>(
                                  "PorousFlow_fluid_phase_enthalpy_nodal")
                            : nullptr),
    _denthalpy_dvar(_has_enthalpy ? &getMaterialPropertyByName<std::vector<std::vector<Real>>>(
                                        "dPorousFlow_fluid_phase_enthalpy_nodal_dvar")
                                  : nullptr),
    _internal_energy(_has_internal_energy ? &getMaterialPropertyByName<std::vector<Real>>(
                                                "PorousFlow_fluid_phase_internal_energy_nodal")
                                          : nullptr),
    _dinternal_energy_dvar(_has_internal_energy
                               ? &getMaterialPropertyByName<std::vector<std::vector<Real>>>(
                                     "dPorousFlow_fluid_phase_internal_energy_nodal_dvar")
                               : nullptr),
    _multiplying_var(coupledValue("multiplying_var"))
{
  // zero the outflow mass
  _total_outflow_mass.zero();

  if (_ph >= _dictator.numPhases())
    paramError("fluid_phase",
               "The Dictator proclaims that the maximum phase index in this simulation is ",
               _dictator.numPhases() - 1,
               " whereas you have used ",
               _ph,
               ". Remember that indexing starts at 0. You must try harder.");

  if (_use_mass_fraction && _sp >= _dictator.numComponents())
    paramError(
        "mass_fraction_component",
        "The Dictator proclaims that the maximum fluid component index in this simulation is ",
        _dictator.numComponents() - 1,
        " whereas you have used ",
        _sp,
        ". Remember that indexing starts at 0. Please be assured that the Dictator has noted your "
        "error.");

  if (_p_or_t == PorTchoice::pressure && !_has_porepressure)
    mooseError("PorousFlowLineSink: You have specified function_of=porepressure, but you do not "
               "have a quadpoint porepressure material");

  if (_p_or_t == PorTchoice::temperature && !_has_temperature)
    mooseError("PorousFlowLineSink: You have specified function_of=temperature, but you do not "
               "have a quadpoint temperature material");

  if (_use_mass_fraction && !_has_mass_fraction)
    mooseError("PorousFlowLineSink: You have specified a fluid component, but do not have a nodal "
               "mass-fraction material");

  if (_use_relative_permeability && !_has_relative_permeability)
    mooseError("PorousFlowLineSink: You have set use_relative_permeability=true, but do not have a "
               "nodal relative permeability material");

  if (_use_mobility && !_has_mobility)
    mooseError("PorousFlowLineSink: You have set use_mobility=true, but do not have nodal density, "
               "relative permeability or viscosity material");

  if (_use_enthalpy && !_has_enthalpy)
    mooseError("PorousFlowLineSink: You have set use_enthalpy=true, but do not have a nodal "
               "enthalpy material");

  if (_use_internal_energy && !_has_internal_energy)
    mooseError("PorousFlowLineSink: You have set use_internal_energy=true, but do not have a nodal "
               "internal-energy material");

  // To correctly compute the Jacobian terms,
  // tell MOOSE that this DiracKernel depends on all the PorousFlow Variables
  const std::vector<MooseVariableFEBase *> & coupled_vars = _dictator.getCoupledMooseVars();
  for (unsigned int i = 0; i < coupled_vars.size(); i++)
    addMooseVariableDependency(coupled_vars[i]);
}

void
PorousFlowLineSink::addPoints()
{
  // This function gets called just before the DiracKernel is evaluated
  // so this is a handy place to zero this out.
  _total_outflow_mass.zero();

  PorousFlowLineGeometry::addPoints();
}

Real
PorousFlowLineSink::computeQpResidual()
{
  // Get the ID we initially assigned to this point
  const unsigned current_dirac_ptid = currentPointCachedID();
  Real outflow = computeQpBaseOutflow(current_dirac_ptid);
  if (outflow == 0.0)
    return 0.0;

  outflow *= _multiplying_var[_qp];

  if (_use_relative_permeability)
    outflow *= (*_relative_permeability)[_i][_ph];

  if (_use_mobility)
    outflow *= (*_relative_permeability)[_i][_ph] * (*_fluid_density_node)[_i][_ph] /
               (*_fluid_viscosity)[_i][_ph];

  if (_use_mass_fraction)
    outflow *= (*_mass_fractions)[_i][_ph][_sp];

  if (_use_enthalpy)
    outflow *= (*_enthalpy)[_i][_ph];

  if (_use_internal_energy)
    outflow *= (*_internal_energy)[_i][_ph];

  _total_outflow_mass.add(
      outflow * _dt); // this is not thread safe, but DiracKernel's aren't currently threaded

  return outflow;
}

Real
PorousFlowLineSink::computeQpJacobian()
{
  return jac(_var.number());
}

Real
PorousFlowLineSink::computeQpOffDiagJacobian(unsigned int jvar)
{
  return jac(jvar);
}

Real
PorousFlowLineSink::jac(unsigned int jvar)
{
  if (_dictator.notPorousFlowVariable(jvar))
    return 0.0;
  const unsigned pvar = _dictator.porousFlowVariableNum(jvar);

  Real outflow;
  Real outflowp;
  const unsigned current_dirac_ptid = currentPointCachedID();
  computeQpBaseOutflowJacobian(jvar, current_dirac_ptid, outflow, outflowp);
  if (outflow == 0.0 && outflowp == 0.0)
    return 0.0;

  outflow *= _multiplying_var[_qp];
  outflowp *= _multiplying_var[_qp];

  if (_use_relative_permeability)
  {
    const Real relperm_prime = (_i != _j ? 0.0 : (*_drelative_permeability_dvar)[_i][_ph][pvar]);
    outflowp = (*_relative_permeability)[_i][_ph] * outflowp + relperm_prime * outflow;
    outflow *= (*_relative_permeability)[_i][_ph];
  }

  if (_use_mobility)
  {
    const Real mob = (*_relative_permeability)[_i][_ph] * (*_fluid_density_node)[_i][_ph] /
                     (*_fluid_viscosity)[_i][_ph];
    const Real mob_prime =
        (_i != _j
             ? 0.0
             : (*_drelative_permeability_dvar)[_i][_ph][pvar] * (*_fluid_density_node)[_i][_ph] /
                       (*_fluid_viscosity)[_i][_ph] +
                   (*_relative_permeability)[_i][_ph] *
                       (*_dfluid_density_node_dvar)[_i][_ph][pvar] / (*_fluid_viscosity)[_i][_ph] -
                   (*_relative_permeability)[_i][_ph] * (*_fluid_density_node)[_i][_ph] *
                       (*_dfluid_viscosity_dvar)[_i][_ph][pvar] /
                       Utility::pow<2>((*_fluid_viscosity)[_i][_ph]));
    outflowp = mob * outflowp + mob_prime * outflow;
    outflow *= mob;
  }

  if (_use_mass_fraction)
  {
    const Real mass_fractions_prime =
        (_i != _j ? 0.0 : (*_dmass_fractions_dvar)[_i][_ph][_sp][pvar]);
    outflowp = (*_mass_fractions)[_i][_ph][_sp] * outflowp + mass_fractions_prime * outflow;
    outflow *= (*_mass_fractions)[_i][_ph][_sp];
  }

  if (_use_enthalpy)
  {
    const Real enthalpy_prime = (_i != _j ? 0.0 : (*_denthalpy_dvar)[_i][_ph][pvar]);
    outflowp = (*_enthalpy)[_i][_ph] * outflowp + enthalpy_prime * outflow;
    outflow *= (*_enthalpy)[_i][_ph];
  }

  if (_use_internal_energy)
  {
    const Real internal_energy_prime = (_i != _j ? 0.0 : (*_dinternal_energy_dvar)[_i][_ph][pvar]);
    outflowp = (*_internal_energy)[_i][_ph] * outflowp + internal_energy_prime * outflow;
    // this multiplication was performed, but the code does not need to know: outflow *=
    // (*_internal_energy)[_i][_ph];
  }

  return outflowp;
}

Real
PorousFlowLineSink::ptqp() const
{
  return (_p_or_t == PorTchoice::pressure ? (*_pp)[_qp][_ph] : (*_temperature)[_qp]);
}

Real
PorousFlowLineSink::dptqp(unsigned pvar) const
{
  return (_p_or_t == PorTchoice::pressure ? (*_dpp_dvar)[_qp][_ph][pvar]
                                          : (*_dtemperature_dvar)[_qp][pvar]);
}
