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

#ifndef ASSIGNELEMENTSUBDOMAINID_H
#define ASSIGNELEMENTSUBDOMAINID_H

// MOOSE includes
#include "MeshModifier.h"

// Forward declerations
class AssignElementSubdomainID;

template <>
InputParameters validParams<AssignElementSubdomainID>();

/**
 * MeshModifier for assigning subdomain IDs of all elements
 */
class AssignElementSubdomainID : public MeshModifier
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  AssignElementSubdomainID(const InputParameters & parameters);

protected:
  virtual void modify() override;
};

#endif // ASSIGNELEMENTSUBDOMAINID_H
