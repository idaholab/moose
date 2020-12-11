"""Python utilities for performing SQA related checks"""
from .SilentRecordHandler import SilentRecordHandler
import logging
logger = logging.getLogger('moosesqa')
logger.addHandler(SilentRecordHandler())

from .get_sqa_reports import get_sqa_reports
from .check_syntax import check_syntax, file_is_stub, find_md_file
from .get_requirements import get_requirements_from_tests, get_requirements_from_file
from .get_requirements import number_requirements, get_test_specification
from .check_requirements import check_requirements
from .SQAReport import SQAReport
from .SQADocumentReport import SQADocumentReport
from .SQARequirementReport import SQARequirementReport, SQARequirementDiffReport
from .SQAMooseAppReport import SQAMooseAppReport
from .Requirement import Requirement, Detail, TestSpecification
from .LogHelper import LogHelper

MOOSESQA_COLLECTIONS = dict()
MOOSESQA_COLLECTIONS['FAILURE_ANALYSIS'] = "Requirements that perform check for simulation " \
                                           "integrity such as error handling and convergence " \
                                           "failures."

MOOSESQA_CLASSIFICATION = dict()
MOOSESQA_CLASSIFICATION['FUNCTIONAL'] = ('F', "Requirements that define uses cases and are correct, unambiguous, complete, consistent, verifiable, and traceable.")
MOOSESQA_CLASSIFICATION['USABILITY'] = ('U', "Requirements for the system that include measurable effectiveness, efficiency, and satisfaction criteria in specific contexts of use.")
MOOSESQA_CLASSIFICATION['PERFORMANCE'] = ('P', "Requirements to define the critical performance conditions and associated capabilities.")
MOOSESQA_CLASSIFICATION['SYSTEM'] = ('S', "Requirements for interfaces among system elements and with external entities.")
