#ifndef MyUserActionInitialization_hh
#define MyUserActionInitialization_hh

#include "G4VUserActionInitialization.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorConstruction;
class PhysicsList;

class MyUserActionInitialization : public G4VUserActionInitialization{
	public:
	MyUserActionInitialization();
	~MyUserActionInitialization();
	
	void Build() const;
	void BuildForMaster() const;
	
	private:
	
};

#endif
