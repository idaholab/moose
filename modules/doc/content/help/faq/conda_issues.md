## Conda Issues id=condaissues

Conda issues can be the root cause for just about any issue on this page. Scroll through this section, for what may look familiar, and follow those instructions:

- #### `conda: command not found`

  An easy fix which either means; you have yet to install conda, or your path to it, is incorrect or not set. You will need to recall how you installed conda; Was it Miniconda, or Anaconda? Was it the bash package or a double-click install method?
  
  +If it was a double-click install method+, that can unfortunately mean conda is installed, well, anywhere. Your best bet, is to re-attempt the install and pay attention to where it says it's going to install it. Once you figure that out, you can set that PATH yourself, and continue from where you left off on our instructions:
  
  ```bash
  export PATH=/some/path/to/conda/bin:$PATH
  ```
  
  Note the 'bin:$PATH' at the end of that line. Very necesssary. Don't forget it.
  
  With the above PATH set, you can see if you have set it correctly by running `which`:
  
  ```bash
  which conda
  ```
  
  The above command should return a familiar path you exported. If not, unfortunately you'll have to figure out as to where Conda truly is.
  
  +If it was the bash package method+ (our perferred method), odds are you still have that information located in your terminal. The quickest way to find it, is to search through your history:
  
  ```bash
  history | grep "bash \|sh "
  ```
  
  If you're lucky, the above command will contain a line or two about how you install Conda. And among that line, is the path you need to export:
  
  ```pre
  $ history | grep "bash \|sh "
  999  sh ~/Downloads/Miniconda3-latest-MacOSX-x86_64.sh -b -p ~/miniconda3
  ```
  
  According to the above, I installed miniconda to my home directory. So I need to export PATH like this:
  
  ```bash
  export PATH=~/miniconda3/bin:$PATH
  ```
  
  Note: Don't forget the 'bin:$PATH'

- #### `conda activate`

  If activate is failing, it's possible you have yet to perform a `conda init` *properly*. See conda init below. It could also mean you have an older version of Conda, or that the environment you are trying to activate is somewhere other than where conda thinks it should be, or simply missing. Unfortunately, much of what can be diagnosed, is going to be beyond this document. So instead, try to create a new environment instead:
  
  ```bash
  conda create --name testing --quiet --yes
  ```
  
  The above should create an empty environment. Try and activate it:
  
  ```bash
  conda activate testing
  ```
  
  If this fails, or the create command before it, the error will likely be involved with how Conda was first installed (perhaps with sudo rights, or as another user). You should look into removing this installation of conda, and starting over. Or, install a new instance of conda, and adjust your PATH to the new one instead. Failures of this nature can also mean your conda resource file (~/.condarc) is in bad shape. We have no way of diagnosing this in a troubleshooting fashion, as this file can contain more than just moose-related configs. But the bare minimum for moose development should resemble the following:
  
  ```bash
  channels:
    - https://mooseframework.org/conda/moose
    - conda-forge
    - defaults
  ```

- #### `conda init`

  If init is failing, it is possible that Conda simply does not know what shell you are operating in, and it created a 'configuration' for the wrong shell, or not at all.
  
  WIP

- #### `conda update`

  WIP

- ### Not listed

  The quick fix-attempt, is to re-install the moose-packages:

  ```bash
  conda deactivate
  conda remove moose --all --yes
  conda create moose moose-libmesh moose-tools
  ```
