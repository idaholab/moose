# IT System Design Description for {{PROJECT}}

##SDD-ID_NO
##Rev X

<!--
Enter the names of the technical lead, independent reviewer (for Quality Level [QL]‑1 and QL‑2 only), and information technology (IT) asset owner in the spaces provided. For QL‑3, delete the rows for the independent reviewer. Enter the Electronic Change Request (eCR) number in the space provided that documents the review and approval signatures of the technical lead, independent reviewer (if applicable), and IT asset owner.
-->
###Prepared by:
*[Enter text here.]*
___
Technical Lead
###Reviewed by:
*[Enter text here.]*
___
Independent Reviewer
###Approved by:
*[Enter text here.]*
___
IT Asset Owner

Review and approval signatures are documented on eCR No.: *[Enter text here.]*

#CONTENTS
## INTRODUCTION
### Purpose of this Plan
!SQA-template system_purpose default=True
<!--Briefly describe the objectives and rationale of the system design description (SDD) (e.g., describe the system design in the user’s terminology, provide a guide for a more technical design document, or ensure that customers and technical staff have a common understanding of the system design). Explain how this SDD might evolve throughout the product lifecycle. [Ref. NQA SP2.7 402]
-->

The purpose of the System Design Description (SDD) is to:

* Provide an as-built description of the system (software, servers, and network)
* Ensure technical staff has a common understanding of how the system was designed; including both the base software and the configured software
* Provide a tool for tracing requirements through system design, development, and verification.

!END-template

### System Scope
<!--An integral part of software design is the design of a computer program that is part of an overall system. Identify the product(s) to be produced by name (e.g., computer program, network infrastructure, host database management system, report generator, and high performance computing server). Describe the operating environment for the computer program. [Ref. NQA  SP2.7 402] Describe the overall system, its high level functionality, interfaces, and any limitations or functions that it will not perform. Be consistent with similar statements in higherlevel specifications (e.g., project management plan, software quality assurance plan, and system requirements specification).
-->

*[Enter text here.]*

###Dependencies and Limitations
<!--List the dependencies or limitations that may affect the design of the system. Include the use of and availability of third‑party components to be reused and software tools that the software design may be based (e.g., database management system and graphical user interface design tools). Describe how each will affect the functional design. Also, address any limitations related to categorization of any data to be used or collected and other security requirements. Address design limitations due to cyber security requirements.
-->

*[Enter text here.]*


## DESIGN STAKEHOLDERS AND CONCERNS

###Design Stakeholders
<!--Identify the stakeholders of the information technology (IT) system design. These may include the organizations that provide funding for the project, maintain the system over its lifecycle, provide interfaces to the system, provide personnel or other resources for the development or maintenance of the system, and the users of the system.
-->

*[Enter text here.]*

###Stakeholder Design Concerns
<!--Discuss the concerns expressed by each of the stakeholders as they relate to the system design including measures to mitigate consequences of problems. Reference the risk management plan as appropriate.
-->

*[Enter text here.]*


##SYSTEM DESIGN
<!--Include in this section and associated attachments the description of the system design. The methods and techniques identified below are more applicable to software than hardware. Ensure that hardware meets the appropriate design standards and conventions and is described using appropriate methods (i.e., graphical descriptions). The documented software design shall define the computational sequence necessary to meet the software requirements. The design can be described through the use of a number of methods and techniques. At a minimum, the methods and techniques to be used where applicable are included in the list below. Ensure that the detail included in the design is commensurate with the level of risk and demonstrate the fulfillment of requirements providing guidance for implementation. [Ref. NQA R3 801.2] For QL‑1, QL‑2, and QL‑3 configurable and utility calculations as QL‑3 custom‑developed software the detail for describing the system design may only be needed the top and perhaps the second level to address the lower risk.

Minimum methods and techniques to use where applicable are listed below. Appendix A includes examples of each of these methods. Section 3 identifies potential uses of these methods.

* _Numerical methods_
* _Physical models_
* _Control logic_
* _Process flow_
* _Process structures_
* _Mathematical models_
* _Control flow_
* _Data flow_
* _Data structures_
* _Interfaces between data structures and process structures_

For safety software, describe measures to mitigate the consequences of problems, as identified through analysis. These potential problems include external and internal abnormal conditions and events that can affect the computer program. These measures include techniques, such as failure modes and effects analysis, fault‑tree modeling, event‑tree modeling, cause‑consequence diagrams, hazard and operability analysis, and interface analysis. [Ref. NQA SP2.7 402]
In addition, for safety software, consider implementing the design principles of simplicity, decoupling, and isolation to eliminate hazards. Safety features should be separate from nonsafety modules minimizing the impact of failure of one module on another.
-->

###System Structure and Functional Decomposition
<!--Decompose the system into functional and structural design entities or objects that perform the required system objectives. Assign a unique identifier/name to each design entity, and group entities by type (e.g., class, object, and procedure/module). Describe how each design entity satisfies system requirements. In user terminology, specify the inputs, outputs, and transformation rules for each design entity. Depict how design entities interface and depend on one another. Identify any required scientific/engineering algorithms or equations and its source that must be used to meet software requirements. Potential methods and techniques that can be used include numerical methods, mathematical models, control flow, process flow, physical models, control logic, and process structures.
-->

*[Enter text here or reference attachments.]*

