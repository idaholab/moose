//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VERIFYNODALUNIQUEID_H
#define VERIFYNODALUNIQUEID_H

// MOOSE includes
#include "NodalUserObject.h"

// Forward Declarations
class VerifyNodalUniqueID;

template <>
InputParameters validParams<VerifyNodalUniqueID>();

class VerifyNodalUniqueID : public NodalUserObject
{
public:
  VerifyNodalUniqueID(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  std::vector<dof_id_type> _all_ids;
};

#endif // VERIFYNODALUNIQUEID_H
