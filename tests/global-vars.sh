TEST_INPUTS_DIR=$HERE_DIR/test-inputs
CSSLINT=$HERE_DIR/../csslint/.libs/csslint
if ! test -x $CSSLINT ; then
    echo "Aarg Could not find an executable csslint. I was looking for $CSSLINT"
    echo $
fi

if ! test x"$VALGRIND" = x ; then
    CSSLINT="$VALGRIND $CSSLINT"
fi
