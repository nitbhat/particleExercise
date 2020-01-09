#!/bin/bash
#=====================================================================================
# Script creates a tar file folderName.tar.gz which will be passed on to the students
#=====================================================================================

#Set particleSimDir to the home directory of particleSimulation
particleSimDir="/Users/nitinbhat/Work/software/particleSimulation"
folderName="charm-exercise"

cd $particleSimDir
pwd
make clean
mkdir $folderName

cp ./README.md ./$folderName
cp ./Makefile ./$folderName
cp -r ./img ./$folderName
cp -r ./src ./$folderName
mkdir $folderName/obj $folderName/output

tar -czvf $folderName.tar.gz ./$folderName
rm -rf ./$folderName
