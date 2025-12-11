//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPostprocessorConvergence.h"

registerMooseObject("MooseApp", MultiPostprocessorConvergence);

InputParameters
MultiPostprocessorConvergence::validParams()
{
  InputParameters params = IterationCountConvergence::validParams();

  params.addClassDescription("Converges if multiple post-processors are all less than tolerances.");

  params.addRequiredParam<std::vector<PostprocessorName>>("postprocessors",
                                                          "Postprocessors to check");
  params.addRequiredParam<std::vector<std::string>>("descriptions",
                                                    "Description of each Postprocessor");
  params.addRequiredParam<std::vector<Real>>("tolerances", "Tolerance for each Postprocessor");

  return params;
}

MultiPostprocessorConvergence::MultiPostprocessorConvergence(const InputParameters & parameters)
  : IterationCountConvergence(parameters),
    _descriptions(getParam<std::vector<std::string>>("descriptions")),
    _tolerances(getParam<std::vector<Real>>("tolerances"))
{
  const auto & pp_names = getParam<std::vector<PostprocessorName>>("postprocessors");
  for (const auto & pp_name : pp_names)
    _pp_values.push_back(&getPostprocessorValueByName(pp_name));

  if (_descriptions.size() != pp_names.size() || _tolerances.size() != pp_names.size())
    mooseError(
        "The parameters 'postprocessors', 'descriptions', and 'tolerances' must be the same size.");
}

void
MultiPostprocessorConvergence::initialSetup()
{
  IterationCountConvergence::initialSetup();

  _max_desc_length = getMaxDescriptionLength();
}

Convergence::MooseConvergenceStatus
MultiPostprocessorConvergence::checkConvergenceInner(unsigned int /*iter*/)
{
  bool all_converged = true;
  std::ostringstream oss;
  oss << "\n";

  for (const auto i : index_range(_descriptions))
  {
    const Real abs_pp_value = std::abs(*_pp_values[i]);
    if (abs_pp_value > _tolerances[i])
      all_converged = false;

    std::string desc = _descriptions[i];
    desc.resize(_max_desc_length + 1, ' '); // pad with spaces for alignment
    oss << comparisonLine(desc, abs_pp_value, _tolerances[i]);
  }

  verboseOutput(oss);

  if (all_converged)
    return Convergence::MooseConvergenceStatus::CONVERGED;
  else
    return Convergence::MooseConvergenceStatus::ITERATING;
}

unsigned int
MultiPostprocessorConvergence::getMaxDescriptionLength() const
{
  size_t max_desc_length = 0;
  for (const auto & description : _descriptions)
    max_desc_length = std::max(max_desc_length, description.length());

  return max_desc_length;
}

std::string
MultiPostprocessorConvergence::comparisonLine(const std::string & description,
                                              Real err,
                                              Real tol) const
{
  std::string color, compare_str;
  if (std::abs(err) > tol)
  {
    color = COLOR_RED;
    compare_str = ">";
  }
  else
  {
    color = COLOR_GREEN;
    compare_str = "<";
  }

  std::ostringstream oss;
  oss << "  " << description << ": " << color << std::scientific << std::setprecision(5)
      << std::abs(err) << " " << compare_str << " " << tol << COLOR_DEFAULT << "\n";
  return oss.str();
}
