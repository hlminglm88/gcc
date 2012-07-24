[+ AutoGen5 template -*- Mode: Shell-script -*-
sh
+][+COMMENT use 'autogen --trace=everything melt-build-script.def' to debug this
  See http://www.gnu.org/software/autogen/
+]#!/bin/bash
[+ (. (dne "#@#@# " "#@! ")) +]
# Generated shell script for MELT modules and MELT translator bootstrap
#   Copyright (C) 2012  Free Software Foundation
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
#@ [+ (. (tpl-file-line))+] generated by Autogen [+ (. autogen-version)+] using [+ (.(def-file))+]

[+(. (define comefromcount 0))+]
[+(. (define (fromline) (set! comefromcount (+ comefromcount 1)) 
	(sprintf "%s/%d" (tpl-file-line "%s:%d") comefromcount)
	))+]

## source the parameters  [+(.(fromline))+]
. ./melt-build-param.sh

export GAWK=${GAWK:=gawk}
export MD5SUM=${MD5SUM:=md5sum}

## internal variables for this script  [+(.(fromline))+]
## GCCMELT_STAGE is an internal variable; it keeps the current MELT stage
export GCCMELT_STAGE=""
## GCCMELT_BASE is an internal variable; it keeps the current MELT base
export GCCMELT_BASE=""
## GCCMELT_SKIPEMITC is an internal variable; it skips the emission of C code when non-empty
export GCCMELT_SKIPEMITC=""



date +"/*empty file for MELT build %c*/" > meltbuild-empty-file.c

[ -d meltbuild-workdir ] || mkdir  meltbuild-workdir

## our error function  [+(.(fromline))+]
function meltbuild_error () {
    echo MELT BUILD SCRIPT FAILURE: $@ > /dev/stderr
    exit 1
}

## symbolic linking
function meltbuild_symlink () {
    ln -svf `realpath $1` $2
}

## our info function
function meltbuild_info () {
    echo MELT BUILD SCRIPT INFO: $@ > /dev/stderr
}

## utility to build MELT specific arguments in meltbuild_emit
function meltbuild_arg () {
    local meltarg=$1
    if [ -z "$GCCMELT_IS_PLUGIN" ]; then
	echo " -fmelt-$meltarg"
    else
	echo " -fplugin-arg-melt-$meltarg"
    fi
}


