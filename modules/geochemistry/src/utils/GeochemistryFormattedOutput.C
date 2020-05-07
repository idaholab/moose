//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeochemistryFormattedOutput.h"

namespace GeochemistryFormattedOutput
{
std::string
reaction(const DenseMatrix<Real> & stoi,
         unsigned row,
         const std::vector<std::string> & names,
         Real stoi_tol,
         int precision)
{
  if (row >= stoi.m())
    mooseError("GeochemistryFormattedOutput::reaction called with stoichiometric matrix having ",
               stoi.m(),
               " rows, but row = ",
               row);
  const unsigned num_cols = stoi.n();
  if (num_cols != names.size())
    mooseError("GeochemistryFormattedOutput::reaction called with stoichiometric matrix having ",
               num_cols,
               " columns, but names has size ",
               names.size());
  std::stringstream ss;
  ss << std::setprecision(precision);
  bool printed_something = false;
  for (unsigned i = 0; i < num_cols; ++i)
    if (stoi(row, i) > stoi_tol)
    {
      if (!printed_something)
      {
        ss << stoi(row, i) << "*" << names[i];
        printed_something = true;
      }
      else
        ss << " + " << stoi(row, i) << "*" << names[i];
    }
    else if (stoi(row, i) < -stoi_tol)
    {
      if (!printed_something)
      {
        ss << "-" << -stoi(row, i) << "*" << names[i];
        printed_something = true;
      }
      else
        ss << " - " << -stoi(row, i) << "*" << names[i];
    }
  return ss.str();
}
}
