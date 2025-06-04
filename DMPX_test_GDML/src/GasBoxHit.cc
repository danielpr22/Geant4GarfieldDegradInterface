#include "../include/GasBoxHit.hh"

#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4ThreadLocal G4Allocator<GasBoxHit>* GasBoxHitAllocator;

GasBoxHit::GasBoxHit(): G4VHit(), fTime(-1), fPos(G4ThreeVector()){}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

GasBoxHit::~GasBoxHit(){}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

GasBoxHit::GasBoxHit(const GasBoxHit& rhs) : G4VHit() {
    fPos= rhs.fPos;
    fTime=rhs.fTime;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const GasBoxHit& GasBoxHit::operator=(const GasBoxHit& rhs) {
    fPos= rhs.fPos;
    fTime=rhs.fTime;
    return *this;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4int GasBoxHit::operator==(const GasBoxHit& rhs) const {
    return (this==&rhs) ? 1 : 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void GasBoxHit::Draw() {
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  G4cout << "(Debug: GasBoxHit.cc) Drawing detector hit at " << fPos.getY() << G4endl;
  if(pVVisManager)
  {
    G4Circle circle(fPos);
    circle.SetScreenSize(1.);
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(1.,0.,0.);
    G4VisAttributes attribs(colour);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void GasBoxHit::Print() {
    G4cout << "(Debug: GasBoxHit.cc) Printing hits..." << G4endl;
}
