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

#ifndef ASSIGNSUBDOMAINID_H
#define ASSIGNSUBDOMAINID_H

// MOOSE includes
#include "MeshModifier.h"

// Forward declerations
class AssignSubdomainID;

template<>
InputParameters validParams<AssignSubdomainID>();

/**
 * MeshModifier for assigning a subdomain ID to all elements
 */
class AssignSubdomainID : public MeshModifier
{
public:
  AssignSubdomainID(const InputParameters & parameters);

protected:
  virtual void modify() override;

  /// The subdomain ID to assign to every elemennt
  SubdomainID _subdomain_id;
};

#endif //ASSIGNSUBDOMAINID_H