################################################################
## function to run MELT to emit C code
function meltbuild_emit () {
    local meltfrom=$1
    local meltmode=$2
    local meltbase=$3
    local meltstage=$4
    local meltprevstage=$5
    local meltinit=$6
    local meltinclude=$7
    local meltargs=$meltstage/$meltbase.args
    local meltsrc=$(realpath $GCCMELT_MELTSOURCEDIR/$meltbase.melt)
    meltbuild_info meltfrom=$meltfrom meltmode=$meltmode meltbase=$meltbase meltstage=$meltstage meltprevstage=$meltprevstage meltinit=$meltinit meltinclude=$meltinclude meltsrc=$meltsrc
    local meltsum
    local meltprevf
    if [ -z "$meltmode" ]; then
	meltbuild_error $meltfrom no MELT mode at stage $meltstage
    fi
    if [ ! -f "$meltsrc" ]; then
	meltbuild_error $meltfrom no MELT file $meltsrc at stage $meltstage base $meltbase
    fi
    if [ -z "$meltinit" ]; then
	meltbuild_error $meltfrom no MELT init  at stage $meltstage base $meltbase
    fi
    meltsum=$($MD5SUM $meltsrc | $GAWK '{print $1}')
    meltbuild_info $meltfrom emit C code for $meltbase of $meltstage
    echo -Wno-shadow -frandom-seed=$meltsum > $meltargs-$$-tmp
    ## various arguments
    echo " -DGCCMELT_FROM_ARG=\"$meltfrom\"" >>  $meltargs-$$-tmp
    meltbuild_arg mode=$meltmode >> $meltargs-$$-tmp
    meltbuild_arg arg=$meltsrc >>  $meltargs-$$-tmp
    meltbuild_arg output=$meltstage/$meltbase  >>  $meltargs-$$-tmp
    meltbuild_arg "module-make-command='$GCCMELT_MAKE'" >> $meltargs-$$-tmp
    meltbuild_arg module-makefile=$GCCMELT_MODULE_MK >> $meltargs-$$-tmp
    meltbuild_arg "module-cflags='$GCCMELT_INCLUDES -I. -I$meltstage -I$meltprevstage $GCCMELT_COMPILER_FLAGS'" >>  $meltargs-$$-tmp
    meltbuild_arg init=$meltinit >> $meltargs-$$-tmp
    meltbuild_arg workdir=meltbuild-workdir >> $meltargs-$$-tmp
    meltbuild_arg tempdir=. >> $meltargs-$$-tmp
    meltbuild_arg source-path=$meltstage:$meltprevstage:. >> $meltargs-$$-tmp
    meltbuild_arg module-path=$meltstage:$meltprevstage:. >> $meltargs-$$-tmp
    meltbuild_arg bootstrapping  >> $meltargs-$$-tmp
    meltbuild_arg generate-work-link  >> $meltargs-$$-tmp
    meltbuild_arg generated-c-file-list=$meltstage/$meltbase.cfilist  >> $meltargs-$$-tmp
    ## final empty file
    echo meltbuild-empty-file.c >>  $meltargs-$$-tmp
    mv $meltargs-$$-tmp $meltargs
    meltbuild_info $meltfrom argument file $meltargs is
    cat  $meltargs > /dev/stderr
    if [ -z "$GCCMELT_SKIPEMITC" ]; then
	$GCCMELT_CC1_PREFIX $GCCMELT_CC1 @$meltargs || meltbuild_error $meltfrom failed with arguments @$meltargs
    else
	meltbuild_info $meltfrom skips emission of C code with  @$meltargs stage $meltstage prevstage $meltprevstage skipreason $GCCMELT_SKIPEMITC
	ls -l $meltprevstage/$meltbase*
	for meltprevf in $meltprevstage/$meltbase.c  $meltprevstage/$meltbase+[0-9][0-9].c  $meltprevstage/$meltbase+meltdesc.c  $meltprevstage/$meltbase+melttime.h   $meltprevstage/$meltbase+meltbuild.mk ; do
	    meltbuild_symlink $meltprevf $meltstage
	done
	meltbuild_info $meltfrom symlinked previous stage $meltprevstage/$meltbase
    fi
    GCCMELT_STAGE=$meltstage
    GCCMELT_BASE=$meltbase
} ################################ end function meltbuild_emit


################ stage zero

GCCMELT_ZERO_FLAVOR=${GCCMELT_STAGE_ZERO#meltbuild-stage0-}

## The base name of the MELT translator files [+ (. (fromline))+]
GCCMELT_TRANSLATOR_BASE=([+FOR melt_translator_file " \\\n"+]  [+base+][+ENDFOR melt_translator_file+] )

case $GCCMELT_ZERO_FLAVOR in
    optimized) ;;
    dynamic) ;;
    debugnoline) ;;
    quicklybuilt) ;;
    *) meltbuild_error  [+(.(fromline))+] bad zero flavor $GCCMELT_ZERO_FLAVOR ;;
esac


## our stage0 [+(.(fromline))+]

[ -d $GCCMELT_STAGE_ZERO ] || mkdir  $GCCMELT_STAGE_ZERO


[+FOR melt_translator_file+]
meltbuild_info making stage0 [+base+]  [+(.(fromline))+]

##  stage0 [+(.(fromline))+] symlink descriptor file [+base+]
if [ ! -f "$GCCMELT_STAGE_ZERO/[+base+]+meltdesc.c" ]; then
    meltbuild_symlink $GCCMELT_MELTSOURCEDIR/generated/[+base+]+meltdesc.c $GCCMELT_STAGE_ZERO/ 
