#ifdef MOOSE_MFEM_ENABLED

#include "gtest/gtest.h"

#include "MFEMMesh.h"
#include "MFEMProblem.h"
#include "AppFactory.h"
#include "MooseMain.h"
#include "MFEMGenericFESpace.h"
#include "MFEMVectorFESpace.h"
#include "MFEMScalarFESpace.h"

// Mesh file name, finite element collection parameters, expected FECollection name, expected vdim
template <class... T>
using FESpaceParameters = std::tuple<std::string, std::tuple<T...>, std::string, int, bool>;

template <class T, class... Args>
class MFEMFESpaceUnitTest : public testing::TestWithParam<FESpaceParameters<Args...>>
{
public:
  MFEMFESpaceUnitTest()
    : _app(Moose::createMooseApp("MooseUnitApp", 0, nullptr)), _factory(_app->getFactory())
  {
    InputParameters mesh_params = _factory.getValidParams("MFEMMesh");
    mesh_params.set<MeshFileName>("file") =
        "../test/tests/mfem/mesh/" + std::get<0>(this->GetParam());
    _mfem_mesh_ptr = _factory.createUnique<MFEMMesh>("MFEMMesh", "moose_mesh", mesh_params);
    _mfem_mesh_ptr->setMeshBase(_mfem_mesh_ptr->buildMeshBaseObject());
    _mfem_mesh_ptr->buildMesh();

    InputParameters problem_params = _factory.getValidParams("MFEMProblem");
    problem_params.set<MooseMesh *>("mesh") = _mfem_mesh_ptr.get();
    problem_params.set<std::string>(MooseBase::name_param) = "name2";
    _mfem_problem = _factory.create<MFEMProblem>("MFEMProblem", "problem", problem_params);

    _app->actionWarehouse().problemBase() = _mfem_problem;
  }

  virtual InputParameters GetInputParameters() const = 0;
  virtual std::string GetFESpaceClass() const = 0;

  void buildFESpace()
  {
    InputParameters fespace_params = GetInputParameters();
    auto objects = _mfem_problem->addObject<T>(GetFESpaceClass(), "TestSpace", fespace_params);
    mooseAssert(objects.size() == 1, "Doesn't work with threading");
    fespace = objects[0];
  }

  void validate()
  {
    auto & params = this->GetParam();
    EXPECT_EQ(fespace->getFEC()->Name(), std::get<2>(params));
    EXPECT_EQ(fespace->getFESpace()->GetVDim(), std::get<3>(params));
    EXPECT_EQ(fespace->isScalar(), std::get<4>(params));
    EXPECT_EQ(fespace->isVector(), !std::get<4>(params));
  }

  static FESpaceParameters<Args...> makeParam(std::string mesh_name,
                                              Args &&... input_params,
                                              std::string expected_fec,
                                              int expected_vdim,
                                              bool expected_scalar)
  {
    return std::make_tuple(
        mesh_name, std::make_tuple(input_params...), expected_fec, expected_vdim, expected_scalar);
  }

  std::shared_ptr<MFEMFESpace> fespace;

protected:
  std::unique_ptr<MFEMMesh> _mfem_mesh_ptr;
  std::shared_ptr<MooseApp> _app;
  Factory & _factory;
  std::shared_ptr<MFEMProblem> _mfem_problem;
};

class GenericFESpaceTest : public MFEMFESpaceUnitTest<MFEMGenericFESpace, std::string, int>
{
  virtual std::string GetFESpaceClass() const override { return "MFEMGenericFESpace"; }

  virtual InputParameters GetInputParameters() const override
  {
    const auto & raw_params = std::get<1>(GetParam());
    InputParameters params = _factory.getValidParams("MFEMGenericFESpace");
    params.set<std::string>("fec_name") = std::get<0>(raw_params);
    params.set<int>("vdim") = std::get<1>(raw_params);
    return params;
  }
};

TEST_P(GenericFESpaceTest, TestExpectedGenericFESpace)
{
  buildFESpace();
  validate();
}

