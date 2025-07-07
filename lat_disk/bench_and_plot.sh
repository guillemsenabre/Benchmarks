#!/bin/bash

source ./config.cfg


# Check if required variables are set
REQUIRED_VARS=(ITERATIONS C_BIN DIR_PLOTS DIR_FILES WRITE_ME_FILE DEVICE MACHINE_NAME NUMBER_VALUES SIZE)

if [[ -n ${WRITE_TO_LOCAL_DISK} && -n ${WRITE_TO_DEVICE} ]]; then
	echo "Set a device or local file to write into"
	exit 1
elif [[ -z ${WRITE_TO_LOCAL_DISK} && -z ${WRITE_TO_DEVICE} ]]; then
	echo "Set either a device or a local file to write into"
	exit 1
fi

for var in "${REQUIRED_VARS[@]}"; do
	if [[ -z "${!var}" ]]; then
		echo "Missing required variable: ${var}"
		exit 1
	fi
done

C_ARGS=" -i ${ITERATIONS}"

# Check if writing to internal disk is enabled and extend the args var
if [[ -n ${WRITE_TO_LOCAL_DISK} ]]; then
	if [[ ! -d ${DIR_FILES} ]]; then
		echo "Directory '$DIR' not found. Creating it..."
    		mkdir -p "$DIR"
    		if [[ $? -ne 0 ]]; then
        		echo "Directory creation failed. Exiting."
        		exit 1
    		fi
	fi

	if [[ ! -f "${DIR_FILES}/${WRITE_ME_FILE}" ]]; then
    		echo "File '${DIR_FILES}/${WRITE_ME_FILE}' not found. Creating it..."
    		touch "${DIR_FILES}/${WRITE_ME_FILE}"
    		if [[ $? -ne 0 ]]; then
        		echo "File creation failed. Exiting."
        		exit 1
    		fi

		C_ARGS+=" -d ${DIR_FILES}/${WRITE_ME_FILE}"
	fi
fi

# Check if writing to external disk is enabled and extend the args var
if [[ -n ${WRITE_TO_EXTERNAL_DISK} ]]; then
	echo "WARINING: Make sure ${DEVICE} is empty, or contains non-important data, as the test can break the MBR partition of the disk"
	C_ARGS+=" -d ${DEVICE}"
fi

# Check if lat_disk binary exists and is executable
if [[ ! -x ${C_BIN} ]]; then
    	echo "${C_BIN} binary not found or not executable. Compiling..."
    	gcc lat_disk.c -o ${C_BIN}
    	if [[ $? -ne 0 ]]; then
        	echo "Compilation failed. Exiting."
        	exit 1
    	fi
fi

# Check if directory to store plots exists
if [[ ! -d ${DIR_PLOTS} ]]; then
    	echo "${DIR_PLOTS} directory not found, creating it..."
    	mkdir -p ${DIR_PLOTS}
    	if [[ $? -ne 0 ]]; then
        	echo "Directory creation failed. Exiting."
        	exit 1
    	fi
fi

# Run the binary
${C_BIN} ${C_ARGS}
if [[ $? -ne 0 ]]; then
    	echo "${C_BIN} execution failed. Exiting."
    	exit 1
fi

# Run the plotting script
python3 lat_plot.py $MACHINE_NAME $NUMBER_VALUES $SIZE
if [[ $? -ne 0 ]]; then
   	echo "Python plotting script failed."
    	exit 1
fi

C_ARGS=""
