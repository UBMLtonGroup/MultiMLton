#!/bin/sh
echo "Starting Refactor..."
for file in $1
do
    echo ""
    for field in $(cat field-replacements.txt)
    do
	echo "Replacing \"->$field\" with \"->globalState->$field\" in $file"
	perl -p -i -e "s/->$field/->globalState->$field/g" $file
    done
done
