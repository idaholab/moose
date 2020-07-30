# Step 1: Create a Custom MOOSE Application

The first step of this tutorial is to create a custom MOOSE application so that that C++ objects that are already available in MOOSE, as well as new ones that will be created as part of this exercise, may be executed and applied to solve the given problem. Development of the MOOSE Framework is facilitated by [GitHub](https://github.com), and thus, the reader shall [open an account](#account) and [create a *repository*](#repo) to store, and track the progress of, their new application. Following the completion of each tutorial step, for which new files have been created and tested, the reader shall *commit* and *push* these changes to their remote repository.

## Run the Stork Script and Initialize the New Application id=stork

To begin, please visit the [getting_started/installation/index.md] page to obtain instructions on how to install the MOOSE Environment package. Once the package has been installed, and a local copy, or *clone*, of the [MOOSE repository](https://github.com/idaholab/moose) has been downloaded to `~/projects/moose`, a template application can be automatically generated. Before proceeding, be sure that the local copy of MOOSE is up-to-date (see [getting_started/installation/index.md#update]). MOOSE is constantly evolving, and it is important that developer's update their local repositories as often as possible, especially prior to testing their own applications.

To initialize a new application, a name must be selected. For MOOSE applications, this is usually the name of an animal, e.g., [BISON](https://mooseframework.org/bison/). Sometimes, the application name is also an acronym for a more descriptive title, e.g., [MASTODON](https://mooseframework.org/mastodon/) stands for "Multi-hazard Analysis for STOchastic time-DOmaiN phenomena." Once a name has been selected, substitute this for `YourAppName` in the following command:

```bash
cd ~/projects
./moose/scripts/stork.sh YourAppName
```

This should create a directory, `~/projects/YourAppName`, for which to store the new MOOSE application. The terminal will also prompt the user to initialize the new application as a GitHub repository, which will be discussed in the [#git] section.

### Compile the Application's Executable id=make

C++ is sometimes referred to as a compiled, or machine, language, because an application's entire code must be first transcribed into a single file that is more suitable for binary computations. During this transcription, a code may even be optimized for speed and data allocation, which makes C/C++, perhaps, the most computationally efficient programming language in existence. To compile the new application, one may run the following commands in their system's terminal:

```bash
cd ~/projects/YourAppName
make -j4 # use number of processors available on your system, i.e., 2, 4, ..., 12, or <N_procs>
```

This should create an executable (binary) file called `YourAppName-opt` in the application's root directory, which can be used to run [!ac](FE) simulations. In addition to the application's code, the above command will also compile code available from [`moose/framework/`](https://github.com/idaholab/moose/tree/master/framework). A MOOSE-based application always has the full power of MOOSE plus its own.

!alert note title=Compiling C++ Code
Each time you make a change to a MOOSE C++ file, you will have to recompile your application, by running `make`, in order for the changes to take effect.

*For more information about compiling MOOSE applications, please visit the [application_development/build_system.md] page.*

### Test the Application id=test

MOOSE Applications have a simple testing system for user's to quickly verify that the executable was built properly and that the source code is performing in accordance with developer standards. It works by running simple models on the user's machine and comparing the output to a database of expected results. Once the `YourAppName-opt` executable exists, a simple test can be ran:

```bash
cd ~/projects/YourAppName
./run_tests -j4
```

If the test passed, the terminal output should look something like that shown below.

```
test:kernels/simple_diffusion.test ........................................................................ OK
--------------------------------------------------------------------------------------------------------------
Ran 1 tests in 0.3 seconds.
1 passed, 0 skipped, 0 pending, 0 failed
```

<!--Can I make this code colored somehow? The "OK" should be green-->

Later in this tutorial, MOOSE's testing system will be explored in greater detail and the reader will create more simple tests for their application.

*For more information about the MOOSE testing system, please visit the [application_development/test_system.md] page.*

## Enable Use of GitHub id=git

In many ways, [Git](https://git-scm.com) is a version control system that enables large and diverse teams of software developers to manage each other's individual contributions to a single application. When using Git, a `commit` is an update to the repository that marks a checkpoint to be revisited even after further changes are made. A repository's *commit log* shows the history of commits, and helps developers track the progression of their code. A `push` uploads the local version of the commit log, along with the changes to files that each commit made, to the remote (online) repository.

### Open an Account id=account

To create an account with GitHub, please proceed to [github.com/join](https://github.com/join) and enter your credentials. Keep in mind that the chosen username is sometimes required as part of commands for interacting with Git and that it will appear on all published work. It is recommended that the reader chose an alias that is simple, professional, and that they are comfortable with.

### Create a Repository id=repo

Once an account has been created, Github's root page will transform into the user's personal dashboard. To create a new repository, click on the dashboard link that says "new," and name the repository `YourAppName`. A description of the new MOOSE application may be helpful, but the user need not include a README, since the application directory that was just created already has one.

The stork script initializes a new application with a single git commit whose message is "Initial Files." To publish the application to the new GitHub repository, run the following commands:

```bash
cd ~/projects/YourAppName
git remote add origin https://github.com/YourGitHubUserName/YourAppName
git push -u origin master
```

The terminal will prompt the user to enter their GitHub account credentials before uploading the new repository. Once the data has been uploaded, a copy of the application will be found at `github.com/YourGitHubUserName/YourAppName`

### Interacting with Git Using a Secure Shell (SSH) id=ssh

As developments to the application are made, and more `push` commands are ran, it may become a nuisance to enter GitHub credentials each time. To avoid this, it is possible for a user to associate their computer with their GitHub account using [SSH](https://www.ssh.com/ssh/). To generate an SSH key, open the terminal and enter the following commands:

```bash
ssh-keygen -t rsa -C YourEmail
cat ~/.ssh/id_rsa.pub
```

Copy the output from the terminal, which should begin with `ssh-rsa` and end with `YourEmail`. Then, in the GitHub account [Personal settings](https://github.com/settings/profile) page, navigate to the "SSH and GPG keys" tab and paste the output to the "New SSH key" form. The user's machine will now have secure access to their GitHub account.

The local repository, where the new application has been stored, needs to be linked to the SSH version of the remote url:

```bash
cd ~/projects/YourAppName
git remote set-url origin git@github.com:YourGitHubUserName/YourAppName.git
```

From now on, the user is no longer required to enter their GitHub credentials when interacting with this repository from their system's terminal. There are a lot of ways to interact with Git, and the reader is encouraged to learn more at their own will.

*For more information about Git, please visit the [framework_development/git.md] page.*

!content pagination previous=tutorial01_app_development/problem_statement.md
                    next=tutorial01_app_development/step02_input_file.md
