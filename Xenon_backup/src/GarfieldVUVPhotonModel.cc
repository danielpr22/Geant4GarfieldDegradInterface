#include "GarfieldVUVPhotonModel.hh"
#include "GarfieldExcitationHit.hh"
#include "GasModelParameters.hh"
#include "DetectorConstruction.hh"
#include "GasBoxSD.hh"

#include <fstream>
#include "G4Electron.hh"
#include "G4SystemOfUnits.hh"
#include "G4Region.hh"
#include "G4ParticleDefinition.hh"
#include "G4UnitsTable.hh"
#include "G4Track.hh"
#include "Randomize.hh"
#include "G4UIcommand.hh"
#include "G4TransportationManager.hh"
#include "G4DynamicParticle.hh"
#include "G4RandomDirection.hh"
#include "globals.hh"
#include "MediumMagboltz.hh"
#include "GeometrySimple.hh"
#include "ComponentConstant.hh"
#include "Sensor.hh"
#include "AvalancheMicroscopic.hh"
#include "Medium.hh"
#include "SolidTube.hh"
#include "G4OpticalPhoton.hh"
#include "G4ProcessManager.hh"


const static G4double torr = 750.062 * bar;

GarfieldVUVPhotonModel::GarfieldVUVPhotonModel(GasModelParameters* gmp, 
	G4String modelName,G4Region* envelope,DetectorConstruction* dc,GasBoxSD* sd) 
	:G4VFastSimulationModel(modelName, envelope),detCon(dc),fGasBoxSD(sd) {
	thermalE=gmp->GetThermalEnergy();
	InitialisePhysics();
}

G4bool GarfieldVUVPhotonModel::IsApplicable(const G4ParticleDefinition& particleType) {	
	if (particleType.GetParticleName()=="e-") {
		G4cout << "(Debug: GarfieldVUVPhotonModel.cc) Electron generated, the model is applicable..." << G4endl;
		return true;
	}
	return false;		
}

G4bool GarfieldVUVPhotonModel::ModelTrigger(const G4FastTrack& fastTrack) {
  G4double ekin = fastTrack.GetPrimaryTrack()->GetKineticEnergy();
  if (ekin<thermalE) {
		G4cout << "(Debug: GarfieldVUVPhotonModel.cc) Triggered! The Garfield model is triggered below energies of " <<  G4BestUnit(thermalE, "Energy") << G4endl;
		G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The kinetic energy of the particle is: " << G4BestUnit(ekin, "Energy") << G4endl;
		return true;
  }
  else {return false;} 
} 
	
void GarfieldVUVPhotonModel::DoIt(const G4FastTrack& fastTrack, G4FastStep& fastStep) 
{
	G4int id = fastTrack.GetPrimaryTrack()->GetTrackID();
    if (id == 1) {
        G4cout << "(Debug: GarfieldVUVPhotonModel.cc) Incoming primary photon detected, proceeding to kill..." << G4endl;
    }
	else {
        G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The current trackID is: " << id << G4endl;
    }

    G4cout<<"(Debug: GarfieldVUVPhotonModel.cc) Garfield++ is here..."<<G4endl;
    // The details of the Garfield model are implemented here
    fastStep.KillPrimaryTrack(); // Kill Degrad tracks
    garfPos =fastTrack.GetPrimaryTrack()->GetVertexPosition();
    garfTime = fastTrack.GetPrimaryTrack()->GetGlobalTime();
    G4cout << "(Debug: GarfieldVUVPhotonModel.cc) Global time: " << G4BestUnit(garfTime,"Time") << ", Position: " << G4BestUnit(garfPos,"Length") << G4endl;
    GenerateVUVPhotons(fastTrack,fastStep,garfPos,garfTime);
	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The VUV photon has been generated..." << G4endl;
}

GarfieldExcitationHitsCollection *garfExcHitsCol;

