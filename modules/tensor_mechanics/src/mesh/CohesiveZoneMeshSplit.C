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

#include "CohesiveZoneMeshSplit.h"
#include "Parser.h"
#include "MooseUtils.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseError.h"

#include "libmesh/exodusII_io.h"
#include "libmesh/nemesis_io.h"
#include "libmesh/parallel_mesh.h"
#include "libmesh/serial_mesh.h"

registerMooseObject("TensorMechanicsApp", CohesiveZoneMeshSplit);

template <>
InputParameters
validParams<CohesiveZoneMeshSplit>()
{
  InputParameters params = validParams<CohesiveZoneMeshSplitBase>();
  params.addClassDescription("Read a mesh from a file.");
  return params;
}

CohesiveZoneMeshSplit::CohesiveZoneMeshSplit(const InputParameters & parameters)
  : CohesiveZoneMeshSplitBase(parameters)
{

  getMesh().set_mesh_dimension(getParam<MooseEnum>("dim"));
}

CohesiveZoneMeshSplit::CohesiveZoneMeshSplit(const CohesiveZoneMeshSplit & other_mesh)
  : CohesiveZoneMeshSplitBase(other_mesh)
{
}

CohesiveZoneMeshSplit::~CohesiveZoneMeshSplit() {}

std::unique_ptr<MooseMesh>
CohesiveZoneMeshSplit::safeClone() const
{
  return libmesh_make_unique<CohesiveZoneMeshSplit>(*this);
}

void
CohesiveZoneMeshSplit::init()
{

  MooseMesh::init();

  checkInputParameter();
  buildNodeSupport();
  buildInterfacialNodes();
  duplicateNodes();
  tearElements();
  addInterfaceBoundary();
}

void
CohesiveZoneMeshSplit::buildNodeSupport()
{
  for (MeshBase::element_iterator it = getMesh().elements_begin(); it != getMesh().elements_end();
       ++it)
  {
    Elem * elem = *it;
    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      _node_support[elem->node_id(i)].push_back(elem->id());
  }
}

void
CohesiveZoneMeshSplit::buildInterfacialNodes()
{

  std::set<subdomain_id_type> matSet;

  // loop over all the nodes
  for (MeshBase::node_iterator it = getMesh().nodes_begin(); it != getMesh().nodes_end(); ++it)
  {
    Node * node = *it;
    std::vector<dof_id_type> support = _node_support[node->id()];
    unsigned int suppCount = support.size();

    if (suppCount == 1)
      continue;

    for (unsigned int ie = 0; ie < suppCount; ie++)
    {
      subdomain_id_type imat = getMesh().elem(support[ie])->subdomain_id();
      matSet.insert(imat);
    }

    unsigned int matCount = matSet.size();

    // if interface elements are created everywhere then
    // duplicity = nodal support. Else duplicity = materials.
    unsigned int duplicity = matCount;

    if (matCount != 1) // interfacial node
    {
      _node_duplicity[node->id()] = duplicity;

      _duplicated_node_materials[node->id()].insert(
          _duplicated_node_materials[node->id()].end(), matSet.begin(), matSet.end());
    }

    matSet.clear(); // clear for the next node
  }
}

void
CohesiveZoneMeshSplit::duplicateNodes()
{
  int idd(0);

  // after we have found the node duplicity we duplicate all the nodes as needed
  for (auto & dn : _node_duplicity)
  {
    _duplicated_node[dn.first].push_back(dn.first);

    // duplicate the nodes dn.second - 1 times (the original node remin assigned to one element)
    for (unsigned int id = 0; id < dn.second - 1; id++)
    {
      idd++;

      Node * new_node = Node::build(getMesh().node(dn.first), getMesh().n_nodes()).release();
      new_node->processor_id() = getMesh().node(dn.first).processor_id();
      getMesh().add_node(new_node);

      _duplicated_node[dn.first].push_back(new_node->id());
    }
  }
}

