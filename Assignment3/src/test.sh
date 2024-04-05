#!/bin/bash

# Compile the code
make

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Run tests
for i in {0..4}; do
    echo "Running test $i..."
    ./parser ../Oat-v1-LL1-grammar.txt ../testcases/test${i}-tokens.txt ../output${i}.txt
    if [ $? -eq 0 ]; then
        echo "Test $i successful."
    else
        echo "Test $i failed."
    fi
done

echo "All tests completed."