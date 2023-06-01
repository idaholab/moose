//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometricCutUserObject.h"

// MOOSE includes
#include "MooseError.h"
#include "XFEM.h"
#include "DataIO.h"
#include "EFAElement2D.h"
#include "EFAElement3D.h"
#include "XFEMElementPairLocator.h"
#include "DisplacedProblem.h"

InputParameters
GeometricCutUserObject::validParams()
{
  InputParameters params = CrackFrontPointsProvider::validParams();
  params.addClassDescription("Base UserObject class for XFEM Geometric Cuts");
  params.addParam<bool>("heal_always", false, "Heal previous cuts at every time step");
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_MARK);
  params.setDocString("execute_on", exec.getDocString());
  params.set<ExecFlagEnum>("execute_on") = EXEC_XFEM_MARK;

  return params;
}

GeometricCutUserObject::GeometricCutUserObject(const InputParameters & parameters,
                                               const bool uses_mesh)
  : CrackFrontPointsProvider(parameters, uses_mesh), _heal_always(getParam<bool>("heal_always"))
{
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(_fe_problem.getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in GeometricCutUserObject");

  _xfem->addGeometricCut(this);

  auto new_xfem_epl = std::make_shared<XFEMElementPairLocator>(_xfem, _interface_id);
  _fe_problem.geomSearchData().addElementPairLocator(_interface_id, new_xfem_epl);

  if (_fe_problem.getDisplacedProblem() != NULL)
  {
    auto new_xfem_epl2 = std::make_shared<XFEMElementPairLocator>(_xfem, _interface_id, true);
    _fe_problem.getDisplacedProblem()->geomSearchData().addElementPairLocator(_interface_id,
                                                                              new_xfem_epl2);
  }
}

void
GeometricCutUserObject::initialize()
{
  _marked_elems_2d.clear();
  _marked_elems_3d.clear();
}

void
GeometricCutUserObject::execute()
{
  if (_current_elem->dim() == 2)
  {
    std::vector<Xfem::CutEdge> elem_cut_edges;
    std::vector<Xfem::CutNode> elem_cut_nodes;
    std::vector<Xfem::CutEdge> frag_cut_edges;
    std::vector<std::vector<Point>> frag_edges;

    EFAElement2D * EFAElem = _xfem->getEFAElem2D(_current_elem);

    // Don't cut again if elem has been already cut twice
    if (!EFAElem->isFinalCut())
    {
      // get fragment edges
      _xfem->getFragmentEdges(_current_elem, EFAElem, frag_edges);

      // mark cut edges for the element and its fragment
      bool cut = cutElementByGeometry(_current_elem, elem_cut_edges, elem_cut_nodes);
      if (EFAElem->numFragments() > 0)
        cut |= cutFragmentByGeometry(frag_edges, frag_cut_edges);

      if (cut)
      {
        Xfem::GeomMarkedElemInfo2D gmei2d;
        gmei2d._elem_cut_edges = elem_cut_edges;
        gmei2d._elem_cut_nodes = elem_cut_nodes;
        gmei2d._frag_cut_edges = frag_cut_edges;
        gmei2d._frag_edges = frag_edges;
        _marked_elems_2d[_current_elem->id()].push_back(gmei2d);
      }
    }
  }
  else if (_current_elem->dim() == 3)
  {
    std::vector<Xfem::CutFace> elem_cut_faces;
    std::vector<Xfem::CutFace> frag_cut_faces;
    std::vector<std::vector<Point>> frag_faces;

    EFAElement3D * EFAElem = _xfem->getEFAElem3D(_current_elem);

    // Don't cut again if elem has been already cut twice
    if (!EFAElem->isFinalCut())
    {
      // get fragment edges
      _xfem->getFragmentFaces(_current_elem, EFAElem, frag_faces);

      // mark cut faces for the element and its fragment
      bool cut = cutElementByGeometry(_current_elem, elem_cut_faces);
      // TODO: This would be done for branching, which is not yet supported in 3D
      // if (EFAElem->numFragments() > 0)
      //  cut |= cutFragmentByGeometry(frag_faces, frag_cut_faces, _t);

      if (cut)
      {
        Xfem::GeomMarkedElemInfo3D gmei3d;
        gmei3d._elem_cut_faces = elem_cut_faces;
        gmei3d._frag_cut_faces = frag_cut_faces;
        gmei3d._frag_faces = frag_faces;
        _marked_elems_3d[_current_elem->id()].push_back(gmei3d);
      }
    }
  }
}

void
GeometricCutUserObject::threadJoin(const UserObject & y)
{
  const GeometricCutUserObject & gcuo = dynamic_cast<const GeometricCutUserObject &>(y);

  for (const auto & it : gcuo._marked_elems_2d)
  {
    mooseAssert(_marked_elems_2d.find(it.first) == _marked_elems_2d.end(),
                "Element already inserted in map from a different thread");
    _marked_elems_2d[it.first] = it.second;
  }
  for (const auto & it : gcuo._marked_elems_3d)
  {
    mooseAssert(_marked_elems_3d.find(it.first) == _marked_elems_3d.end(),
                "Element already inserted in map from a different thread");
    _marked_elems_3d[it.first] = it.second;
  }
}

// custom data load and data store methods for structs with std::vector members
template <>
inline void
dataStore(std::ostream & stream, Xfem::CutFace & cf, void * context)
{
  dataStore(stream, cf._face_id, context);
  dataStore(stream, cf._face_edge, context);
  dataStore(stream, cf._position, context);
}

template <>
inline void
dataLoad(std::istream & stream, Xfem::CutFace & cf, void * context)
{
  dataLoad(stream, cf._face_id, context);
  dataLoad(stream, cf._face_edge, context);
  dataLoad(stream, cf._position, context);
}

template <>
inline void
dataStore(std::ostream & stream, Xfem::GeomMarkedElemInfo2D & gmei, void * context)
{
  dataStore(stream, gmei._elem_cut_edges, context);
  dataStore(stream, gmei._elem_cut_nodes, context);
  dataStore(stream, gmei._frag_cut_edges, context);
  dataStore(stream, gmei._frag_edges, context);
}

template <>
inline void
dataLoad(std::istream & stream, Xfem::GeomMarkedElemInfo2D & gmei, void * context)
{
  dataLoad(stream, gmei._elem_cut_edges, context);
  dataLoad(stream, gmei._elem_cut_nodes, context);
  dataLoad(stream, gmei._frag_cut_edges, context);
  dataLoad(stream, gmei._frag_edges, context);
}

template <>
inline void
dataStore(std::ostream & stream, Xfem::GeomMarkedElemInfo3D & gmei, void * context)
{
  dataStore(stream, gmei._elem_cut_faces, context);
  dataStore(stream, gmei._frag_cut_faces, context);
  dataStore(stream, gmei._frag_faces, context);
}

template <>
inline void
dataLoad(std::istream & stream, Xfem::GeomMarkedElemInfo3D & gmei, void * context)
{
  dataLoad(stream, gmei._elem_cut_faces, context);
  dataLoad(stream, gmei._frag_cut_faces, context);
  dataLoad(stream, gmei._frag_faces, context);
}

void
GeometricCutUserObject::serialize(std::string & serialized_buffer)
{
  // stream for serializing the _marked_elems_2d and _marked_elems_3d data structures to a byte
  // stream
  std::ostringstream oss;
  dataStore(oss, _marked_elems_2d, this);
  dataStore(oss, _marked_elems_3d, this);

  // Populate the passed in string pointer with the string stream's buffer contents
  serialized_buffer.assign(oss.str());
}

void
GeometricCutUserObject::deserialize(std::vector<std::string> & serialized_buffers)
{
  mooseAssert(serialized_buffers.size() == _app.n_processors(),
              "Unexpected size of serialized_buffers: " << serialized_buffers.size());

  // The input string stream used for deserialization
  std::istringstream iss;

  // Loop over all datastructures for all processors to perfrom the gather operation
  for (unsigned int rank = 0; rank < serialized_buffers.size(); ++rank)
  {
    // skip the current processor (its data is already in the structures)
    if (rank == processor_id())
      continue;

    // populate the stream with a new buffer and reset stream state
    iss.clear();
    iss.str(serialized_buffers[rank]);

    // Load the communicated data into temporary structures
    std::map<unsigned int, std::vector<Xfem::GeomMarkedElemInfo2D>> other_marked_elems_2d;
    std::map<unsigned int, std::vector<Xfem::GeomMarkedElemInfo3D>> other_marked_elems_3d;
    dataLoad(iss, other_marked_elems_2d, this);
    dataLoad(iss, other_marked_elems_3d, this);

    // merge the data in with the current processor's data
    _marked_elems_2d.insert(other_marked_elems_2d.begin(), other_marked_elems_2d.end());
    _marked_elems_3d.insert(other_marked_elems_3d.begin(), other_marked_elems_3d.end());
  }
}

void
GeometricCutUserObject::finalize()
{
  // for single processor runs we do not need to do anything here
  if (_app.n_processors() > 1)
  {
    // create send buffer
    std::string send_buffer;

    // create byte buffers for the streams received from all processors
    std::vector<std::string> recv_buffers;

    // pack the complex datastructures into the string stream
    serialize(send_buffer);

    // broadcast serialized data to and receive from all processors
    _communicator.allgather(send_buffer, recv_buffers);

    // unpack the received data and merge it into the local data structures
    deserialize(recv_buffers);
  }

  for (const auto & it : _marked_elems_2d)
    for (const auto & gmei : it.second)
      _xfem->addGeomMarkedElem2D(it.first, gmei, _interface_id);

  for (const auto & it : _marked_elems_3d)
    for (const auto & gmei : it.second)
      _xfem->addGeomMarkedElem3D(it.first, gmei, _interface_id);

  _marked_elems_2d.clear();
  _marked_elems_3d.clear();
}

CutSubdomainID
GeometricCutUserObject::getCutSubdomainID(const Elem * elem) const
{
  return _xfem->getCutSubdomainID(this, elem);
}
