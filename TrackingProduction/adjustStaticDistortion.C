



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
    TH3* hist = (TH3*)inputFile->Get(histName);
    if (!hist) {
        printf("Error: could not find histogram %s in file %s\n", histName, inputFileName);
        inputFile->Close();
        return;
    }

    // Modify the histograms
    TH3* hModified;

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
    delete histoModified;

    printf("Histogram has been modified and saved to %s\n", outputFileName);
}

void adjustStaticDistortion() {
  adjustOnce();
  
}
