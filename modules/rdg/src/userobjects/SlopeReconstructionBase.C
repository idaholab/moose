//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeReconstructionBase.h"

// Static mutex definition
Threads::spin_mutex SlopeReconstructionBase::_mutex;

InputParameters
SlopeReconstructionBase::validParams()
{
  InputParameters params = ElementLoopUserObject::validParams();
  params.addClassDescription("Base class for piecewise linear slope reconstruction to get the "
                             "slopes of element average variables.");
  return params;
}

SlopeReconstructionBase::SlopeReconstructionBase(const InputParameters & parameters)
  : ElementLoopUserObject(parameters),
    _rslope(declareRestartableData<std::map<dof_id_type, std::vector<RealGradient>>>(
        "reconstructed_slopes")),
    _avars(declareRestartableData<std::map<dof_id_type, std::vector<Real>>>("avg_var_values")),
    _bnd_avars(
        declareRestartableData<std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>>>(
            "avg_bnd_var_values")),
    _side_centroid(declareRestartableData<std::map<std::pair<dof_id_type, dof_id_type>, Point>>(
        "side_centroid")),
    _bnd_side_centroid(
        declareRestartableData<std::map<std::pair<dof_id_type, unsigned int>, Point>>(
            "bnd_side_centroid")),
    _side_area(
        declareRestartableData<std::map<std::pair<dof_id_type, dof_id_type>, Real>>("side_area")),
    _bnd_side_area(declareRestartableData<std::map<std::pair<dof_id_type, unsigned int>, Real>>(
        "bnd_side_area")),
    _side_normal(declareRestartableData<std::map<std::pair<dof_id_type, dof_id_type>, Point>>(
        "side_normal")),
    _bnd_side_normal(declareRestartableData<std::map<std::pair<dof_id_type, unsigned int>, Point>>(
        "bnd_side_normal")),
    _q_point_face(_assembly.qPointsFace()),
    _qrule_face(_assembly.qRuleFace()),
    _JxW_face(_assembly.JxWFace()),
    _normals_face(_assembly.normals()),
    _side(_assembly.side()),
    _side_elem(_assembly.sideElem()),
    _side_volume(_assembly.sideElemVolume()),
    _neighbor_elem(_assembly.neighbor()),
    _side_geoinfo_cached(false)
{
}

void
SlopeReconstructionBase::initialize()
{
  ElementLoopUserObject::initialize();

  _rslope.clear();
  _avars.clear();
}

void
SlopeReconstructionBase::finalize()
{
  ElementLoopUserObject::finalize();

  if (_app.n_processors() > 1)
  {
    _side_geoinfo_cached = true;

    std::vector<std::string> send_buffers(1);
    std::vector<std::string> recv_buffers;

    recv_buffers.reserve(_app.n_processors());
    serialize(send_buffers[0]);
    comm().allgather_packed_range((void *)(nullptr),
                                  send_buffers.begin(),
                                  send_buffers.end(),
                                  std::back_inserter(recv_buffers));
    deserialize(recv_buffers);
  }
}

void
SlopeReconstructionBase::meshChanged()
{
  ElementLoopUserObject::meshChanged();

  _side_geoinfo_cached = false;
  _side_centroid.clear();
  _bnd_side_centroid.clear();
  _side_normal.clear();
  _bnd_side_normal.clear();
  _side_area.clear();
  _bnd_side_area.clear();
}

const std::vector<RealGradient> &
SlopeReconstructionBase::getElementSlope(dof_id_type elementid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<dof_id_type, std::vector<RealGradient>>::const_iterator pos = _rslope.find(elementid);

  if (pos == _rslope.end())
    mooseError(
        "Reconstructed slope is not cached for element id '", elementid, "' in ", __FUNCTION__);

  return pos->second;
}

