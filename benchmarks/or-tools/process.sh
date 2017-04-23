#!/bin/sh

find . -name "*.res" | parallel -P 4 --progress nice ./processone.sh {}







