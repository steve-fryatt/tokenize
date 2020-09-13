Tokenize
========

Turn ASCII Files into Tokenized ARM BASIC on any platform.


Introduction
------------

Tokenize is a cross-platform tokeniser for ARM variants of BBC BASIC. It will take ASCII text files and convert them into tokenized BASIC (of the ARM BASIC V variety) at the command line. It is written in C, and can be compiled to run on platforms other than RISC OS.

In addition to tokenizing files, Tokenize can link multiple library files into a single output -- both by passing multiple source files on the command line and by processing (and removing) `LIBRARY` statements found in the source.

Finally, Tokenize can perform the actions of the BASIC `CRUNCH` command on the tokenized output, substitute "constant" variables with values passed in on the command-line, and convert textual `SYS` names into their numeric equivalents.

Be aware that Tokenize is **not** a BASIC syntax checker. While it will check some aspects of syntax that it requires to do its job, it will not verify that a program is completely syntactically correct and that it will load into the BASIC Interpreter and execute as intended.


Installation
------------

To install and use Tokenize, it will be necessary to have suitable Linux system with a working installation of the [GCCSDK](http://www.riscos.info/index.php/GCCSDK).

It will also be necessary to ensure that the `SFTOOLS_BIN` and `$SFTOOLS_MAKE` variables are set to a suitable location within the current environment. For example

	export SFTOOLS_BIN=/home/steve/sftools/bin
	export SFTOOLS_MAKE=/home/steve/sftools/make

where the path is changed to suit your local settings and installation requirements. Finally, you will also need to have installed the Shared Makefiles, ManTools and PackTools.

To install Tokenize, use

	make install

from the root folder of the project, which will copy the necessary files in to the location indicated by `$SFTOOLS_BIN`.

A ReadMe for Tokenize will be generated in the buildlinux folder.


Building for native use
-----------------------

To build Tokenize for use natively on RISC OS, you can use

	make TARGET=riscos

and the resulting files (executable and ReadMe) will be generated in the buildro folder. To create a Zip archive of the release, use

	make release TARGET=riscos

and a Zip file will appear in the parent folder to the location of the project itself.


Licence
-------

Tokenize is licensed under the EUPL, Version 1.2 only (the "Licence"); you may not use this work except in compliance with the Licence.

You may obtain a copy of the Licence at <http://joinup.ec.europa.eu/software/page/eupl>.

Unless required by applicable law or agreed to in writing, software distributed under the Licence is distributed on an "**as is**"; basis, **without warranties or conditions of any kind**, either express or implied.

See the Licence for the specific language governing permissions and limitations under the Licence.