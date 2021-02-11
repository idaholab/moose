#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import collections
import mooseutils

@mooseutils.addProperty('name', ptype=str, required=True)
@mooseutils.addProperty('filename', ptype=str)
class Document(mooseutils.AutoPropertyMixin):
    pass

INL_DOCUMENTS = ['safety_software_determination', 'quality_level_determination', 'enterprise_architecture_entry', # Initiation
                 'project_management_plan', 'software_quality_plan', 'configuration_management_plan', # Planning
                 'software_requirements_specification', # Requirements
                 'software_design_description', # Design
                 'software_test_plan', 'requirements_traceablity_matrix', 'asset_management_plan', 'verification_validation_plan', # Testing
                 'user_manual', 'theory_manual', # Other
                 'failure_analysis_report'] # PLN-4005 v8

def get_documents(required_docs=INL_DOCUMENTS, **kwargs):
    """
    Build SQA document dictionary from the provided directories.
    """
    return [Document(name=name, filename=kwargs.get(name, None)) for name in required_docs]
