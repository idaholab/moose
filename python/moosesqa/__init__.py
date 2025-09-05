"""Python utilities for performing SQA related checks"""

from .SilentRecordHandler import SilentRecordHandler
import logging

from .get_sqa_reports import get_sqa_reports as get_sqa_reports
from .check_syntax import (
    check_syntax as check_syntax,
    file_is_stub as file_is_stub,
    find_md_file as find_md_file,
)
from .get_requirements import (
    get_requirements_from_tests as get_requirements_from_tests,
    get_requirements_from_file as get_requirements_from_file,
)
from .get_requirements import (
    number_requirements as number_requirements,
    get_test_specification as get_test_specification,
)
from .check_requirements import check_requirements as check_requirements
from .SQAReport import SQAReport as SQAReport
from .SQADocumentReport import SQADocumentReport as SQADocumentReport
from .SQARequirementReport import (
    SQARequirementReport as SQARequirementReport,
    SQARequirementDiffReport as SQARequirementDiffReport,
)
from .SQAMooseAppReport import SQAMooseAppReport as SQAMooseAppReport
from .Requirement import (
    Requirement as Requirement,
    Detail as Detail,
    TestSpecification as TestSpecification,
)
from .LogHelper import LogHelper as LogHelper

logger = logging.getLogger("moosesqa")
logger.addHandler(SilentRecordHandler())

MOOSESQA_COLLECTIONS = {
    "FUNCTIONAL",
    "USABILITY",
    "PERFORMANCE",
    "SYSTEM",
    "FAILURE_ANALYSIS",
}
