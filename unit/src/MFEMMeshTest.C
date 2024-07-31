#include "gtest/gtest.h"
#include "MFEMMesh.h"
#include "MooseMain.h"

class MFEMMeshTest : public ::testing::Test
{
protected:
  void SetUp() override;
  void buildMFEMMesh(MeshFileName filename);

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
  _app = Moose::createMooseApp("PlatypusApp", 1, (char **)argv);
  _factory = &_app->getFactory();
  _mesh_type = "MFEMMesh";
}

/**
 * Helper method to set up and build mesh given mesh filename.
 */
void
MFEMMeshTest::buildMFEMMesh(MeshFileName filename)
{
  InputParameters params = _factory->getValidParams(_mesh_type);
  params.set<MeshFileName>("file") = filename;
  _mfem_mesh_ptr = _factory->create<MFEMMesh>(_mesh_type, "moose_mesh", params);
  _app->actionWarehouse().mesh() = _mfem_mesh_ptr;
  _mfem_mesh_ptr->setMeshBase(_mfem_mesh_ptr->buildMeshBaseObject());
  _mfem_mesh_ptr->buildMesh();
}

/**
 * Test MFEMMesh can read Exodus II format meshes. Note that nodesets must be present.
 */
TEST_F(MFEMMeshTest, ExodusIIFormatReader) { buildMFEMMesh("data/mug.e"); }

/**
 * Test MFEMMesh can read MFEM format meshes.
 */
TEST_F(MFEMMeshTest, MFEMMeshFormatReader) { buildMFEMMesh("data/beam-tet.mesh"); }

/**
 * Test MFEMMesh can be cloned.
 */
TEST_F(MFEMMeshTest, MFEMMeshClone)
{
  buildMFEMMesh("data/mug.e");
  _mfem_mesh_ptr->safeClone();
}