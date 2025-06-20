from xml.etree.ElementTree import Element, SubElement, ElementTree
import uuid
import zipfile
import os
import shutil

def generate_model_description_xml(model_name, model_description, file_name):

    # Generate a unique GUID
    guid = str(uuid.uuid4())

    fmiModelDescription = Element('fmiModelDescription', {
        "fmiVersion": '2.0',
        "modelName": model_name,
        "guid": guid,
        "description": model_description,
        "generationTool": "Python",
        "variableNamingConvention": "flat",
        "numberOfEventIndicators": "0"
    })

    coSimulation = SubElement(fmiModelDescription, 'CoSimulation', {
        "modelIdentifier": model_name,
        "needsExecutionTool": "true",
        "canHandleVariableCommunicationStepSize": "true",
        "canInterpolateInputs": "false",
        "maxOutputDerivativeOrder": "0",
        "canRunAsynchronuously": "false",
        "canBeInstantiatedOnlyOncePerProcess": "false",
        "canNotUseMemoryManagementFunctions": "false",
        "providesDirectionalDerivative": "false"
    })

    modelVariables = SubElement(fmiModelDescription, 'ModelVariables')

    # Input variable
    external_value = SubElement(modelVariables, 'ScalarVariable', {
        "name": "external_value",
        "valueReference": "1",
        "variability": "continuous",
        "causality": "input",
        "initial":"exact"
    })

    SubElement(external_value, 'Real',{"start":"1.0"})

    # Output varible
    postprocessor = SubElement(modelVariables, 'ScalarVariable', {
        "name": "postprocessor",
        "valueReference": "2",
        "variability": "continuous",
        "causality": "output"
    })

    SubElement(postprocessor, 'Real')

    model_structure = SubElement(fmiModelDescription,"ModelStructure")

    outputs = SubElement(model_structure, "Outputs")
    SubElement(outputs, "Unknown", {"index": "2"})

    tree = ElementTree(fmiModelDescription)

    with open(file_name, 'wb') as file:
        tree.write(file, encoding="UTF-8", xml_declaration=True)

    print(f"{file_name} created successfully!")


def export_fmu():
    print("Exporting MOOSE FMU...")

    # Define FMU model description
    model_name = "MOOSE_FMU"
    model_description = "Get postprocessor value from MOOSE"
    file_name = "moose_fmu.xml"
    generate_model_description_xml(model_name, model_description, file_name)

    shutil.copy(file_name, 'modelDescription.xml')

    # Ensure the necessary directories exist
    os.makedirs('binaries/darwin64', exist_ok=True)

    # Packing
    with zipfile.ZipFile('MOOSE_FMU.fmu', 'w') as fmu:
        fmu.write('moose_fmu.xml', 'modelDescription.xml')
        fmu.write(__file__, 'binaries/darwin64/MOOSE_FMU.py')

    print("MOOSE FMU exported as MOOSE_FMU.fmu")


if __name__ == "__main__":
    export_fmu()
