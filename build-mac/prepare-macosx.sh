#!/bin/sh
cd dependencies
for script in prepare-*-macosx.sh ; do
	echo running $script
	sh "$script"
done
