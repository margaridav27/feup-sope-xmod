#!/usr/bin/bash

if [ $# -lt 2 ]; then
  echo "FORMAT: folder_name file_name"
  exit 1
fi

DIR_NAME=$1
FILE_NAME=$2
find "$DIR_NAME" "$FILE_NAME" || exit 1
ROOT=$PWD
OUR_EXEC=$ROOT/xmod
CHMOD_EXEC="chmod"

TEMP_DIR=$ROOT/temp
OUR_DIR=$TEMP_DIR/xmod
CHMOD_DIR=$TEMP_DIR/chmod
rm -rf "$TEMP_DIR"
mkdir "$TEMP_DIR" "$OUR_DIR" "$CHMOD_DIR" "$OUR_DIR"/results "$CHMOD_DIR"/results || exit 1
cp -R "$DIR_NAME" "$OUR_DIR" || exit 1
cp "$FILE_NAME" "$OUR_DIR" || exit 1
cp -R "$DIR_NAME" "$CHMOD_DIR" || exit 1
cp "$FILE_NAME" "$CHMOD_DIR" || exit 1

readarray -t tests < options.txt || exit 1
no_tests=${#tests[@]}

cd "$OUR_DIR" || exit 1
i=0
echo "-------XMOD-------"
while [ $i -lt "$no_tests" ]; do
  echo  ${tests[$i]}
  $OUR_EXEC ${tests[$i]} "$FILE_NAME" | sort -b 1>results/file_test_$i
  $OUR_EXEC ${tests[$i]} "$DIR_NAME" | sort -b 1>results/output_directory_test_$i
  ((++i))
done

cd "$CHMOD_DIR" || exit 1
i=0
echo "-------CHMOD-------"
while [ $i -lt "$no_tests" ]; do
  echo  ${tests[$i]}
  $CHMOD_EXEC ${tests[$i]} "$FILE_NAME" | sort -b 1>results/file_test_$i
  $CHMOD_EXEC ${tests[$i]} "$DIR_NAME" | sort -b 1>results/output_directory_test_$i
  ((++i))
done

no_failed=0
i=0
while [ $i -lt "$no_tests" ]; do
  diff -b "$CHMOD_DIR"/results/output_directory_test_$i "$OUR_DIR"/results/output_directory_test_$i
  no_failed+=$?
  diff -b "$CHMOD_DIR"/results/output_directory_test_$i "$OUR_DIR"/results/output_directory_test_$i
  no_failed+=$?
  ((++i))
done
rm -rf "$TEMP_DIR"
exit $no_failed
