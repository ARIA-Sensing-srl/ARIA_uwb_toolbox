## Makefile to simplify Octave Forge package maintenance tasks
##
## Copyright 2015-2016 Carnë Draug
## Copyright 2015-2016 Oliver Heimlich
## Copyright 2015-2019 Mike Miller
##
## Copying and distribution of this file, with or without modification,
## are permitted in any medium without royalty provided the copyright
## notice and this notice are preserved.  This file is offered as-is,
## without any warranty.

MKOCTFILE ?= mkoctfile
OCTAVE    ?= octave
SED       := sed
SHA256SUM := sha256sum
TAR       := tar
MAKEINFO  ?= makeinfo
MAKEINFO_HTML_OPTIONS := --no-headers --set-customization-variable 'COPIABLE_LINKS 0' --set-customization-variable 'COPIABLE_ANCHORS 0' --no-split 

PACKAGE := $(shell $(SED) -n -e 's/^Name: *\(\w\+\)/\1/p' DESCRIPTION)
VERSION := $(shell $(SED) -n -e 's/^Version: *\(\w\+\)/\1/p' DESCRIPTION)
DATE := $(shell $(SED) -n -e 's/^Date: *\(\w\+\)/\1/p' DESCRIPTION)
DEPENDS := $(shell $(SED) -n -e 's/^Depends[^,]*, *\(.*\)/\1/p' DESCRIPTION | $(SED) 's/ *([^()]*)//g; s/ *, */ /g')

BASEDIR ?= $(realpath $(CURDIR))

TAR_OPTIONS  := --format=ustar $(TAR_REPRODUCIBLE_OPTIONS)

RELEASE_DIR     := $(PACKAGE)-$(VERSION)
RELEASE_TARBALL := $(PACKAGE)-$(VERSION).tar.gz

.PHONY: help dist release install all check run clean maintainer-clean

help:
        @echo "Targets:"
	@echo "   dist             - Create $(RELEASE_TARBALL) for release"
	@echo "   release          - Create both of the above and show md5sums"
	@echo
	@echo "   install          - Install the package in GNU Octave"
	@echo "   all              - Build all oct files"
	@echo "   check            - Execute package tests (w/o install)"
	@echo "   run              - Run Octave with development in PATH (no install)"
	@echo
	@echo "   clean            - Remove releases, html documentation, and oct files"
	@echo "   maintainer-clean - Additionally remove all generated files"

$(RELEASE_DIR): 
	@echo "Creating package version $(VERSION) release ..."
	-rm -rf $@
	$(MAKE) BASEDIR=$(BASEDIR) -C $@ build-docs
	cd "$@" && $(RM) -f Makefile 
	# doc/mkfuncdocs.py doc/mkqhcp.py
	chmod -R a+rX,u+w,go-w $@

$(RELEASE_TARBALL): $(RELEASE_DIR)
	$(TAR) -cf - $(TAR_OPTIONS) $< | gzip -9n > $@
	-rm -rf $<

dist: $(RELEASE_TARBALL)

install: $(RELEASE_TARBALL)
	@echo "Installing package locally ..."
	$(OCTAVE) --silent --eval 'pkg install $(RELEASE_TARBALL);'

all:
	cd src && $(MAKE) $@
	cd src && $(MAKE) PKG_ADD PKG_DEL

# using __run_test_suite__ as is available in octave 3.8+
check: all
	$(OCTAVE) --silent \
	  --eval 'if (! isempty ("$(DEPENDS)")); pkg load $(DEPENDS); endif;' \
	  --eval 'addpath (fullfile (pwd, "inst"));' \
	  --eval 'addpath (fullfile (pwd, "src"));' \
	  --eval 'if exist("oruntests") == 2, runtestsfunc=@oruntests;, else runtestsfunc=@runtests;, endif;' \
	  --eval 'runtestsfunc ("inst"); runtestsfunc ("src");'

run: all
	$(OCTAVE) --silent --persist \
	  --eval 'if (! isempty ("$(DEPENDS)")); pkg load $(DEPENDS); endif;' \
	  --eval 'addpath (fullfile (pwd, "inst"));' \
	  --eval 'addpath (fullfile (pwd, "src"));'
	  
clean: all 
	cd src && $(MAKE) $@


maintainer-clean: clean
