#!/usr/bin/bash

if [ $# -lt 2 ]; then
  echo "FORMAT: folder_name file_name"
  exit 1
fi

ROOT=$PWD
DIR_NAME=$1
FILE_NAME=$2
find "$DIR_NAME" "$FILE_NAME" &>/dev/null || exit 1
OUR_EXEC=$ROOT/xmod
CHMOD_EXEC="chmod"

TEMP_DIR=$ROOT/temp
RESULTS_DIR=$TEMP_DIR/results
CHMOD_RESULTS_DIR=$TEMP_DIR/results/chmod
OUR_RESULTS_DIR=$TEMP_DIR/results/xmod
OUR_DIR=$TEMP_DIR/xmod
CHMOD_DIR=$TEMP_DIR/chmod

rm -rf "$TEMP_DIR"
mkdir "$TEMP_DIR" "$RESULTS_DIR" "$OUR_RESULTS_DIR" "$CHMOD_RESULTS_DIR" || exit 1
function init_files() {
  cd "$ROOT" || exit 1
  chmod -R 777 .
  rm -rf "$CHMOD_DIR" "$OUR_DIR"
  mkdir "$OUR_DIR" "$CHMOD_DIR" || exit 1
  cp -R "$DIR_NAME" "$OUR_DIR" || exit 1
  cp "$FILE_NAME" "$OUR_DIR" || exit 1
  cp -R "$DIR_NAME" "$CHMOD_DIR" || exit 1
  cp "$FILE_NAME" "$CHMOD_DIR" || exit 1
  cd "$OLDPWD" || exit 1
}

init_files

readarray -t tests <options.txt || exit 1
no_tests=${#tests[@]}

cd "$OUR_DIR" || exit 1
i=0
echo "-------XMOD-------"
while [ $i -lt "$no_tests" ]; do
  echo ${tests[$i]}
  init_files
  $OUR_EXEC ${tests[$i]} "$FILE_NAME" | sort -b 1>"$OUR_RESULTS_DIR"/file_test_$i
  $OUR_EXEC ${tests[$i]} "$DIR_NAME" | sort -b 1>"$OUR_RESULTS_DIR"/directory_test_$i
  ((++i))
done

init_files
cd "$CHMOD_DIR" || exit 1
i=0
echo "-------CHMOD-------"
while [ $i -lt "$no_tests" ]; do
  echo ${tests[$i]}
  init_files
  $CHMOD_EXEC ${tests[$i]} "$FILE_NAME" | sort -b 1>"$CHMOD_RESULTS_DIR"/file_test_$i
  $CHMOD_EXEC ${tests[$i]} "$DIR_NAME" | sort -b 1>"$CHMOD_RESULTS_DIR"/directory_test_$i
  ((++i))
done

no_failed=0
i=0
while [ $i -lt "$no_tests" ]; do
  diff -b "$CHMOD_RESULTS_DIR"/directory_test_$i "$OUR_RESULTS_DIR"/directory_test_$i
  no_failed+=$?
  diff -b "$CHMOD_RESULTS_DIR"/directory_test_$i "$OUR_RESULTS_DIR"/directory_test_$i
  no_failed+=$?
  ((++i))
done
chmod -R 777 "$TEMP_DIR"
rm -rf "$TEMP_DIR" &>/dev/null
exit $no_failed
