#!/bin/bash
# Program: 
# 	log sender output and generate figure
#
GREEN=\\e[1m\\e[32m
RED=\\e[1m\\e[31m
YELLOW=\\e[1m\\e[33m
RESET=\\e[m

BIN_PATH=../bin/
LOG_PATH=../log/
TimeStamp=$(date +%y%m%d%H%M%S)
FORMAT=.log

LOG_FILENAME=sender_test
TARGET=sender_test
LOG_FILE=${LOG_FILENAME}_${TimeStamp}${FORMAT}
#####################
echo -e ${RED} Start to run ${TARGET}, please check log file to see runing details ${RESET}

${BIN_PATH}${TARGET} &> ${LOG_PATH}${LOG_FILE}

echo -e ${YELLOW} End ${TARGET} ${RESET}
###################


#################
echo -e ${RED} Start to analysis, please make sure log file have valid records ${RESET}

PY_FILE=sender_log_analysis.py
FIGURE_OUTPUT_DIR=

ACTIVATEDIR=/home/carr/Documents/virtualenv/venv/bin/activate
source $ACTIVATEDIR

python3 ${PY_FILE} --log_filename=${LOG_FILE} --log_dir=${LOG_PATH} 

deactivate

echo -e ${YELLOW} End analysis ${RESET}

