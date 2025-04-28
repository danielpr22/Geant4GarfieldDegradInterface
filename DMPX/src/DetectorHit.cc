#include "DetectorHit.hh"
#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

G4ThreadLocal G4Allocator<DetectorHit>* DetectorHitAllocator;

DetectorHit::DetectorHit() : G4VHit(), fTime(-1), fPos(G4ThreeVector()){}

DetectorHit::~DetectorHit(){}

DetectorHit::DetectorHit(const DetectorHit& rhs) : G4VHit() {
    fPos= rhs.fPos;
    fTime=rhs.fTime;
}

const DetectorHit& DetectorHit::operator=(const DetectorHit& rhs){
    fPos= rhs.fPos;
    fTime=rhs.fTime;
    return *this;
}

G4int DetectorHit::operator==(const DetectorHit& rhs) const{
    return (this==&rhs) ? 1 : 0;
}

void DetectorHit::Draw(){

  G4cout << "(Debug: DetectorHit.cc) Drawing detector hit at " << fPos.getY() << G4endl;
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
  if(pVVisManager)
  {
    G4Circle circle(fPos);
    circle.SetScreenSize(1.);
    circle.SetFillStyle(G4Circle::filled);
    G4Colour colour(0.,1.,0.);
    G4VisAttributes attribs(colour);
    circle.SetVisAttributes(attribs);
    pVVisManager->Draw(circle);
  }
}

void DetectorHit::Print(){
    G4cout << "(Debug: DetectorHit.cc) Printing hits..." << G4endl;
}