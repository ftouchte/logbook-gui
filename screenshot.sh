#!/bin/bash

# if $1 is not empty
if [[ -n $1 ]]; then
	if [[ ! -d "./screenshots" ]]; then
		mkdir screenshots
	fi
	gnome-screenshot -w -f "./screenshots/$1"
fi
