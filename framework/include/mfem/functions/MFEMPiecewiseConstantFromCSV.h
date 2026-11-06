//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "Function.h"
#include "DelimitedFileReader.h"

class MFEMProblem;

namespace mfem
{
class PWConstCoefficient;
}

/**
 * Creates an MFEM piecewise-constant scalar coefficient from delimited data.
 *
 * Values are mapped to existing MFEM element attributes. The attribute IDs may
 * come from a file column (selected by name or zero-based index) or from the
 * explicit `attributes` input parameter. The value column may likewise be
 * selected by name or zero-based index.
 */
class MFEMPiecewiseConstantFromCSV : public Function
{
public:
  static InputParameters validParams();

  MFEMPiecewiseConstantFromCSV(const InputParameters & parameters);

  void initialSetup() override;
  Real value(Real t, const Point & p) const override;

protected:
  const std::vector<double> & getColumn(const MooseUtils::DelimitedFileReader & reader,
                                        const std::string & name_param,
                                        const std::string & index_param) const;

  std::vector<int> getAttributes(const MooseUtils::DelimitedFileReader & reader,
                                 std::size_t number_of_values) const;

  MFEMProblem & _mfem_problem;
  mfem::PWConstCoefficient & _coefficient;

  const FileName _csv_file;
  const Real _default_value;
  const bool _ignore_empty_lines;
};

#endif
