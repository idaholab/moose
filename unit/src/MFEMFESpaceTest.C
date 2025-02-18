#include "gtest/gtest.h"

#include "MFEMMesh.h"
#include "MFEMProblem.h"
#include "AppFactory.h"
#include "MooseMain.h"

struct FECollectionParameters
{
  std::string mesh, fec_type;
  int vector_dim;
  std::string expected_name;
  int expected_fespace_vdim;

  FECollectionParameters(std::string _mesh,
                         std::string _fec_type,
                         int _vector_dim,
                         std::string _expected_name,
                         int _expected_fespace_vdim)
    : mesh(_mesh),
      fec_type(_fec_type),
      vector_dim(_vector_dim),
      expected_name(_expected_name),
      expected_fespace_vdim(_expected_fespace_vdim)
  {
  }
};

class MFEMFESpaceUnitTest : public testing::TestWithParam<FECollectionParameters>
{
public:
  MFEMFESpaceUnitTest()
    : _app(Moose::createMooseApp("PlatypusApp", 0, nullptr)), _factory(_app->getFactory())
  {
    auto test_params = GetParam();
    InputParameters mesh_params = _factory.getValidParams("MFEMMesh");
    mesh_params.set<MeshFileName>("file") = "data/" + test_params.mesh;
    _mfem_mesh_ptr = _factory.createUnique<MFEMMesh>("MFEMMesh", "moose_mesh", mesh_params);
    _mfem_mesh_ptr->setMeshBase(_mfem_mesh_ptr->buildMeshBaseObject());
    _mfem_mesh_ptr->buildMesh();

    InputParameters problem_params = _factory.getValidParams("MFEMProblem");
    problem_params.set<MooseMesh *>("mesh") = _mfem_mesh_ptr.get();
    problem_params.set<std::string>("_object_name") = "name2";
    _mfem_problem = _factory.create<MFEMProblem>("MFEMProblem", "problem", problem_params);

    _app->actionWarehouse().problemBase() = _mfem_problem;
  }

  void buildFECollection()
  {
    auto test_params = GetParam();
    InputParameters fec_params = _factory.getValidParams("MFEMFESpace");
    fec_params.set<MooseEnum>("fec_type") = test_params.fec_type;
    fec_params.set<int>("vdim") = test_params.vector_dim;
    auto objects =
        _mfem_problem->addObject<MFEMFESpace>("MFEMFESpace", "TestCollection", fec_params);
    mooseAssert(objects.size() == 1, "Doesn't work with threading");
    fespace = objects[0];
  }

  std::shared_ptr<MFEMFESpace> fespace;

protected:
  std::unique_ptr<MFEMMesh> _mfem_mesh_ptr;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<MFEMProblem> _mfem_problem;
};

TEST_P(MFEMFESpaceUnitTest, TestExpectedFECollection)
{
  buildFECollection();
  auto params = GetParam();
  EXPECT_EQ(fespace->getFEC()->Name(), params.expected_name);
  EXPECT_EQ(fespace->getFESpace()->GetVDim(), params.expected_fespace_vdim);
}

INSTANTIATE_TEST_CASE_P(
    ScalarFECollections,
    MFEMFESpaceUnitTest,
    testing::Values(FECollectionParameters("ref-segment.mesh", "H1", 0, "H1_1D_P1", 1),
                    FECollectionParameters("ref-segment.mesh", "H1", 1, "H1_1D_P1", 1),
                    FECollectionParameters("ref-segment.mesh", "H1", 2, "H1_1D_P1", 2),
                    FECollectionParameters("ref-segment.mesh", "H1", 3, "H1_1D_P1", 3),
                    FECollectionParameters("ref-square.mesh", "H1", 0, "H1_2D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "H1", 1, "H1_2D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "H1", 2, "H1_2D_P1", 2),
                    FECollectionParameters("ref-square.mesh", "H1", 3, "H1_2D_P1", 3),
                    FECollectionParameters("ref-cube.mesh", "H1", 0, "H1_3D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "H1", 1, "H1_3D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "H1", 2, "H1_3D_P1", 2),
                    FECollectionParameters("ref-cube.mesh", "H1", 3, "H1_3D_P1", 3),
                    FECollectionParameters("ref-segment.mesh", "L2", 0, "L2_1D_P1", 1),
                    FECollectionParameters("ref-segment.mesh", "L2", 1, "L2_1D_P1", 1),
                    FECollectionParameters("ref-segment.mesh", "L2", 2, "L2_1D_P1", 2),
                    FECollectionParameters("ref-segment.mesh", "L2", 3, "L2_1D_P1", 3),
                    FECollectionParameters("ref-square.mesh", "L2", 0, "L2_2D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "L2", 1, "L2_2D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "L2", 2, "L2_2D_P1", 2),
                    FECollectionParameters("ref-square.mesh", "L2", 3, "L2_2D_P1", 3),
                    FECollectionParameters("ref-cube.mesh", "L2", 0, "L2_3D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "L2", 1, "L2_3D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "L2", 2, "L2_3D_P1", 2),
                    FECollectionParameters("ref-cube.mesh", "L2", 3, "L2_3D_P1", 3)));

INSTANTIATE_TEST_CASE_P(
    VectorFECollections,
    MFEMFESpaceUnitTest,
    testing::Values(FECollectionParameters("ref-square.mesh", "RT", 0, "RT_2D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "RT", 0, "RT_3D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "RT", 2, "RT_2D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "RT", 3, "RT_3D_P1", 1),
                    FECollectionParameters("ref-segment.mesh", "ND", 0, "ND_1D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "ND", 0, "ND_2D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "ND", 0, "ND_3D_P1", 1),
                    FECollectionParameters("ref-segment.mesh", "ND", 1, "ND_1D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "ND", 2, "ND_2D_P1", 1),
                    FECollectionParameters("ref-cube.mesh", "ND", 3, "ND_3D_P1", 1)

                        ));

INSTANTIATE_TEST_CASE_P(
    LowerDimVectorFECollections,
    MFEMFESpaceUnitTest,
    testing::Values(FECollectionParameters("ref-segment.mesh", "RT", 3, "RT_R1D_1D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "RT", 3, "RT_R2D_2D_P1", 1),
                    FECollectionParameters("ref-segment.mesh", "ND", 3, "ND_R1D_1D_P1", 1),
                    FECollectionParameters("ref-square.mesh", "ND", 3, "ND_R2D_2D_P1", 1)

                        ));

class InvalidMFEMFESpaceUnitTest : public MFEMFESpaceUnitTest
{
};

TEST_P(InvalidMFEMFESpaceUnitTest, TestFECollectionError)
{
  EXPECT_THROW(buildFECollection(), std::runtime_error);
}

INSTANTIATE_TEST_CASE_P(ExpectError,
                        InvalidMFEMFESpaceUnitTest,
                        testing::Values(FECollectionParameters("ref-segment.mesh", "RT", 2, "", -1),
                                        FECollectionParameters("ref-square.mesh", "RT", 1, "", -1),
                                        FECollectionParameters("ref-cube.mesh", "RT", 1, "", -1),
                                        FECollectionParameters("ref-cube.mesh", "RT", 2, "", -1),
                                        FECollectionParameters("ref-segment.mesh", "ND", 2, "", -1),
                                        FECollectionParameters("ref-square.mesh", "ND", 1, "", -1),
                                        FECollectionParameters("ref-cube.mesh", "ND", 1, "", -1),
                                        FECollectionParameters("ref-cube.mesh", "ND", 2, "", -1)));
