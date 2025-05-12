#ifdef MFEM_ENABLED

#include "gtest/gtest.h"
#include "MFEMMesh.h"
#include "MooseMain.h"

class MFEMMeshTest : public ::testing::Test
{
protected:
  void SetUp() override;
  void buildMFEMMesh(MeshFileName filename, int serial_ref = 0, int parallel_ref = 0);

  std::shared_ptr<MooseApp> _app;
  Factory * _factory;
  std::string _mesh_type;
  std::shared_ptr<MFEMMesh> _mfem_mesh_ptr;
};

/**
 * Create app MFEMMesh object will be added to.
 */
void
MFEMMeshTest::SetUp()
{
  const char * argv[2] = {"foo", "\0"};
  _app = Moose::createMooseApp("MooseUnitApp", 1, (char **)argv);
  _factory = &_app->getFactory();
  _mesh_type = "MFEMMesh";
}

/**
 * Helper method to set up and build mesh given mesh filename.
 */
void
MFEMMeshTest::buildMFEMMesh(MeshFileName filename, int serial_ref, int parallel_ref)
{
  InputParameters params = _factory->getValidParams(_mesh_type);
  params.set<MeshFileName>("file") = filename;
  params.set<unsigned int>("serial_refine") = serial_ref;
  params.set<unsigned int>("parallel_refine") = parallel_ref;
  _mfem_mesh_ptr = _factory->create<MFEMMesh>(_mesh_type, "moose_mesh", params);
  _app->actionWarehouse().mesh() = _mfem_mesh_ptr;
  _mfem_mesh_ptr->setMeshBase(_mfem_mesh_ptr->buildMeshBaseObject());
  _mfem_mesh_ptr->buildMesh();
}

/**
 * Test MFEMMesh can read Exodus II format meshes. Note that nodesets must be present.
 */
TEST_F(MFEMMeshTest, ExodusIIFormatReader)
{
  buildMFEMMesh("../test/tests/mfem/mesh/mug.e");
  mfem::ParMesh & pmesh(_mfem_mesh_ptr->getMFEMParMesh());

  // Check expected number of vertices have been read
  EXPECT_EQ(pmesh.GetNV(), 3774);
  // Check expected number of elements have been read
  EXPECT_EQ(pmesh.GetNE(), 2476);
  // Check expected number of boundary elements have been read
  EXPECT_EQ(pmesh.GetNBE(), 554);
  // Check expected number of edges have been read
  EXPECT_EQ(pmesh.GetNEdges(), 10001);
  // Check expected number of faces have been read
  EXPECT_EQ(pmesh.GetNFaces(), 8704);
  // Check expected number of boundary attributes (sidesets) have been read
  EXPECT_EQ(pmesh.bdr_attributes.Size(), 2);
  // Check expected number of element attributes (blocks) have been read
  EXPECT_EQ(pmesh.attributes.Size(), 1);

  // Test MFEMMesh can be cloned
  ASSERT_TRUE(_mfem_mesh_ptr->safeClone() != nullptr);
}

/**
 * Test MFEMMesh can read MFEM format meshes.
 */
TEST_F(MFEMMeshTest, MFEMMeshFormatReader)
{
  buildMFEMMesh("../test/tests/mfem/mesh/beam-tet.mesh");
  mfem::ParMesh & pmesh(_mfem_mesh_ptr->getMFEMParMesh());

  // Check expected number of vertices have been read
  EXPECT_EQ(pmesh.GetNV(), 36);
  // Check expected number of elements have been read
  EXPECT_EQ(pmesh.GetNE(), 48);
  // Check expected number of boundary elements have been read
  EXPECT_EQ(pmesh.GetNBE(), 68);
  // Check expected number of edges have been read
  EXPECT_EQ(pmesh.GetNEdges(), 117);
  // Check expected number of faces have been read
  EXPECT_EQ(pmesh.GetNFaces(), 130);
  // Check expected number of boundary attributes (sidesets) have been read
  EXPECT_EQ(pmesh.bdr_attributes.Size(), 3);
  // Check expected number of element attributes (blocks) have been read
  EXPECT_EQ(pmesh.attributes.Size(), 2);

  // Test MFEMMesh can be cloned
  ASSERT_TRUE(_mfem_mesh_ptr->safeClone() != nullptr);
}

/**
 * Test MFEMMesh can read a high order MFEM format mesh.
 */
TEST_F(MFEMMeshTest, MFEMHighOrderMeshFormatReader)
{
  buildMFEMMesh("../test/tests/mfem/mesh/fichera-q3.mesh");
  mfem::ParMesh & pmesh(_mfem_mesh_ptr->getMFEMParMesh());

  // Check expected number of vertices have been read
  EXPECT_EQ(pmesh.GetNV(), 26);
  // Check expected number of elements have been read
  EXPECT_EQ(pmesh.GetNE(), 7);
  // Check expected number of boundary elements have been read
  EXPECT_EQ(pmesh.GetNBE(), 24);
  // Check expected number of edges have been read
  EXPECT_EQ(pmesh.GetNEdges(), 51);
  // Check expected number of faces have been read
  EXPECT_EQ(pmesh.GetNFaces(), 33);
  // Check expected number of boundary attributes (sidesets) have been read
  EXPECT_EQ(pmesh.bdr_attributes.Size(), 24);
  // Check expected number of element attributes (blocks) have been read
  EXPECT_EQ(pmesh.attributes.Size(), 1);

  // Test MFEMMesh can be cloned
  ASSERT_TRUE(_mfem_mesh_ptr->safeClone() != nullptr);
}

TEST_F(MFEMMeshTest, Refinement)
{
  buildMFEMMesh("../test/tests/mfem/mesh/fichera-q3.mesh", 1, 2);
  mfem::ParMesh & pmesh(_mfem_mesh_ptr->getMFEMParMesh());

  // Check expected number of vertices have been read
  EXPECT_EQ(pmesh.GetNV(), 4401);
  // Check expected number of elements have been read
  EXPECT_EQ(pmesh.GetNE(), 3584);
  // Check expected number of boundary elements have been read
  EXPECT_EQ(pmesh.GetNBE(), 1536);
  // Check expected number of edges have been read
  EXPECT_EQ(pmesh.GetNEdges(), 12336);
  // Check expected number of faces have been read
  EXPECT_EQ(pmesh.GetNFaces(), 11520);
  // Check expected number of boundary attributes (sidesets) have been read
  EXPECT_EQ(pmesh.bdr_attributes.Size(), 24);
  // Check expected number of element attributes (blocks) have been read
  EXPECT_EQ(pmesh.attributes.Size(), 1);

  // Test MFEMMesh can be cloned
  ASSERT_NE(_mfem_mesh_ptr->safeClone(), nullptr);
}

#endif
