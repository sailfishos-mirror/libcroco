#!/bin/sh

HERE_DIR=`dirname $0`
. $HERE_DIR/global-vars.sh

$CSSLINT $TEST_INPUTS_DIR/lots-of-comments.css
