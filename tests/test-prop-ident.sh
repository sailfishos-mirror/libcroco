#!/bin/sh

HERE_DIR=`dirname $0`
. $HERE_DIR/global-vars.sh

$CSSLINT $TEST_INPUTS_DIR/prop-vendor-ident.css
