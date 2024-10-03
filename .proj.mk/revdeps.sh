#!/bin/sh
	OUT=""
	read DEPS
	for DEP in $DEPS; do
		OUT="$DEP $OUT"
	done
	DEPS=$OUT
	OUT=""
	for DEP in $DEPS; do
		if echo $OUT | egrep -v "(^|[[:space:]]+)$DEP([[:space:]]+|$)" >/dev/null 2>&1; then
			OUT="$OUT $DEP"
		fi
	done
	echo $OUT
