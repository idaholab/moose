//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPostprocessorConvergence.h"

InputParameters
MultiPostprocessorConvergence::validParams()
{
  InputParameters params = Convergence::validParams();
  return params;
}

MultiPostprocessorConvergence::MultiPostprocessorConvergence(const InputParameters & parameters)
  : Convergence(parameters)
{
}

void
MultiPostprocessorConvergence::initialSetup()
{
  Convergence::initialSetup();

  _max_desc_length = getMaxDescriptionLength();
}

Convergence::MooseConvergenceStatus
MultiPostprocessorConvergence::checkConvergence(unsigned int iter)
{
  bool all_converged = true;
  std::ostringstream oss;
  oss << "\n";

  const auto desc_err_tol_tuples = getDescriptionErrorToleranceTuples();
  for (const auto & desc_err_tol_tuple : desc_err_tol_tuples)
  {
    const Real err = std::get<1>(desc_err_tol_tuple);
    const Real tol = std::get<2>(desc_err_tol_tuple);
    if (std::abs(err) > tol)
      all_converged = false;

    std::string desc = std::get<0>(desc_err_tol_tuple);
    desc.resize(_max_desc_length + 1, ' '); // pad with spaces for alignment
    oss << comparisonLine(desc, err, tol);
  }

  if (iter < getMinimumIterations())
  {
    oss << "  Continuing iteration due to minimum iterations\n";
    verboseOutput(oss);
    return Convergence::MooseConvergenceStatus::ITERATING;
  }
  else
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
  const auto desc_err_tol_tuples = getDescriptionErrorToleranceTuples();
  for (const auto & desc_err_tol_tuple : desc_err_tol_tuples)
  {
    const auto desc = std::get<0>(desc_err_tol_tuple);
    max_desc_length = std::max(max_desc_length, desc.length());
  }

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