const std::vector<Real> &
SlopeReconstructionBase::getElementAverageValue(dof_id_type elementid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<dof_id_type, std::vector<Real>>::const_iterator pos = _avars.find(elementid);

  if (pos == _avars.end())
    mooseError("Average variable values are not cached for element id '",
               elementid,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

const std::vector<Real> &
SlopeReconstructionBase::getBoundaryAverageValue(dof_id_type elementid, unsigned int side) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<std::pair<dof_id_type, unsigned int>, std::vector<Real>>::const_iterator pos =
      _bnd_avars.find(std::pair<dof_id_type, unsigned int>(elementid, side));

  if (pos == _bnd_avars.end())
    mooseError("Average variable values are not cached for element id '",
               elementid,
               "' and side '",
               side,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

const Point &
SlopeReconstructionBase::getSideCentroid(dof_id_type elementid, dof_id_type neighborid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<std::pair<dof_id_type, dof_id_type>, Point>::const_iterator pos =
      _side_centroid.find(std::pair<dof_id_type, dof_id_type>(elementid, neighborid));

  if (pos == _side_centroid.end())
    mooseError("Side centroid values are not cached for element id '",
               elementid,
               "' and neighbor id '",
               neighborid,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

const Point &
SlopeReconstructionBase::getBoundarySideCentroid(dof_id_type elementid, unsigned int side) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<std::pair<dof_id_type, unsigned int>, Point>::const_iterator pos =
      _bnd_side_centroid.find(std::pair<dof_id_type, unsigned int>(elementid, side));

  if (pos == _bnd_side_centroid.end())
    mooseError("Boundary side centroid values are not cached for element id '",
               elementid,
               "' and side '",
               side,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

const Point &
SlopeReconstructionBase::getSideNormal(dof_id_type elementid, dof_id_type neighborid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<std::pair<dof_id_type, dof_id_type>, Point>::const_iterator pos =
      _side_normal.find(std::pair<dof_id_type, dof_id_type>(elementid, neighborid));

  if (pos == _side_normal.end())
    mooseError("Side normal values are not cached for element id '",
               elementid,
               "' and neighbor id '",
               neighborid,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

const Point &
SlopeReconstructionBase::getBoundarySideNormal(dof_id_type elementid, unsigned int side) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<std::pair<dof_id_type, unsigned int>, Point>::const_iterator pos =
      _bnd_side_normal.find(std::pair<dof_id_type, unsigned int>(elementid, side));

  if (pos == _bnd_side_normal.end())
    mooseError("Boundary side normal values are not cached for element id '",
               elementid,
               "' and side '",
               side,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

const Real &
SlopeReconstructionBase::getSideArea(dof_id_type elementid, dof_id_type neighborid) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<std::pair<dof_id_type, dof_id_type>, Real>::const_iterator pos =
      _side_area.find(std::pair<dof_id_type, dof_id_type>(elementid, neighborid));

  if (pos == _side_area.end())
    mooseError("Side area values are not cached for element id '",
               elementid,
               "' and neighbor id '",
               neighborid,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

const Real &
SlopeReconstructionBase::getBoundarySideArea(dof_id_type elementid, unsigned int side) const
{
  Threads::spin_mutex::scoped_lock lock(_mutex);
  std::map<std::pair<dof_id_type, unsigned int>, Real>::const_iterator pos =
      _bnd_side_area.find(std::pair<dof_id_type, unsigned int>(elementid, side));

  if (pos == _bnd_side_area.end())
    mooseError("Boundary side area values are not cached for element id '",
               elementid,
               "' and side '",
               side,
               "' in ",
               __FUNCTION__);

  return pos->second;
}

void
SlopeReconstructionBase::computeElement()
{
  reconstructElementSlope();
}

void
SlopeReconstructionBase::serialize(std::string & serialized_buffer)
{
  std::ostringstream oss;

  // First store the number of elements to send
  unsigned int size = _interface_elem_ids.size();
  oss.write((char *)&size, sizeof(size));

  for (auto it = _interface_elem_ids.begin(); it != _interface_elem_ids.end(); ++it)
  {
    storeHelper(oss, *it, this);
    storeHelper(oss, _rslope[*it], this);
  }

  // Populate the passed in string pointer with the string stream's buffer contents
  serialized_buffer.assign(oss.str());
}

void
SlopeReconstructionBase::deserialize(std::vector<std::string> & serialized_buffers)
{
  // The input string stream used for deserialization
  std::istringstream iss;

  mooseAssert(serialized_buffers.size() == _app.n_processors(),
              "Unexpected size of serialized_buffers: " << serialized_buffers.size());

  for (auto rank = decltype(_app.n_processors())(0); rank < serialized_buffers.size(); ++rank)
  {
    if (rank == processor_id())
      continue;

    iss.str(serialized_buffers[rank]); // populate the stream with a new buffer
    iss.clear();                       // reset the string stream state

    // Load the communicated data into all of the other processors' slots

    unsigned int size = 0;
    iss.read((char *)&size, sizeof(size));

    for (unsigned int i = 0; i < size; i++)
    {
      dof_id_type key;
      loadHelper(iss, key, this);

      std::vector<RealGradient> value;
      loadHelper(iss, value, this);

      // merge the data we received from other procs
      _rslope.insert(std::pair<dof_id_type, std::vector<RealGradient>>(key, value));
    }
  }
}
