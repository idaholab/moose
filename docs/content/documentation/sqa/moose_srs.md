!SQA-load system_requirements_specification.md

!SQA-template-item project_description
{{PROJECT}} is an open-source tool for solving partial differential equations using the finite element method.

!END-template-item

!SQA-template-item system_scope
{{PROJECT}} is a finite element simulation framework design for preforming fully-coupled nonlinear
solves.
!END-template-item

!SQA-template-item system_context
{{PROJECT}} utilizes an object-oriented design to allow users to customize existing
code and build up a simulation in a modular fashion.
!END-template-item

!SQA-template-item system_requirements

!SQA-requirement-list title=Transient Analysis
   F1.10 The system shall perform time dependent analysis.
   F1.50 The system shall include the ability to create custom input syntax.

!END-template-item
