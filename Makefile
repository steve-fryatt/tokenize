# Copyright 2014, Stephen Fryatt (info@stevefryatt.org.uk)
#
# This file is part of Tokenize:
#
#   http://www.stevefryatt.org.uk/software/
#
# Licensed under the EUPL, Version 1.1 only (the "Licence");
# You may not use this work except in compliance with the
# Licence.
#
# You may obtain a copy of the Licence at:
#
#   http://joinup.ec.europa.eu/software/page/eupl
#
# Unless required by applicable law or agreed to in
# writing, software distributed under the Licence is
# distributed on an "AS IS" basis, WITHOUT WARRANTIES
# OR CONDITIONS OF ANY KIND, either express or implied.
#
# See the Licence for the specific language governing
# permissions and limitations under the Licence.

# This file really needs to be run by GNUMake.
# It is intended for native compilation on Linux (for use in a GCCSDK
# environment) or cross-compilation under the GCCSDK.

.PHONY: all clean documentation release install


# The build date.

BUILD_DATE := $(shell date "+%d %b %Y")
HELP_DATE := $(shell date "+%-d %B %Y")

# Construct version or revision information.

ifeq ($(VERSION),)
  RELEASE := $(shell svnversion --no-newline)
  VERSION := r$(RELEASE)
  RELEASE := $(subst :,-,$(RELEASE))
  HELP_VERSION := ----
else
  RELEASE := $(subst .,,$(VERSION))
  HELP_VERSION := $(VERSION)
endif

$(info Building for $(TARGET) with version $(VERSION) ($(RELEASE)) on date $(BUILD_DATE))

# The archive to assemble the release files in.  If $(RELEASE) is set, then the file can be given
# a standard version number suffix.

ifeq ($(TARGET),riscos)
  ZIPFILE := tokenize$(RELEASE)ro.zip
else
  ZIPFILE := tokenize$(RELEASE)linux.zip
endif
SRCZIPFILE := tokenize$(RELEASE)src.zip
BUZIPFILE := tokenize$(shell date "+%Y%m%d").zip


# Build Tools

ifeq ($(TARGET),riscos)
  CC := $(wildcard $(GCCSDK_INSTALL_CROSSBIN)/*gcc)
else
  CC := gcc
endif

MKDIR := mkdir
RM := rm -rf
CP := cp

ZIP := /home/steve/GCCSDK/env/bin/zip

SFBIN := /home/steve/GCCSDK/sfbin

MANTOOLS := $(SFBIN)/mantools
BINDHELP := $(SFBIN)/bindhelp
TEXTMERGE := $(SFBIN)/textmerge
MENUGEN := $(SFBIN)/menugen


# Build Flags

ifeq ($(TARGET),riscos)
  CCFLAGS := -mlibscl -mhard-float -static -mthrowback -Wall -O2 -D'BUILD_VERSION="$(VERSION)"' -D'BUILD_DATE="$(BUILD_DATE)"' -fno-strict-aliasing -mpoke-function-name
  ZIPFLAGS := -x "*/.svn/*" -r -, -9
else
  CCFLAGS := -Wall -O2 -fno-strict-aliasing -D'BUILD_VERSION="$(VERSION)"' -D'BUILD_DATE="$(BUILD_DATE)"'
  ZIPFLAGS := -x "*/.svn/*" -r -9
endif
SRCZIPFLAGS := -x "*/.svn/*" -r -9
BUZIPFLAGS := -x "*/.svn/*" -r -9
BINDHELPFLAGS := -f -r -v
MENUGENFLAGS := -d


# Includes and libraries.

INCLUDES := -I$(GCCSDK_INSTALL_ENV)/include
LINKS := -L$(GCCSDK_INSTALL_ENV)/lib


# Set up the various build directories.

SRCDIR := src
MANUAL := manual
OBJDIR := obj
OUTDIRLINUX := buildlinux
OUTDIRRO:= buildro
ifeq ($(TARGET),riscos)
  OUTDIR := $(OUTDIRRO)
else
  OUTDIR := $(OUTDIRLINUX)
endif



# Set up the named target files.

ifeq ($(TARGET),riscos)
  RUNIMAGE := tokenize,ff8
  README := ReadMe,fff
  LICENSE := Licence,fff
else
  RUNIMAGE := tokenize
  README := ReadMe.txt
  LICENSE := Licence.txt
endif


# Set up the source files.

MANSRC := Source
MANSPR := ManSprite

OBJS := tokenize.o

#data.o menugen.o parse.o stack.o


# Build everything, but don't package it for release.

all: documentation $(OUTDIR)/$(RUNIMAGE)


# Build the complete !RunImage from the object files.

OBJS := $(addprefix $(OBJDIR)/, $(OBJS))

$(OUTDIR)/$(RUNIMAGE): $(OBJS) $(OBJDIR)
	$(CC) $(CCFLAGS) $(LINKS) -o $(OUTDIR)/$(RUNIMAGE) $(OBJS)

# Create a folder to hold the object files.

$(OBJDIR):
	$(MKDIR) $(OBJDIR)

# Build the object files, and identify their dependencies.

-include $(OBJS:.o=.d)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c $(CCFLAGS) $(INCLUDES) $< -o $@
	@$(CC) -MM $(CCFLAGS) $(INCLUDES) $< > $(@:.o=.d)
	@mv -f $(@:.o=.d) $(@:.o=.d).tmp
	@sed -e 's|.*:|$@:|' < $(@:.o=.d).tmp > $(@:.o=.d)
	@sed -e 's/.*://' -e 's/\\$$//' < $(@:.o=.d).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(@:.o=.d)
	@rm -f $(@:.o=.d).tmp


# Build the documentation

documentation: $(OUTDIR)/$(README)

$(OUTDIR)/$(README): $(MANUAL)/$(MANSRC)
	$(MANTOOLS) -MTEXT -I$(MANUAL)/$(MANSRC) -O$(OUTDIR)/$(README) -D'version=$(HELP_VERSION)' -D'date=$(HELP_DATE)'


# Build the release Zip file.

release: clean all
	$(RM) ../$(ZIPFILE)
	(cd $(OUTDIR) ; $(ZIP) $(ZIPFLAGS) ../../$(ZIPFILE) $(RUNIMAGE) $(README) $(LICENSE))
	$(RM) ../$(SRCZIPFILE)
	$(ZIP) $(SRCZIPFLAGS) ../$(SRCZIPFILE) $(OUTDIRLINUX) $(OUTDIRRO) $(SRCDIR) $(MANUAL) Makefile


# Build a backup Zip file

backup:
	$(RM) ../$(BUZIPFILE)
	$(ZIP) $(BUZIPFLAGS) ../$(BUZIPFILE) *


# Install the finished version in the GCCSDK, ready for use.

install: clean all
	$(CP) -r $(OUTDIR)/$(RUMIMAGE) $(SFBIN)


# Clean targets

clean:
	$(RM) $(OBJDIR)/*
	$(RM) $(OUTDIR)/$(RUNIMAGE)
	$(RM) $(OUTDIR)/$(README)

