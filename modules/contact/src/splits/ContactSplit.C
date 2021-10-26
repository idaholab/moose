//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactSplit.h"
#include "InputParameters.h"
#include "FEProblem.h"
#include "Conversion.h"

registerMooseObject("ContactApp", ContactSplit);

InputParameters
ContactSplit::validParams()
{
  InputParameters params = Split::validParams();
  params.addParam<std::vector<BoundaryName>>("contact_primary",
                                             "Primary surface list for included contacts");
  params.addParam<std::vector<BoundaryName>>("contact_secondary",
                                             "Secondary surface list for included contacts");
  params.addParam<std::vector<int>>(
      "contact_displaced",
      "List of indicators whether displaced mesh is used to define included contact");
  params.addParam<std::vector<BoundaryName>>("uncontact_primary",
                                             "Primary surface list for excluded contacts");
  params.addParam<std::vector<BoundaryName>>("uncontact_secondary",
                                             "Secondary surface list for excluded contacts");
  params.addParam<std::vector<int>>(
      "uncontact_displaced",
      "List of indicators whether displaced mesh is used to define excluded contact");
  params.addRequiredParam<bool>("include_all_contact_nodes",
                                "Whether to include all nodes on the contact surfaces");
  params.addClassDescription("Split-based preconditioner that partitions the domain into DOFs "
                             "directly involved in contact (on contact surfaces) and those "
                             "that are not");
  return params;
}

ContactSplit::ContactSplit(const InputParameters & params)
  : Split(params),
    _contact_pairs(getParam<BoundaryName, BoundaryName>("contact_primary", "contact_secondary")),
    _contact_displaced(getParam<std::vector<int>>("contact_displaced")),
    _uncontact_pairs(
        getParam<BoundaryName, BoundaryName>("uncontact_primary", "uncontact_secondary")),
    _uncontact_displaced(getParam<std::vector<int>>("uncontact_displaced")),
    _include_all_contact_nodes(getParam<bool>("include_all_contact_nodes"))
{
  if (!_contact_displaced.empty() && _contact_pairs.size() != _contact_displaced.size())
    mooseError("Primary and displaced contact lists must have matching sizes: ",
               _contact_pairs.size(),
               " != ",
               _contact_displaced.size());

  if (_contact_displaced.empty())
    _contact_displaced.resize(_contact_pairs.size());

  if (!_uncontact_displaced.empty() && _uncontact_pairs.size() != _uncontact_displaced.size())
    mooseError("Primary and displaced uncontact lists must have matching sizes: ",
               _uncontact_pairs.size(),
               " != ",
               _uncontact_displaced.size());

  if (!_uncontact_displaced.size())
    _uncontact_displaced.resize(_uncontact_pairs.size());
}

void
ContactSplit::setup(const std::string & prefix)
{
  // A reference to the PetscOptions
  Moose::PetscSupport::PetscOptions & po = _fe_problem.getPetscOptions();
  // prefix
  const std::string dmprefix = prefix + "dm_moose_";

  // contacts options
  if (!_contact_pairs.empty())
  {
    // append PETSc options
    po.pairs.emplace_back(dmprefix + "ncontacts", Moose::stringify(_contact_pairs.size()));

    for (std::size_t j = 0; j < _contact_pairs.size(); ++j)
    {
      auto opt = dmprefix + "contact_" + Moose::stringify(j);
      po.pairs.emplace_back(opt, Moose::stringify(_contact_pairs[j], ","));

      if (_contact_displaced[j])
        po.pairs.emplace_back(opt + "_displaced", "yes");
    }
  }

  // uncontacts options
  if (!_uncontact_pairs.empty())
  {
    po.pairs.emplace_back(dmprefix + "nuncontacts", Moose::stringify(_uncontact_pairs.size()));

    for (std::size_t j = 0; j < _uncontact_pairs.size(); ++j)
    {
      auto opt = dmprefix + "uncontact_" + Moose::stringify(j);
      po.pairs.emplace_back(opt, Moose::stringify(_uncontact_pairs[j], ","));

      if (_uncontact_displaced[j])
        po.pairs.emplace_back(opt + "_displaced", "yes");
    }
  }

  // Whether to include all nodes on the contact surfaces
  // into the contact subsolver
  po.pairs.emplace_back(dmprefix + "includeAllContactNodes",
                        _include_all_contact_nodes ? "yes" : "no");
  Split::setup(prefix);
}
