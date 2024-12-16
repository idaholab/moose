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

## Commands

Note that letters shown upper case in commands do not mean literal upper case. You do not have to press shift when typing these letters. This is just the notation (confusing I know). A good reference for hotkeys can be found [here](https://go.microsoft.com/fwlink/?linkid=832143). Linux hotkeys can be found [here](https://go.microsoft.com/fwlink/?linkid=832144). The hotkeys we give below assume MacOS.

- +`Cmd+Shift+P`+ opens the command palette. Every available command can be accessed by typing a
  few letters here. The dropdown list shows the keyboard shortcuts.
- +`Cmd+P`+ opens a file anywhere in the current project tree (i.e. below the directory in which you
  issued the `code .` command. No need to know the precise path or even the precise spelling of the
  filename!
- +`Cmd+[`+ Decrease indent (Outdent)
- +`Cmd+]`+ Increase indent (Indent)
- +`Cmd+F`+ Find
- +`Cmd+Option+F`+ Find and replace
  - +`Enter`+ Find next match without replacement
  - +`Ctrl+Enter`+ Replace
- +`Cmd+Option+Left/Right Arrow`+ Tab between open editor files as ordered on the screen
- +`Ctrl+Tab`+ Tab between editor files in the order they were opened
- +`Shift+Option+Drag Mouse`+ Rectangular text selection
- +`Option+Repeated Clicks`+ Place multiple cursors (one per each click)
- +`Cmd+S`+ Save current file
- +`Cmd+J`+ Toggle terminal visibility
- +`` Ctrl+` ``+ Toggles on terminal visibility if not currently visible. Switch focus to terminal if focus is elsewhere. If focus is already on terminal, toggles off terminal visibility
- +`Cmd+1`+ Change focus to code editor
- +`Cmd+\`+ Split screen vertically
- +`Cmd+W`+ Close current file. If last file in split, then split will disappear
- +`Ctrl+G`+ Goto line
- +`Ctrl+N`+ Move cursor to next line
- +`Ctrl+P`+ Move cursor to previous line
- +`Ctrl+E`+ Move cursor to end of line
- +`Ctrl+A`+ Move cursor to beginning of line
- +`Ctrl+K`+ Kill from cursor to end of line
- +`Ctrl+Y`+ Redo last undone action (not yank like in emacs)
- +`Ctrl+F`+ Move cursor to next character
- +`Ctrl+B`+ Move cursor to previous character
- +`Cmd+B`+ Toggle left sidebar visibility
- +`Cmd+Left/Right Arrow`+ Move cursor to beginning/end of line
- +`Option+Left/Right Arrow`+ Move cursor over one word to left/right
- +`Cmd+K Cmd+M`+ Maximize/Unmaximize current editor group
- +`Cmd+Up/Down Arrow`+ Go to beginning/end of file
- +`Fn+Up/Down Arrow`+ Page Up/Down in file
- +`Cmd+Shift+\`+ Go to matching brace
- +`Ctrl+-`+ Go to previous cursor location (useful after doing things like +`Cmd+Click`+)
- +`Cmd+L`+ Select current line
- +`Cmd+D`+ Select current word. Repeated invocations will also select next matches of that word.
  This is useful if wanting to edit multiple occurrences of the word without going through the
  Find/Replace focus.

### Focuses

- +`findInputFocussed`+ Cursor is in the Find box (note that cursor in the Replace box of the Find and Replace box *does not count*)
- +`editorFocus`+ Cursor is within the code editor


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
    "files.associations": {
        "*.C": "cpp",
        "*.h": "cpp"
    },
    "files.trimTrailingWhitespace": true,
    "editor.tabSize": 2,
    "editor.formatOnSave": true,
    "files.insertFinalNewline": true
}
```

The `editor.formatOnSave` , `files.insertFinalNewline` , and  `files.trimTrailingWhitespace` options are helpful to avoid failing the automated code check in MOOSE's test suite. The `C_Cpp.default.includePath` settings ensure that includes installed with Conda are found. The _Atom One Dark_ theme is available as an extension and provides the familiar Atom colors. [GitLens](https://marketplace.visualstudio.com/items?itemName=eamodio.gitlens) is an extension that provides extensive git integration.
