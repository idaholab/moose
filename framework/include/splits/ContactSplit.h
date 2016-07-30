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

#ifndef CONTACTSPLIT_H
#define CONTACTSPLIT_H

// MOOSE includes
#include "Split.h"

/**
 * Split-based preconditioner for contact problems.
 */
class ContactSplit : public Split
{
 public:
  ContactSplit(const InputParameters & params);
  virtual void setup(const std::string& prefix = "-") override;

#if defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
 protected:
  std::vector<std::string>              _contact_master;
  std::vector<std::string>              _contact_slave;
  std::vector<bool>                     _contact_displaced;
  std::vector<std::string>              _uncontact_master;
  std::vector<std::string>              _uncontact_slave;
  std::vector<bool>                     _uncontact_displaced;
#endif // defined(LIBMESH_HAVE_PETSC) && !PETSC_VERSION_LESS_THAN(3,3,0)
};


#endif /* CONTACTSPLIT_H */
