#!/bin/bash
#=====================================================================================
# Script evaluates the tar file submitted by the students to check for correctness
# and determine the total time taken by the application
#=====================================================================================

#Set compareDir to the directory that contains the verified output of the application
#The output submitted by the students will be compared against this output

inputTar=$1

if [[ -z "$inputTar" ]]; then
  echo "Tar file value not passed in the first parameter! Usage: ./evaluateOutput.sh <path-to-tar-file> <input-type>";
  exit 1
fi


if [[ ! -f "$inputTar" ]]; then
  echo "Tar file $inputTar does not exist!"
  exit 1
fi

simType=$2

if [[ -z "$simType" ]]; then
  echo "Input type not passed in the second parameter! Usage: ./evaluateOutput.sh <path-to-tar-file> <input-type>";
  exit 1
fi

echo "Input type is ----$simType----"


if [[ "$simType" != "simple" && "$simType" != "bench" ]]; then
  echo "Input type should be either \"simple\" (make test output) or \"bench\" (make test-bench output)";
  exit 1
fi

compareDir="/Users/nitinbhat/Work/software/particleSimulation/scripts/compareOutput/$simType/"
resultDir="/Users/nitinbhat/Work/software/particleSimulation/scripts/resultDirs/"


if [[ ! -d "$compareDir" ]]; then
  echo "Comparison directory $compareDir does not exist!"
  exit 1
fi

if [[ ! -d "$resultDir" ]]; then
  echo "Results directory $resultDir does not exist!"
  exit 1
fi

tar -xvf $inputTar -C $resultDir
inputDir=`ls -tdu -- $resultDir/* | head -n 1`

#echo "DEBUG: Compare dir is $compareDir"
#echo "DEBUG: Input tar is $inputTar"
#echo "DEBUG: Input dir is $inputDir"

declare -a compareOutput
declare -a compareInput
declare -a evalOutput
declare -a evalInput
declare -a inputArr
declare -a outputArr

function parse_main_output {
  file=$1;
  while IFS= read -r line
  do
    if [[ $line =~ ^Input:Grid.*:([0-9]+) ]]; then
      inputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Input:Particles.*:([0-9]+) ]]; then
      inputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Input:Number.*:([0-9]+) ]]; then
      inputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Input:Particle.*:([0-9]+),([0-9]+),([0-9]+),([0-9]+) ]]; then
      inputArr+=(${BASH_REMATCH[1]})
      inputArr+=(${BASH_REMATCH[2]})
      inputArr+=(${BASH_REMATCH[3]})
      inputArr+=(${BASH_REMATCH[4]})
    elif [[ $line =~ ^Input:Velocity.*:([0-9]+) ]]; then
      inputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Total.*:([0-9]+\.[0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Time.*:([0-9]+\.[0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Min.*:(-?[0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Max.*:(-?[0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Cell.*Min.*:\((-?[0-9]+),(-?[0-9]+)\) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
      outputArr+=(${BASH_REMATCH[2]})
    elif [[ $line =~ ^Output:Cell.*Max.*:\((-?[0-9]+),(-?[0-9]+)\) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
      outputArr+=(${BASH_REMATCH[2]})
    fi
  done <"$file"
}

mv $inputDir/sim_output_main ./sim_output_main_input
mv $compareDir/sim_output_main ./sim_output_main_compare

parse_main_output ./sim_output_main_compare
compareOutput=("${outputArr[@]}")
compareInput=("${inputArr[@]}")
unset inputArr
unset outputArr

parse_main_output ./sim_output_main_input
evalOutput=("${outputArr[@]}")
evalInput=("${inputArr[@]}")

lastIndex="$((${#evalInput[@]}-1))"

#echo "DEBUG: compare output is ${compareOutput[*]}"
#echo "DEBUG: eval output is ${evalOutput[*]}"

#echo "DEBUG: compare input is ${compareInput[*]}"
#echo "DEBUG: eval input is ${evalInput[*]}"

#check correctness of inputs
for i in $(seq 0 $lastIndex);
  do
    if [ ${compareInput[$i]} -ne ${evalInput[$i]} ]; then
      echo "ERROR !!!!!!!!!!!!!!!!! Inputs are not same! Eval:${evalInput[$i]} Compare:${compareInput[$i]} ";
      exit 1
    fi
done

diffResult=`diff -rq $inputDir $compareDir`

echo "=========================================================================================";
if [[ -z "$diffResult" ]]; then
  echo "SUCCESS **************** Particle Output is correct!"
else
  echo "ERROR !!!!!!!!!!!!!!!!! Particle Output is incorrect!"
fi

echo "=========================================================================================";
if [ ${evalOutput[5]} -eq -1 ]; then
  echo "WARNING &&&&&&&&&&&&&&&&&  Minimum Bonus Question not answered!";
else
  if [ ${evalOutput[5]} -ne ${compareOutput[5]} ]; then
    echo "ERROR !!!!!!!!!!!!!!!!! Minimum Bonus Question answered but incorrect!";
  else
    echo "SUCCESS **************** Minimum Bonus Question answered - Min value correct";
    if [ ${evalOutput[6]} -ne ${compareOutput[6]} ]; then
      echo "ERROR !!!!!!!!!!!!!!!!! Minimum Bonus Question - Min Cell X coordinate incorrect";
    else
      echo "SUCCESS **************** Minimum Bonus Question - Min Cell X coordinate correct";
      if [ ${evalOutput[7]} -ne ${compareOutput[7]} ]; then
        echo "ERROR !!!!!!!!!!!!!!!!! Minimum Bonus Question - Min Cell Y coordinate incorrect";
      else
        echo "SUCCESS **************** Minimum Bonus Question - Min Cell Y coordinate correct";
      fi
    fi
  fi
fi

echo "=========================================================================================";
if [ ${evalOutput[2]} -eq -1 ]; then
  echo "WARNING &&&&&&&&&&&&&&&&&  Maximum Bonus Question not answered!";
else
  if [ ${evalOutput[2]} -ne ${compareOutput[2]} ]; then
    echo "ERROR !!!!!!!!!!!!!!!!! Maximum Bonus Question answered but incorrect!";
  else
    echo "SUCCESS **************** Maximum Bonus Question answered - Max value correct";
    if [ ${evalOutput[3]} -ne ${compareOutput[3]} ]; then
      echo "ERROR !!!!!!!!!!!!!!!!! Maximum Bonus Question - Max Cell X coordinate incorrect";
    else
      echo "SUCCESS **************** Maximum Bonus Question - Max Cell X coordinate correct";
      if [ ${evalOutput[4]} -ne ${compareOutput[4]} ]; then
        echo "ERROR !!!!!!!!!!!!!!!!! Maximum Bonus Question - Max Cell Y coordinate incorrect";
      else
        echo "SUCCESS **************** Maximum Bonus Question - Max Cell Y coordinate correct";
      fi
    fi
  fi
fi

echo "=========================================================================================";
echo "Total Application Time                                                ${evalOutput[0]}";
echo "=========================================================================================";
echo "Total Application Time Per Step                                       ${evalOutput[1]}";
echo "=========================================================================================";

#Reset back files
mv ./sim_output_main_compare $compareDir/sim_output_main
mv ./sim_output_main_input $inputDir/sim_output_main