void GarfieldVUVPhotonModel::GenerateVUVPhotons(const G4FastTrack& fastTrack, G4FastStep& fastStep,
	G4ThreeVector garfPos,G4double garfTime) {
		G4double x0=garfPos.getX()*0.1;//Garfield length units are in cm
		G4double y0=garfPos.getY()*0.1;
		G4double z0=garfPos.getZ()*0.1;
		G4double t0=garfTime;
		G4double e0=thermalE;// starting energy [eV]
		garfExcHitsCol = new GarfieldExcitationHitsCollection();

		G4cout << "(Debug: GarfieldVUVPhotonModel.cc) Avalanche input parameters: "
		<< "x0=" << x0 << ", y0=" << y0 << ", z0=" << z0
		<< ", t0=" << t0 << ", e0=" << e0 << G4endl;

		G4cout << "(Debug: GarfieldVUVPhotonModel.cc) Starting to calculate an avalanche..." << G4endl; 

		fAvalanche->AvalancheElectron(x0, y0, z0, t0, e0, 0., 0., 0.);

		unsigned int nElastic, nIonising, nAttachment, nInelastic, nExcitation, nSuperelastic;
		fMediumMagboltz->GetNumberOfElectronCollisions(nElastic, nIonising, nAttachment, nInelastic, nExcitation, nSuperelastic);
		
		G4cout<<"(Debug: GarfieldVUVPhotonModel.cc) NExcitation: " << nExcitation << G4endl;	

		G4int colHitsEntries=garfExcHitsCol->entries();
		G4cout<<"(Debug: GarfieldVUVPhotonModel.cc) Number of entries...: " << colHitsEntries << G4endl;	

		for (G4int i=0;i<colHitsEntries;i++){
			G4cout << "(Debug: GarfieldVUVPhotonModel.cc) We are here..." << G4endl;
			GarfieldExcitationHit* newExcHit=new GarfieldExcitationHit();
			newExcHit->SetPos((*garfExcHitsCol)[i]->GetPos());
			newExcHit->SetTime((*garfExcHitsCol)[i]->GetTime());
			fGasBoxSD->InsertGarfieldExcitationHit(newExcHit);
			fastStep.SetNumberOfSecondaryTracks(1);	//1 photon per excitation
			if(i % (colHitsEntries/1) == 0){
				G4DynamicParticle VUVphoton(G4OpticalPhoton::OpticalPhotonDefinition(),G4RandomDirection(), 7.2*eV);
				// Create photons track
				G4Track *newTrack=fastStep.CreateSecondaryTrack(VUVphoton, (*garfExcHitsCol)[i]->GetPos(),(*garfExcHitsCol)[i]->GetTime(),false);
			//	G4ProcessManager* pm= newTrack->GetDefinition()->GetProcessManager();
			//	G4ProcessVectorfAtRestDoItVector = pm->GetAtRestProcessVector(typeDoIt);
			}						
		}
		delete garfExcHitsCol;
}
// Selection of Xenon exitations and ionizations

void GarfieldVUVPhotonModel::InitialisePhysics(){
	fMediumMagboltz = new Garfield::MediumMagboltz();
	double pressure = detCon->GetGasPressure();
	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The pressure in Garfield++ is: " << G4BestUnit(pressure, "Pressure") << G4endl;

	double temperature = detCon->GetTemperature();
	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The temperature in Garfield++ is: " << G4BestUnit(temperature, "Temperature") << G4endl;

	fMediumMagboltz->SetTemperature(temperature);
	fMediumMagboltz->SetPressure(pressure);
	fMediumMagboltz->SetComposition("Xe", 100.);

	// Added line
	fMediumMagboltz->LoadGasFile("../../Xenon/ar_93_co2_7.gas");

	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) Loading the gas file..." << G4endl;
	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The composition in Garfield++ has been set..." << G4endl;

	Garfield::GeometrySimple* geo = new Garfield::GeometrySimple();
	// Make a box
	G4double detectorRadius=detCon->GetGasBoxR(); // cm
	G4double detectorHalfZ=detCon->GetGasBoxH()*0.5; // cm

	Garfield::SolidTube* tube = new Garfield::SolidTube(0.0, detectorHalfZ/CLHEP::cm,0.,0.0, detectorRadius/CLHEP::cm,detectorHalfZ/CLHEP::cm,0.,1.,0.); //Tube oriented in Y'axis (0.,1.,0.,)

	// Add the solid to the geometry, together with the medium inside
	geo->AddSolid(tube, fMediumMagboltz);

	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The tube has been added to the geometry..." << G4endl;

	// Make a component with analytic electric field
	Garfield::ComponentConstant* ComponentConstant = new Garfield::ComponentConstant();
	ComponentConstant->SetGeometry(geo);
	//SetElectricField(const double ex, const double ey, const double ez);
	ComponentConstant->SetElectricField(0., -3000.0, 0.);
 
	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The electric field has been added to the geometry..." << G4endl;

	// Make a sensor
	Garfield::Sensor* sensor = new Garfield::Sensor();
	sensor->AddComponent(ComponentConstant);

	// Added the two following lines
	sensor->SetTimeWindow(0,200000,1200000); // Initial and final times in ns, number of steps
	//sensor->AddElectrode(ComponentConstant, "a");


	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The sensor has been added to the geometry..." << G4endl;

	/*
	This is heap allocation: 
	When the object needs to persist beyond the scope in which it is created.
	When the object needs to be shared across different parts of the program.
	*/

	fAvalanche = new Garfield::AvalancheMicroscopic();
	fAvalanche->SetUserHandleInelastic(userHandle);
	fAvalanche->SetSensor(sensor);	
	G4cout << "(Debug: GarfieldVUVPhotonModel.cc) The avalanche has been properly set and configured..." << G4endl;	
}

// Selection of Xenon exitations and ionizations
void userHandle(double x, double y, double z, double t, int type, int level,Garfield::Medium * m) {
	G4ThreeVector Pos;

	if (level > 2 && level < 53) { // Xenon	
		GarfieldExcitationHit* newExcHit=new GarfieldExcitationHit();
		Pos.setX(x*10); // Back to cm to GEANT4
		Pos.setY(y*10); // Back to cm to GEANT4
		Pos.setZ(z*10); // Back to cm to GEANT4
		newExcHit->SetPos(Pos);
		newExcHit->SetTime(t);
		garfExcHitsCol->insert(newExcHit);
		// If choose to draw change the visualizer from OGL to HepRep in vis.mac file
		newExcHit->Draw();	
	}
}	