fi

##  stage0 [+(.(fromline))+] symlink source code [+base+]
if [ ! -f "$GCCMELT_STAGE_ZERO/[+base+].c" ]; then
    meltbuild_symlink $GCCMELT_MELTSOURCEDIR/generated/[+base+].c $GCCMELT_STAGE_ZERO/ 
    for f in $GCCMELT_MELTSOURCEDIR/generated/[+base+]+[0-9]*.c ; do
	meltbuild_symlink $f $GCCMELT_STAGE_ZERO/ 
    done
fi

##  stage0 [+(.(fromline))+] symlink stamp [+base+]
if  [ ! -f "$GCCMELT_STAGE_ZERO/[+base+]+melttime.h" ]; then
    meltbuild_symlink $GCCMELT_MELTSOURCEDIR/generated/[+base+]+melttime.h $GCCMELT_STAGE_ZERO/ 
fi

MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5=$($GAWK -F\" '/extern/{next} /melt_cumulated_hexmd5/{print $2}' $GCCMELT_MELTSOURCEDIR/generated/[+base+]+meltdesc.c)


## manually generate the stage0 [+base+]+meltbuild.mk file  [+(.(fromline))+]
MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK=$GCCMELT_STAGE_ZERO/[+base+]+meltbuild.mk

date +"# file $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK script-generated %c" > $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo "# generated " [+(.(fromline))+] >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo "MELTGEN_MODULENAME=$GCCMELT_STAGE_ZERO/[+base+]"  >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo "MELTGEN_MODULEIDENT=melt_stage_zero_[+varsuf+]"  >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$

echo '# zerostage objects of [+base+] [+(.(fromline))+]' >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo $GCCMELT_STAGE_ZERO/[+base+].$MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5.descriptor.meltpic.o: $GCCMELT_STAGE_ZERO/[+base+]+meltdesc.c  >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
for f in $GCCMELT_STAGE_ZERO/[+base+].c $GCCMELT_STAGE_ZERO/[+base+]+[0-9]*.c; do
 echo >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
 echo $GCCMELT_STAGE_ZERO/`basename $f .c`._NOMDFIVESUM_.$GCCMELT_ZERO_FLAVOR.meltpic.o: $f >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
done

echo >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo '# zerostage module of [+base+] [+(.(fromline))+]'  >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo $GCCMELT_STAGE_ZERO/[+base+].meltmod-$MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5.$GCCMELT_ZERO_FLAVOR.so: \\  >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo " " $GCCMELT_STAGE_ZERO/[+base+].$MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5.descriptor.meltpic.o \\  >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
for f in $GCCMELT_STAGE_ZERO/[+base+]+[0-9]*.c; do
    echo " " $GCCMELT_STAGE_ZERO/`basename $f .c`._NOMDFIVESUM_.$GCCMELT_ZERO_FLAVOR.meltpic.o \\ >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
done
echo " " $GCCMELT_STAGE_ZERO/[+base+]._NOMDFIVESUM_.$GCCMELT_ZERO_FLAVOR.meltpic.o >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo MELTGENMOD_CUMULATED_MD5SUM_melt_stage_zero_[+varsuf+]=$MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5 >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo MELTGENMOD_NAKED_NAME_melt_stage_zero_[+varsuf+]=[+base+]  >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
echo '#end of generated file ' $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK >> $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$
##
mv $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK-tmp$$ $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK
meltbuild_info [+(.(fromline))+] generated stagezero makedep $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK
ls -l $MELT_ZERO_GENERATED_[+varsuf+]_BUILDMK > /dev/stderr

$GCCMELT_MAKE -f $GCCMELT_MODULE_MK melt_module \
   GCCMELT_FROM=stagezero-[+(.(fromline))+] \
   GCCMELTGEN_BUILD=$GCCMELT_STAGE_ZERO/ \
   GCCMELT_MODULE_WORKSPACE=meltbuild-workdir \
   GCCMELT_MODULE_FLAVOR=$GCCMELT_ZERO_FLAVOR \
   GCCMELT_CFLAGS="$GCCMELT_COMPILER_FLAGS" \
   GCCMELT_MODULE_SOURCEBASE=$GCCMELT_STAGE_ZERO/[+base+] \
   GCCMELT_CUMULATED_MD5=$MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5 \
   GCCMELT_MODULE_BINARYBASE=$GCCMELT_STAGE_ZERO/[+base+] \
   GCCMELT_MODULE_DEPENDENCIES="$GCCMELT_CC1_DEPENDENCIES" \
 || meltbuild_error  [+(.(fromline))+] stage0 [+base+] did not build with $GCCMELT_MAKE 

meltbuild_info [+(.(fromline))+] stage0 [+base+] module 
ls -l "$GCCMELT_STAGE_ZERO/[+base+].meltmod-$MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5.$GCCMELT_ZERO_FLAVOR.so" > /dev/stderr \
  || meltbuild_error  [+(.(fromline))+] stage0 [+base+] fail to build \
    "$GCCMELT_STAGE_ZERO/[+base+].meltmod-$MELT_ZERO_GENERATED_[+varsuf+]_CUMULMD5.$GCCMELT_ZERO_FLAVOR.so"

meltbuild_info [+(.(fromline))+] successfully build stage0 [+base+]

[+ENDFOR melt_translator_file+]


################################################################
################################################################
#################@ before our stages [+(.(fromline))+] 
### Our stages [+FOR melt_stage " "+][+melt_stage+][+ENDFOR meltstage+] 
### are incrementally built, with the former modules of
### the current stage and the later modules of the previous stages
### used to emit the source of the current module in the current stage.
### This is a kind of "diagonalization".
GCCMELT_SKIPEMITC=

[+FOR melt_stage+]
#@  begin for stage "[+melt_stage+]" [+(.(fromline))+]

[ -d [+melt_stage+] ] || mkdir [+melt_stage+]

meltbuild_info [+(.(fromline))+] at start of [+melt_stage+] GCCMELT_SKIPEMITC=$GCCMELT_SKIPEMITC.

#### rules for [+melt_stage+][+ 
  (define stageindex (+ 1 (for-index)))
  (define previndex (for-index))
  (define prevstage (if (> stageindex 1) (sprintf "meltbuild-stage%d" previndex) "$GCCMELT_STAGE_ZERO"))
  (define prevflavor (if (> stageindex 1) "quicklybuilt" "$GCCMELT_ZERO_FLAVOR"))
+], previous [+ (. prevstage)+] [+(.(fromline))+]

