#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, datetime

def csvEnabled(input_tree):
    outputs = input_tree.getBlockInfo("/Outputs")
    if outputs and outputs.included:
        p = outputs.getParamInfo("csv")
        return p.value == "true"
    return False

def _getFileBase(outputs, inputfilename):
    """
    Get the file base to be written out based on parameters in /Outputs
    Input:
        outputs[BlockInfo]: corresponding to /Outputs
        inputfilename[str]: The input file name
    Return:
        (bool whether file_base was set, file base)
    """
    file_base = outputs.getParamInfo("file_base")
    file_base_set = False
    if outputs.included and file_base and file_base.value:
        file_base_set = True
        fname = file_base.value
    else:
        fname = os.path.splitext(os.path.basename(inputfilename))[0]
    return file_base_set, fname

def getPostprocessorFiles(input_tree, inputfilename):
    """
    Get a list of /Postprocessors files that will be written.
    Input:
        input_tree[InputTree]: The InputTree to get blocks from
        inputfilename[str]: The input file name
    Return:
        list[str]: file names
    """
    outputs = input_tree.getBlockInfo("/Outputs")
    output_file_names = []
    is_set, common_file_base = _getFileBase(outputs, inputfilename)
    if not is_set:
        common_file_base += "_out"
    output_file_names.append("%s.csv" % common_file_base)

    return output_file_names

def getVectorPostprocessorFiles(input_tree, inputfilename):
    """
    Get a list of /VectorPostprocessors files that will be written.
    Input:
        input_tree[InputTree]: The InputTree to get blocks from
        inputfilename[str]: The input file name
    Return:
        list[str]: file names
    """
    outputs = input_tree.getBlockInfo("/Outputs")
    output_file_names = []
    is_set, common_file_base = _getFileBase(outputs, inputfilename)
    if not is_set:
        common_file_base += "_out"

    pp = input_tree.getBlockInfo("/VectorPostprocessors")
    for p in pp.children.values():
        file_base = _getChildFileBase(common_file_base, p)
        output_file_names.append("%s_*.csv" % file_base)
    return output_file_names

def _getChildFileBase(common_file_base, child):
    """
    Get the file base for outputs.
    Input:
        common_file_base[str]: The default file base
        child[BlockInfo]: Child node of /Outputs
    Return:
        str: file base
    """
    file_base = "%s_%s" % (common_file_base, child.name)
    file_base_param = child.getParamInfo("file_base")
    if file_base_param and file_base_param.value:
        file_base = file_base_param.value
    return file_base

def getOutputFiles(input_tree, inputfilename):
    """
    Inspects the "/Output" node and gets a list of output files that the input file will write.
    Input:
        input_tree[InputTree]: The InputTree to get blocks from
        inputfilename[str]: The input file name
    Return:
        list[str]: Output filenames
    """

    outputs = input_tree.getBlockInfo("/Outputs")
    output_file_names = []

    is_set, common_file_base = _getFileBase(outputs, inputfilename)

    exodus = outputs.getParamInfo("exodus")
    if outputs.included and exodus and exodus.value == "true":
        if is_set:
            output_file_names.append("%s.e" % common_file_base)
        else:
            output_file_names.append("%s_out.e" % common_file_base)

    for child in outputs.children.values():
        if not child.included:
            continue

        type_param = child.getParamInfo("type")
        if type_param.value != "Exodus":
            continue

        file_base = _getChildFileBase(common_file_base, child)

        oversample = child.getParamInfo("oversample")
        append_oversample = child.getParamInfo("append_oversample")
        if oversample and oversample.value != "false" and append_oversample and append_oversample.value != "false":
            file_base = file_base + '_oversample'

        append_date = child.getParamInfo("append_date")
        if append_date and append_date.value != "false":
            utc = datetime.datetime.utcnow()
            date_format = child.getParamInfo("append_date_format")
            d_str = utc.isoformat()
            if date_format and date_format.value != "":
                try:
                    d_str = utc.strftime(date_format.value)
                except:
                    pass
            file_base = "%s_%s" % (file_base, d_str)
        output_file_names.append(file_base + '.e')

    return output_file_names
