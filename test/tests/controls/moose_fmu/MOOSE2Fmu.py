"""Define the abstract facade class."""
import json
import datetime
from collections import OrderedDict, namedtuple
from pathlib import Path
from typing import Any, ClassVar, Dict, List, Optional
# from uuid import uuid1
from xml.etree.ElementTree import Element, SubElement

# from .logmsg import LogMsg
# from .default_experiment import DefaultExperiment

from pythonfmu.enums import Fmi2Type, Fmi2Status, Fmi2Causality, Fmi2Initial, Fmi2Variability
from pythonfmu.variables import Boolean, Integer, Real, ScalarVariable, String
from pythonfmu import Fmi2Slave

ModelOptions = namedtuple("ModelOptions", ["name", "value", "cli"])

FMI2_MODEL_OPTIONS: List[ModelOptions] = [
    ModelOptions("needsExecutionTool", True, "no-external-tool"),
    ModelOptions("canHandleVariableCommunicationStepSize", True, "no-variable-step"),
    ModelOptions("canInterpolateInputs", False, "interpolate-inputs"),
    ModelOptions("canBeInstantiatedOnlyOncePerProcess", False, "only-one-per-process"),
    ModelOptions("canGetAndSetFMUstate", False, "handle-state"),
    ModelOptions("canSerializeFMUstate", False, "serialize-state")
]


class MOOSE2Fmu(Fmi2Slave):
    """Abstract class for the creation of FMU from an OpenMOOSE case

    Args:
        Fmi2Slave: Abstract class defining FMU in details see pythomfmu
    """

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.host = '"127.0.0.1"' #local host aka this computer
        self.port = 8000
        self.outputPath = "MOOSECase"
        self.oscmd = "bash -i"
        self.arguments = ""
        self.control = None

        self.register_variable(String("host", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(Integer("port", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("outputPath", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("oscmd", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))
        self.register_variable(String("arguments", causality=Fmi2Causality.parameter, variability=Fmi2Variability.tunable))


    def to_xml(self, model_options: Dict[str, str] = dict()) -> Element:
        """Build the XML representation of the model.

        Args:
            model_options (Dict[str, str]) : FMU model options

        Returns:
            (xml.etree.TreeElement.Element) XML description of the FMU
        """

        t = datetime.datetime.now(datetime.timezone.utc)
        date_str = t.isoformat(timespec="seconds")

        attrib = dict(
            fmiVersion="2.0",
            modelName=self.modelName,
            guid=f"{self.guid!s}",
            generationTool=f"FMU4MOOSE 0.1.0",
            generationDateAndTime=date_str,
            variableNamingConvention="structured"
        )
        if self.description is not None:
            attrib["description"] = self.description
        if self.author is not None:
            attrib["author"] = self.author
        if self.license is not None:
            attrib["license"] = self.license
        if self.version is not None:
            attrib["version"] = self.version
        if self.copyright is not None:
            attrib["copyright"] = self.copyright

        root = Element("fmiModelDescription", attrib)

        options = dict()
        for option in FMI2_MODEL_OPTIONS:
            value = model_options.get(option.name, option.value)
            options[option.name] = str(value).lower()
        options["modelIdentifier"] = self.modelName
        options["canNotUseMemoryManagementFunctions"] = "true"

        SubElement(root, "CoSimulation", attrib=options)

        if len(self.log_categories) > 0:
            categories = SubElement(root, "LogCategories")
            for category, description in self.log_categories.items():
                categories.append(
                    Element(
                        "Category",
                        attrib={"name": category, "description": description},
                    )
                )

        variables = SubElement(root, "ModelVariables")
        for v in self.vars.values():
            if ScalarVariable.requires_start(v):
                self.__apply_start_value(v)
            variables.append(v.to_xml())

        structure = SubElement(root, "ModelStructure")
        outputs = list(
            filter(lambda v: v.causality == Fmi2Causality.output, self.vars.values())
        )

        if outputs:
            outputs_node = SubElement(structure, "Outputs")
            for i, v in enumerate(self.vars.values()):
                if v.causality == Fmi2Causality.output:
                    SubElement(outputs_node, "Unknown", attrib=dict(index=str(i + 1)))

        if self.default_experiment is not None:
            attrib = dict()
            if self.default_experiment.start_time is not None:
                attrib["startTime"] = self.default_experiment.start_time
            if self.default_experiment.stop_time is not None:
                attrib["stopTime"] = self.default_experiment.stop_time
            if self.default_experiment.tolerance is not None:
                attrib["tolerance"] = self.default_experiment.tolerance
            SubElement(root, "DefaultExperiment", attrib)

        return root

    def __apply_start_value(self, var: ScalarVariable):
        """apply start values

        Args:
            var (ScalarVariable): scalarVariable
        Raises:
            Exception: unsupported type
        """
        vrs = [var.value_reference]

        if isinstance(var, Integer):
            refs = self.get_integer(vrs)
        elif isinstance(var, Real):
            refs = self.get_real(vrs)
        elif isinstance(var, Boolean):
            refs = self.get_boolean(vrs)
        elif isinstance(var, String):
            refs = self.get_string(vrs)
        else:
            raise Exception(f"Unsupported type!")

        var.start = refs[0]

    def do_step(self, current_time: float, step_size: float) -> bool:
        "not expected to be implemented"
        return True



