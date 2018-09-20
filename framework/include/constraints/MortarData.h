#ifndef MORTARDATA_H
#define MORTARDATA_H

#include "AutomaticMortarGeneration.h"

#include <map>

class SubProblem;

class MortarData
{
public:
  MortarData(SubProblem &);

  AutomaticMortarGeneration &
  getMortarInterface(const std::pair<BoundaryID, BoundaryID> & boundary_key,
                     const std::pair<SubdomainID, SubdomainID> & subdomain_key);

  void update();

protected:
  std::map<std::pair<BoundaryID, BoundaryID>, std::unique_ptr<AutomaticMortarGeneration>>
      _mortar_interfaces;

  SubProblem & _subproblem;
};

#endif
