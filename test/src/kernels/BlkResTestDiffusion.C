#include "BlkResTestDiffusion.h"
#include "MooseEnum.h"
#include "MooseTypes.h"

template<>
InputParameters validParams<BlkResTestDiffusion>()
{
  MooseEnum test("none, fe_problem_null, mesh_null, use_mesh, hasBlocks, blocks, blockIDs, isBlockSubset, hasBlockMaterialProperty_true, hasBlockMaterialProperty_false", "none", "Select a test");
  InputParameters params = validParams<Kernel>();
  params.addParam<MooseEnum>("test", test, "Select the desired test");
  return params;
}

// A function to modify the paramters for testing purposes
InputParameters & modifyParams(InputParameters & params)
{
  // Get the FEProblem pointer
  FEProblem* fe_ptr = params.get<FEProblem*>("_fe_problem");

  // Get the test enum
  MooseEnum test = params.get<MooseEnum>("test");

  // test_fe_problem_null
  switch (test)
  {
  case 1: // null fe_problem pointer
    params.set<FEProblem*>("_fe_problem") = NULL;
    break;

  case 2: // null mesh pointer
    params.suppressParameter<std::vector<SubdomainName> >("block");
    params.suppressParameter<FEProblem*>("_fe_problem");
    params.suppressParameter<NonlinearVariableName>("variable");
    params.set<MooseMesh*>("_mesh") = NULL;
    break;

  case 3: // use _mesh
    params.suppressParameter<std::vector<SubdomainName> >("block");
    params.suppressParameter<FEProblem*>("_fe_problem");
    params.suppressParameter<NonlinearVariableName>("variable");
    params.set<MooseMesh*>("_mesh") = &fe_ptr->mesh();
    break;
  }

  // Return the modified parameters
  return params;
}


BlkResTestDiffusion::BlkResTestDiffusion(const std::string & name, InputParameters parameters) :
    Kernel(name, modifyParams(parameters))
{

  // Get an test enum from the kernel parameters
  MooseEnum test = parameters.get<MooseEnum>("test");

  // test that hasBlocks is working
  if (test == "hasBlocks")
  {
    // Define a SubdomainName vector for testing against
    std::vector<SubdomainName> id_names(2);
    id_names[0] = "1";
    id_names[1] = "2";

    // Define a SubdomainID vector for testing against
    std::vector<SubdomainID> ids(2);
    ids[0] = 1;
    ids[1] = 2;

    // Define a SubdomainID set for testing against
    std::set<SubdomainID> id_set(ids.begin(), ids.end());

    // Test true single SudomainName input
    if (!hasBlocks("1"))
      mooseError("Test 1: hasBlocks(SubdomainName) = true failed");

    // Test false of single Subdomain input
    if (hasBlocks("3"))
      mooseError("Test 2: hasBlocks(SubdomainName) = false failed");

    // Test true vector SubdomainName input
    if (!hasBlocks(id_names))
      mooseError("Test 3: hasBlocks(std::vector<SubdomainName>) = true failed");

    // Test false vector SudomainName input
    id_names.push_back("3");
    if (hasBlocks(id_names))
      mooseError("Test 4: hasBlocks(std::vector<SubdomainName>) = false failed");

    // Test true single SubdomainID input
    if (!hasBlocks(1))
      mooseError("Test 5: hasBlocks(SubdomainID) = true failed");

    // Test false single SubdomainID input
    if (hasBlocks(5))
      mooseError("Test 6: hasBlocks(SubdomainID) = false failed");

    // Test true for std::vector<SubdomainID>
    if (!hasBlocks(ids))
      mooseError("Test 7: hasBlocks(std::vector<SubdomainID>) = true failed");

    // Test false for std::vector<SubdomainID>
    ids.push_back(4);
    if (hasBlocks(ids))
      mooseError("Test 8: hasBlocks(std::vector<SubdomainID>) = false failed");

    // Test true for std::set<SubdomainID>
    if (!hasBlocks(id_set))
      mooseError("Test 9: hasBlocks(std::set<SubdomainID) = true failed");

    // Test false for std::set<SubdomainID>
    id_set.insert(12);
    if (hasBlocks(id_set))
      mooseError("Test 10: hasBlocks(std::set<SubdomainID>) = false failed");

    // This is the expected error, all the above tests passed
    mooseError("hasBlocks testing passed");
  }

  // Test that the blocks() method is working
  else if (test == "blocks")
  {
    const std::vector<SubdomainName> & blks = blocks();
    if (blks[0] == "1" && blks[1] == "2" && blks.size() == 2)
      mooseError("Blocks testing passed"); // expected error
    else
      mooseError("Blocks testing failed");
  }

  // Test that the getSubdomains() is working
  else if (test == "blockIDs")
  {
    const std::set<SubdomainID> & ids = blockIDs();
    if (ids.count(1) == 1 && ids.count(2) == 1)
      mooseError("blockIDs testing passed"); // expected error
    else
      mooseError("blockIDs testing failed");
  }

  // Test that the isSubset() is working
  else if (test == "isBlockSubset")
  {
    std::set<SubdomainID> sub_id;
    sub_id.insert(10);
    sub_id.insert(1);
    sub_id.insert(4);
    sub_id.insert(2);
    if (isBlockSubset(sub_id))
      mooseError("isBlockSubset testing passed"); // expected error
    else
      mooseError("isBlockSubset testing failed");
  }

  // Test that hasMaterialPropertyBlock is working properly
  else if (test == "hasBlockMaterialProperty_true")
  {
    if (hasBlockMaterialProperty<Real>("a"))
      mooseError("hasBlockMaterialProperty is true, test passed"); // expected error
    else
      mooseError("hasBlockMaterialProperty is false, test failed");
  }

  else if (test == "hasBlockMaterialProperty_false")
  {
    if (hasBlockMaterialProperty<Real>("b"))
      mooseError("hasBlockMaterialProperty is true, test failed");
    else
      mooseError("hasBlockMaterialProperty is false, test passed"); // expected error
  }
}

Real
BlkResTestDiffusion::computeQpResidual()
{
  return _grad_test[_i][_qp]*_grad_u[_qp];
}

Real
BlkResTestDiffusion::computeQpJacobian()
{
  return _grad_test[_i][_qp]*_grad_phi[_j][_qp];
}
