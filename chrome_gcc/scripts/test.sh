#!/bin/bash
# Program: 
# 	test shell: enter venv and run .py file to generate figure
#

ACTIVATEDIR=/home/carr/Documents/virtualenv/venv/bin/activate
source $ACTIVATEDIR
python3 test.py
deactivate

