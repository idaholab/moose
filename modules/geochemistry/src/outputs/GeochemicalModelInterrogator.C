//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemicalModelInterrogator.h"
#include "GeochemistryFormattedOutput.h"
#include "EquilibriumConstantInterpolator.h"
#include <limits>

registerMooseObject("GeochemistryApp", GeochemicalModelInterrogator);

InputParameters
GeochemicalModelInterrogator::sharedParams()
{
  InputParameters params = emptyInputParameters();
  params.addRequiredParam<UserObjectName>("model_definition",
                                          "The name of the GeochemicalModelDefinition user object");
  params.addParam<std::vector<std::string>>(
      "swap_out_of_basis",
      "Species that should be removed from the model_definition's basis and be replaced with the "
      "swap_into_basis species.  There must be the same number of species in swap_out_of_basis and "
      "swap_into_basis.  If this list contains more than one species, the swapping is performed "
      "one-by-one, starting with the first pair (swap_out_of_basis[0] and swap_into_basis[0]), "
      "then the next pair, etc");
  params.addParam<std::vector<std::string>>(
      "swap_into_basis",
      "Species that should be removed from the model_definition's "
      "equilibrium species list and added to the basis");
  params.addParam<std::vector<std::string>>(
      "activity_species",
      "Species that are provided numerical values of activity (or fugacity for gases) in the "
      "activity_value input");
  params.addParam<std::vector<Real>>(
      "activity_values",
      "Numerical values for the activity (or fugacity) for the "
      "species in the activity_species list.  These are activity values, not log10(activity).");
  params.addParam<unsigned int>(
      "precision",
      4,
      "Precision for printing values.  Also, if the absolute value of a stoichiometric coefficient "
      "is less than 10^(-precision) then it is set to zero.  Also, if equilibrium temperatures are "
      "desired, they will be computed to a relative error of 10^(-precision)");
  params.addParam<std::string>("equilibrium_species",
                               "",
                               "Only output results for this equilibrium species.  If not "
                               "provided, results for all equilibrium species will be outputted");
  MooseEnum interrogation_choice("reaction activity eqm_temperature", "reaction");
  params.addParam<MooseEnum>(
      "interrogation",
      interrogation_choice,
      "Type of interrogation to perform.  reaction: Output equilibrium species reactions and "
      "log10K.  activity: determine activity products at equilibrium.  eqm_temperature: determine "
      "temperature to ensure equilibrium");
  params.addParam<Real>(
      "temperature",
      25,
      "Equilibrium constants will be computed at this temperature [degC].  This is "
      "ignored if interrogation=eqm_temperature.");
  params.addRangeCheckedParam<Real>(
      "stoichiometry_tolerance",
      1E-6,
      "stoichiometry_tolerance >= 0.0",
      "Swapping involves inverting matrices via a singular value decomposition. During this "
      "process: (1) if abs(singular value) < stoi_tol * L1norm(singular values), then the "
      "matrix is deemed singular (so the basis swap is deemed invalid); (2) if abs(any "
      "stoichiometric coefficient) < stoi_tol then it is set to zero.");
  return params;
}

InputParameters
GeochemicalModelInterrogator::validParams()
{
  InputParameters params = Output::validParams();
  params += GeochemicalModelInterrogator::sharedParams();
  params.addClassDescription("Performing simple manipulations of and querying a "
                             "geochemical model");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_FINAL};
  return params;
}

GeochemicalModelInterrogator::GeochemicalModelInterrogator(const InputParameters & parameters)
  : Output(parameters),
    UserObjectInterface(this),
    _mgd(getUserObject<GeochemicalModelDefinition>("model_definition").getDatabase()),
    _swapper(_mgd.basis_species_index.size(), getParam<Real>("stoichiometry_tolerance")),
    _swap_out(getParam<std::vector<std::string>>("swap_out_of_basis")),
    _swap_in(getParam<std::vector<std::string>>("swap_into_basis")),
    _precision(getParam<unsigned int>("precision")),
    _interrogation(getParam<MooseEnum>("interrogation").getEnum<InterrogationChoiceEnum>()),
    _temperature(getParam<Real>("temperature")),
    _activity_species(getParam<std::vector<std::string>>("activity_species")),
    _activity_values(getParam<std::vector<Real>>("activity_values")),
    _equilibrium_species(getParam<std::string>("equilibrium_species"))
{
  if (_swap_out.size() != _swap_in.size())
    paramError("swap_out_of_basis must have same length as swap_into_basis");
  if (_activity_species.size() != _activity_values.size())
    paramError("activity_species must have same length as activity_values");
}

