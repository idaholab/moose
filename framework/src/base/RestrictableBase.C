/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "RestrictableBase.h"

RestrictableBase::RestrictableBase(InputParameters & parameters) :
    _r_feproblem(parameters.isParamValid("_fe_problem") ? parameters.get<FEProblem *>("_fe_problem") : NULL),
    _r_mesh(parameters.isParamValid("_mesh") ? parameters.get<MooseMesh *>("_mesh") : NULL)
{

  // If the mesh pointer is not defined, but FEProblem is, get it from there
  if (_r_feproblem != NULL && _r_mesh == NULL)
    _r_mesh = &_r_feproblem->mesh();

  // Check that the mesh pointer was defined, it is required for this class to operate
  if (_r_mesh == NULL)
    mooseError("The input paramters must contain a pointer to FEProblem via '_fe_problem' or a pointer to the MooseMesh via '_mesh'");
}

RestrictableBase::~RestrictableBase()
{
}