[+FOR melt_translator_file+][+ 
  (define outbase (get "base")) (define outindex (for-index)) +]

### the C source of [+melt_stage+] for [+ (. outbase)+] [+ (. (fromline))+]
#@ [+(.(fromline))+] base [+base+] stage [+melt_stage+] prevstage [+ (. prevstage)+] prevflavor  [+ (. prevflavor)+] 

meltbuild_info [+(.(fromline))+] generating C code of [+base+] in [+melt_stage+]

meltbuild_emit [+(.(fromline))+] \
    [+IF (= outindex 0)+]translateinit[+ELSE+]translatefile[+ENDIF+] \
    [+base+] \
    "[+melt_stage+]" \
    "[+(. prevstage)+]" \
    [+FOR melt_translator_file ":"+][+
  (define inbase (get "base")) (define inindex (for-index)) 
  (define depstage (if (< inindex outindex) (get "melt_stage") prevstage))
  (define depflavor (if (< inindex outindex) "quicklybuilt" prevflavor))
  (define depindex (if (< inindex outindex) stageindex (- stageindex 1)))
+][+(. depstage)+]/[+(. inbase)+].[+ (. depflavor)+][+ENDFOR melt_translator_file+] \
    "[+FOR includeload " "+][+includeload+][+ENDFOR includeload+]"

