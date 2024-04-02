/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

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