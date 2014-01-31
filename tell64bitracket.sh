#! /bin/bash

# replace escaped space by just the space
RACKETBIN=`echo $RACKET_HOME | sed 's|\\\ | |g'`

# double quotes needed to prevent spaces in filename to split the filename up
"${RACKETBIN}/bin/racket" -e \
  '(if (fixnum? (expt 2 33)) (display "x86_64") (display "i386"))'