###Software Safety System Design
<!--For safety software, describe what measures are being performed to ensure that credible failure of the computer program will not affect a safety function. These potential problems and credible failures include external and internal abnormal conditions and events that can affect proper operation of the system. [Ref. NQA R3 100, NQA SP2.7 402]

Describe or reference the failure analysis to be performed for safety software. There are several resources available to assist in the identification of methods such as failure mode effects analysis, including this information from the American Society for Quality (ASQ).

*Describe the system design approaches that will be implemented in the design to ensure that the credible failures are eliminated or mitigated
-->

*[Enter text here or reference attachments.]*


###Data Design and Control
<!--Using data modeling techniques identify specific data elements and logical data groupings that are stored and processed by the design entities. Outline data dependencies, relationships, and integrity rules in the data dictionary. Specify the format and attributes of all data elements or data groupings.

Develop a logical model of data flow through the system by depicting how design elements transform input data into outputs. Potential methods and techniques that can be used include physical models, data flow, data structures, and interfaces between data structures and process structures.

Specific details about the data dictionary must be documented and maintained in the Enterprise Architecture repository. Include data elements to be implemented, a business definition, protection requirements, and important business rules that drive data quality for each data element.

Contact the Laboratory Enterprise Architect for assistance with data dictionary development and data management standards.
-->

*[Enter text here or reference attachments.]*


###User Interface Design
<!--Describe the user interface and the operating environment, including the menu hierarchy, data entry screens, display screens, online help, and system messages. Specify where in this environment the necessary inputs are made, and list the methods of data outputs (e.g., printer, screen, and file). Note any user interface design standards to be applied. If Idaho National Laboratory or your organization has developed user interface design standards that will be used, discuss these in this section. Potential methods and techniques that can be used include process flow and control flow.
-->

*[Enter text here or reference attachments.]*

###System Interface Design
<!--Specify how the product will interface with other systems. For each interface, describe the inputs and outputs for the interacting systems. Explain how data is formatted for transmission and validated upon arrival. Note the frequency of data exchange. Discuss how the interfaces will be controlled such that changes do not affect the system design. [Ref. NQA R3 100] Potential methods and techniques that can be used include physical models, process flow, data flow, and interfaces between data structures and process structures.
Specific details about data interfaces must be documented and maintained in the Enterprise Architecture repository. [3.22.12]
-->

*[Enter text here or reference attachments.]*

###Security Structure
<!--Describe the software functionality supporting the security architecture of the system design. This would list features such as user privilege restrictions, logging of auditable events, and the encryption technology for securing data in storage or transmission.
-->

*[Enter text here or reference attachments.]*

##REQUIREMENTS TRACEABILITY
<!--Each software requirement shall identify where it is addressed in the system design. Reference the requirements traceability matrix (RTM) documented using TEM‑214, “IT System Requirements Traceability Matrix,” or a separate RTM. [Ref. NQA SP2.7 402.1]
-->

*[Enter text here.]*

##REVIEW AND CONTROL OF THE SYSTEM DESIGN
The system design shall be controlled and verified as specified in the *[Enter text here for the software quality assurance plan (SQAP) identifier]* and the *[Enter text here for the configuration management plan (CMP) identifier].*

##DOCUMENT MAINTENANCE
<!--Add text or revise the following to address the specific activities for maintaining this document.
-->

[Enter text here.]
>>>>>>> Add SDD template.

The technical lead is responsible for maintaining this system design document (SDD). This SDD is controlled per LWP‑1201, “Document Management.” Revisions to this SDD will occur on an as‑needed basis as a result of reviews, audits, and requested changes. Modifications to this SDD must be independently reviewed and approved by the information technology (IT) asset owner. [Ref. NQA R3 100]

##REFERENCES
The following are references for this SDD. All Idaho National Laboratory (INL) policies and procedures referenced are the current version at the time this SDD was approved.

<!--Revise the following list of documents to provide a complete list of all documents and other sources of information referenced in this SDD. Identify any deviations from referenced standards or policies and provide justifications.
-->

* ASME NQA‑1‑2008 with the NQA‑1a‑2009 addenda, “Quality Assurance Requirements for Nuclear Facility Applications,” American Society of Mechanical Engineers, First Edition, August 31, 2009.

* LWP‑1201, “Document Management.”

##DEFINITIONS, ACRONYMS, AND ABBREVIATIONS
This section defines, or provides the definition of, all terms and acronyms required to properly understand this SDD.

###Definitions
*[Enter text here.]*

###Acronyms
<!--
Revise the following list of acronyms to provide a complete list of all acronyms used in this specification.
-->

| Acronym | Name |
| --- | --- |
ASME | American Society of Mechanical Engineers
CMP	| configuration management plan
INL	 | Idaho National Laboratory
IT | information technology
NQA	 | Nuclear Quality Assurance
QA | quality assurance
RTM	 | requirements traceability matrix
SDD	 | system design description
SQAP | software quality assurance plan

##ATTACHMENTS
<!--List and include all attachments created to describe the system design to the required level of detail.
-->

*[Enter text here or state None.]*

###Appendix A - Design Method Examples
* Numerical methods
* Physical models
* Control logic
* Process flow
* Process structures
* Mathematical models
* Control flow
* Data flow
* Data structures
* Interfaces between data structures and process structures