void
CohesiveZoneMeshSplit::tearElements()
{
  for (auto & dn : _duplicated_node) // loop over diplicated nodes
  {
    std::vector<dof_id_type> support = _node_support[dn.first];
    unsigned int suppCount = support.size();

    Elem * ref_elem = getMesh().elem(support[0]);

    // block id of the first element in the suppor of the dn
    subdomain_id_type ref_mat = ref_elem->subdomain_id();

    for (unsigned int ie = 1; ie < suppCount; ie++) // loop over adjacent elments in the support
    {

      // 1 -----> get the currenet element
      Elem * i_elem = getMesh().elem(support[ie]);
      subdomain_id_type imat = i_elem->subdomain_id();

      if (imat == ref_mat)
        continue;
      // if material is different then we ne need to duplicate the node and store the boundary pair

      // 1 -----> store boundary pair by block ID
      std::pair<subdomain_id_type, subdomain_id_type> materials_pair;

      if (imat > ref_mat)
        materials_pair = std::make_pair(ref_mat, imat);
      else
        materials_pair = std::make_pair(imat, ref_mat);

      // store the boundary pair
      _boundary_pairs.insert(materials_pair);

      // 2 -----> find the shared node and assign it to the appropriate duplicate node
      unsigned int imat_id = 0;
      for (unsigned int i = 0; i < i_elem->n_nodes(); ++i)
      {

        if (i_elem->node_id(i) == dn.first) // dn.first is the index of the original duplicated node
        {

          std::vector<subdomain_id_type> mats = _duplicated_node_materials[dn.first];
          // find the appropriate duplicate node by cycling materials
          for (unsigned int i = 0; i < mats.size(); i++)
          {
            if (imat == mats[i])
            {
              imat_id = i;
              break;
            }
          }

          // assign new node to the element
          i_elem->set_node(i) = getMesh().node_ptr((dn.second)[imat_id]);
        }
      }

      // 3 -----> find adjacent faces (if any) always sorted according to the material id
      // element on the master side of the boundary (i.e. the one that
      // will contain the interface)
      Elem * boundary_elem;

      // element on slave side of the boundary
      Elem * adjacent_elem;

      // the material of the boundary side
      // subdomain_id_type boundary_mat;

      // identify master(boundary) and slave(adjcent) element by looking at the
      // material ID the master side is always the one with the lowest ID
      if (imat > ref_mat)
      {
        boundary_elem = ref_elem;
        adjacent_elem = i_elem;
        // boundary_mat = ref_mat;
      }
      else
      {
        boundary_elem = i_elem;
        adjacent_elem = ref_elem;
        // boundary_mat = imat;
      }

      // save all boundary faces
      for (unsigned int i = 0; i < boundary_elem->n_sides(); i++)
      { // loop over faces
        if (boundary_elem->neighbor(i) != nullptr)
        {
          if (boundary_elem->neighbor(i)->id() == adjacent_elem->id())
          {
            _boundary_sides[materials_pair].insert(std::make_pair(boundary_elem->id(), i));
            break; // between two elements only one face can be common
          }
        }
      }
    }
  }
}

void
CohesiveZoneMeshSplit::addInterfaceBoundary()
{
  BoundaryInfo & boundary_info = getMesh().get_boundary_info();

  BoundaryID boundaryID = _interface_id;
  std::string boundaryName = _interface_name;

  // loop over boundary sides
  for (auto & bs : _boundary_sides)
  {

    if (_split_interface)
    {
      // find the appropriate boundary name and id
      //  given master and slave block ID
      findBoundaryNameAndInd(
          bs.first.first, bs.first.second, boundaryName, boundaryID, boundary_info);
    }

    // loop over all the side belonging to each pair and add it to the proper interface
    for (auto & es : bs.second)
    {
      boundary_info.add_side(es.first, es.second, boundaryID);
    }
  }
}
