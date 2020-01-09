#!/bin/bash
#=====================================================================================
# Script evaluates the tar file submitted by the students to check for correctness
# and determine the total time taken by the application
#=====================================================================================

#Set compareDir to the directory that contains the verified output of the application
#The output submitted by the students will be compared against this output
compareDir="/Users/nitinbhat/Work/software/particleSimulation/scripts/compareOutput/"
inputTar=$1

if [[ -z "$inputTar" ]]; then
  echo "Tar file value not passed in the first parameter! Usage: ./evaluateOutput.sh <path-to-tar-file>";
  exit 1
fi

if [[ ! -f "$inputTar" ]]; then
  echo "Tar file $inputTar does not exist!"
  exit 1
fi

tar -xvf $inputTar
inputDir=`ls -td -- */ | head -n 1`

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
    elif [[ $line =~ ^Output:Total.*:([0-9]+\.[0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Time.*:([0-9]+\.[0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Min.*:([0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    elif [[ $line =~ ^Output:Max.*:([0-9]+) ]]; then
      outputArr+=(${BASH_REMATCH[1]})
    fi
  done <"$file"
}

mv $inputDir/sim_output_main ./sim_output_main_input
mv $compareDir/sim_output_main ./sim_output_main_compare

diffResult=`diff -rq $inputDir $compareDir`

if [[ -z "$diffResult" ]]; then
  echo "Particle Output is correct!"
else
  echo "Particle Output is incorrect!"
fi

parse_main_output ./sim_output_main_compare
compareOutput=("${outputArr[@]}")
compareInput=("${inputArr[@]}")
unset inputArr
unset outputArr

parse_main_output ./sim_output_main_input
evalOutput=("${outputArr[@]}")
evalInput=("${inputArr[@]}")

lastIndex="$((${#evalInput[@]}-1))"

#check correctness of inputs
for i in $(seq 0 $lastIndex);
  do
    if [ ${compareInput[$i]} -ne ${evalInput[$i]} ]; then
      echo "Inputs are not same! Eval:${evalInput[$i]} Compare:${compareInput[$i]} ";
      exit 1
    fi
done

if [ ${evalOutput[2]} -eq -1 ]; then
  echo "Minimum Bonus Question not answered!";
else
  if [ ${evalOutput[2]} -ne ${compareOutput[2]} ]; then
    echo "Minimum Bonus Question answered but incorrect!";
  else
    echo "Minimum Bonus Question answered and correct!";
  fi
fi

if [ ${evalOutput[3]} -eq -1 ]; then
  echo "Maximum Bonus Question not answered!";
else
  if [ ${evalOutput[3]} -ne ${compareOutput[3]} ]; then
    echo "Maximum Bonus Question answered but incorrect!";
  else
    echo "Maximum Bonus Question answered and correct!";
  fi
fi

echo "Total Application Time:${evalOutput[0]}";
echo "Total Application Time Per Step:${evalOutput[1]}";

mv ./sim_output_main_compare $compareDir/sim_output_main
mv ./sim_output_main_input $inputDir/sim_output_main