INSTANTIATE_TEST_SUITE_P(
    GenericFESpaces,
    GenericFESpaceTest,
    testing::Values(
        GenericFESpaceTest::makeParam("ref-segment.mesh", "H1_1D_P1", 1, "H1_1D_P1", 1, true),
        GenericFESpaceTest::makeParam("ref-segment.mesh", "H1_3D_P1", 1, "H1_3D_P1", 1, true),
        GenericFESpaceTest::makeParam("ref-segment.mesh", "H1_2D_P1", 7, "H1_2D_P1", 7, false),
        GenericFESpaceTest::makeParam("ref-cube.mesh", "H1_3D_P3", 1, "H1_3D_P3", 1, true),
        GenericFESpaceTest::makeParam("ref-segment.mesh", "L2_1D_P1", 1, "L2_1D_P1", 1, true),
        GenericFESpaceTest::makeParam("ref-segment.mesh", "L2_2D_P1", 2, "L2_2D_P1", 2, false),
        GenericFESpaceTest::makeParam("ref-segment.mesh", "L2_3D_P1", 3, "L2_3D_P1", 3, false),
        GenericFESpaceTest::makeParam("ref-cube.mesh", "L2_3D_P3", 1, "L2_3D_P3", 1, true),
        GenericFESpaceTest::makeParam("ref-segment.mesh", "ND_1D_P1", 1, "ND_1D_P1", 1, false),
        GenericFESpaceTest::makeParam("ref-square.mesh", "ND_2D_P1", 1, "ND_2D_P1", 1, false),
        GenericFESpaceTest::makeParam("ref-cube.mesh", "ND_3D_P1", 1, "ND_3D_P1", 1, false),
        GenericFESpaceTest::makeParam("ref-square.mesh", "ND_2D_P1", 3, "ND_2D_P1", 3, false),
        GenericFESpaceTest::makeParam(
            "ref-segment.mesh", "ND_R1D_1D_P1", 1, "ND_R1D_1D_P1", 1, false),
        GenericFESpaceTest::makeParam("ref-square.mesh", "RT_2D_P1", 1, "RT_2D_P1", 1, false),
        GenericFESpaceTest::makeParam("ref-cube.mesh", "RT_3D_P1", 1, "RT_3D_P1", 1, false),
        GenericFESpaceTest::makeParam("ref-square.mesh", "RT_2D_P1", 3, "RT_2D_P1", 3, false),
        GenericFESpaceTest::makeParam(
            "ref-square.mesh", "RT_R2D_1D_P3", 1, "RT_R2D_1D_P3", 1, false),
        GenericFESpaceTest::makeParam(
            "ref-cube.mesh", "DG_Iface_3D_P3", 1, "DG_Iface_3D_P3", 1, true),
        GenericFESpaceTest::makeParam(
            "ref-segment.mesh", "H1_Trace_2D_P4", 2, "H1_Trace_2D_P4", 2, false)));

class ScalarFESpaceTest : public MFEMFESpaceUnitTest<MFEMScalarFESpace, std::string, int>
{
  virtual std::string GetFESpaceClass() const override { return "MFEMScalarFESpace"; }

  virtual InputParameters GetInputParameters() const override
  {
    const auto & raw_params = std::get<1>(GetParam());
    InputParameters params = _factory.getValidParams("MFEMScalarFESpace");
    params.set<MooseEnum>("fec_type") = std::get<0>(raw_params);
    params.set<MooseEnum>("fec_order") = std::get<1>(raw_params);
    return params;
  }
};

TEST_P(ScalarFESpaceTest, TestExpectedScalarFESpace)
{
  buildFESpace();
  validate();
}

INSTANTIATE_TEST_SUITE_P(
    ScalarFESpaces,
    ScalarFESpaceTest,
    testing::Values(ScalarFESpaceTest::makeParam("ref-segment.mesh", "H1", 1, "H1_1D_P1", 1, true),
                    ScalarFESpaceTest::makeParam("ref-square.mesh", "H1", 1, "H1_2D_P1", 1, true),
                    ScalarFESpaceTest::makeParam("ref-cube.mesh", "H1", 1, "H1_3D_P1", 1, true),
                    ScalarFESpaceTest::makeParam("ref-segment.mesh", "H1", 2, "H1_1D_P2", 1, true),
                    ScalarFESpaceTest::makeParam("ref-square.mesh", "H1", 3, "H1_2D_P3", 1, true),
                    ScalarFESpaceTest::makeParam("ref-cube.mesh", "H1", 4, "H1_3D_P4", 1, true),
                    ScalarFESpaceTest::makeParam("ref-segment.mesh", "L2", 1, "L2_1D_P1", 1, true),
                    ScalarFESpaceTest::makeParam("ref-square.mesh", "L2", 1, "L2_2D_P1", 1, true),
                    ScalarFESpaceTest::makeParam("ref-cube.mesh", "L2", 1, "L2_3D_P1", 1, true),
                    ScalarFESpaceTest::makeParam("ref-segment.mesh", "L2", 2, "L2_1D_P2", 1, true),
                    ScalarFESpaceTest::makeParam("ref-square.mesh", "L2", 3, "L2_2D_P3", 1, true),
                    ScalarFESpaceTest::makeParam("ref-cube.mesh", "L2", 4, "L2_3D_P4", 1, true)));

class VectorFESpaceTest : public MFEMFESpaceUnitTest<MFEMVectorFESpace, std::string, int, int>
{
  virtual std::string GetFESpaceClass() const override { return "MFEMVectorFESpace"; }