meltbuild_info [+(.(fromline))+] before compilation of C generated at [+melt_stage+] GCCMELT_SKIPEMITC=$GCCMELT_SKIPEMITC.


if [ -z "$GCCMELT_SKIPEMITC" ]; then
    meltbuild_info [+(.(fromline))+] compiling module [+base+] in [+melt_stage+]
    $GCCMELT_MAKE -f $GCCMELT_MODULE_MK melt_module \
	GCCMELT_FROM=[+melt_stage+]-[+(.(fromline))+] \
	GCCMELT_MODULE_WORKSPACE=meltbuild-workdir \
	GCCMELT_MODULE_FLAVOR=quicklybuilt \
	GCCMELT_CFLAGS="$GCCMELT_COMPILER_FLAGS" \
	GCCMELT_MODULE_SOURCEBASE=[+melt_stage+]/[+base+] \
	GCCMELT_MODULE_BINARYBASE=[+melt_stage+]/[+base+] \
	|| meltbuild_error  [+(.(fromline))+] stage [+melt_stage+] failed to make module [+base+]

else
    MELT_COMPUTED_[+varsuf+]_CUMULMD5=$(cat [+melt_stage+]/[+base+].c [+melt_stage+]/[+base+]+[0-9]*.c | $MD5SUM | cut -b 1-32)
    meltbuild_info [+(.(fromline))+] NOT compiling module [+base+] in [+melt_stage+] but symlinking previous [+ (. prevstage)+] module [+base+] checksum MELT_COMPUTED_[+varsuf+]_CUMULMD5=$MELT_COMPUTED_[+varsuf+]_CUMULMD5 skipemitc=$GCCMELT_SKIPEMITC.
    meltbuild_symlink [+ (. prevstage)+]/[+base+].meltmod-$MELT_COMPUTED_[+varsuf+]_CUMULMD5.[+ (. prevflavor)+].so [+melt_stage+]/
    
fi
#@ [+(.(fromline))+] end base [+base+] stage [+melt_stage+] 
[+ENDFOR melt_translator_file+]

