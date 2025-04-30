#include "../include/DegradModel.hh"
#include "../include/GasBoxSD.hh"
#include "../include/XenonHit.hh"
#include "../include/GasModelParameters.hh"

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


DegradModel::DegradModel(GasModelParameters* gmp, G4String modelName, G4Region* envelope,DetectorConstruction* dc, GasBoxSD* sd)
    : G4VFastSimulationModel(modelName, envelope),detCon(dc), fGasBoxSD(sd) {
        thermalE=gmp->GetThermalEnergy();
        G4cout << "(Debug: DegradModel.cc) Now setting the thermal energy of the Degrad model..." << G4endl;
        processOccured = false;
    }

DegradModel::~DegradModel() {}

G4bool DegradModel::IsApplicable(const G4ParticleDefinition& particleType) {
  if (particleType.GetParticleName()=="e-")
    G4cout << "(Debug: DegradModel.cc) Electron generated, the model is applicable..." << G4endl; 
    return true;
  return false;
}

G4bool DegradModel::ModelTrigger(const G4FastTrack& fastTrack) {
  G4int id = fastTrack.GetPrimaryTrack()->GetParentID();
    if (id == 1){
        G4cout << "(Debug: DegradModel.cc) The Degrad model is triggered..." << G4endl;
        return true;
    }
  return false;
}

void DegradModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) {

    G4cout << "(Debug: DegradModel.cc) In the DoIt method of the Degrad model..." << G4endl;
    fastStep.KillPrimaryTrack();
    if(!processOccured){
        G4ThreeVector degradPos =fastTrack.GetPrimaryTrack()->GetVertexPosition();
        G4double degradTime = fastTrack.GetPrimaryTrack()->GetGlobalTime();
        
        fastStep.ProposePrimaryTrackPathLength(0.0);
        G4cout<<"(Debug: DegradModel.cc) Global time: "<< G4BestUnit(degradTime,"Time") << ", Position: " << G4BestUnit(degradPos,"Length") << G4endl;

        G4int stdout;
        G4int SEED=54217137*G4UniformRand();
        G4String seed = G4UIcommand::ConvertToString(SEED);
        G4String degradString="printf \"1,1,3,-1,"+seed+",5900.0,7.0,0.0\n7,0,0,0,0,0\n100.0,0.0,0.0,0.0,0.0,0.0,20.0,900.0\n3000.0,0.0,0.0,1,0\n100.0,0.5,1,1,1,1,1,1,1\n0,0,0,0,0,0\" > conditions_Degrad.txt";
        G4cout << "(Debug: DegradModel.cc) String sent to conditions_Degrad.txt: " << degradString << G4endl;
        
        /*
        The command below involves writing data to a file named conditions_Degrad.txt using 
        the printf command. The output of the command is directed to the file, and the system() 
        function returns an integer status code indicating the success or failure of the 
        command execution. This return value is stored in the stdout variable, which can 
        be used later to check for errors.
        */
        
        stdout=system(degradString.data());
        G4cout << "(Debug: DegradModel.cc) Getting the environment variable for Degrad..." << G4endl;
        const char* degradpath = std::getenv("DEGRAD_HOME");

        if (degradpath) {
            G4cout << "(Debug: DegradModel.cc) DEGRAD_HOME is set to: " << degradpath << G4endl;
        } else {
            G4cerr << "(Debug: DegradModel.cc) Error: DEGRAD_HOME is not set!" << G4endl;
        }

        std::string exec = "/degrad.exe < conditions_Degrad.txt";
        std::string full_path = degradpath + exec;
        const char *mychar = full_path.c_str();
        G4cout << "(Debug: DegradModel.cc) The mychar pointer is set to: " << mychar << G4endl;
        stdout=system(mychar); // This command runs the mychar string command in the shell
        stdout=system("./convertDegradFile.py");

        G4cout << "(Debug: DegradModel.cc) The Degrad file was properly converted..." << G4endl;

        GetElectronsFromDegrad(fastStep,degradPos,degradTime);
        processOccured=true;
    }
}