void
GeochemicalModelInterrogator::output(const ExecFlagType & type)
{
  if (!shouldOutput(type))
    return;
  for (const auto & sp : eqmSpeciesOfInterest())
  {
    switch (_interrogation)
    {
      case InterrogationChoiceEnum::REACTION:
        outputReaction(sp);
        break;
      case InterrogationChoiceEnum::ACTIVITY:
        outputActivity(sp);
        break;
      case InterrogationChoiceEnum::EQM_TEMPERATURE:
        outputTemperature(sp);
        break;
    }
  }
  for (unsigned i = 0; i < _swap_out.size(); ++i)
  {
    // any exception here is an error
    try
    {
      _swapper.performSwap(_mgd, _swap_out[i], _swap_in[i]);
    }
    catch (const MooseException & e)
    {
      mooseError(e.what());
    }
    for (const auto & sp : eqmSpeciesOfInterest())
    {
      switch (_interrogation)
      {
        case InterrogationChoiceEnum::REACTION:
          outputReaction(sp);
          break;
        case InterrogationChoiceEnum::ACTIVITY:
          outputActivity(sp);
          break;
        case InterrogationChoiceEnum::EQM_TEMPERATURE:
          outputTemperature(sp);
          break;
      }
    }
  }
}

std::vector<std::string>
GeochemicalModelInterrogator::eqmSpeciesOfInterest() const
{
  if (_equilibrium_species == "")
    return _mgd.eqm_species_name;
  else if (_mgd.eqm_species_index.count(_equilibrium_species) == 1)
    return {_equilibrium_species};
  return {};
}

void
GeochemicalModelInterrogator::outputReaction(const std::string & eqm_species) const
{
  if (_mgd.eqm_species_index.count(eqm_species) == 0)
    return;
  std::stringstream ss;
  const unsigned row = _mgd.eqm_species_index.at(eqm_species);
  const Real cutoff = std::pow(10.0, -1.0 * _precision);
  const std::vector<Real> temps = _mgd.original_database->getTemperatures();
  const unsigned numT = temps.size();
  const std::string model_type = _mgd.original_database->getLogKModel();
  EquilibriumConstantInterpolator log10K(
      temps, _mgd.eqm_log10K.sub_matrix(row, 1, 0, numT).get_values(), model_type);
  log10K.generate();
  const Real log10_eqm_const = log10K.sample(_temperature);
  ss << std::setprecision(_precision);
  ss << eqm_species << " = ";
  ss << GeochemistryFormattedOutput::reaction(
      _mgd.eqm_stoichiometry, row, _mgd.basis_species_name, cutoff, _precision);
  ss << "  .  log10(K) = " << log10_eqm_const;
  ss << std::endl;
  _console << ss.str() << std::flush;
}

bool
GeochemicalModelInterrogator::knownActivity(const std::string & species) const
{
  for (const auto & sp : _activity_species)
    if (sp == species)
      return true;
  if (_mgd.basis_species_index.count(species))
    return _mgd.basis_species_mineral[_mgd.basis_species_index.at(species)];
  if (_mgd.eqm_species_index.count(species))
    return _mgd.eqm_species_mineral[_mgd.eqm_species_index.at(species)];
  return false;
}

Real
GeochemicalModelInterrogator::getActivity(const std::string & species) const
{
  unsigned ind = 0;
  for (const auto & sp : _activity_species)
  {
    if (sp == species)
      return _activity_values[ind];
    ind += 1;
  }
  return 1.0; // must be a mineral
}

