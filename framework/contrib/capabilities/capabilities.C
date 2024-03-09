#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <map>
#include <string>
#include <variant>

static PyObject *
capabilities_check(PyObject *self, PyObject *args)
{
  // function arguments
  const char *requirement;
  PyObject *capdict;
  if (!PyArg_ParseTuple(args, "sO", &requirement, &capdict))
    return NULL;

  // make sure argument 2 is a dictionary
  if (!PyDict_Check(capdict))
  {
    // TODO: set exception
    return NULL;
  }

  // map to be populated
  std::map<std::string, std::variant<double, bool, std::string>> capabilities;

  // get dictionary items
  PyObject *items = PyDict_Items(capdict);
  Py_ssize_t len = PyList_Size(items);
  for (Py_ssize_t i = 0; i < len; ++i)
  {
    PyObject *item = PyList_GetItem(items, i);
    // check if the item is a tuple
    if (!PyTuple_Check(item) || PyTuple_Size(item) != 2)
    {
      // TODO: set exception
      return NULL;
    }

    PyObject *key_obj = PyTuple_GetItem(item, 0);
    PyObject *value = PyTuple_GetItem(item, 0);

    // ensure key is a string
    if (!PyUnicode_Check(key_obj))
    {
      // TODO: set exception
      return NULL;
    }
    const std::string key(PyUnicode_AsUTF8(key_obj));

    // determine value type
    if (PyBool_Check(value))
      capabilities[key] = bool(PyObject_IsTrue(value));
    else if (PyLong_Check(value))
      capabilities[key] = static_cast<double>(PyLong_AsLong(value));
    else if (PyFloat_Check(value))
      capabilities[key] = PyFloat_AsDouble(value);
    else if (PyUnicode_Check(value))
      capabilities[key] = std::string(PyUnicode_AsUTF8(value));
    else
    {
      // TODO: set exception
      return NULL;
    }
  }

  // call capabilities C++ code with capabilities map
  auto [status, message] = CapabilityUtils::check(requirement, capabilities);
  return Py_BuildValue("(NN)", PyLong_FromLong(status), PyUnicode_FromString(message.c_str()));
}

static PyMethodDef CapabilitiesMethods[] = {
    {"check", capabilities_check, METH_VARARGS,
     "Check a requirement against a capabilities dictionary."},
    {NULL, NULL, 0, NULL}};

PyDoc_STRVAR(capabilities_doc, "Interface to the Moose::Capabilities system.");

static struct PyModuleDef capabilitiesmodule = {
    PyModuleDef_HEAD_INIT,
    "capabilities",   /* name of module */
    capabilities_doc, /* module documentation, may be NULL */
    -1,               /* size of per-interpreter state of the module,
                         or -1 if the module keeps state in global variables. */
    CapabilitiesMethods};

PyMODINIT_FUNC
PyInit_capabilities(void)
{
  auto ret = PyModule_Create(&capabilitiesmodule);
  ret &= PyModule_AddIntConstant(&capabilitiesmodule, "FAIL", 2);
  ret &= PyModule_AddIntConstant(&capabilitiesmodule, "UNKNOWN", 1);
  ret &= PyModule_AddIntConstant(&capabilitiesmodule, "PASS", 2);
  return ret;
}
