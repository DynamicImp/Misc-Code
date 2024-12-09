#!/bin/bash

# Compile the shell and test program
make

# Test Scenarios
echo "Starting Scenario Tests"

# Scenario 1: Ctrl-C Test
echo "Scenario 1: Ctrl-C Test"
./smallsh <<EOF
./prog1&
./prog1&
./prog1
^C
EOF

# Scenario 2: Ctrl-Z Test
echo "Scenario 2: Ctrl-Z Test"
./smallsh <<EOF
./prog1&
./prog1&
./prog1
^Z
EOF

# Scenario 3: Jobs Command After Ctrl-C
echo "Scenario 3: Jobs Command"
./smallsh <<EOF
./prog1&
./prog1&
./prog1
^C
jobs
EOF

# Scenario 4: Jobs Command After Ctrl-Z
echo "Scenario 4: Jobs Command"
./smallsh <<EOF
./prog1&
./prog1&
./prog1
^Z
jobs
EOF

# Scenario 5: Bring Background Job to Foreground
echo "Scenario 5: Bring Job to Foreground"
./smallsh <<EOF
./prog1&
./prog1&
./prog1
^Z
jobs
fg 3
EOF

# Scenario 6: Resume Background Job
echo "Scenario 6: Resume Background Job"
./smallsh <<EOF
./prog1&
./prog1&
./prog1
^Z
jobs
bg 3
jobs
EOF

echo "Tests Completed!"
