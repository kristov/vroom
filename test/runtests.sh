#!/bin/bash

for TEST in ./test/test_*; do
    ${TEST};
    if [ $? -eq 0 ]; then
        echo "${TEST}: OK";
    else
        echo "${TEST}: FAILURES $?";
    fi
done
