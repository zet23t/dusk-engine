#!/bin/bash

# Check if DUSK_ENGINE_HOME is set
if [ -z "$DUSK_ENGINE_HOME" ]; then
    echo "DUSK_ENGINE_HOME is not set. Please set it to the directory of the Dusk Engine."
    exit 1
fi

PROJECTDIR=$(dirname "$(readlink -f "$0")")

# Navigate to DUSK_ENGINE_HOME and run the make command
cd "$DUSK_ENGINE_HOME"
make run PROJECTDIR="$PROJECTDIR"