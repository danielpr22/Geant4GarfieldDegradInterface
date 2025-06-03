#include "../include/DegradModel.hh"
#include "../include/GasModelParameters.hh"
#include "../include/GasBoxSD.hh"
#include "../include/GasBoxHit.hh"
#include "../include/EventAction.hh"
#include "../include/DetectorConstruction.hh"
#include "../include/DetectorMessenger.hh"

#include <fstream>
#include "G4SystemOfUnits.hh"
#include "G4Electron.hh"
#include "G4Region.hh"
#include "G4ParticleDefinition.hh"
#include "G4UnitsTable.hh"
#include "G4Track.hh"
#include "Randomize.hh"
#include "G4UIcommand.hh"
#include "G4TransportationManager.hh"
#include "G4DynamicParticle.hh"
#include "G4RandomDirection.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"

const static G4double torr = 1. / 760. * atmosphere;

DegradModel::DegradModel(GasModelParameters* gmp, G4String modelName, G4Region* envelope,DetectorConstruction* dc, GasBoxSD* sd)
    : G4VFastSimulationModel(modelName, envelope),detCon(dc), fGasBoxSD(sd), fGasModelParameters(gmp){
        thermalE=gmp->GetThermalEnergy();
        voltageCathodePlane=gmp->GetVoltageCathodePlane();
        G4cout << "(Debug: DegradModel.cc) Now setting the thermal energy of the Degrad model: " << thermalE / eV << " eV" << G4endl;
        processOccured = false;
        numberOfGases = gmp->GetNumberOfGases();  
        gasList = gmp->GetGasList();
        gasPercentages = gmp->GetGasPercentages();
        G4cout << "(Debug: DegradModel.cc) The gas percentages are: " << gasPercentages << G4endl;
        temperature = gmp->GetTemperature();
        messenger = detCon->GetDetectorMessenger();
        pressure = messenger->GetPressure(); // Get the pressure from the DetectorMessenger
        secondaryElectronsPerPhoton = gmp->GetSecondaryElectronsPerPhoton();
        nbOfSecondaries = 0;
}

DegradModel::~DegradModel() {}

G4bool DegradModel::IsApplicable(const G4ParticleDefinition& particleType) {
  if (particleType.GetParticleName()=="e-")
    G4cout << "(Debug: DegradModel.cc) Electron detected, the model is applicable..." << G4endl; 
    return true;
  return false;
}

G4bool DegradModel::ModelTrigger(const G4FastTrack& fastTrack) {
  G4int id = fastTrack.GetPrimaryTrack()->GetParentID();
  G4ThreeVector currentPos = fastTrack.GetPrimaryTrack()->GetVertexPosition();
    if (id == 1){ // If it's the first ionization, Degrad is triggered
        G4cout << "(Debug: DegradModel.cc) The Degrad model is triggered for the first ionization..." << G4endl;
        nbOfSecondaries++;
        G4cout << "(Debug: DegradModel.cc) Number of secondaries created: " << nbOfSecondaries << G4endl;
        G4cout << "(Debug: DegradModel.cc) The position of the primary track is: " << G4BestUnit(currentPos,"Length") << G4endl;
        return true;
    }
  return false;
}

void DegradModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {

    G4int id = fastTrack.GetPrimaryTrack()->GetTrackID();
    
    // If the volatage is updated during the run, we get the new value here
    voltageAnodeWires = fGasModelParameters->GetVoltageAnodeWires();
    voltageCathodePlane = fGasModelParameters->GetVoltageCathodePlane();

    G4cout << "(Debug: DegradModel.cc) In the DoIt method of the Degrad model..." << G4endl;
    fastStep.KillPrimaryTrack(); // Kill the Geant4 track for the primary ionization electrons
    G4cout << "(Debug: DegradModel.cc) The primary track has been killed..." << G4endl;

    G4cout << "(Debug: DegradModel.cc) Value of processOccured: " << processOccured << G4endl;
    // This condition avoids that, if Geant4 produces more than one electron from photoionization, 
    // the calculation from Degrad is done multiple times
    if(!processOccured){
        // Retrieving the EventAction instance for the energy of the primary
        auto eventAction = dynamic_cast<EventAction*>(
            const_cast<G4UserEventAction*>(G4RunManager::GetRunManager()->GetUserEventAction()));
        
            G4double energyPrimary;
        if (eventAction) {
            energyPrimary = eventAction->GetEnergyPrimary();
            energyPrimary = energyPrimary / eV; // Convert to eV
            G4cout << "(Debug: DegradModel.cc) Energy of the primary particle: " << energyPrimary << " eV" << G4endl;
        } else {
            energyPrimary = 0.0;
            G4cerr << "(Debug: DegradModel.cc) Error: EventAction is not set or cannot be cast." << G4endl;
        }  

        G4ThreeVector degradPos =fastTrack.GetPrimaryTrack()->GetVertexPosition();
        G4double degradTime = fastTrack.GetPrimaryTrack()->GetGlobalTime();
        
        // Set the true path length of the primary track during the step.
        fastStep.ProposePrimaryTrackPathLength(0.0);

        G4cout<<"(Debug: DegradModel.cc) Global time: "<< G4BestUnit(degradTime,"Time") << ", Position: " << G4BestUnit(degradPos,"Length") << G4endl;

        G4int SEED=53217137*G4UniformRand();
        G4String seed = G4UIcommand::ConvertToString(SEED);

        // Calculation of the electric field for Degrad in V/cm
        distanceAnodeCathodes = 0.8; // cm
        //distanceAnodeCathodes = 0.5 * detCon->GetGasBoxLengthY() / cm; // Distance from the anodes to the source of photons
        G4double voltageDifference = voltageAnodeWires - voltageCathodePlane; // V
        G4cout << "(Debug: DegradModel.cc) Voltage difference: " << voltageDifference << " V" << G4endl;

        G4double electricField = voltageDifference / distanceAnodeCathodes; // V/cm
        G4cout << "(Debug: DegradModel.cc) Electric field: " << electricField << " V/cm" << G4endl;


        // Formatting the input that we are going to feed to Degrad
        // The main issue is that the input double values should have one and only one decimal point
        // Here, we configure the input that will be sent to Degrad for the avalanche calculation.
        std::string numberOfGasesString = std::to_string(numberOfGases); // No formatting needed
        std::ostringstream oss;

        // Format energyString
        oss << std::fixed << std::setprecision(1) << energyPrimary;
        std::string energyString = oss.str();
        oss.str(""); // Clear the stream
        oss.clear(); // Reset the state

        // Format thermalEString
        oss << std::fixed << std::setprecision(1) << (thermalE / eV); // Convert thermal energy to eV
        std::string thermalEString = oss.str();
        oss.str("");
        oss.clear();

        // Format temperatureString
        double temperatureCentigrade = temperature - 273.15; // Convert Kelvin to Celsius
        oss << std::fixed << std::setprecision(1) << temperatureCentigrade;
        std::string temperatureString = oss.str();
        oss.str("");
        oss.clear();

        // Format pressureString
        double pressureTorr = pressure / torr; // Convert pressure to Torr
        oss << std::fixed << std::setprecision(1) << pressureTorr;
        std::string pressureString = oss.str();
        oss.str("");
        oss.clear();

        // Format electricFieldString
        oss << std::fixed << std::setprecision(1) << electricField;
        std::string electricFieldString = oss.str();
        oss.str("");
        oss.clear();

        // Please search for "INPUT CARDS" in the Degrad Fortran source code for more information
        G4String degradString=numberOfGasesString + ",1,3,1," + seed + "," 
        + energyString + "," + thermalEString + ",0.0\n" + gasList + "\n"
        + gasPercentages + "," + temperatureString + "," + pressureString + "\n"
        + electricFieldString + ",0.0,0.0,1,0\n100.0,0.5,1,1,1,1,1,1,1\n0,0,0,0,0,0";

        G4cout << "(Debug: DegradModel.cc) String sent to conditions_Degrad.txt: " 
        << degradString << G4endl;
        
        /*
        The command below involves writing data to a file named conditions_Degrad.txt using 
        the printf command. The output of the command is directed to the file, and the system() 
        function returns an integer status code indicating the success or failure of the 
        command execution. This return value is stored in the stdout variable, which can 
        be used later to check for errors.
        */
        
        std::ofstream outFile("conditions_Degrad.txt");
        if (!outFile.is_open()) {
            G4cerr << "(Error: DegradModel.cc) Failed to open conditions_Degrad.txt for writing." << G4endl;
            return;
        }
        outFile << degradString;
        if (outFile.fail()) {
            G4cerr << "(Error: DegradModel.cc) Failed to write to conditions_Degrad.txt." << G4endl;
            outFile.close();
            return;
        }
        outFile.close();
        G4cout << "(Debug: DegradModel.cc) Successfully wrote to conditions_Degrad.txt." << G4endl;

        G4cout << "(Debug: DegradModel.cc) Getting the environment variable for Degrad..." << G4endl;
        const char* degradpath = std::getenv("DEGRAD_HOME");

        if (!degradpath) {
            G4cerr << "(Error: DegradModel.cc) DEGRAD_HOME is not set!" << G4endl;
            return;
        }

        std::string exec = std::string(degradpath) + "/degrad.exe < conditions_Degrad.txt";
        G4cout << "(Debug: DegradModel.cc) Full command: " << exec << G4endl;
        int execStatus = system(exec.c_str());
        if (execStatus != 0) {
            G4cerr << "(Error: DegradModel.cc) Failed to execute Degrad." << G4endl;
            return;
        }

        execStatus = system(exec.c_str()); // This command runs the mychar string command in the shell
        execStatus = system("./convertDegradFile.py");

        G4cout << "(Debug: DegradModel.cc) The Degrad file was properly converted..." << G4endl;

        GetElectronsFromDegrad(fastStep,degradPos,degradTime);
        processOccured=true; // Once Degrad has finished calculating the positions and times of the generated electrons
    }
    nbOfSecondaries = 0; // The number of secondaries created by the primary photon is reset
}

