#!/bin/bash

for TEST in ./test/test_*; do
    echo -n "${TEST}: "
    ${TEST};
    if [ $? -eq 0 ]; then
        echo "OK";
    else
        echo "FAILURES $?";
    fi
done
