//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMesh.h"
#include "libmesh/mesh_generation.h"

registerMooseObject("MooseApp", MFEMMesh);

InputParameters
MFEMMesh::validParams()
{
  InputParameters params = FileMesh::validParams();
  params.addParam<unsigned int>(
      "serial_refine",
      0,
      "Number of serial refinements to perform on the mesh. Equivalent to uniform_refine.");
  params.addParam<unsigned int>(
      "uniform_refine",
      0,
      "Number of serial refinements to perform on the mesh. Equivalent to serial_refine");
  params.addParam<unsigned int>(
      "parallel_refine", 0, "Number of parallel refinements to perform on the mesh.");
  params.addParam<std::string>("displacement", "Optional variable to use for mesh displacement.");
  params.addParam<bool>("periodic", false, "Optional variable to indicate whether we make the mesh periodic.");
  params.addParam<std::string>("boundary1", "Optional variable to use for mesh displacement.");
  params.addParam<std::string>("boundary2", "Optional variable to use for mesh displacement.");

  params.addClassDescription("Class to read in and store an mfem::ParMesh from file.");

  return params;
}

MFEMMesh::MFEMMesh(const InputParameters & parameters) : FileMesh(parameters),
  _periodic(getParam<bool>("periodic"))
{
  if (_periodic) {
    // fetch the boundaries we want to glue together
    _bdryAttr1 = std::stoi( getParam<std::string>("boundary1") );
    _bdryAttr2 = std::stoi( getParam<std::string>("boundary2") );
  }
}

MFEMMesh::~MFEMMesh() {}

void
MFEMMesh::buildMesh()
{
  TIME_SECTION("buildMesh", 2, "Reading Mesh");

  // Build a dummy MOOSE mesh to enable this class to work with other MOOSE classes.
  buildDummyMooseMesh();

  // Build the MFEM ParMesh from a serial MFEM mesh
  mfem::Mesh mfem_ser_mesh(getFileName());

  for (int be = 0; be < mfem_ser_mesh.GetNBE(); be++)
  {
    int attr = mfem_ser_mesh.GetBdrAttribute(be);
    if (attr==1 or attr==4) {
      mfem::Element* el = mfem_ser_mesh.GetBdrElement(be);
      mfem::Array<int> vertices;
      el->GetVertices(vertices);
      std::cout << "Boundary element " << be << " has vertices ";
      for(int i=0; i<vertices.Size(); i++) std::cout << vertices[i] << " ";
      std::cout << "\n";
    }
    // std::cout << "Boundary element " << be
    //           << " has boundary attribute " << attr 
    //           << ", mfem_ser_mesh.GetBdrElement(be)->attribute=" << mfem_ser_mesh.GetBdrElement(be)->GetAttribute()
    //           << std::endl;
  }

  if (_periodic) {
    mfem_ser_mesh = applyPeriodicBoundary(mfem_ser_mesh);
  }

  if (isParamSetByUser("serial_refine") && isParamSetByUser("uniform_refine"))
    paramError(
        "Cannot define serial_refine and uniform_refine to be nonzero at the same time (they "
        "are the same variable). Please choose one.\n");

  uniformRefinement(mfem_ser_mesh,
                    isParamSetByUser("serial_refine") ? getParam<unsigned int>("serial_refine")
                                                      : getParam<unsigned int>("uniform_refine"));

  // multi app should take the mpi comm from moose so is split correctly??
  auto comm = this->comm().get();
  _mfem_par_mesh = std::make_shared<mfem::ParMesh>(comm, mfem_ser_mesh);

  // Perform parallel refinements
  uniformRefinement(*_mfem_par_mesh, getParam<unsigned int>("parallel_refine"));

  if (isParamSetByUser("displacement"))
  {
    _mesh_displacement_variable.emplace(getParam<std::string>("displacement"));
  }
}

/*
  This function is very ugly - its intention is to read all the vertices
  on the boundaries you wanna pin together and create the v2v mapping manually
*/
mfem::Mesh
MFEMMesh::applyPeriodicBoundary(mfem::Mesh& input) {
  // create the mfem vertex-vertex mapping
  // almost every vertex should be mapped to itself
  std::vector<int> v2v(input.GetNV());
  for (auto i=0; i<v2v.size(); i++) v2v[i]=i;

  std::vector<int> boundary1_elems;
  std::vector<int> boundary2_elems;

  // gather the elements for attr1 and attr2
  for (int be=0; be<input.GetNBE(); be++)
  {
    if ( input.GetBdrAttribute(be) == _bdryAttr1 ) boundary1_elems.push_back( be );
    if ( input.GetBdrAttribute(be) == _bdryAttr2 ) boundary2_elems.push_back( be );
  }

  // loop again. this time, fetch all the vertices
  for (auto be=0; be<boundary1_elems.size(); be++) {
    mfem::Element* el1 = input.GetBdrElement( boundary1_elems[be] );
    int* v1 = el1->GetVertices();
    mfem::Element* el2 = input.GetBdrElement( boundary2_elems[be] );
    int* v2 = el2->GetVertices();
    // assert( el1->GetNVertices() == el2->GetNVertices() );

    int nv = el1->GetNVertices();
    for (int v=0; v<nv; v++)     {
      v2v[ v1[v] ] = v2[v];
    }
  }
  
  // finally, make the mesh periodic
  return mfem::Mesh::MakePeriodic(input, v2v);
}



void
MFEMMesh::displace(mfem::GridFunction const & displacement)
{
  _mfem_par_mesh->EnsureNodes();
  mfem::GridFunction * nodes = _mfem_par_mesh->GetNodes();

  *nodes += displacement;
}

void
MFEMMesh::buildDummyMooseMesh()
{
  MeshTools::Generation::build_point(static_cast<UnstructuredMesh &>(getMesh()));
}

void
MFEMMesh::uniformRefinement(mfem::Mesh & mesh, const unsigned int nref) const
{
  for (unsigned int i = 0; i < nref; ++i)
    mesh.UniformRefinement();
}

std::unique_ptr<MooseMesh>
MFEMMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

#endif