void DegradModel::GetElectronsFromDegrad(G4FastStep& fastStep, G4ThreeVector degradPos,G4double degradTime)
{
    G4cout << "(Debug: DegradModel.cc) Getting the electrons from Degrad..." << G4endl;
    G4cout << "(Debug: DegradModel.cc) Position from Degrad: " << G4BestUnit(degradPos, "Length") << G4endl;

    // 'Nep' is the number of primaries that corresponds to what Biagi calls ‘ELECTRON CLUSTER SIZE (NCLUS)'
    // 'Nexc' is what Biagi calls EXCITATION CLUSTER SIZE
    G4int eventNumber,Nep, Nexc, nline, i;
    G4double posX,posY,posZ,time,n;
    G4double  posXDegrad,posYDegrad,posZDegrad,timeDegrad;
    G4double  posXInitial=degradPos.getX(); // in mm
    G4double  posYInitial=degradPos.getY(); // in mm
    G4double  posZInitial=degradPos.getZ(); // in mm
    G4double  timeInitial=degradTime; // in ns
    G4String line;
    std::vector<G4double> v;
    
    std::ifstream inFile;
    G4String fname= "DEGRAD.OUT";
    inFile.open(fname,std::ifstream::in);

    if (!inFile.is_open()) {
        G4cerr << "(Error: DegradModel.cc) Failed to open file: " << fname << G4endl;
        return; // Exit the function or handle the error appropriately
    } else {
        G4cout << "(Debug: DegradModel.cc) Successfully opened file: " << fname << G4endl;
    }

    G4cout<< "(Debug: DegradModel.cc) Working in "<< fname << G4endl;
    
    nline=1;
    nbOfElectronsInBox = 0; 

    // While there is still data in the file, we read it
    while (getline(inFile, line,'\n'))// '\n'is used to indicate the end of the line
    {
        std::istringstream iss(line);
        if (nline ==1) // Summary
        {
            while (iss >> n) // Each stream will assign a value to n
            {
                v.push_back(n); // n is added to the vector
            }
            
            eventNumber=v[0]; // (1st column of the first line in DEGRAD.OUT)
            Nep=v[1]; // NCLUS (2nd column of the first line in DEGRAD.OUT)
            Nexc=v[2]; // NSTEXC (3rd column of the first line in DEGRAD.OUT)
            v.clear();
        }
        if (nline == 2) // Ionizations
        {
            G4cout << "(Debug: DegradModel.cc) Now reading the ionizations..." << G4endl; 

            while (iss >> n) // each stream will assign a value to 'n'
            {
                v.push_back(n); // 'n' is added to the vector
            }

            // Since every electron has 7 items
            G4cout << "(Debug: DegradModel.cc) Total number of electrons generated (inside and outside the box): " 
            << v.size()/7 << G4endl; 

            int nbOfElectronsGenerated = 0; // Flag to check the number of electrons generated and stop the generation
            for (i=0;i<v.size();i=i+7){
                posXDegrad=v[i]; // in micrometers
                posYDegrad=v[i+1]; // in micrometers
                posZDegrad=v[i+2]; // in micrometers
                timeDegrad=v[i+3]; // in ps
                // Convert from um to mm in GEANT4
                // CAREFUL: Also Y and Z axes are swaped in GEANT4 and Garfield++ relatively to Degrad
                posX=posXDegrad*0.001 + posXInitial;
                posY=posZDegrad*0.001 + posYInitial; // Careful with the units and the coordinate change!
                posZ=posYDegrad*0.001 + posZInitial;
                // Convert ps to ns
                time=timeDegrad*0.001 + timeInitial;
                
                G4ThreeVector myPoint;
                myPoint.setX(posX);
                myPoint.setY(posY);
                myPoint.setZ(posZ);
                
                // Check in which Physical volume the point bellongs
                G4Navigator* theNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
                G4VPhysicalVolume* myVolume = theNavigator->LocateGlobalPointAndSetup(myPoint);
                G4String solidName = myVolume->GetName();

                G4cout << "(Debug: DegradModel.cc) solidName is: " << solidName << G4endl; 

                // For the secondary electrons to be generated from the primary photon, we need the 
                // generated secondary electrons to be inside the gasBox
                if (G4StrUtil::contains(solidName, "physGasBox")){
                    nbOfElectronsInBox++; // One more electron is inside the box
                    
                    // Get the GasBox emission spectrum here
                    GasBoxHit* gbh = new GasBoxHit();
                    gbh->SetPos(myPoint);
                    gbh->SetTime(time);
                    fGasBoxSD->InsertGasBoxHit(gbh);

                    // The condition is just set to limit the number of electrons in tests
                    G4DynamicParticle electron(G4Electron::ElectronDefinition(),G4RandomDirection(), thermalE); // Here we write the energy cut in Degrad
                    G4Track* newTrack=fastStep.CreateSecondaryTrack(electron, myPoint, time, false);
                    
                    G4cout << "(Debug: DegradModel.cc) Creating secondary electron..." << G4endl; 
                    nbOfElectronsGenerated += 1; 

                    if (nbOfElectronsGenerated == secondaryElectronsPerPhoton) {
                        break; 
                    }
                }
            }
            v.clear(); // Reset the vector otherwise it will store the data for the next electron
            nline=0;
        }
        nline++;
    }
    inFile.close();
    G4cout << "(Debug: DegradModel.cc) Number of secondary electrons inside the gas box: " << nbOfElectronsInBox << G4endl;
}