  virtual InputParameters GetInputParameters() const override
  {
    const auto & raw_params = std::get<1>(GetParam());
    InputParameters params = _factory.getValidParams("MFEMVectorFESpace");
    params.set<MooseEnum>("fec_type") = std::get<0>(raw_params);
    params.set<MooseEnum>("fec_order") = std::get<1>(raw_params);
    params.set<int>("range_dim") = std::get<2>(raw_params);
    return params;
  }
};

TEST_P(VectorFESpaceTest, TestExpectedVectorFESpace)
{
  buildFESpace();
  validate();
}

INSTANTIATE_TEST_SUITE_P(
    VectorFESpaces,
    VectorFESpaceTest,
    testing::Values(
        VectorFESpaceTest::makeParam("ref-segment.mesh", "H1", 1, 0, "H1_1D_P1", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "H1", 10, 1, "H1_1D_P10", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "H1", 1, 2, "H1_1D_P1", 2, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "H1", 2, 3, "H1_1D_P2", 3, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "H1", 1, 0, "H1_2D_P1", 2, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "H1", 2, 1, "H1_2D_P2", 1, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "H1", 1, 2, "H1_2D_P1", 2, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "H1", 2, 3, "H1_2D_P2", 3, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "H1", 3, 0, "H1_3D_P3", 3, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "H1", 3, 1, "H1_3D_P3", 1, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "H1", 3, 2, "H1_3D_P3", 2, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "H1", 3, 3, "H1_3D_P3", 3, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "L2", 1, 0, "L2_1D_P1", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "L2", 2, 1, "L2_1D_P2", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "L2", 1, 2, "L2_1D_P1", 2, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "L2", 2, 3, "L2_1D_P2", 3, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "L2", 4, 0, "L2_2D_P4", 2, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "L2", 3, 1, "L2_2D_P3", 1, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "L2", 2, 2, "L2_2D_P2", 2, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "L2", 1, 3, "L2_2D_P1", 3, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "L2", 1, 0, "L2_3D_P1", 3, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "L2", 3, 1, "L2_3D_P3", 1, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "L2", 4, 2, "L2_3D_P4", 2, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "L2", 3, 3, "L2_3D_P3", 3, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "RT", 2, 0, "RT_2D_P2", 1, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "RT", 1, 0, "RT_3D_P1", 1, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "RT", 2, 2, "RT_2D_P2", 1, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "RT", 1, 3, "RT_3D_P1", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "ND", 4, 0, "ND_1D_P4", 1, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "ND", 5, 0, "ND_2D_P5", 1, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "ND", 6, 0, "ND_3D_P6", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "ND", 1, 1, "ND_1D_P1", 1, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "ND", 1, 2, "ND_2D_P1", 1, false),
        VectorFESpaceTest::makeParam("ref-cube.mesh", "ND", 2, 3, "ND_3D_P2", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "RT", 2, 3, "RT_R1D_1D_P2", 1, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "RT", 3, 3, "RT_R2D_2D_P3", 1, false),
        VectorFESpaceTest::makeParam("ref-segment.mesh", "ND", 5, 3, "ND_R1D_1D_P5", 1, false),
        VectorFESpaceTest::makeParam("ref-square.mesh", "ND", 1, 3, "ND_R2D_2D_P1", 1, false)

            ));

class InvalidVectorFESpaceTest : public VectorFESpaceTest
{
};

TEST_P(InvalidVectorFESpaceTest, TestInvalidVectorFESpace)
{
  buildFESpace();
  // The fespace object is lazily-constructed, so isn't actually built
  // until we ask to retreive it
  EXPECT_THROW(fespace->getFESpace(), std::runtime_error);
}

INSTANTIATE_TEST_SUITE_P(
    ExpectError,
    InvalidVectorFESpaceTest,
    testing::Values(VectorFESpaceTest::makeParam("ref-segment.mesh", "RT", 3, 2, "", -1, false),
                    VectorFESpaceTest::makeParam("ref-square.mesh", "RT", 1, 1, "", -1, false),
                    VectorFESpaceTest::makeParam("ref-cube.mesh", "RT", 0, 1, "", -1, false),
                    VectorFESpaceTest::makeParam("ref-cube.mesh", "RT", 1, 2, "", -1, false),
                    VectorFESpaceTest::makeParam("ref-segment.mesh", "ND", 1, 2, "", -1, false),
                    VectorFESpaceTest::makeParam("ref-square.mesh", "ND", 1, 1, "", -1, false),
                    VectorFESpaceTest::makeParam("ref-cube.mesh", "ND", 0, 1, "", -1, false),
                    VectorFESpaceTest::makeParam("ref-cube.mesh", "ND", 5, 2, "", -1, false)));

#endif
