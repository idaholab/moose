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

#include "ElementUserObject.h"
#include "libmesh/id_types.h"

//Forward Declarations
class VerifyElementUniqueID;

template<>
InputParameters validParams<VerifyElementUniqueID>();

class VerifyElementUniqueID : public ElementUserObject
{
public:
  VerifyElementUniqueID(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual void threadJoin(const UserObject & y);
  virtual void finalize();

protected:
  std::vector<dof_id_type> _all_ids;
};

#endif //VERIFYELEMENTUNIQUEID_H
