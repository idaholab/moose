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

#include "BndTestDirichletBC.h"

template<>
InputParameters validParams<BndTestDirichletBC>()
{
  InputParameters p = validParams<NodalBC>();
  MooseEnum test("none, boundaryNames, boundaryIDs, hasBoundary", "none", "Select a test");
  p.addParam<MooseEnum>("test", test, "Select the desired test");
  p.addRequiredParam<Real>("value", "Value of the BC");
  return p;
}


BndTestDirichletBC::BndTestDirichletBC(const std::string & name, InputParameters parameters) :
  NodalBC(name, parameters),
  _value(getParam<Real>("value"))
{

  // Get an test enum from the kernel parameters
  MooseEnum test = parameters.get<MooseEnum>("test");

  // test that hasBlocks is working
  if (test == "hasBoundary")
  {
    // Define a SubdomainName vector for testing against
    std::vector<BoundaryName> id_names(1);
    id_names[0] = "1";

    // Define a SubdomainID vector for testing against
    std::vector<BoundaryID> ids(1);
    ids[0] = 1;

    // Define a SubdomainID set for testing against
    std::set<BoundaryID> id_set(ids.begin(), ids.end());

    // Test true single SudomainName input
    if (!hasBoundary("1"))
      mooseError("Test 1: hasBoundary(SubdomainName) = true failed");

    // Test false of single Subdomain input
    if (hasBoundary("3"))
      mooseError("Test 2: hasBoundary(BoundaryName) = false failed");

    // Test true vector BoundaryName input
    if (!hasBoundary(id_names))
      mooseError("Test 3: hasBoundary(std::vector<BoundaryName>) = true failed");

    // Test false vector SudomainName input
    id_names.push_back("3");
    if (hasBoundary(id_names))
      mooseError("Test 4: hasBoundary(std::vector<BoundaryName>) = false failed");

    // Test true single BoundaryID input
    if (!hasBoundary(1))
      mooseError("Test 5: hasBoundary(BoundaryID) = true failed");

    // Test false single BoundaryID input
    if (hasBoundary(5))
      mooseError("Test 6: hasBoundary(BoundaryID) = false failed");

    // Test true for std::vector<BoundaryID>
    if (!hasBoundary(ids))
      mooseError("Test 7: hasBoundary(std::vector<BoundaryID>) = true failed");

    // Test false for std::vector<BoundaryID>
    ids.push_back(4);
    if (hasBoundary(ids))
      mooseError("Test 8: hasBoundary(std::vector<BoundaryID>) = false failed");

    // Test true for std::set<BoundaryID>
    if (!hasBoundary(id_set))
      mooseError("Test 9: hasBoundary(std::set<BoundaryID) = true failed");

    // Test false for std::set<BoundaryID>
    id_set.insert(12);
    if (hasBoundary(id_set))
      mooseError("Test 10: hasBoundary(std::set<BoundaryID>) = false failed");

    // This is the expected error, all the above tests passed
    mooseError("hasBoundary testing passed");
  }

  // Test that the boundarhNames() method is working
  else if (test == "boundaryNames")
  {
    const std::vector<BoundaryName> & bnds = boundaryNames();
    if (bnds.size() == 1 && bnds[0] == "1")
      mooseError("boundaryNames test passed"); // expected error
    else
      mooseError("boundaryNames test failed");
  }

  // Test that the boundaryIDS() is working
  else if (test == "boundaryIDs")
  {
    const std::set<BoundaryID> & ids = boundaryIDs();
    if (ids.count(1) == 1)
      mooseError("boundaryIDs test passed"); // expected error
    else
      mooseError("boundaryIDs test faild");
  }

  // Test that the isBoundarySubset() is working
  else if (test == "isBoundarySubset")
  {
    std::set<BoundaryID> sub_id;
    sub_id.insert(10);
    sub_id.insert(1);
    sub_id.insert(4);
    sub_id.insert(2);
    if (isBoundarySubset(sub_id))
      mooseError("isBoundarySubset test passed"); // expetect error
    else
      mooseError("isBoundarySubset test failed");
  }
}

Real
BndTestDirichletBC::computeQpResidual()
{
  return _u[_qp] - _value;
}
