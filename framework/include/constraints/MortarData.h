#ifndef MORTARDATA_H
#define MORTARDATA_H

#include "AutomaticMortarGeneration.h"

#include <map>

class SubProblem;

class MortarData
{
public:
  MortarData(SubProblem &);

  AutomaticMortarGeneration & getMortarInterface(const std::pair<unsigned, unsigned> &);

  void update();

protected:
  std::map<std::pair<unsigned, unsigned>, std::unique_ptr<AutomaticMortarGeneration>>
      _mortar_interfaces;

  SubProblem & _subproblem;
};

#endif
