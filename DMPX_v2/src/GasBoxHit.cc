//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// $Id: GasBoxHit.cc 69706 2013-05-13 09:12:40Z gcosmo $
//
/// \file GasBoxHit.cc
/// \brief Implementation of the GasBoxHit class

#include "../include/GasBoxHit.hh"

#include "G4UnitsTable.hh"
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

#include <iomanip>

G4ThreadLocal G4Allocator<GasBoxHit>* GasBoxHitAllocator=0;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

GasBoxHit::GasBoxHit()
 : G4VHit(),
   fTrackID(-1),
   fTime(-1),
   fPos(G4ThreeVector()),
   fEnergy(-1)
{}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

GasBoxHit::~GasBoxHit() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

GasBoxHit::GasBoxHit(const GasBoxHit& right)
  : G4VHit()
{
  fTrackID   = right.fTrackID;
  fPos       = right.fPos;
  fTime      =right.fTime;
  fEnergy    =right.fEnergy;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const GasBoxHit& GasBoxHit::operator=(const GasBoxHit& right)
{
  fTrackID   = right.fTrackID;
  fPos       = right.fPos;
  fTime      =right.fTime;
  fEnergy    =right.fEnergy;

  return *this;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4int GasBoxHit::operator==(const GasBoxHit& right) const
{
  return (this == &right) ? 1 : 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void GasBoxHit::Draw()
{
  G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();
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

void GasBoxHit::Print()
{
	G4cout<<"(Debug: GasBoxHit.cc) HIT"<<G4endl;

  G4cout<< "(Debug: GasBoxHit.cc) trackID: " << fTrackID <<  ", Time: "<< std::setw(7) 
  << G4BestUnit(fTime,"Time") << ", Position: "<< std::setw(7) << G4BestUnit( fPos,"Length") << G4endl;
}