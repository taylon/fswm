#!/bin/bash

echo "==========================="
echo "      Starting tests       "
echo "==========================="
echo

displayID=:1

# Launch Xephyr
Xephyr -ac -screen 800x600 $displayID &
xephyrPID=$!
# echo "Xephyr PID: $xephyrPID"
sleep 0.5

DISPLAY=$displayID

# Launch WM
./fswm &
wmPID=$!
# echo "WM PID: $wmPID"

sleep 0.5

xterm &
programPID=$!
# echo "Program PID: $programPID"

sleep 2

# Kill everything
kill $programPID
kill $wmPID
sleep 0.5
kill $xephyrPID

echo
echo "==========================="
echo "     tests complete :)     "
echo "==========================="
echo
