//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/***
 * Streams all of the given arguments into the given stream
 */

template <class StreamType>
void
streamArguments(StreamType &)
{
}

template <class StreamType, typename T, typename... Args>
void
streamArguments(StreamType & ss, T && val, Args &&... args)
{
  ss << val;
  streamArguments(ss, std::forward<Args>(args)...);
}
