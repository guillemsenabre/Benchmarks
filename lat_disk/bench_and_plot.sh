#!/bin/bash

TARGET_MACHINE_IP="100.126.103.11"
TARGET_MACHINE_USER="gsenabre"
TARGET_MACHINE_DIRECTORY="/home/${TARGET_MACHINE_USER}//projects/vitamin-v-wp4-benchmarks/bus_latency/plots/"

LOCAL_PLOT_DIRECTORY="./plots/"

ITERATIONS=100
NUMBER_VALUES=$ITERATIONS
MACHINE_NAME="x86"
SIZE=512

# Check if lat_disk binary exists and is executable
if [[ ! -x ./lat_disk ]]; then
    echo "lat_disk binary not found or not executable. Compiling..."
    gcc lat_disk.c -o lat_disk
    if [[ $? -ne 0 ]]; then
        echo "Compilation failed. Exiting."
        exit 1
    fi
fi

# Run the binary
./lat_disk -i $ITERATIONS
if [[ $? -ne 0 ]]; then
    echo "lat_disk execution failed. Exiting."
    exit 1
fi

# Run the plotting script
python3 lat_plot.py $MACHINE_NAME $NUMBER_VALUES $SIZE
if [[ $? -ne 0 ]]; then
    echo "Python plotting script failed."
    exit 1
fi

# Copy plots
#scp ${TARGET_PLOT_DIRECTORY}* $TARGET_MACHINE_USER@$TARGET_MACHINE_IP:$TARGET_MACHINE_DIRECTORY

