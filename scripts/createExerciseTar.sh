#/bin/bash
#=====================================================================================
# Script creates a tar file folderName.tar.gz which will be passed on to the students
#=====================================================================================

myPath="$( cd "$(dirname "$0")" >/dev/null 2>&1; pwd -P )"
baseDir="$(dirname ${myPath})"

if [[ ! -d "$baseDir" ]]; then
  echo "Base directory $baseDir does not exist!"
  exit 1
fi

folderName="charm-exercise"

cd $baseDir
pwd
make clean
mkdir $folderName

cp ./README ./$folderName
cp ./Makefile ./$folderName
cp -r ./img ./$folderName
cp -r ./src ./$folderName
mkdir $folderName/obj $folderName/output $folderName/scripts
#cp ./scripts/evaluateOutput.sh $folderName/scripts
cp -r ./scripts/compareOutput $folderName/scripts/
rm -rf $folderName/src/solution.cpp                                 #Remove solution.cpp

tar -czvf $folderName.tar.gz --exclude .git --exclude "*.log" ./$folderName
rm -rf ./$folderName
echo "Created tar file $folderName.tar.gz successfully in $baseDir"
