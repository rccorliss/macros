



// Function to modify the histogram
TH3* modifyHistogram(TH3* histo, double scale, double offset) {
    // Create a new 3D histogram for the modified data
    TH3* histoModified = (TH3*)histo->Clone();
    //histoModified->SetName(Form("%s_modified", histo->GetName()));

    // Loop over all bins of the histogram and modify the contents
    for (int x = 1; x <= histo->GetNbinsX(); ++x) {
        for (int y = 1; y <= histo->GetNbinsY(); ++y) {
            for (int z = 1; z <= histo->GetNbinsZ(); ++z) {
                double binContent = histo->GetBinContent(x, y, z);
                double newContent = scale * binContent + offset;
                histoModified->SetBinContent(x, y, z, newContent);
            }
        }
    }

    return histoModified;
}

void adjustOnce(const char* inputFileName,const char* outputFileName, double scaleR, double offsetR, double scaleP, double offsetP) {
    // Open the input file
    TFile* inputFile = TFile::Open(inputFileName, "READ");
    if (!inputFile || inputFile->IsZombie()) {
        printf("Error: could not open file %s\n", inputFileName);
        return;
    }

    // Get the 3D histogram from the file
    TH3* hist;

    // Modify the histograms
    TH3* hmod;

    // Open the output file
    TFile* outputFile = TFile::Open(outputFileName, "RECREATE");
    if (!outputFile || outputFile->IsZombie()) {
        printf("Error: could not create file %s\n", outputFileName);
        inputFile->Close();
        return;
    }

    //cycle through all the histograms
    const std::array<const std::string, 2> extension = {{"_negz", "_posz"}};
    for (int j = 0; j < 2; ++j)
    {
      hist = dynamic_cast<TH3*>(inputFile->Get((std::string("hIntDistortionP")+extension[j]).c_str()));
      hmod=modifyHistogram(hist, scaleP, offsetP);
      hmod->Write();
      hist = dynamic_cast<TH3*>(inputFile->Get((std::string("hIntDistortionR")+extension[j]).c_str()));
      hmod=modifyHistogram(hist, scaleR, offsetR);
      hmod->Write();
      hist = dynamic_cast<TH3*>(inputFile->Get((std::string("hIntDistortionZ")+extension[j]).c_str()));
      hmod=modifyHistogram(hist, 1, 0);
      hmod->Write();
     }

    // Close the files
    outputFile->Close();
    inputFile->Close();

    // Clean up
    delete outputFile;
    delete inputFile;
    delete hmod;

    printf("Histogram has been modified and saved to %s\n", outputFileName);
}

void adjustStaticDistortion() {
  //source taken from $CALIBRATIONROOT/distortion_maps
  std::string static_filename="/cvmfs/sphenix.sdcc.bnl.gov/gcc-12.1.0/release/release_ana/ana.420/share/calibrations/distortion_maps/static_only_inverted_10-new.root";
  adjustOnce(static_filename.c_str(),"static_correction_0.75.root",0.75,0,0.75,0);
  adjustOnce(static_filename.c_str(),"static_correction_1.00.root",1,0,1,0);
  adjustOnce(static_filename.c_str(),"static_correction_1.25.root",1.25,0,1.25,0);
  adjustOnce(static_filename.c_str(),"static_correction_1.00_phi_p0.1.root",1,0,1,0.1);
  adjustOnce(static_filename.c_str(),"static_correction_1.00_phi_m0.1.root",1,0,1,-0.1);
}