void DegradModel::GetElectronsFromDegrad(G4FastStep& fastStep,G4ThreeVector degradPos,G4double degradTime)
{
    G4cout << "(Debug: DegradModel.cc) Getting the electrons from Degrad..." << G4endl;

    // 'Nep' is the number of primaries that corresponds to what Biagi calls ‘ELECTRON CLUSTER SIZE (NCLUS)'
    G4int eventNumber,Nep, nline, i, electronNumber;
    G4double posX,posY,posZ,time,n;
    G4double  posXDegrad,posYDegrad,posZDegrad,timeDegrad;
    G4double  posXInitial=degradPos.getX();
    G4double  posYInitial=degradPos.getY();
    G4double  posZInitial=degradPos.getZ();
    G4double  timeInitial=degradTime;
    G4String line;
    std::vector<G4double> v;
    
    std::ifstream inFile;
    G4String fname= "DEGRAD.OUT";
    inFile.open(fname,std::ifstream::in);
    
    G4cout<< "(Debug: DegradModel.cc) Working in "<< fname << G4endl;
    
    nline=1;
    electronNumber=0;
    while (getline(inFile, line,'\n'))// '\n'is used to indicate the end of the line
    {
        std::istringstream iss(line);
        if (nline ==1) // Summary
        {
            while (iss >> n) // Each stream will assign a value to n
            {
                v.push_back(n); // n is added to the vector
            }
            
            eventNumber=v[0];
            Nep=v[1];
            // Nexc=v[2];
            v.clear();
        }
        if (nline ==2) // Ionizations
        {
            while (iss >> n) // each stream will assign a value to 'n'
            {
                v.push_back(n); // 'n' is added to the vector
            }
            for (i=0;i<v.size();i=i+7){
                posXDegrad=v[i];
                posYDegrad=v[i+1];
                posZDegrad=v[i+2];
                timeDegrad=v[i+3];
                // Convert from um to mm in GEANT4
                // Also Y and Z axes are swaped in GEANT4 and Garfield++ relatively to Degrad
                posX=posXDegrad*0.001+posXInitial;
                posY=posZDegrad*0.001+posYInitial;
                posZ=posYDegrad*0.001+posZInitial;
                // Cnvert ps to ns
                time=timeDegrad*0.001+timeInitial;
                
                G4ThreeVector myPoint;
                myPoint.setX(posX);
                myPoint.setY(posY);
                myPoint.setZ(posZ);
                
                // Check in which Physical volume the point bellongs
                G4Navigator* theNavigator= G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
                
                G4VPhysicalVolume* myVolume = theNavigator->LocateGlobalPointAndSetup(myPoint);
                
                G4String solidName=myVolume->GetName();
                
                if (G4StrUtil::contains(solidName, "detectorPhysical")){
                    // Just to limit the number of electrons in tests
                    G4cout << "(Debug: DegradModel.cc) Inside the solid..." << G4endl;
                    
                    // Get the Xenon emission spectrum here
                    electronNumber++;
                    XenonHit* xh = new XenonHit();
                    xh->SetPos(myPoint);
                    xh->SetTime(time);
                    fGasBoxSD->InsertXenonHit(xh);
                    
                    // Create secondary electron
                    if(electronNumber % 50 == 0){   
                        G4cout << "(Debug: DegradModel.cc) Creating secondary electron..." << G4endl; 
                        G4DynamicParticle electron(G4Electron::ElectronDefinition(),G4RandomDirection(), 7.0*eV);
                        G4Track *newTrack=fastStep.CreateSecondaryTrack(electron, myPoint, time,false);
                    }
                }
            }
            v.clear(); // Reset the vector otherwise it will continue to add data
            nline=0;
        }
        nline++;
    }
    inFile.close();
    G4cout << "(Debug: DegradModel.cc) Number of initial electrons: " << electronNumber << G4endl;
}


