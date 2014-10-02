#!/bin/sh
while true; do
    echo "Cleaning. Please wait...";
    make clean 2> /dev/null 1> /dev/null;
    echo "Building. Please wait...";
    make runtime 2> build.error.log 1> build.message.log;
    emacs -nw build.error.log
done

# while true; do
#     change=$(inotifywait -r -e close_write,moved_to,create .)
#     change=${change#./ * }
#     if [["$change" == "*.c"]]; then
# 	make clean;
# 	make runtime 2> build.error.log 1> build.log;
# 	less build.error.log
#     fi
# done
