#!/bin/bash
export FCS_PATH="`pwd`"
./Tatzer --prefix="$HOME"/apps/test/fcs/ --rcs --notest-suite --kaz && \
    make fc-solve && \
    cd t && \
    perl -d:Trace t/compare-digests-and-lens.t
