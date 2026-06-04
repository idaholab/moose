//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMEigenproblem.h"

registerMooseObject("MooseApp", MFEMEigenproblem);

InputParameters
MFEMEigenproblem::validParams()
{
  InputParameters params = MFEMEigenproblemBase::validParams();
  params.addClassDescription("Problem type for building and solving a real-valued finite element "
                             "eigenproblem using the MFEM finite element library.");
  return params;
}

MFEMEigenproblem::MFEMEigenproblem(const InputParameters & params) : MFEMEigenproblemBase(params)
{
  getProblemData().mode_separator = getParam<std::string>("mode_separator");

  if (getNumericType() == NumericType::COMPLEX)
    mooseError("MFEMEigenproblem is real-valued; use MFEMComplexEigenproblem for complex "
               "eigenproblems.");
}

mfem::Coefficient &
MFEMEigenproblem::getRHSCoefficient()
{
  return getCoefficients().getScalarCoefficient(
      getParam<MFEMScalarCoefficientName>("rhs_coefficient"));
}

void
MFEMEigenproblem::addMFEMSolver(const std::string & type,
                                const std::string & name,
                                InputParameters & parameters)
{
  getProblemData().jacobian_solver =
      addObject<Moose::MFEM::LinearSolverBase>(type, name, parameters).front();

  if (!std::dynamic_pointer_cast<Moose::MFEM::EigensolverBase>(getProblemData().jacobian_solver))
    mooseError("The selected solver '" + name +
               "' is not an eigensolver, but the problem is marked as an eigenproblem.");
}

void
MFEMEigenproblem::addVariable(const std::string & var_type,
                              const std::string & var_name,
                              InputParameters & parameters)
{
  // Reject names that would collide with the mode-suffix convention or with any
  // already-registered eigenmode storage entry.
  const auto num_modes = getParam<int>("num_modes");
  const auto & sep = getProblemData().mode_separator;
  if (getProblemData().gridfunctions.Has(var_name))
    mooseError("MFEM variable '",
               var_name,
               "' clashes with an existing gridfunction (likely an eigenmode entry from another "
               "variable). Choose a different variable name or set 'mode_separator' to a string "
               "that avoids the clash.");

  for (int i = 0; i < num_modes; ++i)
  {
    const auto mode_name = var_name + sep + std::to_string(i);
    if (getProblemData().gridfunctions.Has(mode_name))
      mooseError("Eigenmode storage name '",
                 mode_name,
                 "' for variable '",
                 var_name,
                 "' clashes with an already-registered variable. Set 'mode_separator' to a "
                 "string that avoids the clash, or rename the conflicting variable.");
  }

  addGridFunction(var_type, var_name, parameters);

  for (int i = 0; i < num_modes; ++i)
    addGridFunction(var_type, var_name + sep + std::to_string(i), parameters);
}

#endif
