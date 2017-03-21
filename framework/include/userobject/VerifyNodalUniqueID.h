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
