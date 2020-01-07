//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JvarMapTest.h"
#include "Enumerate.h"

#include <vector>
#include <algorithm>

registerMooseObject("MooseTestApp", JvarMapTest);

InputParameters
JvarMapTest::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addCoupledVar("v0", "First variable vector");
  params.addCoupledVar("v1", "Second variable vector");
  return params;
}

JvarMapTest::JvarMapTest(const InputParameters & parameters)
  : JvarMapKernelInterface<Kernel>(parameters),
    _v0_map(getParameterJvarMap("v0")),
    _v1_map(getParameterJvarMap("v1"))
{
  // test if every variable in the couple vectors shows up in the map _and_ if
  // every entry in the map points to the coupled variable vector (bijective
  // relationship)
  testMap("v0", _v0_map);
  testMap("v1", _v1_map);
}

void
JvarMapTest::testMap(const std::string & name, const JvarMap & map)
{
  int nvar = coupledComponents(name);
  std::vector<bool> seen(coupledComponents(name), false);
  for (auto i : Moose::enumerate(map))
  {
    if (i.value() >= 0)
    {
      if (i.value() >= nvar)
        mooseError("out of range ", name, " map entry");

      if (seen[i.value()])
        mooseError("Variable seen twice in ", name, " map");

      // check-off variable as seen
      seen[i.value()] = true;

      // does the variable pointed to by the map have the number corresponding
      // to the map index?
      if (getVar(name, i.value())->number() != i.index())
        mooseError(name, " map inconsistency");
    }
  }

  // any variable left unseen?
  auto it = std::find(seen.begin(), seen.end(), false);
  if (it != seen.end())
    mooseError("Variable missing in ", name, " map");
}
