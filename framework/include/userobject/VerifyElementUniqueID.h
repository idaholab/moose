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

#ifndef VERIFYELEMENTUNIQUEID_H
#define VERIFYELEMENTUNIQUEID_H

// MOOSE includes
#include "ElementUserObject.h"

// Forward Declarations
class VerifyElementUniqueID;

template <>
InputParameters validParams<VerifyElementUniqueID>();

class VerifyElementUniqueID : public ElementUserObject
{
public:
  VerifyElementUniqueID(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;

protected:
  std::vector<dof_id_type> _all_ids;
};

#endif // VERIFYELEMENTUNIQUEID_H
