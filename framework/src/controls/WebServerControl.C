//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WebServerControl.h"

#include "minijson.h"

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
  return params;
}

WebServerControl::WebServerControl(const InputParameters & parameters)
  : Control(parameters),
    _currently_waiting(false),
    _port(getParam<unsigned int>("port")),
    _server_thread(startServer())
{
}

WebServerControl::~WebServerControl()
{
  if (processor_id() == 0)
  {
    // Kill the server
    _server.shutdown();
    _server_thread->join();
  }
}

std::unique_ptr<std::thread>
WebServerControl::startServer()
{
  // Only start the server on the root node
  if (processor_id() == 0)
  {
    // Helper for returning an error response
    const auto error = [](const std::string & error)
    {
      miniJson::Json::_object response;
      response["error"] = error;
      return HttpResponse{400, response};
    };

    const auto get_name = [&error](const auto & msg, const std::string & description)
    {
      using result = std::variant<std::string, HttpResponse>;
      const auto name_it = msg.find("name");
      if (name_it == msg.end())
        return result(error("The entry 'name' is missing which should contain the name of the " +
                            description));
      const auto & name_json = name_it->second;
      if (!name_json.isString())
        return result(error("The entry 'name' which should contain the name of the " + description +
                            " must be a string"));
      return result(name_json.toString());
    };

    const auto require_waiting = [&error](auto & control)
    {
      using result = std::optional<HttpResponse>;
      if (!control.currentlyWaiting())
        return result(error("This control is not currently waiting for data"));
      return result{};
    };

    const auto require_parameters = [&error](const auto & msg, const std::set<std::string> & params)
    {
      using result = std::optional<HttpResponse>;
      for (const auto & key_value_pair : msg)
        if (!params.count(key_value_pair.first))
          return result(error("The key '" + key_value_pair.first + "' is unused"));
      return result{};
    };

    // GET /waiting, returns code 200 on success and JSON:
    //  'waiting' (bool): Whether or not the control is waiting
    //  'execute_on_flag' (string): Only exists if waiting=true, the execute
    //                              flag that is being waited on
    _server.when("/waiting")
        ->requested(
            [this](const HttpRequest & /*req*/)
            {
              miniJson::Json::_object res_json;
              if (this->_currently_waiting.load())
              {
                res_json["waiting"] = true;
                res_json["execute_on_flag"] =
                    static_cast<std::string>(this->_fe_problem.getCurrentExecuteOnFlag());
              }
              else
                res_json["waiting"] = false;

              return HttpResponse{200, res_json};
            });

    // POST /get/postprocessor, with data:
    //   'name' (string): The name of the Postprocessor
    // Returns code 200 on success and JSON:
    //   'value' (double): The postprocessor value
    _server.when("/get/postprocessor")
        ->posted(
            [this, &error, &get_name, &require_waiting, &require_parameters](
                const HttpRequest & req)
            {
              const auto & msg = req.json().toObject();

              // Get the postprocessor name
              const auto name_result = get_name(msg, "postprocessor to retrieve");
              if (const auto response = std::get_if<HttpResponse>(&name_result))
                return *response;
              const auto & name = std::get<std::string>(name_result);

              // Should only have a name and a value
              if (const auto response = require_parameters(msg, {"name"}))
                return *response;
              // Should be waiting for data
              if (const auto response = require_waiting(*this))
                return *response;
              // Postprocessor should exist
              if (!this->hasPostprocessorByName(name))
                return error("The postprocessor '" + name + "' was not found");

              miniJson::Json::_object res_json;
              res_json["value"] = getPostprocessorValueByName(name);
              return HttpResponse{200, res_json};
            });

    // POST /set/controllable, with data:
    //   'name' (string): The path to the controllable data
    //   'value' (double, array(double), or array(string)): The data to set
    // Returns code 201 on success and JSON:
    //   'error' (string): The error (only set if an error occurred)
    _server.when("/set/controllable")
        ->posted(
            [this, &error, &get_name, &require_waiting, &require_parameters](
                const HttpRequest & req)
            {
              const auto & msg = req.json().toObject();

              // Get the parameter name
              const auto name_result = get_name(msg, "name of the parameter to control");
              if (const auto response = std::get_if<HttpResponse>(&name_result))
                return *response;
              const auto & name = std::get<std::string>(name_result);
              // Get the parameter value
              const auto value_it = msg.find("value");
              if (value_it == msg.end())
                return error(
                    "The entry 'value' is missing which should contain the value of the parameter");
              const auto & value = value_it->second;

              // Should only have a name and a value
              if (const auto response = require_parameters(msg, {"name", "value"}))
                return *response;
              // Should be waiting for data
              if (const auto response = require_waiting(*this))
                return *response;
              // Parameter should exist
              if (!this->hasControllableParameterByName(name))
                return error("The controllable parameter '" + name + "' was not found");

              // Will be modifying _real_data or _vec_real_data
              std::lock_guard<std::mutex> lock(this->_data_mutex);

              // This could be generalized more in the future, but this is good for now
              // Real parameter value
              if (value.isNumber())
              {
                _real_data[name] = value.toDouble();
              }
              else if (value.isString())
              {
                _string_data[name] = value.toString();
              }
              // Array, currently std::vector<Real/std::string>
              else if (value.isArray())
              {
                const auto & array_value = value.toArray();

                if (array_value.size() == 0)
                  return error("Cannot send empty vector data as we cannot distinguish the type");

                if (array_value[0].isString())
                {
                  std::vector<std::string> string_values(array_value.size());
                  for (const auto i : index_range(array_value))
                  {
                    if (!array_value[i].isString())
                      return error("The " + std::to_string(i) + "-th value is not a string.");
                    string_values[i] = array_value[i].toString();
                  }
                  _vec_string_data[name] = std::move(string_values);
                }
                else if (array_value[0].isNumber())
                {
                  std::vector<Real> real_values(array_value.size());
                  for (const auto i : index_range(array_value))
                  {
                    if (!array_value[i].isNumber())
                      return error("The " + std::to_string(i) + "-th value is not a number.");
                    real_values[i] = array_value[i].toDouble();
                  }

                  _vec_real_data[name] = std::move(real_values);
                }
                else
                  return error("The vector data type is not a supported type (float or string)");
              }
              else
                return error("The data type is not a supported type");

              return HttpResponse{201};
            });

    // GET /continue, Returns code 200
    _server.when("/continue")
        ->requested(
            [this, &error](const HttpRequest &)
            {
              if (this->_currently_waiting.load())
              {
                this->_currently_waiting.store(false);
                return HttpResponse{200};
              }

              // Not currently waiting
              return error("The control is not currently waiting");
            });

    return std::make_unique<std::thread>([this] { this->_server.startListening(this->_port); });
  }

  // No server other than on rank zero
  return nullptr;
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

  // Broadcast all of the data that we have received
  comm().broadcast(_real_data);
  comm().broadcast(_string_data);
  comm().broadcast(_vec_real_data);
  comm().broadcast(_vec_string_data);

  // Helper for setting values from the data maps
  const auto set_values = [this](const auto & value_map)
  {
    for (const auto & [name, value] : value_map)
    {
      try
      {
        setControllableValueByName(name, value);
      }
      catch (...)
      {
        mooseError("Error setting '",
                   MooseUtils::prettyCppType(&value),
                   "' typed value for parameter '",
                   name,
                   "'; it is likely that the parameter has a different type");
      }
    }
  };

  // Set all of the data that we have
  set_values(_real_data);
  set_values(_string_data);
  set_values(_vec_real_data);
  set_values(_vec_string_data);

  // Done with these
  _real_data.clear();
  _string_data.clear();
  _vec_real_data.clear();
  _vec_string_data.clear();
}
