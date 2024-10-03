#!/bin/sh
	DEP="$1"
	mkdir $DEPSDIR/src/${DEP}.tmp
	cd $DEPSDIR/src/${DEP}.tmp
	FTYPE=`file ../${DEP}.WK | awk '{print $2}' | tr A-Z a-z`
	case $FTYPE in
	gzip)
		gunzip -c ../${DEP}.WK | tar -x
		;;
	bzip2)
		bunzip2 -c ../${DEP}.WK | tar -x
		;;
	xz)
		unxz -c ../${DEP}.WK | tar -x
		;;
	zip)
		unzip ../${DEP}.WK
		;;
	*)
		echo "Unknown file format: $FTYPE" >&2
		exit 1
		;;
	esac
	FCNT=`ls -1 | wc -l`
	if [ $FCNT -eq 1 ]; then
		mv `ls -1` $DEPSDIR/src/${DEP}
		cd ..
		rm -rf ${DEP}.tmp
	else
		cd ..
		mv $DEPSDIR/src/${DEP}.tmp $DEPSDIR/src/${DEP}
	fi
