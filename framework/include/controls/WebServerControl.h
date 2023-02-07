//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Control.h"

// Contrib
#include "http.h"

// C++
#include <atomic>
#include <memory>
#include <thread>

class Function;

/**
 * Starts a webserver that an external process can connect to
 * in order to send/receive JSON messages to control the solve
 */
class WebServerControl : public Control
{
public:
  /**
   * Class constructor
   * @param parameters Input parameters for this Control object
   */
  static InputParameters validParams();

  WebServerControl(const InputParameters & parameters);

  ~WebServerControl();

  virtual void execute() override;

protected:
  // The port to run on
  unsigned int _port;

  // The server instance
  std::unique_ptr<HttpServer> _server;

  // Whether or not the Control is currently waiting
  std::atomic<bool> _currently_waiting;

  // The server thread
  std::unique_ptr<std::thread> _server_thread;
};
