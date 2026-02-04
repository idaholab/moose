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
  InputParameters params = MFEMProblem::validParams();
  params.addClassDescription("Problem type for building and solving a finite element eigenproblem "
                             "using the MFEM finite element library.");
  params.addParam<int>("num_modes", 1, "Set the number of lowest eigenmodes to compute.");

  return params;
}

MFEMEigenproblem::MFEMEigenproblem(const InputParameters & params)
  : MFEMProblem(params)
{
}

void
MFEMEigenproblem::addMFEMSolver(const std::string & user_object_name,
                           const std::string & name,
                           InputParameters & parameters)
{
  FEProblemBase::addUserObject(user_object_name, name, parameters);
  auto object_ptr = getUserObject<MFEMSolverBase>(name).getSharedPtr();

  getProblemData().jacobian_solver = std::dynamic_pointer_cast<MFEMEigensolverBase>(object_ptr);
      
  if (!getProblemData().jacobian_solver)
    mooseError("The selected solver '" + name +
               "' is not an eigensolver, but the problem is marked as an eigenproblem.");
}

void
MFEMEigenproblem::addVariable(const std::string & var_type,
                         const std::string & var_name,
                         InputParameters & parameters)
{
  addGridFunction(var_type, var_name, parameters);
  
  for (int i = 0; i < getParam<int>("num_modes"); ++i)
    addGridFunction(var_type, var_name + "_" + std::to_string(i), parameters);
}



#endif
