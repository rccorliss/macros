#!/bin/bash

# Author: Molly Taylor
# Date: 10/15/18
# Goal: iterate though different INTT configurations and generate Kalman extrapolation uncertainties.

# FIXED ETA PHI HIGH ENERGY
# eta = 0.2
# phi = 0, 15, 30
# z = 0 cm
# pt = 100 GeV

# layout 0 = four (eight) layers, laddertypes 0-1-1-1
# layout 1 = four (eight) layers, laddertypes 1-1-0-1
# layout 2 = three (six) outer layers, laddertypes 1-0-1
# layout 3 = three (six) outer layers, laddertypes 1-1-1
# layout 4 = two (four) outer layers, laddertypes 0-1
# layout 5 = two (four) outer layers, laddertypes 1-1
# layout 6 = one (two) outer layers, laddertype 1
# layout 7 = no outer layers

config=0
n_events=150
particle=pi+ # particle being thrown: pi+, mu+, e-
data=sphenix/u/mitay/Documents/latest/macros/macros/g4simulations/data_gen
parent=high_energy # parent directory

# verified correct layout by looking at truth particle data in ntp_hit

# define particle generation parameters
z_width=0 # number of cm on either side of zero, uniform distribution
eta_min=0.2
eta_max=0.2
pt_min=100
pt_max=100
phi_min=0
phi_max=0

# go through different phi
for phi in 0 15 30
do
  # go through every INTT configuration
  for i in {0..7}
  do
    case $i in
      0) config=0111 ;;
      1) config=1101 ;;
      2) config=101  ;;
      3) config=111  ;;
      4) config=01   ;;
      5) config=11   ;;
      6) config=1    ;;
      7) config=n    ;;
      *) config=x    ;;
    esac

    phi_min="$phi"
    phi_max="$phi"

    # directory="$particle"_pt"$pt_min"-"$pt_max"GeV_phi"$phi_min"-"$phi_max"d_z-"$z_width"-"$z_width"cm_eta"$eta_min"-"$eta_max"
    directory="$particle"_pt"$pt_min"GeV_phi"$phi_min"d_z"$z_width"cm_eta"$eta_max"

    # check to see if there is a directory for this data set; create one if there isn't
    if [ ! -d /"$data"/"$parent" ]; then
      mkdir /"$data"/"$parent"
    fi

    if [ ! -d /"$data"/"$parent"/"$directory" ]; then
      mkdir /"$data"/"$parent"/"$directory"
    fi

    # syntax: Fun4All_G4_sPHENIX(#_events,INTT_layout,output_file_name)
    root.exe Fun4All_G4_sPHENIX.C"("$n_events","$i","$z_width","$eta_min","$eta_max","$pt_min","$pt_max","$phi_min","$phi_max","\"$particle\"","\"/"$data"/"$parent"/"$directory"/G4_sPHENIX_"$config".root\"")"

    # the output file is automatically named PHG4TrackKalmanFitter_eval.root
    # this file name is set in coresoftware/simulation/g4simulation/g4hough/PHG4TrackKalmanFitter.C
    # in order to change it, the library must be recompiled
    # thus we must move and rename each file after it is generated so it will not be overwritten
#    mv PHG4TrackKalmanFitter_eval.root /"$data"/"$parent"/"$directory"/G4_sPHENIX_"$config".root_g4kalman_eval.root
  done
done

