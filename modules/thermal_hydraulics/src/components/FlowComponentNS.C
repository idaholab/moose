//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FlowComponentNS.h"

registerMooseObject("ThermalHydraulicsApp", FlowComponentNS);

InputParameters
FlowComponentNS::validParams()
{
  InputParameters params = NSFVBase<FileMeshComponent>::validParams();

  params.addClassDescription("Navier-Stokes flow component.");

  return params;
}

FlowComponentNS::FlowComponentNS(const InputParameters & parameters)
  : NSFVBase<FileMeshComponent>(parameters)
{
  checkCopyNSNodalVariables();
}

void FlowComponentNS::addRelationshipManagers(Moose::RelationshipManagerType /*input_rm_type*/)
{
  addRelationshipManagersFromParameters(getGhostParametersForRM());
}

void
FlowComponentNS::addVariables()
{
  addNSVariables();
  addNSInitialConditions();
}

void
FlowComponentNS::addMooseObjects()
{
  addNSUserObjects();
  addNSKernels();
  addNSBoundaryConditions();
  addNSMaterials();
  addNSPostprocessors();
}

void
FlowComponentNS::init()
{
  FileMeshComponent::init();

  processMesh();
  copyNSNodalVariables();
}