########@  check that [+melt_stage+] did change something
if grep -q -v '^#' [+melt_stage+]/*.cfilist ; then
    meltbuild_info  [+(.(fromline))+] stage [+melt_stage+] did change some generated C files
else
    meltbuild_info  [+(.(fromline))+] stage [+melt_stage+] did not change any generated C files
    GCCMELT_SKIPEMITC="[+melt_stage+] unchanged from [+(.(fromline))+]"
[+FOR melt_translator_file+]
    MELT_COMPUTED_[+varsuf+]_CUMULMD5=$(cat [+melt_stage+][+base+].c [+melt_stage+][+base+]+[0-9]*.c | cut -b 1-32)
    meltbuild_info  [+(.(fromline))+] stage [+melt_stage+] for [+base+] cumulated checksum is MELT_COMPUTED_[+varsuf+]_CUMULMD5= $MELT_COMPUTED_[+varsuf+]_CUMULMD5
    # testing [+(.(fromline))+] stage  [+melt_stage+] base [+base+] module older
    if [ -f "[+melt_stage+]/[+base+].meltmod-$MELT_COMPUTED_[+varsuf+]_CUMULMD5.$quicklybuilt.so" -a "[+melt_stage+]/[+base+].meltmod-$MELT_COMPUTED_[+varsuf+]_CUMULMD5.$quicklybuilt.so" -nt "[+melt_stage+]/[+base+].cfilist" ]; then
	meltbuild_info  [+(.(fromline))+] [+melt_stage+]/[+base+].meltmod-$MELT_COMPUTED_[+varsuf+]_CUMULMD5.quicklybuilt.so newer than  "[+melt_stage+]/[+base+].cfilist"
	GCCMELT_SKIPEMITC=""
    fi
[+ENDFOR melt_translator_file+]
fi

########@  end for [+melt_stage+] [+ (. (fromline))+] ################
[+ENDFOR melt_stage+]


GCCMELT_LASTSTAGE=$GCCMELT_STAGE

meltbuild_info [+(.(fromline))+] last stage $GCCMELT_LASTSTAGE

################################################################
################################################################
###########@ before generating meltbuild-sources [+(.(fromline))+]

#### the meltbuild-sources is the final sources directory, to be
#### installed.  They are generated from the last stage, using the
#### modules inside it. Notice that in contrast from the intermediate
#### stages no "diagonalization" is involved.

[ -d meltbuild-sources ] || mkdir meltbuild-sources

GCCMELT_SKIPEMITC=""
#@ [+(.(fromline))+] 

[+FOR melt_translator_file+][+ 
  (define outbase (get "base")) (define outindex (for-index)) +]

### the C source of meltbuild-sources for [+base+] from [+ (. (fromline))+]

meltbuild_info [+(.(fromline))+] generating C code of [+base+] in meltbuild-sources

meltbuild_emit [+(.(fromline))+] \
    [+IF (= outindex 0)+]translateinit[+ELSE+]translatefile[+ENDIF+] \
    [+base+] \
    meltbuild-sources \
    "$GCCMELT_LASTSTAGE" \
    [+FOR melt_translator_file ":"+]$GCCMELT_LASTSTAGE/[+base+].quicklybuilt[+ENDFOR melt_translator_file+] \
    "[+FOR includeload " "+][+includeload+][+ENDFOR includeload+]"

### symlinking the MELT code in meltbuild-sources for [+base+] from [+ (. (fromline))+]

meltbuild_info [+(.(fromline))+] putting MELT code of [+base+] in meltbuild-sources

meltbuild_symlink $GCCMELT_MELTSOURCEDIR/[+base+].melt meltbuild-sources/
[+FOR includeload "\n"+]meltbuild_symlink [+includeload+] meltbuild-sources/[+ENDFOR includeload+]

[+ENDFOR melt_translator_file+]


################################################################

#@ from  [+(.(fromline))+]  compiling the modules
[ -d meltbuild-modules ] || mkdir meltbuild-modules

#@ [+(.(fromline))+] 

[+FOR melt_translator_file+][+ 
  (define outbase (get "base")) (define outindex (for-index)) +]

### the modules in all flavors in meltbuild-modules for [+base+] from [+ (. (fromline))+]


[+FOR flavor IN quicklybuilt optimized debugnoline+]

## @ [+(.(fromline))+] something might be wrong, we get e.g. a
## meltbuild-modules/warmelt-outobj.optimized.so symlink, which should
## be something with a md5sum inside.

#@ [+(.(fromline))+] flavor [+flavor+] base [+base+]
$GCCMELT_MAKE -f $GCCMELT_MODULE_MK melt_module \
   GCCMELT_FROM=[+(.(fromline))+] \
   GCCMELT_MODULE_WORKSPACE=meltbuild-workdir \
   GCCMELT_MODULE_FLAVOR=[+flavor+] \
   GCCMELT_CFLAGS="$GCCMELT_COMPILER_FLAGS" \
   GCCMELT_MODULE_SOURCEBASE=meltbuild-sources/[+base+] \
   GCCMELT_MODULE_BINARYBASE=meltbuild-modules/[+base+] \
 || meltbuild_error  [+(.(fromline))+] in meltbuild-modules failed to make module [+base+] [+flavor+]

meltbuild_info [+(.(fromline))+] compiled [+base+] flavor [+flavor+] in meltbuild-modules
[+ENDFOR flavor+]

#@ [+(.(fromline))+] base [+base+]

[+ENDFOR melt_translator_file+]

#@ [+(.(fromline))+] 
################
meltbuild_info [+(.(fromline))+] successfully done
#@ eof [+(.(fromline))+] end of generated melt-build-script.sh
