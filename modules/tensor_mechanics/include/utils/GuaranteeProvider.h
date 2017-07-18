/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GUARANTEEPROVIDER_H
#define GUARANTEEPROVIDER_H

// STL includes
#include <map>

// MOOSE includes
#include "MooseTypes.h"
#include "Guarantee.h"

class MooseObject;

/**
 * Add-on class that provides the functionality to issue guarantees for
 * declared material properties. The types of guarantees are listed in Guarantees.h
 */
class GuaranteeProvider
{
public:
  GuaranteeProvider(const MooseObject * moose_object);

  bool hasGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee);

protected:
  void issueGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee);
  void revokeGuarantee(const MaterialPropertyName & prop_name, Guarantee guarantee);

private:
  std::map<MaterialPropertyName, std::set<Guarantee>> _guarantees;
};

#endif // GUARANTEEPROVIDER_H
