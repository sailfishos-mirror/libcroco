#!/bin/sh

HERE_DIR=`dirname $0`
. $HERE_DIR/global-vars.sh

$CSSLINT --dump-location $TEST_INPUTS_DIR/parsing-location.css
