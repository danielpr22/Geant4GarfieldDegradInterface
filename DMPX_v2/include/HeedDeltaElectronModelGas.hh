/*
 * HeedDeltaElectronModel.h
 *
 *  Created on: May 6, 2025
 *      Author: Daniel Perales Rios
 */

#ifndef HEEDINTERFACEMODEL_H_
#define HEEDINTERFACEMODEL_H_

// Included from the current project
#include "HeedModel.hh"

class G4VPhysicalVolume;
class DetectorConstruction;
class HeedDeltaElectronMessenger;
class GasModelParameters;
class GasBoxSD;
class G4FastStep;
class G4FastTrack;

typedef std::pair<double, double> EnergyRange_keV;
typedef std::multimap<const G4String, EnergyRange_keV> MapParticlesEnergy;

class HeedDeltaElectronModelGas : public HeedModel {
 public:
  //-------------------------
  // Constructor, destructor
  //-------------------------
    HeedDeltaElectronModelGas(GasModelParameters *,G4String, G4Region*,DetectorConstruction*, GasBoxSD*);
  ~HeedDeltaElectronModelGas();
  

  /*The following public methods are user-dependent*/

  //This method is called after each event, to record the relevant data
  virtual void ProcessEvent();
  //This method is called at the beginning of an event to reset some variables of the class
  virtual void Reset();
  
  /*Getters and Setters*/

 private:
  virtual void Run(G4FastStep& fastStep,const G4FastTrack& fastTrack, G4String particleName, double ekin_keV, double t, double x_cm,
            double y_cm, double z_cm, double dx, double dy, double dz);
};

#endif /* HeedDeltaElectronModel_H_ */

