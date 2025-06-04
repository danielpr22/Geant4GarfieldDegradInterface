#ifndef DetectorHit_HH
#define DetectorHit_HH

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class DetectorHit : public G4VHit {
    
public:
    DetectorHit();
    virtual ~DetectorHit();
    DetectorHit(const DetectorHit &);
    
    const DetectorHit& operator=(const DetectorHit&);
    G4int operator==(const DetectorHit&) const;
    
    inline void* operator new(size_t);
    inline void  operator delete(void*);
	
	virtual void Draw();
    virtual void Print();
    
    G4ThreeVector GetPos(){return fPos;};
    G4double GetTime(){return fTime;};
    
    void SetPos(G4ThreeVector xyz){ fPos = xyz; };
    void SetTime(G4double t){ fTime = t; };
    
    
private:
    G4double      fTime;
    G4ThreeVector fPos;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

using DetectorHitsCollection=G4THitsCollection<DetectorHit>;

extern G4ThreadLocal G4Allocator<DetectorHit>* DetectorHitAllocator;

inline void* DetectorHit::operator new(size_t) {
  if (!DetectorHitAllocator) {
         DetectorHitAllocator = new G4Allocator<DetectorHit>;
  }
  return (void*)DetectorHitAllocator->MallocSingle();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

inline void DetectorHit::operator delete(void *aHit){
    DetectorHitAllocator->FreeSingle((DetectorHit*) aHit);
}

#endif
