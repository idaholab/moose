## Our .emacs

If you would like to follow our [Code Standards](/wiki/CodeStandards/) automatically while using Emacs a good choice is to utilize our `.emacs` file.  [You can get that file here](/wiki/57/plugin/attachments/download/16/). You will need to rename it to `~/.emacs`.  You also need to get the `smooth-scrolling.el` ( [attachment:17] ) and put it in `~/.emacs.d/lisp/smooth-scrolling.el`. If the directory ~/.emacs.d/lisp does not exist, you will need to create it.

|Function | Command | 
|-------------- | ---|
|Cancel Command | C-g|
|**Open File** | C-x C-f|
|Switch Open Buffer | C-x b|
|**Save current Buffer** | C-x C-s|
|Save all Buffers | C-x s|
|Quit | C-x C-c|
|Save Buffer as | C-x C-w|
|Kill all Buffers | M-x close-all-buffers|
|Kill Buffer (don't save) | C-x k|
|Toggle Word Wrapping | C-c t|
|Cut a Line | C-k|
|Set a mark (to jump back and forth from) | C-[space] C-[space]|
|Jump from current position to mark | C-x C-x|
|Interactive Search | C-s|
|Interactive Search Back | C-r|
|Regex Search | M-C-s|
|Regex Search Back | M-C-r|
|Find Patterns in files | M-x grep-find|
|Replace String | M-x replace-string|
|Regex Replace | M-x replace-regex-p|
|Interactive Replace | M-x query-replace|
| | M-% |
|Multi-file Query Replace|M-x find-dired 
| | Mark using M|
| | M-x dired-do-query-replace-regexp 
| | Save all buffers|
|Help Key | C-h f|
|Help Function | C-h k|
|Goto Line | M-g M-g|
| | M-g g|
|**Switch between header/body** | C-c c|
|Beginning of C/C++ Function | M C-a|
|End of C/C++ Function | M C-e|
|Re-indent Function | C-c C-q|
| | M-x indent-region|
|Code Comment Region (CUA) | C-c C-c C-c| 
| | M-x comment-region|
|Code Uncomment Region (CUA) | C-u C-c C-c C-c 
| | M-x uncomment-region|
|Block Select Mode |C-enter |
|Select Mode (Set Mark) |C-space |
|F5 |Split Screen Horizontally|
|F6 |Split Screen Vertically|
|Delete Window | M-x delete-window|
|C-c t | Toggle word wrapping (requires our .emacs file)|
|F7 |Go back one Compile Error|
|F8 |Compile (Run make)|
|F9 |Go forward one Compile Error|
| | |
|Shift Left-click | Change your font|
|M-x customize face RET default RET | Save your font changes |

## Remote Editing ##
Emacs supports remote editing too!  Just check out tramp mode - no changes necessary to your config file

Note: You will need to setup ssh keys to make this work effectively
[http://www.emacswiki.org/emacs/TrampMode]

## In-line Spell Checking ##
It is possible to enable in-line spell checking in emacs that only examines comments and strings.

* Install Coco Aspell: [http://people.ict.usc.edu/~leuski/cocoaspell/](http://people.ict.usc.edu/~leuski/cocoaspell/)  
* Install a dictionary (Follow instructions in cocoAspell README, which are summarized below - Make sure aspell runs on the CLI)
    * Download a dictionary: [ftp://ftp.gnu.org/gnu/aspell/dict/en/](ftp://ftp.gnu.org/gnu/aspell/dict/en/)
    * Copy the dictionary folder to `/Library/Application Support/cocoAspell
    * In the dictionary directory run the following commands:
```
./configure
make
make install
```

* Download speck: [http://www.emacswiki.org/emacs/speck.el](http://www.emacswiki.org/emacs/speck.el)
* Copy `speck.el` to the `~/.emacs.d` directory
* Add the following to your `.emacs` file, be sure the dictionary directory matches what you downloaded and installed.

```Lisp
;; Aspell
(setenv "DICTIONARY" "en_US")
(setq ispell-program-name "aspell")
(add-to-list 'exec-path "/usr/local/bin")
(setq ispell-dictionary-alist
      '((nil
	 "[A-Za-z]" "[^A-Za-z]" "[']" nil
	 ("-B" "-d" "en_US" "--dict-dir"
	  "/Library/Application Support/cocoAspell/aspell6-en-2016.01.19-0")
	 nil iso-8859-1)))

;; Spell checking (http://www.emacswiki.org/emacs/speck.el)
(load "speck")

;; Create a function for setting up speck source code mode
(defun speck-prog-mode ()
  (set (make-local-variable 'speck-syntactic) t)
  (speck-mode))

;; Add a hook to your language hooks to start speck
(add-hook 'c++-mode-hook 'speck-prog-mode)
(add-hook 'python-mode-hook 'speck-prog-mode)

;; Create a latex mode and add hook to start with .tex files
(defun speck-tex-mode()
  (speck-mode)
  (setq speck-filter-mode 'TeX))
(add-hook 'latex-mode-hook 'speck-tex-mode)
```

# Explaination of .emacs file Follows #

## CC-Mode Style ##

First up is CC-Mode style that is used for all of the Fuels projects:

```Lisp
(defconst my-c-style
  '((c-tab-always-indent        . t)
    (c-basic-offset             . 2)
    (c-hanging-braces-alist     . ((substatement-open before after)))
    (c-offsets-alist . ((innamespace . 0)
                        (statement-block-intro . +)
			(substatement-open . 0)
		        (substatement-label . 0)
			(label . 0)
			(statement-cont . +))))
  "My C Programming Style")

(c-add-style "PERSONAL" my-c-style)

(defun my-c-mode-common-hook ()
  (c-set-style "PERSONAL")
  (setq indent-tabs-mode nil)
  (c-toggle-auto-hungry-state)
  (c-toggle-auto-newline)
  (c-toggle-auto-state)

  )

(add-hook 'c-mode-common-hook 'my-c-mode-common-hook)
```

## Auto Indent ##

This is a little snippet that automatically indents after pressing RETURN:
```Lisp
(defun my-make-CR-do-indent ()
  (define-key c-mode-base-map "\C-m" 'c-context-line-break))
(add-hook 'c-initialization-hook 'my-make-CR-do-indent)
```

## Useful Behavior ##

IDO mode (Interactive DO) is EXTREMELY useful.  Once you get used to it you will never go back to the old way.  Opening files and switching buffers (Remember to use C-x b) Turn it on by doing:

```Lisp
(ido-mode t)
```

Some ido-mode customizations that I always use are:

Enable flexible matching of files (you don't have to type the name exactly right to have ido-mode complete it.)

```Lisp
(setq ido-enable-flex-matching t)
```

Ignore certain files when using C-x C-f or C-x b (I like to ignore the auto-generated dependency files we have in MOOSE, for example, and you can still get to the ignored files by typing e.g. .d manually...):

```Lisp
(setq ido-ignore-files '("\\.d$"))
```

CUA mode makes it so that the normal Cut/Copy/Paste key combos work (ie C-x,C-c,C-v respectively).  It also makes it so that you can do Shift-(direction) for highlighting things (just like you can in all other applications.  Finally, it also enables "rectangular marking" by pressing C-(Enter) and then moving around.  To turn it on do:

```Lisp
(cua-mode t)
```

If on a Mac and you prefer to use the normal Mac binding keys try this instead [http://www.emacswiki.org/cgi-bin/emacs-en/MacKeyMode]

```Lisp
(setq cua-enable-cua-keys nil)
(cua-mode t)
(mac-key-mode 1)
(global-set-key [(alt >)] 'end-of-buffer)
(global-set-key [(alt <)] 'beginning-of-buffer)
```

When you have lots of buffers open and do an "svn update" it's likely that one or more of the files you have open have been changed.  In order to get those changes reflected in Emacs you can use:

```Lisp
(global-auto-revert-mode t)
```

## Header/Source Switching ##

This snippet will allow you to switch between the header file and source file by pressing C-cc:

```Lisp
;; Define some handy macros for working with C++ source files
(defun loadCh ()
  "Load the current buffers corresponding .C/.h file"
  (interactive)
  ;; Make seaches case-sensitive
  (let ((case-fold-search nil)
	(fn ""))
  ;; Build the sibling file name
  (if (string-match "\.h$" buffer-file-name)
      ;; Change from .h to .C
      (progn
	(setq fn (concat (file-name-sans-extension buffer-file-name) ".C"))
	(if (string-match "/include/" fn)
	    (setq fn (replace-match "/src/" nil nil fn))
	  ))
    ;; Change from .C to .h
    (progn
      (setq fn (concat (file-name-sans-extension buffer-file-name) ".h"))
      (if (string-match "/src/" fn)
	  (setq fn (replace-match "/include/" nil nil fn))
	)))
  ;; Open the file if it exists
  (if (file-exists-p fn)
      (find-file fn)
    (if (string-match "/include/" fn)
	(progn
	  (setq fn (replace-match "/src/" nil nil fn))
	  (if (file-exists-p fn)
	      (find-file fn)))
      (message "File doesn't exist: %s" fn)))))

(define-key global-map "\C-cc" 'loadCh)
```

## Compiling ##

This is a collection of functions and key defines that allow you to compile inside of Emacs and jump directly to errors/warnings.  By default, to compile, press F8 (For Debug compiling use Shift-F8).  To go to the next error press F9 and the previous error is F7 (Note that on Mac keyboards this corresponds to Play, Skip, Skip Back buttons...).  When you press F8 it will automatically search upwards to find the nearest Makefile to the file you are currently editing, then run Make.

```Lisp
(defun upward-find-file (filename &optional startdir)
  "Move up directories until we find a certain filename. If we
  manage to find it, return the containing directory. Else if we
  get to the toplevel directory and still can't find it, return
  nil. Start at startdir or . if startdir not given"

  (let ((dirname (expand-file-name
		  (if startdir startdir ".")))
	(found nil) ; found is set as a flag to leave loop if we find it
	(top nil))  ; top is set when we get
		    ; to / so that we only check it once

    ; While we've neither been at the top last time nor have we found
    ; the file.
    (while (not (or found top))
      ; If we're at / set top flag.
      (if (string= (expand-file-name dirname) "/")
	  (setq top t))
      
      ; Check for the file
      (if (file-exists-p (expand-file-name filename dirname))
	  (setq found t)
	; If not, move up a directory
	(setq dirname (expand-file-name ".." dirname))))
    ; return statement
    (if found dirname nil)))

(defun compile-next-makefile ()                                                           
  (interactive)                                                                           
  (let* ((default-directory (or (upward-find-file "Makefile") "."))                       
         (compile-command (concat "cd " default-directory " && "                          
                                  compile-command " -j" (getenv "JOBS") )))                                      
    (compile compile-command))) 

(defun debug-compile-next-makefile ()
  (interactive)                                                                           
  (let* ((default-directory (or (upward-find-file "Makefile") "."))                       
         (compile-command (concat "cd " default-directory " && METHOD=dbg "           
                                  compile-command " -j" (getenv "JOBS") )))                                      
    (compile compile-command))) 

(define-key global-map [f8] 'compile-next-makefile)
(define-key global-map [(shift f8)] 'debug-compile-next-makefile)
(define-key global-map [(meta f8)] 'kill-compilation)
(define-key global-map [f9] 'next-error)
(define-key global-map [f7] 'previous-error)
```

## Saving Editor State ##

This is a a snippet of code that can be added to the .emacs file to save all open buffers and reopen them whenever you close emacs.  It is based on the directory in which you launch emacs.

```Lisp
;---Save the State of Emacs to be Loaded next time it is started---
(defun gk-state-saver ()
;;Save names and cursor positions of all loaded files in ".emacs.files"
  (interactive)
  (setq fname (format "%s.emacs.files" gk-startdir))
  (cond
   ((buffer-file-name)
    (setq currentbuffer (buffer-name)))
   (t
    (setq currentbuffer nil)))
  (cond
   ((y-or-n-p (format "Save state to %s? " fname))
	(switch-to-buffer "*state-saver*")
	(kill-buffer "*state-saver*")
	(switch-to-buffer "*state-saver*")
	(setq bl (buffer-list))
	(while bl
	  (setq buffer (car bl))
	  (setq file (buffer-file-name buffer))
	  (cond
	   (file
		(insert "(find-file \"" file "\")\n")
		(switch-to-buffer buffer)
		(setq mypoint (point))
		(switch-to-buffer "*state-saver*")
		(insert (format "(goto-char %d)\n" mypoint))))
	  (setq bl (cdr bl)))
	(cond
	 (currentbuffer
	  (insert (format "(switch-to-buffer \"%s\")\n" currentbuffer))))
	(set-visited-file-name fname)
	(save-buffer)
	(kill-buffer ".emacs.files")
	(cond
	 (currentbuffer
	  (switch-to-buffer currentbuffer))))))


;--- Save state when killing emacs ----------
(add-hook
 'kill-emacs-hook
 '(lambda ()
    (gk-state-saver)))

;--- Remember from where emacs was started --
(defvar gk-startdir default-directory)
(message "state save directory is: %s" gk-startdir)
;(sleep-for 1)


;--- Load files from .emacs.files -----------
(cond
 ((file-exists-p ".emacs.files")
  (load-file ".emacs.files")))
```





