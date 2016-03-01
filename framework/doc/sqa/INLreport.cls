% ---------------------------------------------------------------------------- %
% $Id: SANDreport.cls,v 1.79 2007/12/13 21:27:14 rolf Exp $
%
% We start out with the standard LaTeX document class "article" or "report" and
% modify it so it conforms to the Sandia Technical report style.
% The modifications are based on "Guide to Preparing SAND Reports",
% SAND98-0730, edited by Tamara K. Locke, and the newer "Guide to
% Preparing SAND Reports and Other Communication Products", SAND2002-2068P.
%
% Get the latest version and more at
%    http://www.cs.sandia.gov/~rolf/SANDreport
%
% Rolf Riesen, December 13, 2007.
%
%
% 06/14/2002 v1.01
%     Make distribution list wider. Spaces between area codes and phone numbers
%
% 06/17/2002 v1.02
%     Use \raggedright for main title
%
% 06/19/2002 v1.03
%     Set tocdepth to 2 to only list sections and subsections in TOC
%
% 08/07/2002 v1.04
%     Added a Makefile based on one Mike Heroux provided
%     Allow for distribution lists that span more than one page and
%     typeset the distribution in two columns
%
% 11/25/2002 v1.05
%     Some changes from Daniel Segalman to conform to the new guide
%     (SAND2002-2068P):
%     - Adjust top margin of the title page to conform to the MS Word
%         template (this may or may not be correct depending on your
%         latex/dvips initial setup. Use texconfig to setup paper and
%         dvips).
%     - Added the word "Printed" before the date on front page
%     - Classification category is no longer needed on page 3; get
%         rid of \SANDdistcategory
%     - Added "National Nuclear Security Administration" on title page
%     - Changed NTIS URL on page 2
%
%     If "strict" is not used, we'll number sections to two decimal
%     places (1.2.3)
%
% 02/13/2003 v1.06
%     New DOEbwlogo.eps from Tim Draelos. This one is smaller and has some
%     incompatible junk removed.
%
% 10/16/2003 v1.7
%     Using \ifthenelse in the addcontentsline command interfered with
%     the subfigure class. James Gruetzner and Todd Pitts provided a fix.
%
%     Philippe Pebay reports that the color package needs to be imported
%     before the graphicx package. This class file uses the graphicx package,
%     so Philippe could not use the color package because graphicx was already
%     loaded when his \usepackage command got executed.
%
% 11/19/2003 v1.8
%     Make it work using the report class. Put "report" as an option into
%     \documentclass[report]{SANDreport} to activate it. There were some other
%     fixes and clean-up s as well. Mainly in the table of contents.
%     - Changed NTIS URL on page 2
%     - Added fleqn to \LoadClass
%     - Can't get the report title to repeat on the first chapter page.
%         Left it off for now.
%     - The table of contents formatting for report class is more or less like
%         report.cls. For article, we try to make it look like the guideline.
%
% 01/19/2004 v1.9
%     - The distribution list at the end of a SAND report was modifying the
%         description environment which interfered with its regular use. Created
%         a separate environment to fix that.
%     - If in "report", list chapters and sections in TOC
%     - With the jpg option, load the jpg versions of the DOE and Sandia
%         logos. This makes it possible to use pdftex which doesn't
%         understand Postscript.
%
% 02/25/2004 v1.10
%     - It is now SANDIA REPORT, no longer SAND REPORT. Central Technical Files
%         now wants three copies. See the distribution examples. (Info from
%         Philippe Pebay)
%     - Added new commands for markings used in OUO, export controlled, and
%         other reports not labeled "Unlimited Release". Thanks to Mark
%         Ackermann for providing an OUO example.
%     - In report style, the page numbers restarted after the title page. Fixed!
%     - Big clean-up.
%
% 05/14/2004 v1.11
%     - Had to comment out a line in fancyhdr.sty for pdflatex to work.
%         Don't know why.
%
% 05/21/2004 v1.12
%     - Added the options [FIGBOTCAP,normal,bf,tight] to the subfigure
%         package in the example files from a suggestion by Philippe P. Pebay.
%     - Added the CA and NM options to insert the respective standard distributions.
%
% 08/07/2004 v1.13
%     - Use pdf versions of logos. Makes the "jpg" option obsolete. Suggestion
%         and logos by Richard Lehoucq.
%     - Making SANDreport.cls compatible with the hyperref package
%     - Added SANDmath.tex from Philippe Pebay
%
% 08/18/2004 v1.14
%     - \begin{SANDdistribution} now takes an optional argument (CA or NM). The
%         respective \documentclass options are no longer valid. Several new
%         commands are available within the SANDdistribution environment to
%         automate distributions for special cases such as CRADA, LDRD,
%         Classified, Patent, etc.
%     - The distribution list is now better formatted.
%     - Fixed a bug with \SANDreleaseType discovered by Mark Ackermann
%     - Added the command \SANDmarkTop
%
% 05/20/2005 v1.15
%     - A small change in the distribution list (D. Chavez in the LDRD office
%         moved) Thanks to Brandon Rohrer.
%
% 12/05/2005 v1.16
%     - New org numbers reflect new corporate structure 9616->4536, etc. Thanks
%         to Philip Sackinger for doing this.
%     - Philip also found out what the newest distribution rules for NM are.
%     - In the example files, deleted a misleading comment about secnumdepth in
%         the appendix. Thanks to John Mitchell for spotting this.
%     - New SANDmath.tex from Philippe Pebay: (ver. 1.2)
%         + add support for discrete intervals
%         + fix some comments
%         + replace incorrect \RequirePackage's by \usepackage'
%     - Also from Philippe Pebay: CA distribution list changes.
%     - Abstract will start on title page, even with report style. Thanks to
%         Daniel Dolan for noticing it and making suggestions to fix it.
%     - Put the title just above the first chapter in the report style
%
% 12/09/2005 v1.17
%    - Fixed a bug introduced in the last version: The \cleardoublepage is
%        only supposed to happen for chapters in the main body of a report,
%        and it needs to happen before the page number is entered into the TOC.
%    - Fixed several new and old bugs that dealt with markings
%    - Fixed text on usage page about marking commands.
%
% 12/09/2005 v1.18
%    - Make it work with hyperref again....
%
% 12/19/2005 v1.19
%    - There were still problems with the hyperref package. Fixed it so that
%        the page markings on the back cover are now correct.
%
% 12/21/2005 v1.20
%    - Updated the distribution list according to version 2.0 of the Guide
%
% 03/22/2006 v1.21
%    - Introduced \SANDrePrintDate and \begin{SANDreDistribution}
%    - Put a page number (2) on the back cover
%    - No institution in \SANDauthor{} in example files
%    - Move the SNL logo on the cover towards the bottom of the page
%    - Add a back cover with the SNL logo at the bottom of the back
%    - Use \cleardouble page and add comment to example files to make
%        sure TOC always starts on an odd page.
%    - Don't put title on first page of body text (above introduction)
%        anymore. (Changed from Ver 1.0, Sep. 2002 to Ver 2.0, Dec. 2005 of the guide.)
%    - For article, start every section on a new page (report already does this)
%    - Fixed URLs for DOE report ordering
%    - \SANDmarkCover now takes only one argument! This makes it easier to draw a
%        box around the statement and configure it as needed. See MarkOUO.tex for
%        an example. Thanks to John Foster for the example.
%    - Print small class file version number at bottom of inside back cover
%    - Change by Daniel Dolan to make it work and look consistent with the mathptmx
%        package. This leaves the Helvetica fonts on the cover alone, but lets him
%        have the body of the report in Times.
%    - From Debbie Johnson (Creative Arts):
%        - In example files, use First Name, Middle Initial, Last Name on cover and title page
%        - In example files, on cover, put author names on one line
%        - Mail Stop for Central Technical Files is MS9018. (This was an error in the example
%            file, not the class itself.)
%        - Back cover was shifted right too far.
%        - Move chapter headings up a bit.
%        - Use the geometry package to get the margins right
%
% 06/10/2006 v1.22
%    - Remove "Unclassified" from "Unclassified Unlimited Release" (Thanks Brett Bader and Mike Heroux)
%    - Confirmed that M2497 is correct mail stop for classified reports to Central Technical
%          Files. (Thanks Debbie Johnson)
%    - Make the distribution list single column. This makes it easier to print labels.
%    - D. Chavez, LDRD Office mail-stop changed from 0188 to 0123. (Thanks Brett Bader)
%    - In examples, added \phantomsection and moved \addcontentsline{toc}{section}{References}
%        just before the \bibliography{...} command. This helps putting the right page number
%        into the TOC and also with hyperlinking. (Thanks Tammy Kolda)
%    - Clarified usage of \SANDnum{} on Web page. (Thanks Tammy Kolda)
%    - More usage and LaTeX tips on Web page. (Thanks Tammy Kolda)
%    - Newer distributions have a fancyhdr.sty that works. Removed it.
%
% 06/26/2006 v1.23
%    - Tammy Kolda found a bug when using amsmath. Seems to have been a problem with
%        white space in the redefinition of \section. LaTeX would stop when building the TOC.
%
% 08/17/2006 v1.24
%    - Changed tar file extension from .tgz to .tar.gz Some winzip versions under
%        Windows don't seem to like the former. (Rich Lehoucq)
%    - Org. Number for Central Technical Files changed from 8945-1 to 8944 (Tammy and Debbie)
%
% 09/19/2006 v1.25
%    - David Wilson discovered that markings in report style also activated chapter
%        name headings, which are unwanted and sometimes overlap with the markings.
%
% 10/03/2006 v1.26
%    - Some Web page error fixes from Daniel Dunlavy
%    - Mike Eldred suggested to keep internal distributions on a single line
%
% 10/03/2006 v1.27
%    - Forgot to extend the width of external distributions
%
% 12/13/2008 v1.28
%    - No change: Org. Number for Central Technical Files is 8944 (confirmed by Beth Watkins)
%    - Mark the OUO examples as sample to avoid misunderstandings (Roger Suppona)
%    - Paragraph indent is set to 0 for cover pages, but never restored (Daniel Dolan)
%    - Use "Lorem ipsum" instead of "bla bla bla" in the examples (Stephanie Phillips)
%        (There is a LaTeX package, lipsum.sty, that does it, but it is not always installed.
%        Therefore, I included the text verbaitm, instead of generating it.)
%    - If the report is an OUO document, some text on the back cover needs to disappear.
%         Thanks to Marcia Cooper for discovering it, and providing the necessary LaTeX code.
%    - A lot of changes from Michael Kaneshige to better match the formatting of the official
%         SAND report template:
%         1. Removed the odd-even margin asymmetry - made the textwidth 6.5 in and
%	    both margins 1 in.  This is consistent with the official cover
%	    templates, although it's not the standard LaTeX way of formatting double
%	    sided documents.  If the document gets bound, the LaTeX way is better,
%	    but a lot of documents exist only electronically now.
%
%	  2. Added more space to \topmiddlewidth and subtracted it from
%	    \topleftwidth and \toprightwidth so that "official Use Only / Export
%	    Controlled Information" would not get broken across lines on the title
%	    page.
%
%	  3. Reduced the \vspace between the title and authors from 0.75in to
%	    0.375in.  This is specific to my document, but with everything we were
%	    required to add, and a moderate length title, LaTeX was generating a
%	    blank first page because the cover page was overfilled.
%
%	  4. On the back cover, I centered the text block and reduced the minipage
%	    width to make it look more like the official covers.
%
%	  5. The word "NOTICE" on the back cover was significantly larger than the
%	    other text, which was not consistent with the official covers.  I
%	    removed the \fontsize command that was causing this, and it looks more
%	    consistent with the official covers.
%
%	  6. On the title page, the top block (SAND#, OUO/ECI, Printed...) was not
%	    centered because there were topleft and topmiddle boxes, but no topright
%	    box.  I think this was inherited from the report.cls and article.cls
%	    classes, but to fix the asymmetry, I inserted a topright box the same as
%	    the topleft box.  It would seem to work just as well to remove both
%	    topleft and topright boxes, since they contain nothing, but to be
%	    conservative, I didn't remove anything.
%
%	  7. Most of MarkOUO.tex has to be edited by the user, but the "Official
%	    Use Only" (SANDmarkTopBottom) should be fairly generic.  To make it look
%	    more like the official covers, I made it bold, 12 pt, all caps.  In
%	    addition, I reformatted the OUO box from tabular with multicolumn
%	    columns, hlines and vertical bars, to a minipage inside a framebox.  I
%	    think this is smoother.
%
%    - The Technical Library has a new org number again: 4536 --> 9536 (Ann Smaple via Bill Hart)
%        Also changed NM tech library org to 8944, but I am mostly guessing here...
%        Added  (1 electronic and 1 hardcopy) for housekeeping copies and reduce spacing
%        between distribution entries. (Ann Smaple via Bill Hart)
%    - Central Technical Files doesn't seem to want unclassified reports anymore. (Several)



% ******************************************************************************
% Package definitions
%
    % The version and date of this file
    \newcommand\SANDreportVersionOnly{ v1.28 }		% leave the spaces before and after version
    \newcommand\SANDreportVersionDate{2007/12/13}	% Date has to bis this format!

    \typeout{Using Sandia Technical Report class: \SANDreportVersionDate, \SANDreportVersionOnly Rolf Riesen}
    \newcommand\SANDreportVersion{\SANDreportVersionDate \SANDreportVersionOnly}

    % We need LaTeX 2e or better
    \NeedsTeXFormat{LaTeX2e}

    % We need ifthenelse. \newboolean are false by default
    \RequirePackage{ifthen}
    \RequirePackage{color}
    \RequirePackage{graphicx}

    % Variables
    \newboolean{relaxedSAND}
    \newboolean{strictSAND}
    \newboolean{reportSAND}
    \newboolean{SANDmainProvided}
    \newboolean{SANDnumProvided}
    \newboolean{SANDprintDateProvided}
    \newboolean{SANDrePrintDateProvided}
    \newboolean{SANDauthorProvided}
    \newboolean{SANDsupersedProvided}
    \newboolean{SANDdistributionProvided}
    \newboolean{inAppendix}
    \newboolean{SANDmarkingsRequired}
    \newboolean{MarkCover}
    \newboolean{MarkBack}
    \newboolean{MarkTitle}
    \newboolean{MarkBody}
    \newboolean{MarkBottom}
    \newboolean{distCA}
    \newboolean{distNM}


    % Added 1/11/07 M. Cooper to remove additional text unnecessary for OUO documents on pg 2.
    \newboolean{SANDnoticeOUO}

    % Counters
    \newcounter{SANDprintingCnt}
    \setcounter{SANDprintingCnt}{1}
    \newcounter{SANDreDistCnt}
    \setcounter{SANDreDistCnt}{1}


    % We need calc to divide by 3 in setlength and other calculations
    \RequirePackage{calc}

    % We need the fancy headers for the report markings
    \RequirePackage{fancyhdr}


    % Specify what this file provides
    \ProvidesPackage{INLreport}[\SANDreportVersion]
    \ProvidesFile{INLreport.cls}[\SANDreportVersion]


    % We understand three options: "relax", "strict", and "report"
    % Using "relax" make the document look more LaTeXy
    % If no option are given, then we follow the format of SAND2002-2068P
    % Using "strict" tries to follow the guideline even more closely
    % Specifying "report" uses the report class instead of the article class
    %
    \DeclareOption{relax}{
	\typeout{Option "relax": Relaxing some of the SAND98-0730 requirements}
	\setboolean{relaxedSAND}{true}
    }
    \DeclareOption{strict}{
	\typeout{Option "strict": Follow the SAND98-0730 requirements more strictly}
	\setboolean{strictSAND}{true}
    }
    \ifthenelse{\boolean{relaxedSAND} \and \boolean{strictSAND}}   {
	\ClassWarning{SANDreport.cls}{Both, "strict" and "relax" options given}
    }{
    }
    \DeclareOption{report}{
	\typeout{Option "report": Using the report class as a base.}
	\setboolean{reportSAND}{true}
    }
    \DeclareOption{CA}{
	\ClassError{SANDreport.cls}{"CA" option is obsolete}{Use
	\@backslashchar begin{SANDdistribution}[CA] instead.}
    }
    \DeclareOption{NM}{
	\ClassError{SANDreport.cls}{"NM" option is obsolete}{Use
	\@backslashchar begin{SANDdistribution}[NM] instead.}
    }


    % Added 1/11/07 M. Cooper to remove additional text unnecessary for OUO documents on pg 2.
    \DeclareOption{OUO}{
	\typeout{Option "OUO": Formatting page 2 for OUO reports}
	\setboolean{SANDnoticeOUO}{true}
    }

    % We're based on the standard article or report class and assume certain
    % options. Set those, and pass all others to the article class
    \ProcessOptions % We need to do it here so we get "reportSAND", and then again
    \ifthenelse{\boolean{reportSAND}}   {
	\DeclareOption*{\PassOptionsToClass{\CurrentOption}{report}}
	\ProcessOptions % Repeat it here, otherwise LaTeX gets confused
	\LoadClass[letterpaper,twoside,final,fleqn]{report}
    }{
	\DeclareOption*{\PassOptionsToClass{\CurrentOption}{article}}
	\ProcessOptions % Repeat it here, otherwise LaTeX gets confused
	\LoadClass[letterpaper,twoside,final,fleqn]{article}
    }


    %
    % Setup for letter paper (This IS a SAND report!)
    %
    \RequirePackage{geometry}
    % \geometry{pdftex, showframe, inner=1.5in, textwidth=6in, textheight=9in}
    % Changed textwidth to 6.5in and inner to 1in to more closely match official SAND cover templates - MJK, 1/12/07
    %\geometry{pdftex, inner=1in, textwidth=6.5in, textheight=9in}
% GAH - fix this to work for me!
    \geometry{pdftex, top=1.5in, inner=1in, textwidth=6.5in, textheight=9in}



% ******************************************************************************
% Check some of the user directives and setup a few things
%

    \AtBeginDocument{

	% Do some usage checking
	\ifthenelse{\boolean{SANDnumProvided}}   {
	}{
	    \ClassError{SANDreport.cls}{\@backslashchar SANDnum
		not provided}{Insert \@backslashchar SANDnum in the preamble
		of your document}
	}
	\ifthenelse{\boolean{SANDauthorProvided}}   {
	}{
	    \ClassError{SANDreport.cls}{\@backslashchar SANDauthor
		not provided}{Insert \@backslashchar SANDauthor in the preamble
		of your document}
	}
	\ifthenelse{\boolean{SANDprintDateProvided}}   {
	}{
	    \ClassError{SANDreport.cls}{\@backslashchar SANDprintDate
		not provided}{Insert \@backslashchar SANDprintDate in the
		preamble of your document}
	}

	% Save the title, it will be cleared by \maketitle, but we need it later
	\newsavebox{\SANDtitletempbox}
	\sbox{\SANDtitletempbox}{\parbox{\textwidth - 1.5in}{
	    \centering \LARGE \bf \@title \par}
	}

	%
	% Some measurements to format the title page properly
	%
	\newlength{\textwidththird}
	\newlength{\topheight}
	\newlength{\topleftwidth}
	\newlength{\topmiddlewidth}
	\newlength{\toprightwidth}

	\setlength{\textwidththird}{(\textwidth / 3) - (\fboxsep * 2)}
	\setlength{\topleftwidth}{\the\textwidththird}
	\setlength{\topmiddlewidth}{\the\textwidththird}
	\setlength{\toprightwidth}{\the\textwidththird}

	% Changed following to -6, +10, and -6 from -3, +4, -3, to get "Official Use Only - Export Controlled Information
	% to fit on one line on title page - MJK, 1/12/07
	\addtolength{\topleftwidth}{-6em}
	\addtolength{\topmiddlewidth}{+10em}
	\addtolength{\toprightwidth}{-6em}


	\ifthenelse{\boolean{SANDsupersedProvided}}   {
	    \setlength{\topheight}{1.5in}
	}{
	    \setlength{\topheight}{1.0in}
	}

	%
	% We want a blank line between paragraphs
	%
	\setlength\parskip{2ex}


	% We don't want a head rule
	\renewcommand{\headrulewidth}{0pt}
    } % End of AtBeginDocument{}



% ******************************************************************************
% Cover pages
%

    % ------------------------------------------------------------------------ %
    % Font selections for the cover page \Coverfont{series}{size}{skip}
    %
    \newcommand{\CoverFont}[3]{
	\fontencoding{OT1}
	\fontfamily{phv}
	\fontseries{#1}
	\fontshape{n}
	\fontsize{#2}{#3}
	\selectfont
    }

    \newcommand{\doCover}{
	%
	% Print the cover pages
	%
	\SANDmarks{cover}
	\newlength{\coverindent}
	\setlength{\coverindent}{1em}
	\newlength{\parindentsave}
	\setlength{\parindentsave}{\parindent}
	\setlength{\parindent}{0em}


	%
	% Front Cover
	%
	%\hspace*{-0.375in}
	\begin{minipage}[t]{6.0in}
	    \CoverFont{b}{18}{20pt}
	    INL REPORT
	    \vspace{0.125in}\\
	    \CoverFont{m}{12}{14pt}
	    \SANDnumVar\\
	    \SANDreleaseTypeVar\\
	    \ifthenelse{\boolean{SANDrePrintDateProvided}}   {
		Printed \SANDrePrintDateVar
	    }{
		Printed \SANDprintDateVar
	    }
	    \ifthenelse{\boolean{SANDsupersedProvided}}   {
		\vspace{0.125in}\\
		\SANDsupersedVar
	    }{
		\\
	    }

	    \vspace{0.25in}
	    \CoverFont{b}{24}{28pt}
	    \parbox[t]{\textwidth}{
		\raggedright
		\@title
	    }


            % Changed this space from 0.75in to 0.375in to match official SAND covers, and provide more space to prevent
	    % overfilling the page - MJK, 1/12/07
	    \vspace{0.375in}
	    \CoverFont{m}{12}{14pt}
	    \SANDauthorVar

	    \vspace{0.25in}
	    \CoverFont{m}{8}{10pt}
	    \selectfont
	    Prepared by\\
	    Idaho National Laboratory\\
	    Idaho Falls, Idaho 83415\\
	    \\
	    The Idaho National Laboratory is a multiprogram laboratory operated by\\
	    Battelle Energy Alliance for the United States Department of Energy\\
	    under DOE Idaho Operations Office. Contract DE-AC07-05ID14517.\\
	    \\
	    \MarkCoverVar
	\end{minipage}

	\vspace*{\fill} % Move the logo to the bottom of the cover page

	%\hspace*{-0.5in}
	\begin{minipage}[b]{6.0in}
	    \includegraphics{inl-left_black}
	\end{minipage}



	%
	% Back cover
	%
	\newpage
	% Added centering and reduced minipage width to match official SAND covers better - MJK, 1/12/07
	\begin{center}
	\begin{minipage}[t]{5.5in}
	    \fontencoding{OT1}
	    \fontfamily{pnc}
	    \fontseries{m}
	    \fontshape{n}
	    \fontsize{9}{11pt}
	    \selectfont
            Issued by the Idaho National Laboratory, operated for the
	    United States Department of Energy by Battelle Energy Alliance.
	    \vspace{\baselineskip}

            % Removed \fontsize{11}{13} to make NOTICE simply bold, to match official SAND covers better, MJK - 1/12/07
	    {\fontseries{b}\selectfont NOTICE:}
	    This report was prepared
	    as an account of work sponsored by an agency of
	    the United States Government. Neither the United
	    States Government, nor any agency thereof, nor any
	    of their employees, nor any of their contractors,
	    subcontractors, or their employees, make any warranty,
	    express or implied, or assume any legal liability or
	    responsibility for the accuracy, completeness, or
	    usefulness of any information, apparatus, product,
	    or process disclosed, or represent that its use
	    would not infringe privately owned rights. Reference
	    herein to any specific commercial product, process,
	    or service by trade name, trademark, manufacturer, or
	    otherwise, does not necessarily constitute or imply
	    its endorsement, recommendation, or favoring by the
	    United States Government, any agency thereof, or any
	    of their contractors or subcontractors. The views and
	    opinions expressed herein do not necessarily state
	    or reflect those of the United States Government,
	    any agency thereof, or any of their contractors.
	    \vspace{\baselineskip}

	    % Added 1/11/07 M. Cooper to remove additional text unnecessary for OUO documents on pg 2.
	    \ifthenelse{\boolean{SANDnoticeOUO}}{}{
	    Printed in the United States of America. This report has
	    been reproduced directly from the best available copy.
	    \vspace{\baselineskip}

	    }% end of text for non-OUO reports
	    \vspace{\baselineskip}
	    \vspace{0.125in}

	    \includegraphics[width=0.75in]{DOEbwlogo}
	\end{minipage}
	\end{center}

	% Restore the original paragraph indent
	\setlength{\parindent}{\parindentsave}

	\SANDmarks{back}
    }


% ******************************************************************************
% The user commands
%

    % ------------------------------------------------------------------------ %
    % Here we define the mandatory declarations
    %
    \newcommand{\SANDnum}[1]{
	\newcommand{\SANDnumVar}{#1}
	\setboolean{SANDnumProvided}{true}
    }

    \newcommand{\SANDauthor}[1]{
	\newcommand{\SANDauthorVar}{#1}
	\setboolean{SANDauthorProvided}{true}
    }

    \newcommand{\SANDprintDate}[1]{
	\newcommand{\SANDprintDateVar}{#1}
	\setboolean{SANDprintDateProvided}{true}
    }


    % This one is optional
    \newsavebox{\SANDrePrintDistBox}
    \newsavebox{\SANDrePrintDistBoxSecond}
    \newsavebox{\SANDrePrintDistBoxThird}
    \newsavebox{\SANDrePrintDistBoxFourth}
    \newsavebox{\SANDrePrintDistBoxFifth}
    \newcommand{\SANDrePrintDateVar}{}

    \newcommand{\SANDrePrintDate}[1]{
	\renewcommand{\SANDrePrintDateVar}{#1}
	\setboolean{SANDrePrintDateProvided}{true}
	\stepcounter{SANDprintingCnt}
	\ifthenelse{\value{SANDprintingCnt} = 2}   {
	    \sbox{\SANDrePrintDistBoxSecond}{Second Printing, (#1):}
	}{
	    \ifthenelse{\value{SANDprintingCnt} = 3}   {
		\sbox{\SANDrePrintDistBoxThird}{Third Printing, (#1):}
	    }{
		\ifthenelse{\value{SANDprintingCnt} = 4}   {
		    \sbox{\SANDrePrintDistBoxFourth}{Fourth Printing, (#1):}
		}{
		    \ifthenelse{\value{SANDprintingCnt} = 5}   {
			\sbox{\SANDrePrintDistBoxFifth}{Fifth Printing, (#1):}
		    }{
			\ClassError{SANDreport.cls}{\@backslashchar SANDreDistribution
			    used more than four times!}{Do you really have more than
			    five printings? If so, you need to amend SANDreport.cls}
		    }
		}
	    }
	}
    }


    % Used to need the title at the top of the first section/chapter, but that is
    % not required anymore.  Make sure we start on an odd page.
    \newcommand{\SANDmain}{
	\cleardoublepage
	\setboolean{SANDmainProvided}{true}
    }


    \ifthenelse{\boolean{reportSAND}}   {
	%
	% new 1
	% Once we have left the preamble and are in the body of the text,
	% we want each chapter to begin on an odd numbered page
	%
	% new 2
	% After we're done with the title page, we switch to markings for body pages.
	%
	\renewcommand\chapter{%						% \chapter from report.cls
	    %\if@openright\cleardoublepage\else\clearpage\fi		% deleted
	    \ifSANDmainProvided\cleardoublepage\else\clearpage\fi	% new 1
	    \SANDmarks{body}						% new 2
	    % \thispagestyle{plain}%					% deleted
	    \global\@topnum\z@						% \chapter from report.cls
	    \@afterindentfalse						% \chapter from report.cls
	    \secdef\@chapter\@schapter					% \chapter from report.cls
	}								% \chapter from report.cls

	%
	% We redefine \@makechapterhead and \@schapter so we can move
	% the chapter title a little bit higher up.
	%

	%
	% This is for chapters in the body
	\def\@makechapterhead#1{%					% \@makechapterhead from report.cls
	    \vspace*{-10\p@}%						% Changed
	    {\parindent \z@ \raggedright \normalfont			% \@makechapterhead from report.cls
	    \ifnum \c@secnumdepth >\m@ne				% \@makechapterhead from report.cls
		\huge\bfseries \@chapapp\space \thechapter		% \@makechapterhead from report.cls
		\par\nobreak						% \@makechapterhead from report.cls
		\vskip 20\p@						% \@makechapterhead from report.cls
	    \fi								% \@makechapterhead from report.cls
	    \interlinepenalty\@M					% \@makechapterhead from report.cls
	    \Huge \bfseries #1\par\nobreak				% \@makechapterhead from report.cls
	    \vskip 40\p@						% \@makechapterhead from report.cls
	}}								% \@makechapterhead from report.cls

	%
	% This is for chapters in the preamble
	\def\@schapter#1{\if@twocolumn					% \@schapter from report.cls
		\@topnewpage[\@makeschapterhead{#1}]%			% \@schapter from report.cls
	    \else							% \@schapter from report.cls
		\@makeschapterhead{#1}%					% \@schapter from report.cls
		\@afterheading						% \@schapter from report.cls
	    \fi}							% \@schapter from report.cls
	    \def\@makeschapterhead#1{%					% \@schapter from report.cls
	    \vspace*{-10\p@}%						% Changed
	    {\parindent \z@ \raggedright				% \@schapter from report.cls
	    \normalfont							% \@schapter from report.cls
	    \interlinepenalty\@M					% \@schapter from report.cls
	    \Huge \bfseries  #1\par\nobreak				% \@schapter from report.cls
	    \vskip 40\p@						% \@schapter from report.cls
	}}

    }{
	% new 3
	% We want each section starting on a new page The report class already does this
	%
	% new 4
	% Center the section headings
	%
	\renewcommand\section{\clearpage%				% new 3
	    \@startsection {section}{1}{\z@}%				% \section from article.cls
	    {-3.5ex \@plus -1ex \@minus -.2ex}%				% \section from article.cls
	    {2.3ex \@plus.2ex}%						% \section from article.cls
	    %{\normalfont\Large\bfseries}%				% deleted
	    {\normalfont\Large\bfseries\centering}%			% new 4
	}								% \section from article.cls
    }



    % ------------------------------------------------------------------------ %
    % Distribution page at the end
    % We have to create our own description environment, so we don't
    % disturb the original one.
    \newlength{\SANDdistwidth}			% Max width of distribution lines
    \setlength{\SANDdistwidth}{0.8\textwidth}
    \newlength{\SANDMSdistwidth}		% Width of field for MS number
    \setlength{\SANDMSdistwidth}{9ex}

    \newcommand{\SANDdescriptionlabel}[1]{
	    \makebox[3em][r]{#1}
    }
    \newenvironment{SANDdescription}{
	\list{}
	{\labelwidth\z@ \itemindent-\leftmargin \let\makelabel\SANDdescriptionlabel}
    }{
	\endlist
    }

    \newenvironment{SANDdistribution}[1][nothing]{
	\setboolean{SANDdistributionProvided}{true}

	\newboolean{distCRADA}
	\newboolean{distPatent}
	\newboolean{distLDRD}
	\newboolean{distClassified}
	\newcommand{\SANDdistCRADA}{
	    \setboolean{distCRADA}{true}
	}

	\newcommand{\SANDdistPatent}{
	    \setboolean{distPatent}{true}
	}

	\newcommand{\SANDdistLDRD}{
	    \setboolean{distLDRD}{true}
	}

	\newcommand{\SANDdistClassified}{
	    \setboolean{distClassified}{true}
	    \SANDdistInternalM{2}{M2497}{Central Technical Files}{8944}
	}



	% Option NM given
	\ifthenelse{\equal{#1}{NM}}   {
	    \typeout{Option "NM": Using the SNL NM default distribution list.}
	    \setboolean{distNM}{true}
	}{
	}

	% Option CA given
	\ifthenelse{\equal{#1}{CA}}   {
	    \typeout{Option "CA": Using the SNL CA default distribution list.}
	    \setboolean{distCA}{true}
	}{
	}

	\clearpage
	{\Large DISTRIBUTION:}
	\normalsize
	\raggedbottom
	\begin{SANDdescription}
    }{
	\ifthenelse{\boolean{distCA}}   {
	    % Insert the standard CA distribution
	    \ifthenelse{\boolean{distClassified}}   {
		\SANDdistInternal{2}{9018}{Central Technical Files}{8944 (1 electronic and 1 hardcopy)}
	    }{}
	    \SANDdistInternal{2}{0899}{Technical Library}{8944 (1 electronic and 1 hardcopy)}

	    % If this report has a Patent Caution or Patent Interest
	    \ifthenelse{\boolean{distPatent}}   {
		\SANDdistInternal{1}{0161}{Legal Intellectual Property}{11500}
	    }{}

	    % If this report is for CRADA work
	    \ifthenelse{\boolean{distCRADA}}   {
		\SANDdistInternal{1}{0115}{OFA/NFE Agreements}{10112}
	    }{}

	    % If this report is for LDRD work
	    \ifthenelse{\boolean{distLDRD}}   {
		\SANDdistInternal{1}{0123}{D. Chavez, LDRD Office}{1011}
	    }{}

	}{}

	\ifthenelse{\boolean{distNM}}   {
	    % Insert the standard NM distribution
	    \ifthenelse{\boolean{distClassified}}   {
		\SANDdistInternalM{2}{2497}{Central Technical Files}{8944 (1 electronic and 1 hardcopy)}
	    }{}

	   % \SANDdistInternal{2}{0899}{Technical Library}{9536 (1 electronic and 1 hardcopy)}

	    % If this report has a Patent Caution or Patent Interest
	    \ifthenelse{\boolean{distPatent}}   {
		\SANDdistInternal{1}{0161}{Legal Intellectual Property}{11500}
	    }{}

	    % If this report is for CRADA work
	    \ifthenelse{\boolean{distCRADA}}   {
		\SANDdistInternal{1}{0115}{OFA/NFE Agreements}{10112}
	    }{}

	    % If this report is for LDRD work
	    \ifthenelse{\boolean{distLDRD}}   {
		\SANDdistInternal{1}{0123}{D. Chavez, LDRD Office}{1011}
	    }{}

	}{}

	\end{SANDdescription}
    }



    %
    % The distribution list for a reprint
    % No housekeeping copies are needed.
    \newenvironment{SANDreDistribution}{
	\ifthenelse{\boolean{SANDrePrintDateProvided}}   {
	    \setboolean{SANDdistributionProvided}{true}

	    \stepcounter{SANDreDistCnt}
	    \ifthenelse{\value{SANDreDistCnt} = 2}   {
		\sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxSecond}}
	    }{
		\ifthenelse{\value{SANDreDistCnt} = 3}   {
		    \sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxThird}}
		}{
		    \ifthenelse{\value{SANDreDistCnt} = 4}   {
			\sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxFourth}}
		    }{
			\ifthenelse{\value{SANDreDistCnt} = 5}   {
			    \sbox{\SANDrePrintDistBox}{\usebox{\SANDrePrintDistBoxFifth}}
			}{
			    \ClassError{SANDreport.cls}{\@backslashchar SANDreDistribution
				used more than four times!}{Do you really have more than
				five printings? If so, you need to amend SANDreport.cls}
			}
		    }
		}
	    }

	    \usebox{\SANDrePrintDistBox}
	    \normalsize
	    \raggedbottom
	    \begin{SANDdescription}
	}{
	}
    }{
	\ifthenelse{\boolean{SANDrePrintDateProvided}}   {
	    \end{SANDdescription}
	}{
	}
    }


    \newcommand{\SANDdistInternal}[4]{
	\ifthenelse{\boolean{SANDdistributionProvided}}   {
	}{
	    \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
		not (yet) provided}{\@backslashchar SANDdistribution must
		be used before any \@backslashchar SANDdistInternal}
	}
	\item[#1] \parbox[t]{\SANDdistwidth}{\makebox[\SANDMSdistwidth][l]{MS \hfill #2}%
		\hspace{2em} \raggedright #3, #4}
	\vspace{-1.5ex}
    }


    % Use a mail channel
    \newcommand{\SANDdistInternalM}[4]{
	\ifthenelse{\boolean{SANDdistributionProvided}}   {
	}{
	    \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
		not (yet) provided}{\@backslashchar SANDdistribution must
		be used before any \@backslashchar SANDdistInternal}
	}
	\item[#1] \parbox[t]{\SANDdistwidth}{\makebox[\SANDMSdistwidth][l]{#2}\hspace{2em} \raggedright #3, #4}
	\vspace{-1.5ex}
    }


    \newcommand{\SANDdistExternal}[2]{
	\ifthenelse{\boolean{SANDdistributionProvided}}   {
	}{
	    \ClassError{SANDreport.cls}{\@backslashchar SANDdistribution
		not (yet) provided}{\@backslashchar SANDdistribution must
		be used before any \@backslashchar SANDdistExternal}
	}
	\item[#1] \parbox[t]{\SANDdistwidth}{#2}
	\vspace{-1.5ex}
    }



    % ------------------------------------------------------------------------ %
    % And now some optional declarations
    %
    \newcommand{\SANDsupersed}[2]{
	\newsavebox{\SANDsupersedtempbox}
	\newcommand{\SANDsupersedVar}{
	    Supersedes #1 \\
	    Dated #2
	}
	\sbox{\SANDsupersedtempbox}{
	    \begin{tabular}{c}
		Supersedes #1 \\
		dated #2
	    \end{tabular}
	}
	\setboolean{SANDsupersedProvided}{true}
    }


    % Distribution category is no longer needed
    \newcommand{\SANDdistcategory}[1]{
	\ClassWarning{SANDreport.cls}{Distribution category is no longer needed!}
    }


% ******************************************************************************
% The commands to specify the report markings.
%

    % Printed on the cover and title page below the SANDIA REPORT number
%    \newcommand{\SANDreleaseTypeVar}{Unlimited Release} % default
    \newcommand{\SANDreleaseTypeVar}{Limited Release} % default

    % Printed on the cover page
    \newcommand{\MarkCoverVar}{Approved for public release; further dissemination unlimited.} % default
%    \newcommand{\MarkCoverVar}{Approved for limited release; further dissemination limited to distribution list.} % default

    % Printed on the title page
    \newsavebox{\MarkTitletempbox}
    \sbox{\MarkTitletempbox}{} % default

    % ----------------------------

    % Printed on top of cover page
    \newcommand{\MarkTopCoverVar}{} % default

    % Printed on bottom of cover page
    \newcommand{\MarkBotCoverVar}{} % default

    % Printed on top of back cover page
    \newcommand{\MarkTopBackVar}{} % default

    % Printed on bottom of back cover page
    \newcommand{\MarkBotBackVar}{} % default

    % Printed on top of title page
    \newcommand{\MarkTopTitleVar}{} % default

    % Printed on bottom of title page
    \newcommand{\MarkBotTitleVar}{} % default

    % Printed on bottom of all body pages
    \newcommand{\MarkBotBodyVar}{} % default

    % Printed on top of all body pages
    \newcommand{\MarkTopBodyVar}{} % default


    %
    % The user marking commands
    %
    \newcommand{\SANDreleaseType}[1]{
	\renewcommand{\SANDreleaseTypeVar}{#1}
    }

    \newcommand{\SANDmarkBottomCover}[1]{
	\renewcommand{\MarkBotCoverVar}{#1}
	\setboolean{MarkCover}{true}
	\setboolean{SANDmarkingsRequired}{true}
    }

    \newcommand{\SANDmarkTopBottom}[1]{
	\renewcommand{\MarkBotBodyVar}{#1}
	\renewcommand{\MarkTopBodyVar}{#1}
	\renewcommand{\MarkBotBackVar}{#1}
	\renewcommand{\MarkTopBackVar}{#1}
	\renewcommand{\MarkBotTitleVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\renewcommand{\MarkTopCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkBody}{true}
	\setboolean{MarkCover}{true}
	\setboolean{SANDmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }

    \newcommand{\SANDmarkTop}[1]{
	\renewcommand{\MarkTopBodyVar}{#1}
	\renewcommand{\MarkTopBackVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkTopCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkBody}{true}
	\setboolean{MarkCover}{true}
	\setboolean{SANDmarkingsRequired}{true}
    }

    \newcommand{\SANDmarkBottom}[1]{
	\renewcommand{\MarkBotBodyVar}{#1}
	\renewcommand{\MarkBotBackVar}{#1}
	\renewcommand{\MarkBotTitleVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkBody}{true}
	\setboolean{MarkCover}{true}
	\setboolean{SANDmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }

    \newcommand{\SANDmarkTopBottomCoverBackTitle}[1]{
	\renewcommand{\MarkTopCoverVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\renewcommand{\MarkTopBackVar}{#1}
	\renewcommand{\MarkBotBackVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkBotTitleVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkBack}{true}
	\setboolean{MarkCover}{true}
	\setboolean{SANDmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }

    \newcommand{\SANDmarkTopBottomCoverTitle}[1]{
	\renewcommand{\MarkBotTitleVar}{#1}
	\renewcommand{\MarkTopTitleVar}{#1}
	\renewcommand{\MarkBotCoverVar}{#1}
	\renewcommand{\MarkTopCoverVar}{#1}
	\setboolean{MarkTitle}{true}
	\setboolean{MarkCover}{true}
	\setboolean{SANDmarkingsRequired}{true}
	\setboolean{MarkBottom}{true}
    }


    %
    % Internal command used to activate markings depending on what part of
    % the document we are currently processing; e.g. \SANDmarks{body}
    %
    \newcommand{\SANDmarks}[1]{
	\ifthenelse{\boolean{SANDmarkingsRequired}}   {
	    \fancyfoot{} % Clear all foot markings
	    \fancyhead{} % Clear all head markings

	    % Fancyhdr needs a little room
	    \setlength{\headheight}{\baselineskip}

	    \pagestyle{fancy}
	    \thispagestyle{fancy}

	    \ifthenelse{\equal{#1}{cover}}   {
		% We're on the cover page
		\ifthenelse{\boolean{MarkCover}}   {
		    \chead{\MarkTopCoverVar}
		    \cfoot{\MarkBotCoverVar}
		}{
		    \chead{}
		    \cfoot{}
		}
	    }{
	    }

	    \ifthenelse{\equal{#1}{back}}   {
		% We're on the back cover page
		\ifthenelse{\boolean{MarkBack}}   {
		    \chead{\MarkTopBackVar}
		    \cfoot{\MarkBotBackVar}
		}{
		    \chead{}
		    \cfoot{}
		}

		% We do want a page number on the back cover
		\ifthenelse{\boolean{MarkBottom}}   {
		    \fancyfoot[LE,RO]{\thepage}
		}{% else
		    \fancyfoot[CE,CO]{\thepage}
		}
	    }{
	    }

	    \ifthenelse{\equal{#1}{title}}   {
		% We're on the title page
		\ifthenelse{\boolean{MarkTitle}}   {
		    \chead{\MarkTopTitleVar}
		    \cfoot{\MarkBotTitleVar}
		}{% else
		    \chead{}
		    \cfoot{}
		}

		\ifthenelse{\boolean{MarkBottom}}   {
		    \fancyfoot[LE,RO]{\thepage}
		}{% else
		    \fancyfoot[CE,CO]{\thepage}
		}
	    }{
	    }

	    \ifthenelse{\equal{#1}{body}}   {
		\ifthenelse{\boolean{MarkBody}}   {
		    \chead{\MarkTopBodyVar}
		    \cfoot{\MarkBotBodyVar}
		}{% else
		    \chead{}
		    \cfoot{}
		}

		% We're on a body page. If we're not going to mark the bottom,
		% or body pages at all, then we might as well put the page number
		% at the center
		\ifthenelse{\boolean{MarkBottom} \and \boolean{MarkBody}}   {
		    \fancyfoot[LE,RO]{\thepage}
		}{% else
		    \fancyfoot[CE,CO]{\thepage}
		}
	    }{
	    }
	}{
	    %
	    % If not SANDmarkingsRequired
	    %
	    \pagestyle{plain}% The default when we have no markings

	    \ifthenelse{\equal{#1}{cover}}   {
		% We're on the cover page

		% No page number on cover
		\thispagestyle{empty}
	    }{
	    }

	    \ifthenelse{\equal{#1}{back}}   {
		% We're on the back cover page

		% We do want a page number on the back cover
		\thispagestyle{plain}
	    }{
	    }

	}
    }  % end of \SANDmarks{}


    \newcommand{\SANDmarkCoverTitle}[2]{
	\ClassError{SANDreport.cls}{\@backslashchar SANDmarkCoverTitle doesn't work yet; sorry.}{Don't
	know where to put the text on the title page.}
	\sbox{\MarkTitletempbox}{{\bf #1\\}{#2}}
	\renewcommand{\MarkCoverVar}{{{\CoverFont{b}{12}{12pt}#1}\\ {\CoverFont{m}{8}{10pt}}{#2}}}
    }

    \newcommand{\SANDmarkCover}[1]{
	\renewcommand{\MarkCoverVar}{{\CoverFont{m}{8}{10pt}}{#1}}
    }


% ******************************************************************************
% Table of Contents
% The names we use to label the table of contents, and lists of figures,
% and tables.
%

    \ifthenelse{\boolean{reportSAND}}   {
	\renewcommand\bibname{References}
	\setcounter{tocdepth}{3}
    }{
	\renewcommand{\contentsname}{\raggedright \normalfont\Large\bfseries Contents}
	\renewcommand{\listfigurename}{\raggedright \bf Figures}
	\renewcommand\listtablename{\raggedright \bf Tables}
	\renewcommand\refname{\raggedright \bf \centering References}
	\setcounter{tocdepth}{2}
    }

    %
    % Don't enter figures, tables, and subsections of an appendix in
    % the main table of contents, if "strict" is used.
    % Code from James Gruetzner and Todd Pitts. Avoids the ifthen package in
    % \addcontentsline which interferes with the subfigure package.
    \renewcommand{\addcontentsline}[3]{
	\ifinAppendix%
	    % We are in the Appendix
	    \ifthenelse{\equal{#1}{toc}}   {
		% And we are attempting to add something to the TOC
		\ifreportSAND
		    \ifthenelse{\equal{#2}{chapter}}   {
			% It is a chapter, so we're OK
			\addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
		    }{% else
			% It's a section; we'll accept that too
			\ifthenelse{\equal{#2}{section}}   {
			    % It is a section, so we're OK
			    \addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
			}{% else
			    % It is not a section; skip it
			    \relax
			}
		    }
		\else
		    \ifthenelse{\equal{#2}{section}}   {
			% It is a section, so we're OK
			\addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
		    }{% else
			% It is not a section; skip it
			\relax
		    }
		\fi
	    }{
		% We're not in TOC. Probably a figure or table; skip it
		\relax
	    }

	\else%
	    \addtocontents{#1}{\protect\contentsline{#2}{#3}{\thepage}}
	\fi
    }



    % ------------------------------------------------------------------------ %
    % Modify the look of the table of contents if we are using the article style
    % ------------------------------------------------------------------------ %

    % We want the dots closer together, and the same as in \dotfill
    %
    \renewcommand{\@dotsep}{2.27}

    \ifthenelse{\boolean{reportSAND}}   {
    }{
	% For article.sty
	% We don't want blank lines between entries and no boldface.
	% This is the l@section definition from article.sty. We get rid of
	% \addvspace{1.0em... and \bfseries (if relax is not set in the main doc)
	\renewcommand*\l@section[2]{
	    \setlength\parskip{0ex}
	    \ifnum \c@tocdepth >\z@
	    \addpenalty\@secpenalty
	    \ifthenelse{\boolean{relaxedSAND}}   {
		\addvspace{1.0em \@plus\p@}
	    }{
	    }
	    \setlength\@tempdima{1.5em}
	    \begingroup
		\parindent \z@ \rightskip \@pnumwidth
		\parfillskip -\@pnumwidth
		\leavevmode
		\ifthenelse{\boolean{relaxedSAND}}   {
		    \bfseries
		}{
		}
		\advance\leftskip\@tempdima
		\hskip -\leftskip
		#1\nobreak\dotfill \nobreak\hb@xt@\@pnumwidth{\hss #2}\par
	    \endgroup
	\fi}

	\renewcommand*\l@figure{\setlength\parskip{0ex}
	    \@dottedtocline{1}{1.5em}{2.3em}
	}

	\let\l@table\l@figure
    }

    % ------------------------------------------------------------------------ %
    % If there is an appendix, set it apart in the table of contents
    %
    \renewcommand\appendix{
	\setboolean{inAppendix}{true}
	\par
	\setcounter{section}{0}
	\setcounter{subsection}{0}
	\setcounter{subsubsection}{0}
	\setcounter{figure}{0}
	\setcounter{table}{0}

	\ifthenelse{\boolean{reportSAND}}   {
	    \setcounter{chapter}{0}
	    \gdef\@chapapp{\appendixname}
	    \gdef\thechapter{\@Alph\c@chapter}
	    % Insert the Appendix name in the TOC
	    \addtocontents{toc}{\protect\subsection*{\appendixname}}
	}{
	    \renewcommand{\thefigure}{\thesection.\arabic{figure}}
	    \renewcommand{\thetable}{\thesection.\arabic{table}}
	    \gdef\thesection{\@Alph\c@section}
	    % Insert the Appendix name in the TOC
	    \addtocontents{toc}{\protect\subsection*{\appendixname}}
	}

	\ifthenelse{\boolean{strictSAND}}   {
	    \setcounter{secnumdepth}{2}
	}{
	    \setcounter{secnumdepth}{3}
	}
    }



% ******************************************************************************
% Sections & Chapters
% We want unnumbered section/chapter headings
%

    \ifthenelse{\boolean{reportSAND}}   {
	\ifthenelse{\boolean{strictSAND}}   {
	    % The samples in SAND98-0730 don't number sections.
	    \setcounter{secnumdepth}{0}
	}{
	    % The report SAND98-0730 itself does, though ;-)
	    \setcounter{secnumdepth}{3}
	}
    }{% else
	\ifthenelse{\boolean{strictSAND}}   {
	    % The samples in SAND98-0730 don't number sections.
	    \setcounter{secnumdepth}{0}
	}{
	    % The report SAND98-0730 itself does, though ;-)
	    \setcounter{secnumdepth}{3}
	}
    }


% ******************************************************************************
% Captions
% We want the label bold face, and the whole captionwidth about 4",
% and the text small
%

    \newlength{\SANDcaptionwidth}
    \setlength{\SANDcaptionwidth}{4in}
    \newsavebox{\SANDtempbox}
    \renewcommand{\@makecaption}[2]{
	\par\vspace{\abovecaptionskip}
	\sbox{\SANDtempbox}{{\bf #1. }\ #2}
	\ifthenelse{\lengthtest{\wd\SANDtempbox < \SANDcaptionwidth}}   {
	    \begin{center}
		{\bf #1. }\small #2
	    \end{center}
	}{
	    \begin{center}
		\parbox[t]{\SANDcaptionwidth}{{\bf #1. }\small #2}
	    \end{center}
	}
    }



% ******************************************************************************
% We have to import a lot of code from report.cls and article.cls so we can
% change a few things. We do report.cls first.
%

    \ifthenelse{\boolean{reportSAND}}   {
	% We need to redo \maketitle for the report class.
	% Omit the date and add the SAND number and so on.
	% We also need to redo the titlepage environment to not
	% issue a newpage command, so the abstract can go there
	% if it is short enough. (SAND reports are supposed to have
	% an abstract!)

	%
	% Renew the report.cls \maketitle macro
	%
	\if@titlepage							% \maketitle from report.cls
	    \renewcommand\maketitle{%					% \maketitle from report.cls
		\doCover{}
		\begin{titlepage}%					% \maketitle from report.cls
		    \SANDmarks{title}
		    {
			\begin{center}
			% use framebox instead of makebox for debugging
			\makebox[\topleftwidth][t]{
			    \begin{minipage}[c][\topheight][t]{\topleftwidth}
				% There is nothing at top left
			    \end{minipage}
			}
			\makebox[\topmiddlewidth][c]{
			    \begin{minipage}[c][\topheight][t]{\topmiddlewidth}
			    \begin{center}
				\SANDnumVar\\
				\SANDreleaseTypeVar\\
				Printed \SANDprintDateVar\\
				\ifthenelse{\boolean{SANDrePrintDateProvided}}   {
				    Reprinted \SANDrePrintDateVar\\
				}{
				}
				\ifthenelse{\boolean{SANDsupersedProvided}}   {
				    \bigskip
				    \usebox{\SANDsupersedtempbox}\\
				}{
				}
			    \end{center}
			    \end{minipage}
			}
			% Added topright box to balance topleft, and center topmiddle - MJK, 1/12/07
			\makebox[\toprightwidth][c]{
			    \begin{minipage}[c][\topheight][t]{\toprightwidth}
			    % There is nothing at top right
			    \end{minipage}
			}
			\end{center}
		    }
		    \let\footnotesize\small				% \maketitle from report.cls
		    \let\footnoterule\relax				% \maketitle from report.cls
		    \let \footnote \thanks				% \maketitle from report.cls
		    % \null\vfil					% deleted
		    % \vskip 60\p@					% deleted
		    \begin{center}%					% \maketitle from report.cls
			{\LARGE \@title \par}%				% \maketitle from report.cls
			\vskip 3em%					% \maketitle from report.cls
			{\large						% \maketitle from report.cls
			\lineskip .75em%				% \maketitle from report.cls
			\begin{tabular}[t]{c}%				% \maketitle from report.cls
			    \@author					% \maketitle from report.cls
			\end{tabular}\par}%				% \maketitle from report.cls
			\vskip 1.5em%					% \maketitle from report.cls
			% {\large \@date \par}%				% deleted
		    \end{center}\par					% \maketitle from report.cls
		    \@thanks						% \maketitle from report.cls
		    \vfil\null						% \maketitle from report.cls
% added by rcm
            \begin{center}
	        \CoverFont{b}{12}{12pt}
		Prepared for the \\
                U.S. Department of Energy\\
                Office of Nuclear Energy\\
                Under DOE Idaho Operations Office\\
                Contract DE-AC07-05ID14517
		\end{center}

		\end{titlepage}%					% \maketitle from report.cls
		\setcounter{footnote}{0}%				% \maketitle from report.cls
		\global\let\thanks\relax				% \maketitle from report.cls
		\global\let\maketitle\relax				% \maketitle from report.cls
		\global\let\@thanks\@empty				% \maketitle from report.cls
		\global\let\@author\@empty				% \maketitle from report.cls
		\global\let\@date\@empty				% \maketitle from report.cls
		\global\let\@title\@empty				% \maketitle from report.cls
		\global\let\title\relax					% \maketitle from report.cls
		\global\let\author\relax				% \maketitle from report.cls
		\global\let\date\relax					% \maketitle from report.cls
		\global\let\and\relax					% \maketitle from report.cls
	    }
	\else
	    % {---------------------- I doubt we ever get here ------------------------}
	    \ClassError{SANDreport.cls}{\@backslashchar maketitle definition outside
		a title page.}{The SANDreport class cannot handle this case.}
	\fi

	%
	% \titlepage in the report class resets the page
	% number, which is not correct for SANDIA Reports.
	% It also sets the page style to empty.
	\if@compatibility						% titlepage from report.cls
	\renewenvironment{titlepage}					% titlepage from report.cls
	    {%								% titlepage from report.cls
	      \if@twocolumn						% titlepage from report.cls
		\@restonecoltrue\onecolumn				% titlepage from report.cls
	      \else							% titlepage from report.cls
		\@restonecolfalse\newpage				% titlepage from report.cls
	      \fi							% titlepage from report.cls
	      %\thispagestyle{empty}%					% deleted
	      %\setcounter{page}\z@					% deleted
	    }%								% titlepage from report.cls
	    %{\if@restonecol\twocolumn \else \newpage \fi		% deleted, we want abstract on this page
	    {\if@restonecol\twocolumn \fi				% new version
	    }								% titlepage from report.cls
	\else								% titlepage from report.cls
	\renewenvironment{titlepage}					% titlepage from report.cls
	    {%								% titlepage from report.cls
	      \if@twocolumn						% titlepage from report.cls
		\@restonecoltrue\onecolumn				% titlepage from report.cls
	      \else							% titlepage from report.cls
		\@restonecolfalse\newpage				% titlepage from report.cls
	      \fi							% titlepage from report.cls
	      %\thispagestyle{empty}%					% deleted
	      %\setcounter{page}\@ne					% deleted
	    }%								% titlepage from report.cls
	    %{\if@restonecol\twocolumn \else \newpage \fi		% deleted, we want abstract on this page
	    {\if@restonecol\twocolumn \fi				% new version
	     \if@twoside\else						% titlepage from report.cls
		%\setcounter{page}\@ne					% deleted
	     \fi							% titlepage from report.cls
	    }								% titlepage from report.cls
	\fi								% titlepage from report.cls

	\if@titlepage							% abstract from report.cls
	    \renewenvironment{abstract}{%				% abstract from report.cls
		% \titlepage						% deleted
		\null\vfil						% abstract from report.cls
		\@beginparpenalty\@lowpenalty				% abstract from report.cls
		\begin{center}%						% abstract from report.cls
		    \bfseries \abstractname				% abstract from report.cls
		    \@endparpenalty\@M					% abstract from report.cls
		\end{center}}%						% abstract from report.cls
		% {\par\vfil\null\endtitlepage}				% deleted
		{\par\vfil\null}					% new version
	\else								% abstract from report.cls
	    % {---------------------- We should never get here ------------------------}
	    \ClassError{SANDreport.cls}{Abstract definition outside
		a title page.}{The SANDreport class cannot handle this case.}
	\fi								% abstract from report.cls

    }{
	%
	% Now for the changes to article.cls functions
	%


	%
	% In \@maketitle, we omit the \@date and insert the SAND number and so on
	%
	\renewcommand{\@maketitle}{
	    \doCover{}
	    \newpage							% \@maketitle from article.cls
	    {								% new
		\begin{center}						% new
		% use framebox instead of makebox for debugging		% new
		\makebox[\topleftwidth][t]{				% new
		    \begin{minipage}[c][\topheight][t]{\topleftwidth}	% new
			% There is nothing at top left			% new
		    \end{minipage}					% new
		}							% new
		\makebox[\topmiddlewidth][c]{				% new
		    \begin{minipage}[c][\topheight][t]{\topmiddlewidth}	% new
		    \begin{center}					% new
			\SANDnumVar\\					% new
			\SANDreleaseTypeVar\\				% new
			Printed \SANDprintDateVar\\			% new
			\ifthenelse{\boolean{SANDrePrintDateProvided}}   {	% new
			    Reprinted \SANDrePrintDateVar\\		% new
			}{						% new
			}						% new
			\ifthenelse{\boolean{SANDsupersedProvided}}   {	% new
			    \bigskip					% new
			    \usebox{\SANDsupersedtempbox}\\		% new
			}{						% new
			}						% new
		    \end{center}					% new
		    \end{minipage}					% new
		}							% new
		% Add topright box to balance topleft to center topmiddle - MJK, 1/12/07
		\makebox[\toprightwidth][c]{				% new
		    \begin{minipage}[c][\topheight][t]{\toprightwidth}	% new
			% There is nothing at top right			% new
		    \end{minipage}					% new
		}							% new
		\end{center}						% new
	    }								% new

	    \begin{center}						% \@maketitle from article.cls
		\let \footnote \thanks					% \@maketitle from article.cls
		{\LARGE \bf \@title\par}				% \@maketitle from article.cls
		\vskip 1.5em						% \@maketitle from article.cls
		{\large							% \@maketitle from article.cls
		    \lineskip .5em%					% \@maketitle from article.cls
		    \begin{tabular}[t]{c}				% \@maketitle from article.cls
		    \@author						% \@maketitle from article.cls
		    \end{tabular}\par					% \@maketitle from article.cls
		}							% \@maketitle from article.cls
	    \end{center}						% \@maketitle from article.cls
	    \par							% \@maketitle from article.cls
	    \vskip 1.5em						% \@maketitle from article.cls
	}


	%
	% For fancy headers, we have to redefine \maketitle. All we are doing,
	% is to activate the headers again, and then redefine \newpage so it
	% happens for every body page.
	%
	\renewcommand{\maketitle}{\par
	    \begingroup							% \maketitle from article.cls
	    \renewcommand\thefootnote{\@fnsymbol\c@footnote}%		% \maketitle from article.cls
	    \def\@makefnmark{						% \maketitle from article.cls
		\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%	% \maketitle from article.cls
	    \long\def\@makefntext##1{\parindent 1em\noindent		% \maketitle from article.cls
		\hb@xt@1.8em{%						% \maketitle from article.cls
		\hss\@textsuperscript{\normalfont\@thefnmark}}##1}%	% \maketitle from article.cls
	    \if@twocolumn						% \maketitle from article.cls
		\ifnum \col@number=\@ne					% \maketitle from article.cls
		    \@maketitle						% \maketitle from article.cls
		\else							% \maketitle from article.cls
		    \twocolumn[\@maketitle]%				% \maketitle from article.cls
		\fi							% \maketitle from article.cls
	    \else							% \maketitle from article.cls
		\newpage						% \maketitle from article.cls
		% Prevents figures from going at top of page.		% \maketitle from article.cls
		\global\@topnum\z@					% \maketitle from article.cls
		\@maketitle						% \maketitle from article.cls
	    \fi								% \maketitle from article.cls
	    \SANDmarks{title}						% new
	    \@thanks							% \maketitle from article.cls
	    \endgroup							% \maketitle from article.cls
	    \setcounter{footnote}{0}%					% \maketitle from article.cls
	    \global\let\thanks\relax					% \maketitle from article.cls
	    \global\let\maketitle\relax					% \maketitle from article.cls
	    \global\let\@maketitle\relax				% \maketitle from article.cls
	    \global\let\@thanks\@empty					% \maketitle from article.cls
	    \global\let\@author\@empty					% \maketitle from article.cls
	    \global\let\@date\@empty					% \maketitle from article.cls
	    \global\let\@title\@empty					% \maketitle from article.cls
	    \global\let\title\relax					% \maketitle from article.cls
	    \global\let\author\relax					% \maketitle from article.cls
	    \global\let\date\relax					% \maketitle from article.cls
	    \global\let\and\relax					% \maketitle from article.cls


	    %
	    % Now we renew \newpage so that from here on the markings
	    % of the body are used.
	    %
	    \renewcommand{\newpage}{%				% \newpage from latex.ltx
		\if@noskipsec					% \newpage from latex.ltx
		    \ifx \@nodocument\relax			% \newpage from latex.ltx
			\leavevmode				% \newpage from latex.ltx
			\global \@noskipsecfalse		% \newpage from latex.ltx
		    \fi						% \newpage from latex.ltx
		\fi						% \newpage from latex.ltx
		\if@inlabel					% \newpage from latex.ltx
		    \leavevmode					% \newpage from latex.ltx
		    \global \@inlabelfalse			% \newpage from latex.ltx
		\fi						% \newpage from latex.ltx
		\if@nobreak \@nobreakfalse \everypar{}\fi	% \newpage from latex.ltx
		\par						% \newpage from latex.ltx
		\vfil						% \newpage from latex.ltx
		\penalty -\@M					% \newpage from latex.ltx
		\SANDmarks{body}				% new
	    }							% \newpage from latex.ltx
	}								% \maketitle from article.sty
    }



% ******************************************************************************
% Do some checking at the end of the document
%

    \AtEndDocument{
	% Do some more usage checking
	\ifthenelse{\boolean{SANDmainProvided}}   {
	}{
	    \ClassError{SANDreport.cls}{\@backslashchar SANDmain
		not provided}{Add \@backslashchar SANDmain before your
		introduction (first) section of your document}
	}


	%
	% Put the Sandia Logo on the back page
	%

	\cleardoublepage	% We're on the inside end page now
	\mbox{}			% Some invisible text
	\vfill
	\hfill {\CoverFont{m}{6}{6pt} \SANDreportVersionOnly}
	\setboolean{SANDmarkingsRequired}{false}
	\thispagestyle{empty}
	\pagestyle{empty}
	\newpage		% Now we're on the outside of the end page

	\vspace*{\fill}		% Move the logo to the bottom of the page
	\thispagestyle{empty}
	\begin{center}
	    \includegraphics{inl-left_black}
	\end{center}

    } % End of AtEndDocument{}
