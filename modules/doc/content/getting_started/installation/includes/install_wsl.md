First, [Windows Subsystem for Linux (WSL)](https://learn.microsoft.com/en-us/windows/wsl/) must be installed. Open PowerShell or Windows Command Prompt in administrator mode by right-clicking the application and selecting "Run as administrator". In the prompt that appears, run the following command:

```bash
wsl --install
```

This command will require a restart. Restart your computer after the command completes. Once the restart is complete, a prompt will appear on next boot continuing the Ubuntu Linux installation and requesting a username for the new Linux user account. After this, a new Linux bash prompt running in WSL will appear. Run the following in the new bash prompt (in WSL) to update packages:

```bash
sudo apt update
sudo apt upgrade
```

You may now close the new WSL window.

By default, this will install Ubuntu 24.04 (the recommended distribution). If you have changed the default, we suggest using Ubuntu 24.04.

!alert! note
+All commands provided in the remaining instructions must be run within the WSL window!+ This includes the `git clone` of the application.

To open a new WSL window, search for "Ubuntu" within the Start Menu. In addition, you can run the `wsl` command within any Command Prompt or PowerShell window.

+You do not need to run any of these as administrator; administrator access is only needed to install the WSL distribution!+

!alert-end!