void
GeochemicalModelInterrogator::outputActivity(const std::string & eqm_species) const
{
  if (_mgd.eqm_species_index.count(eqm_species) == 0)
    return;

  const unsigned row = _mgd.eqm_species_index.at(eqm_species);
  const unsigned num_cols = _mgd.basis_species_index.size();
  const Real cutoff = std::pow(10.0, -1.0 * _precision);
  const std::vector<Real> temps = _mgd.original_database->getTemperatures();
  const unsigned numT = temps.size();
  const std::string model_type = _mgd.original_database->getLogKModel();
  EquilibriumConstantInterpolator log10K(
      temps, _mgd.eqm_log10K.sub_matrix(row, 1, 0, numT).get_values(), model_type);
  log10K.generate();
  Real rhs = log10K.sample(_temperature);
  std::stringstream lhs;

  lhs << std::setprecision(_precision);
  if (knownActivity(eqm_species))
    rhs += std::log10(getActivity(eqm_species));
  else
    lhs << "(A_" << eqm_species << ")^-1 ";

  for (unsigned i = 0; i < num_cols; ++i)
    if (std::abs(_mgd.eqm_stoichiometry(row, i)) > cutoff)
    {
      const std::string sp = _mgd.basis_species_name[i];
      if (knownActivity(sp))
        rhs -= _mgd.eqm_stoichiometry(row, i) * std::log10(getActivity(sp));
      else
        lhs << "(A_" << sp << ")^" << _mgd.eqm_stoichiometry(row, i) << " ";
    }

  lhs << "= 10^" << rhs << std::endl;
  _console << lhs.str() << std::flush;
}

void
GeochemicalModelInterrogator::outputTemperature(const std::string & eqm_species) const
{
  if (_mgd.eqm_species_index.count(eqm_species) == 0)
    return;

  const unsigned row = _mgd.eqm_species_index.at(eqm_species);
  const unsigned num_cols = _mgd.basis_species_index.size();
  const Real cutoff = std::pow(10.0, -1.0 * _precision);
  Real rhs = 1.0;

  // check that we know all the required activities, otherwise compute the RHS
  if (knownActivity(eqm_species))
    rhs = -std::log10(getActivity(eqm_species));
  else
  {
    _console << "Not enough activites known to compute equilibrium temperature for reaction\n  "
             << std::flush;
    outputReaction(eqm_species);
    return;
  }

  for (unsigned i = 0; i < num_cols; ++i)
    if (std::abs(_mgd.eqm_stoichiometry(row, i)) > cutoff)
    {
      const std::string sp = _mgd.basis_species_name[i];
      if (knownActivity(sp))
        rhs += _mgd.eqm_stoichiometry(row, i) * std::log10(getActivity(sp));
      else
      {
        _console << "Not enough activites known to compute equilibrium temperature for reaction\n  "
                 << std::flush;
        outputReaction(eqm_species);
        return;
      }
    }

  const Real tsoln = solveForT(
      _mgd.eqm_log10K.sub_matrix(row, 1, 0, _mgd.original_database->getTemperatures().size()), rhs);
  _console << eqm_species << ".  T = " << tsoln << "degC" << std::endl;
}

Real
GeochemicalModelInterrogator::solveForT(const DenseMatrix<Real> & reference_log10K, Real rhs) const
{
  const std::vector<Real> temps = _mgd.original_database->getTemperatures();
  const unsigned numT = temps.size();
  const std::string model_type = _mgd.original_database->getLogKModel();

  // find the bracket that contains the rhs
  unsigned bracket = 0;
  for (bracket = 0; bracket < numT - 1; ++bracket)
  {
    if (reference_log10K(0, bracket) <= rhs && reference_log10K(0, bracket + 1) > rhs)
      break;
    else if (reference_log10K(0, bracket) >= rhs && reference_log10K(0, bracket + 1) < rhs)
      break;
  }

  if (bracket == numT - 1)
    return std::numeric_limits<double>::quiet_NaN();

  EquilibriumConstantInterpolator log10K(temps, reference_log10K.get_values(), model_type);
  log10K.generate();
  // now do a Newton-Raphson to find T for which log10K.sample(T) = rhs
  Real temp = _mgd.original_database->getTemperatures()[bracket + 1];
  const Real small_delT =
      (temp + GeochemistryConstants::CELSIUS_TO_KELVIN) * std::pow(10.0, -1.0 * _precision);
  Real del_temp = small_delT;
  unsigned iter = 0;
  while (std::abs(del_temp) >= small_delT && iter++ < 100)
  {
    Real residual = log10K.sample(temp) - rhs;
    del_temp = -residual / log10K.sampleDerivative(temp);
    temp += del_temp;
  }
  return temp;
}
