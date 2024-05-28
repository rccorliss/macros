#!/bin/bash

dists=`ls static*.root`

for file in $dists
do
    root -l Fun4All_FieldOnAllTrackers.C\\\(100,\\\"${file}\\\"\\\)
    mkdir ${file}.output
    mv cluster*  ${file}.output
    mv trkrntuple.root  ${file}.output
done
