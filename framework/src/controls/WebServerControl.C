//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "WebServerControl.h"

// Contribs
#include "http.h"
#include "minijson.h"

// C++
#include <thread>

registerMooseObject("MooseApp", WebServerControl);

InputParameters
WebServerControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription("Starts a webserver for sending/receiving JSON messages to get data "
                             "and control a running MOOSE calculation");
  params.addRequiredParam<unsigned int>(
      "port",
      "The port to utilize.  Note that normally you don't have access to ports lower than 1024");
  params.addRequiredParam<std::string>(
      "parameter",
      "The input parameter(s) to control. Specify a single parameter name and all "
      "parameters in all objects matching the name will be updated");
  return params;
}

WebServerControl::WebServerControl(const InputParameters & parameters)
  : Control(parameters),
    _port(getParam<unsigned int>("port")),
    _server(std::make_unique<HttpServer>()),
    _currently_waiting(false)
{
  // Only want to start the server on the root node (for now?)
  if (processor_id() == 0)
  {
    // Whether or not the Control is waiting on external input
    _server->when("/waiting")
        ->requested(
            [this](const HttpRequest & /*req*/)
            {
              auto cw = this->_currently_waiting.load();

              // _console << "Currently waiting: " << cw << std::endl;

              miniJson::Json::_object res_json;
              res_json["waiting"] = cw;

              return HttpResponse{200, res_json};
            });

    // Set the controlled value
    _server->when("/set_controllable")
        ->posted(
            [this](const HttpRequest & req)
            {
              auto msg = req.json().toObject();
              auto value = msg["value"].toDouble();

              // _console << "Setting controlled value to: " << value << std::endl;

              setControllableValue<Real>("parameter", value);

              return HttpResponse{201};
            });

    // Get any Postprocessor value
    _server->when("/get_pp")->posted(
        [this](const HttpRequest & req)
        {
          auto req_json = req.json().toObject();

          auto pp_name = req_json["pp"].toString();
          auto value = getPostprocessorValueByName(pp_name);

          // _console << "Sending back PP " << pp_name << " value of " << value << std::endl;

          miniJson::Json::_object res_json;
          res_json["value"] = value;

          return HttpResponse{200, res_json};
        });

    // Call to tell execution to continue
    _server->when("/continue")
        ->requested(
            [this](const HttpRequest & /*req*/)
            {
              //_console << "Continuing execution" << std::endl;

              this->_currently_waiting.store(false);

              return HttpResponse{201};
            });

    _server_thread =
        std::make_unique<std::thread>([this] { this->_server->startListening(this->_port); });
  }
}

WebServerControl::~WebServerControl()
{
  if (processor_id() == 0)
  {
    // Kill the server
    _server->shutdown();
    _server_thread->join();
  }
}

void
WebServerControl::execute()
{
  if (processor_id() == 0)
  {
    TIME_SECTION("execute()", 3, "WebServerControl waiting for input")

    _currently_waiting.store(true);

    // While waiting, yield so the server has time to run
    while (_currently_waiting.load())
      std::this_thread::yield();
  }

  // All processes need to wait
  _communicator.barrier();
}
