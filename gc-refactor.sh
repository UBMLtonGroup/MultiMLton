#!/bin/sh
echo "Starting Refactor..."
for file in $1
do
    echo ""
    for field in $(cat field-replacements.txt)
    do
	echo "Replacing \"->$field\" with \"->globalState->$field\" in $file"
	find $1 -type f -name "*.c" -print -exec sed -i "s/s->$field\b/s->globalState.$field/g" {} \;
	find $1 -type f -name "*.c" -print -exec sed -i "s/d->$field\b/d->globalState.$field/g" {} \;
	find $1 -type f -name "*.c" -print -exec sed -i "s/s->globalState.procStates[proc].$field\b/s->globalState.procStates[proc].globalState.$field/g" {} \;
	#find $1 -type f -name "*.h" -print -exec sed -i "s/->$field/->globalState->$field/g" {} \;
	#perl -p -i -e "s/->$field/->globalState.$field/g" $file
	#perl -p -i -e "s/s->procStates[proc]/s->globalState.procStates[proc]/g" $file
	
    done
done
