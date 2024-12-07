#!/bin/bash

output_file="results.txt"

# Clear the output file if it exists
> "$output_file"

# Loop from 1 to 100
for i in {1..100}
do
    # Execute the game and capture both stdout and stderr
    output=$(./Game jim3 Dummy Dummy Dummy -s $i < default.cnf 2>&1)

    # Extract the line starting with "info: player bruh got" from the output
    line=$(echo "$output" | grep "info: player jim3 got")

    # Append the line to the output file, only if it is not empty
    if [[ -n "$line" ]]; then
        echo "$line" >> "$output_file"
    fi
done
