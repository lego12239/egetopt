#!/bin/sh
	OUT=""
	for W in "$@"; do
		W=`echo "$W" | sed -Ee 's/^\/*|\/*$//g'`
		if echo $W | grep '/'; then
			echo "There is slash in the middle: $W" >&2
			exit 1
		fi
		OUT="$OUT $W"
	done
	echo $OUT
