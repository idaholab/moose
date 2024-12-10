//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/// Enum for describing the center, edge and corner subchannels or gap types
enum class EChannelType
{
  CENTER,
  EDGE,
  CORNER
};

// Declare the operator<< overload for EChannelType as inline
inline std::ostringstream &
operator<<(std::ostringstream & oss, const EChannelType & channelType)
{
  // Convert EChannelType to string representation and stream it into oss
  switch (channelType)
  {
    case EChannelType::CENTER:
      oss << "CENTER";
      break;
    case EChannelType::EDGE:
      oss << "EDGE";
      break;
    case EChannelType::CORNER:
      oss << "CORNER";
      break;
  }
  return oss;
}
