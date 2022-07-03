# Setup VSCode for MOOSE code and input file development

!media framework/vscode/autocomplete2.gif
       style=width:300px;padding-left:20px;float:right;
       caption=VSCode autocompletion in MOOSE input files.

!media framework/vscode/autocomplete1.gif
       style=width:300px;padding-left:20px;float:right;clear:right;
       caption=Autocompletion for MOOSE input files.

!media framework/vscode/outline.gif
       style=width:300px;padding-left:20px;float:right;clear:right;
       caption=Outlines for MOOSE input files.

!media framework/vscode/format_document.gif
       style=width:300px;padding-left:20px;float:right;clear:right;
       caption=MOOSE input file formatting.

[VSCode](https://code.visualstudio.com/) is a text editor developed by Microsoft with a flexible plugin structure.
VSCode supersedes Atom. To customize VSCode for MOOSE development the [MOOSE Language Support](https://marketplace.visualstudio.com/items?itemName=DanielSchwen.moose-language-support) extension has been developed.

## Setting up VSCode

Installation packages can be downloaded for Mac, Linux, and Windows operating systems from the
[VSCode website](https://code.visualstudio.com/). The editor has an automatic update system for both the core editor as
well as the installed extensions.

### First Steps

- On macOS, +Start VSCode+ for the first time, then press +`F1`+ (or +`Cmd-Shift-P`+) and type `Shell Command: Install 'code' command in PATH`. Press enter. This activates the
  `code` shell command for use from a terminal.
- On Linux the shell command is automatically installed.
- +Close VSCode+. From now on we will _only_ start it from the command line using the `code`
  terminal command ensuring that VSCode sees the full MOOSE build environment.

## Important commands

- +`Cmd-Shift-P`+ opens the command palette. Every available command can be accessed by typing a
  few letters here. The dropdown list shows the keyboard shortcuts.
- +`Cmd-P`+ opens a file anywhere in the current project tree (i.e. below the directory in which you
  issued the `code .` command. No need to know the precise path or even the precise spelling of the
  filename!

## Extensions


The following extensions should be installed to effectively develop MOOSE based codes and edit MOOSE
input files using VSCode

- [MOOSE Language Support](https://marketplace.visualstudio.com/items?itemName=DanielSchwen.moose-language-support): Syntax highlighting, automatic
  indentation, document outlines, mouse hover info, formatting, and auto-completion for MOOSE input files, C++ code snippets for all MOOSE systems, and
  highlighting of select MOOSE C++ types.
- [C/C++ Extension Pack](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) installs a set of extensions to support C++ code navigation, syntax highlighting, etc.

## Recommended settings

The following settings are recommended to pass the MOOSE GitHub CI pre-checks:

```
{
    "workbench.colorTheme": "Atom One Dark",
    "gitlens.hovers.currentLine.over": "line",
    "[cpp]": {
        "editor.defaultFormatter": "ms-vscode.cpptools"
    },
    "telemetry.telemetryLevel": "off",
    "C_Cpp.default.cppStandard": "c++17",
    "C_Cpp.default.includePath": [
        "${workspaceFolder}/**",
        "${env.CONDA_PREFIX}/**",
        "/usr/include/**"
    ],
    "workbench.editorAssociations": {
        "*.C": "cpp",
        "*.h": "cpp"
    },
    "files.trimTrailingWhitespace": true,
    "editor.tabSize": 2,
    "editor.formatOnSave": true,
    "files.insertFinalNewline": true
}
```

The `editor.formatOnSave` , `files.insertFinalNewline` , and  `files.trimTrailingWhitespace` options are helpful to avoid failing the automated code check in MOOSE's test suite. The `C_Cpp.default.includePath` settings ensure that includes installed with Conda/Mamba are found. The _Atom One Dark_ theme is available as an extension and provides the familiar Atom colors. [GitLens](https://marketplace.visualstudio.com/items?itemName=eamodio.gitlens) is an extension that provides extensive git integration.
